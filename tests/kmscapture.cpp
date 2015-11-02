#include <linux/videodev2.h>
#include <cstdio>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/ioctl.h>

#include "kms++.h"
#include "test.h"
#include "cmdoptions.h"

#define CAMERA_BUF_QUEUE_SIZE	3
#define MAX_CAMERA		9

using namespace std;
using namespace kms;

class Camera
{
public:
	Camera(int camera_id, Card& card, Plane* plane, __u32 x, __u32 y,
	       __u32 iw, __u32 ih, PixelFormat pixfmt);
	~Camera();

	Camera(const Camera& other) = delete;
	Camera& operator=(const Camera& other) = delete;

	void show_next_frame(Crtc* crtc);
	int fd() const { return m_fd; }
private:
	bool Open(int camera_id);
	void Close();
	int Init(Card& card, Plane* plane, __u32 x, __u32 y, __u32 iw, __u32 ih,
		 PixelFormat pixfmt);
	void Deinit();

	int m_fd;	/* camera file descriptor */
	Plane* m_plane;
	DumbFramebuffer* m_fb[CAMERA_BUF_QUEUE_SIZE]; /* framebuffers */
	bool m_initialized; /* FBs are allocated or not */
	int m_prev_fb_index;
	__u32 m_in_width, m_in_height; /* camera capture resolution */
	/* image properties for display */
	__u32 m_out_width, m_out_height;
	__u32 m_out_x, m_out_y;
};

Camera::Camera(int camera_id, Card& card, Plane* plane, __u32 x, __u32 y, __u32 iw, __u32 ih, PixelFormat pixfmt)
{
	ASSERT(Open(camera_id));

	Init(card, plane, x, y, iw, ih, pixfmt);
}


Camera::~Camera()
{
	if (m_initialized)
		Deinit();
	Close();
}

bool Camera::Open(int camera_id)
{
	char dev_name[20];

	sprintf(dev_name, "/dev/video%d", camera_id);
	m_fd = ::open(dev_name, O_RDWR | O_NONBLOCK);

	if (m_fd < 0)
		return false;
	return true;
}

void Camera::Close()
{
	if (m_fd >= 0)
		::close(m_fd);

	m_fd = -1;
}

bool better_size(struct v4l2_frmsize_discrete* v4ldisc, __u32 iw, __u32 ih,
		 __u32 best_w, __u32 best_h)
{
	if (v4ldisc->width <= iw && v4ldisc->height <= ih &&
	    (v4ldisc->width >= best_w || v4ldisc->height >= best_h))
		return true;

	return false;
}

int Camera::Init(Card& card, Plane* plane, __u32 x, __u32 y, __u32 iw, __u32 ih,
		 PixelFormat pixfmt)
{
	int r, i;
	__u32 best_w = 320;
	__u32 best_h = 240;

	struct v4l2_frmsizeenum v4lfrms = { 0 };
	v4lfrms.pixel_format = (__u32) pixfmt;
	while (ioctl(m_fd, VIDIOC_ENUM_FRAMESIZES, &v4lfrms) == 0) {
		if (v4lfrms.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
			if (better_size(&v4lfrms.discrete, iw, ih,
					best_w, best_h)) {
				best_w = v4lfrms.discrete.width;
				best_h = v4lfrms.discrete.height;
			}
		} else {
			break;
		}
		v4lfrms.index++;
	};

	m_out_width = m_in_width = best_w;
	m_out_height = m_in_height = best_h;
	/* Move it to the middle of the requested area */
	m_out_x = x + iw / 2 - m_out_width / 2;
	m_out_y = y + ih / 2 - m_out_height / 2;

	struct v4l2_format v4lfmt = { 0 };
	v4lfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	r = ioctl(m_fd, VIDIOC_G_FMT, &v4lfmt);
	ASSERT(r == 0);

	v4lfmt.fmt.pix.pixelformat = (__u32) pixfmt;
	v4lfmt.fmt.pix.width = m_in_width;
	v4lfmt.fmt.pix.height = m_in_height;

	r = ioctl(m_fd, VIDIOC_S_FMT, &v4lfmt);
	ASSERT(r == 0);

	struct v4l2_requestbuffers v4lreqbuf = { 0 };
	v4lreqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4lreqbuf.memory = V4L2_MEMORY_DMABUF;
	v4lreqbuf.count = CAMERA_BUF_QUEUE_SIZE;
	r = ioctl(m_fd, VIDIOC_REQBUFS, &v4lreqbuf);
	ASSERT(r == 0);

	struct v4l2_buffer v4lbuf = { 0 };
	v4lbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4lbuf.memory = V4L2_MEMORY_DMABUF;

	for (i = 0; i < CAMERA_BUF_QUEUE_SIZE; i++) {
		m_fb[i] = new DumbFramebuffer(card, m_in_width, m_in_height,
					      pixfmt);

		v4lbuf.index = i;
		v4lbuf.m.fd = m_fb[i]->prime_fd(0);
// 		printf("queue: %d\n", i);
		r = ioctl(m_fd, VIDIOC_QBUF, &v4lbuf);
		ASSERT(r == 0);
	}
	m_prev_fb_index = -1;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	r = ioctl(m_fd, VIDIOC_STREAMON, &type);
		ASSERT(r == 0);

	m_plane = plane;
	m_initialized = true;

	return 0;
}

void Camera::Deinit()
{
	for (int i = 0; i < CAMERA_BUF_QUEUE_SIZE; i++)
		delete m_fb[i];

	m_initialized = false;
}

void Camera::show_next_frame(Crtc* crtc)
{
	int r;
	int fb_index;

	struct v4l2_buffer v4l2buf = { 0 };
	v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2buf.memory = V4L2_MEMORY_DMABUF;
	r = ioctl(m_fd, VIDIOC_DQBUF, &v4l2buf);
	if (r != 0) {
		printf("VIDIOC_DQBUF ioctl failed with %d\n", errno);
		return;
	}

	fb_index = v4l2buf.index;
// 	printf("Show: %d ", fb_index);
	r = crtc->set_plane(m_plane, *m_fb[fb_index],
		m_out_x, m_out_y, m_out_width, m_out_height,
		0, 0, m_in_width, m_in_height);
	ASSERT(r == 0);

	if (m_prev_fb_index >= 0) {
		memset(&v4l2buf, 0, sizeof(v4l2buf));
		v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2buf.memory = V4L2_MEMORY_DMABUF;
		v4l2buf.index = m_prev_fb_index;
		v4l2buf.m.fd = m_fb[m_prev_fb_index]->prime_fd(0);
// 		printf("queue: %d\n", prev_fb_index);
		r = ioctl(m_fd, VIDIOC_QBUF, &v4l2buf);
		ASSERT(r == 0);

	}
	m_prev_fb_index = fb_index;
}


static bool is_capture_dev(int fd)
{
	struct v4l2_capability cap = { 0 };
	int r;

	r = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	ASSERT(r == 0);
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
		return false;

	return true;
}

static int count_cameras(int camera_idx[])
{
	int i, fd;
	int cameras = 0;
	char dev_name[20];

	for (i = 0; i < MAX_CAMERA; i++) {
		sprintf(dev_name, "/dev/video%d", i);
		fd = ::open(dev_name, O_RDWR | O_NONBLOCK);
		if (fd >= 0) {
			if (is_capture_dev(fd))
				camera_idx[cameras++] = i;
			close(fd);
		}
	}

	return cameras;
}

static map<string, CmdOption> options = {
	{ "s", NO_PARAM("Single camera mode. Open only /dev/video0") },
};

int main(int argc, char** argv)
{
	uint32_t w;
	int i;
	CmdOptions opts(argc, argv, options);
	int camera_idx[MAX_CAMERA];
	int nr_cameras = count_cameras(camera_idx);

	FAIL_IF(!nr_cameras, "Not a single camera has been found.");

	if (opts.is_set("s"))
		nr_cameras = 1;

	auto pixfmt = FourCCToPixelFormat("YUYV");

	Card card;

	auto conn = card.get_first_connected_connector();
	auto crtc = conn->get_current_crtc();
	printf("Display: %dx%d\n", crtc->width(), crtc->height());

	w = crtc->width() / nr_cameras;
	vector<Camera*> cameras;
	i = 0;
	for (Plane* p : crtc->get_possible_planes()) {
		if (p->plane_type() != PlaneType::Overlay)
			continue;

		if (!p->supports_format(pixfmt))
			continue;

		auto cam = new Camera(camera_idx[i], card, p, i * w, 0,
				      w, crtc->height(), pixfmt);
		cameras.push_back(cam);
		if (++i == nr_cameras)
			break;
	}

	FAIL_IF(i < nr_cameras, "available plane not found");

	struct pollfd fds[nr_cameras + 1] = { 0 };
	for (i = 0; i < nr_cameras; i++) {
		fds[i].fd = cameras[i]->fd();
		fds[i].events =  POLLIN;
	}
	fds[nr_cameras].fd = 0;
	fds[nr_cameras].events =  POLLIN;

	while (true) {
		int r = poll(fds, nr_cameras + 1, -1);
		ASSERT(r > 0);

		if (fds[nr_cameras].revents != 0)
			break;

		for (i = 0; i < nr_cameras; i++) {
			if (!fds[i].revents)
				continue;
			cameras[i]->show_next_frame(crtc);
			fds[i].revents = 0;
		}
	}

	for (auto cam : cameras)
		delete cam;
}
