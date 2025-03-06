#version 460
#extension GL_EXT_ray_tracing : require

// This shader simply returns the payload such that it signals
// that the ray leaving the geometry buffer pixel has not hit a
// geometry in the scene, nothing obscures the light source.

void main() {
    Payload.Color = vec4(1.0);
}