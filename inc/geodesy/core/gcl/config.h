#ifndef GEODESY_CORE_GCL_CONFIG_H
#define GEODESY_CORE_GCL_CONFIG_H

// These values are determined by the Specification minimum.
// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#limits-required
#define GCL_VERTEX_BUFFER_BINDING_COUNT 16
#define GCL_VERTEX_ATTRIBUTE_COUNT 16
#define GCL_COLOR_ATTACHMENT_COUNT 4
#define GCL_DESCRIPTOR_SET_COUNT 4

#define GCL_RASTERIZER_STAGE_COUNT 5
#define GCL_COMPUTE_STAGE_COUNT 1
#define GCL_RAYTRACER_STAGE_COUNT 6

#define GCL_MAX_DESCRIPTOR_SET_SAMPLERS 96
#define GCL_MAX_DESCRIPTOR_SET_UNIFORM_BUFFERS 72
#define GCL_MAX_DESCRIPTOR_SET_STORAGE_BUFFERS 24
//#define GCL_MAX_DESCRIPTOR_SET_SAMPLED_IMAGES 96
#define GCL_MAX_DESCRIPTOR_SET_STORAGE_IMAGES 24
#define GCL_MAX_DESCRIPTOR_SET_INPUT_ATTACHMENTS 4

/* Vulkan API */
#include <vulkan/vulkan.h>

#endif // !GEODESY_CORE_GCL_CONFIG_H