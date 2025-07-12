#version 460
#extension GL_EXT_ray_tracing : require

// This shader simply returns the payload such that it signals
// that the ray leaving the geometry buffer pixel has not hit a
// geometry in the scene, nothing obscures the light source.

struct payload {
    vec3 Origin;
    vec3 Direction;
    int Hit; // 1 if hit, 0 if not
    int HitInstanceID; // Instance ID of the geometry hit
    vec3 HitLocation; // Where the ray hit in world space
};

layout(location = 0) rayPayloadEXT payload Ray;

void main() {
    // Ray missed and hit no geometry.
    // Set destination position to largest float.
    Ray.Hit = 0;
    Ray.HitInstanceID = -1; // No instance hit
    Ray.HitLocation = vec3(1e30);
}