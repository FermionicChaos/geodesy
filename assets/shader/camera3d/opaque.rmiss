#version 460
#extension GL_EXT_ray_tracing : require

// This shader simply returns the payload such that it signals
// that the ray leaving the geometry buffer pixel has not hit a
// geometry in the scene, nothing obscures the light source.

struct lighting_and_shadow_payload {
    vec3 Origin;
    vec3 Direction;
    vec3 Destination;
    int Hit;
};

layout(location = 0) rayPayloadEXT lighting_and_shadow_payload Ray;

void main() {
    // Ray missed and hit no geometry.
    // Set destination position to largest float.
    Ray.Destination = vec3(gl_MaxFloat);
    Ray.Hit = 0;
    terminateRayEXT();
}