#version 460
#extension GL_EXT_ray_tracing : require

// This shader is used to check if the closest hit of the ray has hit a geometry that
// is closer to the pixel origin than the light source. If so, the pixel is in shadow.
// If not, the pixel is illuminated by the light source.

struct lighting_and_shadow_payload {
    vec3 Origin;
    vec3 Direction;
    vec3 Destination;
    int Hit;
};

layout(location = 0) rayPayloadEXT lighting_and_shadow_payload Ray;

void main() {
    // Fill out info on where ray landed in world space.
    Ray.Destination = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT*gl_HitTEXT;
    Ray.Hit = 1;
    terminateRayEXT();
}