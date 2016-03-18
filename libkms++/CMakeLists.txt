include_directories(${LIBDRM_INCLUDE_DIRS})
link_directories(${LIBDRM_LIBRARY_DIRS})

include_directories(${LIBDRM_OMAP_INCLUDE_DIRS})
link_directories(${LIBDRM_OMAP_LIBRARY_DIRS})

file(GLOB SRCS "*.cpp" "*.h")

if(LIBDRM_OMAP_FOUND)
    file(GLOB OMAP_SRCS "omap/*.cpp" "omap/*.h")
endif()

add_library(kms++ ${SRCS} ${OMAP_SRCS})

target_link_libraries(kms++ ${LIBDRM_LIBRARIES} ${LIBDRM_OMAP_LIBRARIES})
target_include_directories(kms++ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
