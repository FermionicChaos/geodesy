#version 450

layout (location = 0) in vec3   VertexPosition;
layout (location = 1) in vec3   VertexNormal;
layout (location = 2) in vec3   VertexTangent;
layout (location = 3) in vec3   VertexBitangent;
layout (location = 4) in vec3   VertexTextureCoordinate;
layout (location = 5) in vec4   VertexColor;
// layout (location = 6) in uvec4  VertexBoneID;
// layout (location = 7) in vec4   VertexBoneWeight;

layout(set = 0, binding = 0) uniform ScaleUBO {
    float Scale;  // Scaling factor
} UniformBuffer;

layout(location = 0) out vec2 TextureCoordinate;    // Pass UV coordinates to fragment shader
layout(location = 1) out vec4 InterpolatedColor;    // Pass color to fragment shader

void main() {
    // Apply scaling to the triangle's vertices
    // vec2 ScaledPosition = VertexPosition.xy * UniformBuffer.Scale;

    // Set the position in clip space
    gl_Position = vec4(UniformBuffer.Scale * VertexPosition.xy, 0.0, 1.0);

    // Pass UV coordinates to fragment shader
    TextureCoordinate = VertexTextureCoordinate.xy; // Simple UV mapping based on position
    InterpolatedColor = VertexColor;      // Pass color to fragment shader
}
