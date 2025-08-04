#version 450 core
#extension GL_ARB_separate_shader_objects : require
#extension GL_KHR_vulkan_glsl : enable

// glsl is garbage, hlsl types make more logical sense.
#define uint2 uvec2
#define uint3 uvec3
#define uint4 uvec4
#define uint2x2 umat2
#define uint2x3 umat2x3
#define uint2x4 umat2x4
#define uint3x2 umat3x2
#define uint3x3 umat3
#define uint3x4 umat3x4
#define uint4x2 umat4x2
#define uint4x3 umat4x3
#define uint4x4 umat4

#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define int2x2 imat2
#define int2x3 imat2x3
#define int2x4 imat2x4
#define int3x2 imat3x2
#define int3x3 imat3
#define int3x4 imat3x4
#define int4x2 imat4x2
#define int4x3 imat4x3
#define int4x4 imat4

#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float2x2 mat2
#define float2x3 mat2x3
#define float2x4 mat2x4
#define float3x2 mat3x2
#define float3x3 mat3
#define float3x4 mat3x4
#define float4x2 mat4x2
#define float4x3 mat4x3
#define float4x4 mat4

#define double2 dvec2
#define double3 dvec3
#define double4 dvec4
#define double2x2 dmat2
#define double2x3 dmat2x3
#define double2x4 dmat2x4
#define double3x2 dmat3x2
#define double3x3 dmat3
#define double3x4 dmat3x4
#define double4x2 dmat4x2
#define double4x3 dmat4x3
#define double4x4 dmat4
