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
layout (location = 6) out vec4 PixelTranslucencyMask;

// -----------------------------------------------------------------------------
//  Parallax mapping by binary search (bisection)
//
//  • Works with any height map where 0 = surface, 1 = highest point.
//  • Uses Material.HeightScale as maximum depth into the surface.
//  • Iteration count is Material.HeightStepCount (6-10 is plenty).
// -----------------------------------------------------------------------------
vec2 bisection_parallax(vec2 aUV, mat3 aTBN) {
	/* 1) View vector in tangent space -------------------------------------- */
	vec3 viewWS = normalize(Subject.Position - WorldPosition);
	vec3 viewTS = transpose(aTBN) * viewWS;

	/* Guard against grazing angles: no parallax if surface is edge-on */
	float vz = max(viewTS.z, 0.05);        // avoid division blow-ups

	/* 2) Maximum UV displacement along view ray --------------------------- */
	vec2 parallaxDir  = viewTS.xy / vz;               // direction in TS
	vec2 uvOffsetMax  = parallaxDir * Material.HeightScale;

	/* 3) Initialise search interval --------------------------------------- */
	vec2  uvNear  = aUV;                 // depth 0   (surface)
	vec2  uvFar   = aUV + uvOffsetMax;   // depth 1   (max depth)
	float dNear   = 0.0;
	float dFar    = 1.0;

	/* 4) Binary search for first depth crossing --------------------------- */
	for (int i = 0; i < Material.HeightStepCount; ++i) {
		vec2  uvMid = mix(uvNear, uvFar, 0.5);
		float dMid  = 0.5 * (dNear + dFar);
		float hMid  = texture(MaterialHeightMap, uvMid).r;

		if (hMid > dMid) {
			uvNear = uvMid;
			dNear  = dMid;
		}
		else {
			uvFar = uvMid;
			dFar  = dMid;
		}
	}

	/* 5) Final UV: midpoint of last interval ------------------------------ */
	return mix(uvNear, uvFar, 0.5);
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

	MaterialProperty.Opacity *= texture(MaterialColor, aUV).a; // Apply alpha from color texture.

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

	// Construct the TBN matrix to transform FROM tangent space TO world space
	// Columns are: [Tangent, Bitangent, Normal]
	mat3 TBN = mat3(t, b, n);

	// Calculate aUV coordinates after applying parallax mapping with height map.
	vec2 aUV;
	if (Material.HeightTextureIndex > 0) {
		// If height map is present, use parallax mapping.
		aUV = bisection_parallax(TextureCoordinate.xy, TBN);
	}
	else {
		aUV = TextureCoordinate.xy;
	}

	// Load Material Propertie for fragment.
	material_property MP = unpack(aUV);

	// Move over material color.
	if (Material.Transparency == 0) {
		// Pixel is to use standard lighting and shadowing.
		PixelTranslucencyMask = vec4(0.0f, 0.0f, 0.0f, 1.0f); // Opaque
	}
	else {
		// Pixel is to use full ray tracing.
		PixelTranslucencyMask = vec4(1.0f, 1.0f, 1.0f, 1.0f); // Translucent
	}

	// How much the pixel glows.
	PixelEmissive = vec4(MP.Emissive, MP.Opacity);

	// Merge Ambient Occlusion, Metallic, and Roughness into a single vec4.
	PixelARM = vec4(MP.AmbientOcclusion, MP.Roughness, MP.Metallic, MP.Opacity);

	// TODO: Figure out how alpha blend this.
	// Merge Specular and Shininess into a single vec4.
	PixelSS = vec4(MP.Specular, MP.Shininess);

	vec3 FinalNormal = n;
	if (Material.NormalTextureIndex >= 0) {
		// Convert from [0,1] to [-1,1] range
		vec3 TextureNormal = (texture(MaterialNormalMap, aUV).rgb*2.0) - 1.0;

		// Calculate the world space normal from the texture normal.
		vec3 WorldSpaceTextureNormal = normalize(TBN*TextureNormal);

		// DEBUG: Use to check if normal map is vaguely aligned with vertex normal.
		// FinalNormal = dot(n, WorldSpaceTextureNormal)*vec3(1.0, 1.0, 1.0);
		// Blend vertex normal and normal map based on weights
		FinalNormal = normalize(n*Material.NormalVertexWeight + WorldSpaceTextureNormal*Material.NormalTextureWeight);
	}

	// Encode normal vector from [-1,1] range to [0,1] range for storage
	// Formula: encoded = (normal + 1.0) * 0.5
	PixelNormal = vec4((FinalNormal + vec3(1.0)) * 0.5, MP.Opacity);

	// Determine World Space Position of the pixel. Maybe modify later to do based on interpolated surface normal?
	PixelPosition = vec4(WorldPosition + FinalNormal*texture(MaterialHeightMap, aUV).r, MP.Opacity);

	// Color Pass through for opaque objects.
	PixelColor = vec4(MP.Albedo, MP.Opacity);//*0.01 + PixelNormal*0.9;
}