#version 450 core
#extension GL_ARB_separate_shader_objects : require

// These are unpacked material properties used in rendering.
struct material_property {
	vec3 Albedo;
	float Opacity;
	// vec3 Normal;
	// float Height;
	vec3 Emissive;
	vec3 Specular;
	float Shininess;
	float AmbientOcclusion;
	float Metallic;
	float Roughness;
	float IOR;
	vec3 SheenColor;
	float SheenRoughness;
	float SheenIntensity;
	vec3 SheenNormal;
	float ClearCoatFactor;
	float ClearCoatRoughness;
	vec3 ClearCoatNormal;
	vec3 ClearCoatTint;
	float ClearCoatThickness;
	float ClearCoatIOR;
};

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

// Texture Data
layout (set = 1, binding = 0) uniform sampler2D MaterialColor; 				// float3 Color of the Surface
layout (set = 1, binding = 1) uniform sampler2D MaterialOpacity; 			// float Opacity Map
layout (set = 1, binding = 2) uniform sampler2D MaterialNormalMap; 			// float3 Normal Map
layout (set = 1, binding = 3) uniform sampler2D MaterialHeightMap; 			// float Height Map
layout (set = 1, binding = 4) uniform sampler2D MaterialEmissive; 			// float3 Emissive Lighting
// Blinn-Phong Specular Highlights
layout (set = 1, binding = 5) uniform sampler2D MaterialSpecular; 			// float3 Specular
layout (set = 1, binding = 6) uniform sampler2D MaterialShininess; 			// float Shininess
// PBR Specific Textures
layout (set = 1, binding = 7) uniform sampler2D MaterialAmbientOcclusion; 	// float AOC
layout (set = 1, binding = 8) uniform sampler2D MaterialRoughness; 			// float Roughness
layout (set = 1, binding = 9) uniform sampler2D MaterialMetallic; 			// float Metallic	
// layout (set = 1, binding = 10) uniform sampler2D MaterialSheen; 			// Sheen Map
// layout (set = 1, binding = 11) uniform sampler2D MaterialClearCoat; 		// Clear Coat Map

// -------------------- OUTPUT DATA -------------------- //

// Geometry Buffer Ouput. Pixel Position is the world space
// position of the pixel, PixelNormal is the world space normal
// for that pixel. These world space positions and normals will
// be used for deferred lighting calculations.
layout (location = 0) out vec4 PixelColor;
layout (location = 1) out vec4 PixelPosition;
layout (location = 2) out vec4 PixelNormal;
layout (location = 3) out vec4 PixelSS;
layout (location = 4) out vec4 PixelARM;
layout (location = 5) out vec4 PixelEmissive;

vec2 bisection_parallax(vec2 aUV, mat3 aTBN) {

	// ITBN takes any vector from world space and converts it to tangent space.
	mat3 ITBN = transpose(aTBN);

	// Converts the view direction from world space to tangent space.
	vec3 InvertedViewDir = ITBN * normalize(Subject.Position - WorldPosition);

	// This determines the maximum aUV offset that can be applied. while still possibly
	// containing the correct aUV coordinate that can possibly intersect the view dir
	// with the height map.
	float UVMaxMag = Material.HeightScale * length(InvertedViewDir.xy) / InvertedViewDir.z;

	// Calculate the view direction along the surface in tangent space, then
	// transforms back to world space.
	vec3 UVMaxDir = aTBN * vec3(InvertedViewDir.xy, 0.0);

	// Initialize the start and end points of the search
	vec2 UVStart = aUV;
	vec2 UVEnd = UVMaxDir.xy * UVMaxMag + aUV;

	// Sample the height at the midpoint
	float hStart = texture(MaterialHeightMap, UVStart).r;
	float hEnd = texture(MaterialHeightMap, UVEnd).r;

	// The y values are going to be the values to determine where an inversion 
	// has occurred. It is the height map height minus expected interception point
	// along view dir.
	float yStart = hStart - (length(UVStart - aUV) / UVMaxMag) * Material.HeightScale;
	float yEnd = hEnd - (length(UVEnd - aUV) / UVMaxMag) * Material.HeightScale;

	// Perform the bisection search here. We can modify later
	// to start biased towards the end and move inwards. That
	// way it finds the first intersection farthest to UVMax.
	vec2 UVMid;
	for (int i = 0; i < Material.HeightStepCount; i++) {

		// Compute the midpoint between UVStart and UVEnd
		UVMid = (UVStart + UVEnd) * 0.5;
		float hMid = texture(MaterialHeightMap, UVMid).r;
		float yMid = hMid - (length(UVMid - aUV) / UVMaxMag) * Material.HeightScale;

		// Early exit condition if height threshold is met.
		if (abs(hMid - (length(UVMid - aUV) / UVMaxMag) * Material.HeightScale) < 0.01) {
			break;
		}

		// Check for sign inversion.
		if (yMid * yEnd < 0.0) {
			UVStart = UVMid;
			hStart = hMid;
			yStart = yMid;
		} else {
			UVEnd = UVMid;
			hEnd = hMid;
			yEnd = yMid;
		}

	}
	
	return UVMid;
}

material_property unpack(vec2 aUV) {
	material_property MaterialProperty;

	// Check if material has a Albedo Texture.
	if (Material.AlbedoTextureIndex >= 0) {
		// Vertex Color + Texture Color + Material Color
		MaterialProperty.Albedo = InterpolatedVertexColor.rgb*Material.AlbedoVertexWeight + texture(MaterialColor, aUV).rgb*Material.AlbedoTextureWeight + Material.Albedo*Material.AlbedoWeight;
	}
	else {
		// Vertex Color + Material Color
		float RenormalizationFactor = Material.AlbedoVertexWeight + Material.AlbedoWeight;
		MaterialProperty.Albedo = InterpolatedVertexColor.rgb*Material.AlbedoVertexWeight + Material.Albedo*Material.AlbedoWeight;
		MaterialProperty.Albedo /= RenormalizationFactor > 0.0 ? RenormalizationFactor : 1.0;
	}

	// Get Opacity
	if (Material.OpacityTextureIndex >= 0) {
		// Texture Opacity + Material Opacity
		MaterialProperty.Opacity = texture(MaterialOpacity, aUV).r*Material.OpacityTextureWeight + Material.Opacity*Material.OpacityWeight;
	}
	else {
		// Material Opacity
		MaterialProperty.Opacity = Material.Opacity;
	}

	// Ignore Normal, and Height Map.

	// Load Emissive Color
	if (Material.EmissiveTextureIndex >= 0) {
		// Texture Emissive + Material Emissive
		MaterialProperty.Emissive = texture(MaterialEmissive, aUV).rgb*Material.EmissiveTextureWeight + Material.Emissive*Material.EmissiveWeight;
	}
	else {
		// Material Emissive
		MaterialProperty.Emissive = Material.Emissive;
	}

	// Load Specular Values
	if (Material.SpecularTextureIndex >= 0) {
		// Texture Specular + Material Specular
		MaterialProperty.Specular = texture(MaterialSpecular, aUV).rgb*Material.SpecularTextureWeight + Material.Specular*Material.SpecularWeight;
	}
	else {
		// Material Specular
		MaterialProperty.Specular = Material.Specular;
	}

	// Load Shininess Value
	if (Material.ShininessTextureIndex >= 0) {
		// Texture Shininess + Material Shininess
		MaterialProperty.Shininess = texture(MaterialShininess, aUV).r*Material.ShininessTextureWeight + Material.Shininess*Material.ShininessWeight;
	}
	else {
		// Material Shininess
		MaterialProperty.Shininess = Material.Shininess;
	}

	// Load Ambient Occlusion
	if (Material.AmbientOcclusionTextureIndex >= 0) {
		// Texture Ambient Occlusion + Material Ambient Occlusion
		MaterialProperty.AmbientOcclusion = texture(MaterialAmbientOcclusion, aUV).r*Material.AmbientOcclusionTextureWeight + Material.AmbientOcclusion*Material.AmbientOcclusionWeight;
	}
	else {
		// Material Ambient Occlusion
		MaterialProperty.AmbientOcclusion = Material.AmbientOcclusion;
	}

	// Load Metallic Value
	if (Material.MetallicTextureIndex >= 0) {
		// Texture Metallic + Material Metallic
		MaterialProperty.Metallic = texture(MaterialMetallic, aUV).r*Material.MetallicTextureWeight + Material.Metallic*Material.MetallicWeight;
	}
	else {
		// Material Metallic
		MaterialProperty.Metallic = Material.Metallic;
	}

	// Load Roughness Value
	if (Material.RoughnessTextureIndex >= 0) {
		// Texture Roughness + Material Roughness
		MaterialProperty.Roughness = texture(MaterialRoughness, aUV).r*Material.RoughnessTextureWeight + Material.Roughness*Material.RoughnessWeight;
	}
	else {
		// Material Roughness
		MaterialProperty.Roughness = Material.Roughness;
	}

	// Load IOR Value
	// if (Material.IORTextureIndex >= 0) {
	// 	// Texture IOR + Material IOR
	// 	MaterialProperty.IOR = texture(MaterialIOR, aUV).r*Material.IORTextureWeight + Material.IOR*Material.IORWeight;
	// }
	// else {
	// 	// Material IOR
	// 	MaterialProperty.IOR = Material.IOR;
	// }
	MaterialProperty.IOR = Material.IOR;

	// TODO: 

	return MaterialProperty;
}

void main() {

	// After rasterization interpolation of the vertex data, some values need to be normalized.
	vec3 n = normalize(WorldNormal);
	vec3 t = normalize(WorldTangent);
	vec3 b = normalize(WorldBitangent);

	// Apply correction to tangent vector after interpolation by rasterizer.
	t = normalize(t - n * dot(n, t));
	// Generate new bitangent vector based on the new tangent and normal vectors.
	b = cross(n, t);

	// Construct the TBN matrix to transform from world space to surface tangent space.
	mat3 TBN = mat3(t, b, n);

	// Calculate aUV coordinates after applying the height map.
	vec2 aUV = TextureCoordinate.xy; //bisection_parallax(TextureCoordinate.xy, TBN);

	// Load Material Propertie for fragment.
	material_property MP = unpack(aUV);

	// Merge Ambient Occlusion, Metallic, and Roughness into a single vec4.
	PixelARM = vec4(MP.AmbientOcclusion, MP.Roughness, MP.Metallic, 1.0);

	// Merge Specular and Shininess into a single vec4.
	PixelSS = vec4(MP.Specular.r, MP.Specular.g, MP.Specular.b, MP.Shininess);

	// How much the pixel glows.
	PixelEmissive = vec4(MP.Emissive, 1.0);

	// Get Texture Normal, z should be 1.0 if directly normal to surface.
	if (Material.NormalTextureIndex >= 0) {
		vec3 TextureNormal = normalize(2.0*texture(MaterialNormalMap, aUV).rgb - 1.0);
		vec3 CombinedNormal = n*Material.NormalVertexWeight + TBN*TextureNormal*Material.NormalTextureWeight;
		PixelNormal = vec4(normalize(CombinedNormal), 1.0);
	}
	else {
		PixelNormal = vec4(n, 1.0);
	}

	// Determine World Space Position of the pixel. Maybe modify later to do based on interpolated surface normal?
	PixelPosition = vec4(WorldPosition, 1.0) + PixelNormal*texture(MaterialHeightMap, aUV).r;

	// Move over material color.
	if (Material.Transparency == 0) {
		// Color Pass through for opaque objects.
		PixelColor = vec4(MP.Albedo, texture(MaterialColor, aUV).a * MP.Opacity);//*0.01 + PixelARM*0.9;
	}
	else {
		// Fill Color with white for render doc inspection.
		PixelColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}