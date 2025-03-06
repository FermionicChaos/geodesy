#version 460
#extension GL_EXT_ray_tracing : require

// This shader is used to check if the closest hit of the ray has hit a geometry that
// is closer to the pixel origin than the light source. If so, the pixel is in shadow.
// If not, the pixel is illuminated by the light source.
