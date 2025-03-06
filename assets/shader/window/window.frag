#version 450 core
#extension GL_ARB_separate_shader_objects : require

// -------------------- INPUT DATA -------------------- //
layout (location = 0) in vec2 TextureCoordinate;
layout (location = 1) in vec4 InterpolatedColor;

// -------------------- UNIFORM DATA -------------------- //
// Material UBO
layout (set = 0, binding = 2) uniform MaterialUBO {
	float ParallaxScale;
	int ParallaxIterations;
	float VertexColorWeight;
	float RefractionIndex;
} Material;

// Textures
layout(set = 1, binding = 0) uniform sampler2D Texture;

// -------------------- OUTPUT DATA -------------------- //
layout (location = 0) out vec4 PixelColor;

void main() {    
    PixelColor = mix(texture(Texture, TextureCoordinate), InterpolatedColor, Material.VertexColorWeight);
}