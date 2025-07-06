#version 450 core
#extension GL_ARB_separate_shader_objects : require

// This shader is the final compositor for any remaining post processing after lighting and shadowing
// has been applied to both opaque and translucent geometry buffers.

// -------------------- INPUT DATA -------------------- //
layout (location = 0) in vec3 WorldPosition;
layout (location = 1) in vec3 WorldNormal;
layout (location = 2) in vec3 WorldTangent;
layout (location = 3) in vec3 WorldBitangent;
layout (location = 4) in vec3 TextureCoordinate;
layout (location = 5) in vec4 InterpolatedVertexColor;

layout (set = 0, binding = 0) uniform sampler2D OpaqueColorTexture; // Color buffer for opaque geometry
layout (set = 0, binding = 1) uniform sampler2D TranslucentColorTexture; // Color buffer for translucent geometry
layout (set = 0, binding = 2) uniform sampler2D EmissiveTexture; // Emissive texture for lighting

// -------------------- OUTPUT DATA ------------------- //
layout (location = 0) out vec4 FinalColor;

void main() {

    // We will use Gaussian blur to generate a bloom effect
}