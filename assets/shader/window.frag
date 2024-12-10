#version 450 core
#extension GL_ARB_separate_shader_objects : require

// Frame Output.
layout (location = 0) out vec4 PixelColor;

// Vertex Shader Input.
layout (location = 0) in vec2 TextureCoordinate;
layout (location = 1) in vec4 InterpolatedColor;

// Primary Surface Texture
layout(set = 1, binding = 0) uniform sampler2D Texture;

// Material UBO
layout (set = 0, binding = 2) uniform MaterialUBO {
	float ParallaxScale;
	int ParallaxIterations;
	float VertexColorWeight;
	float RefractionIndex;
} Material;

void main() {    
    PixelColor = mix(texture(Texture, TextureCoordinate), InterpolatedColor, Material.VertexColorWeight);
}