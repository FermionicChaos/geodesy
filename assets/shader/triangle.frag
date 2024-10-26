#version 450

layout(set = 1, binding = 0) uniform sampler2D TextureSampler; // Texture for coloring the triangle

layout(location = 0) in vec2 TextureCoordinate;    // UV coordinates from vertex shader
layout(location = 1) in vec4 InterpolatedColor;    // Interpolated color from vertex shader

layout(location = 0) out vec4 PixelColor; // Output color of the fragment

void main() {
    // Sample color from the texture using UV coordinates
    PixelColor = texture(TextureSampler, TextureCoordinate) + InterpolatedColor;
}
