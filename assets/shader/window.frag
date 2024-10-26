#version 450 core

layout(set = 0, binding = 1) uniform sampler2D Texture;

layout (location = 0) in vec2 InterpolatedTextureCoordinate;
layout (location = 1) in vec4 InterpolatedColor;

// Frame Output
layout (location = 0) out vec4 PixelColor;

void main() {    
    PixelColor = texture(Texture, InterpolatedTextureCoordinate);
}