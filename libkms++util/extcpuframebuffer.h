#pragma once

#include "kms++.h"

namespace kms
{

class ExtCPUFramebuffer : public IMappedFramebuffer
{
public:
	ExtCPUFramebuffer(uint32_t width, uint32_t height, PixelFormat format,
			  uint8_t* buffer, uint32_t pitch);
	ExtCPUFramebuffer(uint32_t width, uint32_t height, PixelFormat format,
			  uint8_t* buffers[4], uint32_t pitches[4]);
	virtual ~ExtCPUFramebuffer();

	uint32_t width() const { return m_width; }
	uint32_t height() const { return m_height; }

	PixelFormat format() const { return m_format; }
	unsigned num_planes() const { return m_num_planes; }

	uint32_t stride(unsigned plane) const { return m_planes[plane].stride; }
	uint32_t size(unsigned plane) const { return m_planes[plane].size; }
	uint32_t offset(unsigned plane) const { return 0; }
	uint8_t* map(unsigned plane) { return m_planes[plane].map; }

private:
	struct FramebufferPlane {
		uint32_t size;
		uint32_t stride;
		uint8_t *map;
	};

	uint32_t m_width;
	uint32_t m_height;
	PixelFormat m_format;

	unsigned m_num_planes;
	struct FramebufferPlane m_planes[4];
};
}
