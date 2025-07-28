#version 460
#extension GL_EXT_ray_tracing : require

// This shader is used to check if the closest hit of the ray has hit a geometry that
// is closer to the pixel origin than the light source. If so, the pixel is in shadow.
// If not, the pixel is illuminated by the light source.

struct payload {
    vec3 Origin;
    vec3 Direction;
    int Hit;
    int HitInstanceID;
    vec3 HitLocation;
    vec2 UV;
    int MaterialIndex;
};

layout(location = 0) rayPayloadEXT payload Ray;

void main() {
	// Fill out info on where ray landed in world space.
	Ray.Hit = 1;
	Ray.HitInstanceID = gl_InstanceCustomIndexEXT; // Instance ID of the geometry hit
	Ray.HitLocation = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT*gl_HitTEXT;
}