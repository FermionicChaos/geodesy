#version 450 core
#extension GL_ARB_separate_shader_objects : require

// -------------------- INPUT DATA -------------------- //
layout (location = 0) in vec3 WorldPosition;
layout (location = 1) in vec3 WorldNormal;
layout (location = 2) in vec3 WorldTangent;
layout (location = 3) in vec3 WorldBitangent;
layout (location = 4) in vec3 TextureCoordinate;
layout (location = 5) in vec4 InterpolatedVertexColor;

// -------------------- UNIFORM DATA -------------------- //

layout (set = 0, binding = 0) uniform SubjectUBO {
	vec3 Position;
	mat4 Rotation;
	mat4 Projection;
	mat4 PRT;
} Subject;

layout (set = 0, binding = 2) uniform MaterialUBO {
	int 	Transparency;
	float 	AlbedoVertexWeight;
	float 	AlbedoTextureWeight;
	float 	AlbedoWeight;
	int 	AlbedoTextureIndex;
	vec3 	Albedo;
	float 	OpacityTextureWeight;
	float 	OpacityWeight;
	int 	OpacityTextureIndex;
	float 	Opacity;
	float 	NormalVertexWeight;
	float 	NormalTextureWeight;
	int 	NormalTextureIndex;
	int 	HeightTextureIndex;
	float 	HeightScale;
	int 	HeightStepCount;
	float 	AmbientLightingTextureWeight;
	float 	AmbientLightingWeight;
	int 	AmbientLightingTextureIndex;
	vec3 	AmbientLighting;
	float 	EmissiveTextureWeight;
	float 	EmissiveWeight;
	int 	EmissiveTextureIndex;
	vec3 	Emissive;
	float 	SpecularTextureWeight;
	float 	SpecularWeight;
	int 	SpecularTextureIndex;
	vec3 	Specular;
	float 	ShininessTextureWeight;
	float 	ShininessWeight;
	int 	ShininessTextureIndex;
	float 	Shininess;
	float 	AmbientOcclusionTextureWeight;
	float 	AmbientOcclusionWeight;
	int 	AmbientOcclusionTextureIndex;
	float 	AmbientOcclusion;
	float 	RoughnessTextureWeight;
	float 	RoughnessWeight;
	int 	RoughnessTextureIndex;
	float 	Roughness;
	float 	MetallicTextureWeight;
	float 	MetallicWeight;
	int 	MetallicTextureIndex;
	float 	Metallic;
	float 	IORVertexWeight;
	float 	IORTextureWeight;
	float 	IORWeight;
	int 	IORTextureIndex;
	float 	IOR;
	float 	SheenColorTextureWeight;
	float 	SheenColorWeight;
	int 	SheenColorTextureIndex;
	vec3 	SheenColor;
	float 	SheenIntensityTextureWeight;
	float 	SheenIntensityWeight;
	float 	SheenRoughnessTextureWeight;
	float 	SheenRoughnessWeight;
	int 	SheenMaskTextureIndex;
	float 	SheenRoughness;
	float 	SheenIntensity;
	float 	ClearCoatStrengthTextureWeight;
	float 	ClearCoatStrengthWeight;
	float 	ClearCoatRoughnessTextureWeight;
	float 	ClearCoatRoughnessWeight;
	int 	ClearCoatStrengthRoughnessTextureIndex;
	float 	ClearCoatStrength;
	float 	ClearCoatRoughness;
	float 	ClearCoatNormalTextureWeight;
	float 	ClearCoatNormalWeight;
	int 	ClearCoatNormalTextureIndex;
	vec3 	ClearCoatNormal;
	float 	ClearCoatTintTextureWeight;
	float 	ClearCoatTintWeight;
	int 	ClearCoatTintTextureIndex;
	vec3 	ClearCoatTint;
	float 	ClearCoatIOR;
	float 	ClearCoatThickness;
} Material;

// Textures
layout (set = 1, binding = 0) uniform sampler2D MaterialColor; 				// float3 Color of the Surface
layout (set = 1, binding = 1) uniform sampler2D MaterialOpacity; 			// float Opacity Map

// -------------------- OUTPUT DATA -------------------- //
layout (location = 0) out vec4 PixelColor;

void main() {

	vec2 aUV = TextureCoordinate.xy;
	vec3 Color = texture(MaterialColor, aUV).rgb;
	float Opacity = 1.0;

	// // Check if material has a Albedo Texture.
	// if (Material.AlbedoTextureIndex >= 0) {
	// 	// Vertex Color + Texture Color + Material Color
	// 	Color = InterpolatedVertexColor.rgb*Material.AlbedoVertexWeight + texture(MaterialColor, aUV).rgb*Material.AlbedoTextureWeight + Material.Albedo*Material.AlbedoWeight;
	// }
	// else {
	// 	// Vertex Color + Material Color
	// 	float RenormalizationFactor = Material.AlbedoVertexWeight + Material.AlbedoWeight;
	// 	Color = InterpolatedVertexColor.rgb*Material.AlbedoVertexWeight + Material.Albedo*Material.AlbedoWeight;
	// 	Color /= RenormalizationFactor > 0.0 ? RenormalizationFactor : 1.0;
	// }

		// Get Opacity
	if (Material.OpacityTextureIndex >= 0) {
		// Texture Opacity + Material Opacity
		Opacity = texture(MaterialOpacity, aUV).r*Material.OpacityTextureWeight + Material.Opacity*Material.OpacityWeight;
	}
	else {
		// Material Opacity
		Opacity = Material.Opacity;
	}

    PixelColor = vec4(Color, Opacity);
}