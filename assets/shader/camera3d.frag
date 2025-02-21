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

layout (set = 0, binding = 0) uniform Camera3DUBO {
	vec3 Position;
	mat4 Rotation;
	mat4 Projection;
} Camera3D;

layout (set = 0, binding = 3) uniform MaterialUBO {
	// Rendering System Metadata
	int RenderingSystem;
	int Transparency;

	// ----- Material Property Constants & Control Weights ----- //
	float VertexColorWeight;
	float TextureColorWeight;
	float ColorWeight;
	vec3 Color; // Aligns to 16 bytes

	float TextureSpecularWeight;
	float SpecularWeight;
	vec3 Specular; // Aligns to 16 bytes

	float TextureAmbientWeight;
	float AmbientWeight;
	vec3 AmbientLighting; // Aligns to 16 bytes

	float TextureEmissiveWeight;
	float EmissiveWeight;
	vec3 Emissive; // Aligns to 16 bytes

	float TextureShininessWeight;
	float ShininessWeight;
	float Shininess;

	float TextureOpacityWeight;
	float OpacityWeight;
	float Opacity;

	float VertexNormalWeight;
	float TextureNormalWeight;

	float TextureAmbientOcclusionWeight;
	float AmbientOcclusionWeight;
	float AmbientOcclusion;

	float TextureReflectionWeight;
	float ReflectionWeight;
	float Reflection;

	float TextureMetallicWeight;
	float MetallicWeight;
	float Metallic;

	float TextureRoughnessWeight;
	float RoughnessWeight;
	float Roughness;

	float TextureSheenWeight;
	float SheenWeight;
	vec3 SheenColor; // Aligns to 16 bytes
	float SheenRoughness;

	float TextureClearCoatWeight;
	float ClearCoatWeight;
	float ClearCoat;
	float ClearCoatRoughness;

	// ----- Extraneous Material Properties ----- //
	float RefractionIndex;

	float Anisotropy;
	vec3 AnisotropyDirection; // Aligns to 16 bytes

	float SubsurfaceScattering;

	float ParallaxScale;
	int ParallaxIterationCount;
} Material;

// Texture Data
layout (set = 1, binding = 0) uniform sampler2D MaterialColor; 				// Color of the Surface
layout (set = 1, binding = 1) uniform sampler2D MaterialOpacity; 			// Opacity Map
layout (set = 1, binding = 2) uniform sampler2D MaterialNormalMap; 			// Normal Map
layout (set = 1, binding = 3) uniform sampler2D MaterialHeightMap; 			// Height Map
layout (set = 1, binding = 4) uniform sampler2D MaterialEmissive; 			// Emissive Lighting
// Blinn-Phong Specular Highlights
layout (set = 1, binding = 5) uniform sampler2D MaterialSpecular; 			// Specular Highlights
layout (set = 1, binding = 6) uniform sampler2D MaterialShininess; 			// Shininess Map
// PBR Specific Textures
layout (set = 1, binding = 7) uniform sampler2D MaterialAmbientOcclusion; 	// Ambient Occlusion Map
layout (set = 1, binding = 8) uniform sampler2D MaterialMetallic; 			// Metallic Map
layout (set = 1, binding = 9) uniform sampler2D MaterialRoughness; 			// Roughness Map
layout (set = 1, binding = 10) uniform sampler2D MaterialSheen; 			// Sheen Map
layout (set = 1, binding = 11) uniform sampler2D MaterialClearCoat; 		// Clear Coat Map

// -------------------- OUTPUT DATA -------------------- //

// Geometry Buffer Ouput. Pixel Position is the world space
// position of the pixel, PixelNormal is the world space normal
// for that pixel. These world space positions and normals will
// be used for deferred lighting calculations.
layout (location = 0) out vec4 PixelColor;
layout (location = 1) out vec4 PixelPosition;
layout (location = 2) out vec4 PixelNormal;
layout (location = 3) out vec4 PixelEmissive;
layout (location = 4) out vec4 PixelSS;
layout (location = 5) out vec4 PixelORM;

vec2 bisection_parallax(vec2 aUV, mat3 aTBN) {

	// ITBN takes any vector from world space and converts it to tangent space.
	mat3 ITBN = transpose(aTBN);

	// Converts the view direction from world space to tangent space.
	vec3 InvertedViewDir = ITBN * normalize(Camera3D.Position - WorldPosition);

	// This determines the maximum UV offset that can be applied. while still possibly
	// containing the correct UV coordinate that can possibly intersect the view dir
	// with the height map.
	float UVMaxMag = Material.ParallaxScale * length(InvertedViewDir.xy) / InvertedViewDir.z;

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
	float yStart = hStart - (length(UVStart - aUV) / UVMaxMag) * Material.ParallaxScale;
	float yEnd = hEnd - (length(UVEnd - aUV) / UVMaxMag) * Material.ParallaxScale;

	// Perform the bisection search here. We can modify later
	// to start biased towards the end and move inwards. That
	// way it finds the first intersection farthest to UVMax.
	vec2 UVMid;
	for (int i = 0; i < Material.ParallaxIterationCount; i++) {

		// Compute the midpoint between UVStart and UVEnd
		UVMid = (UVStart + UVEnd) * 0.5;
		float hMid = texture(MaterialHeightMap, UVMid).r;
		float yMid = hMid - (length(UVMid - aUV) / UVMaxMag) * Material.ParallaxScale;

		// Early exit condition if height threshold is met.
		if (abs(hMid - (length(UVMid - aUV) / UVMaxMag) * Material.ParallaxScale) < 0.01) {
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

// Comment out to remove lighting testing.
const vec4 AmbientLight = vec4(0.8, 0.8, 0.6, 1.0);
const float LightAmplitude = 100.0;
const vec4 LightColor = vec4(0.0, 1.0, 1.0, 1.0);
const vec3 LightPosition = vec3(0.0, -5.0, 5.0);

float blinn_phong(float aKs, float aS, vec3 aN, vec3 aL, vec3 aV) {
	// Calculate the half vector between the light and view direction.
	return ((1.0 - aKs) * max(dot(aN, aL), 0.0) + aKs * pow(max(dot(aN, normalize(aL + aV)), 0.0), aS));
}

float cook_torrance(float aF0, float aAlpha, vec3 aN, vec3 aL, vec3 aV) {
	// Calculate the half vector between the light and view direction.
	vec3 H = normalize(aL + aV);

	// Calculate the dot products of the normal with the half vector, light, and view direction.
	float NdotH = dot(aN, H);
	float NdotL = dot(aN, aL);
	float NdotV = dot(aN, aV);
	float HdotL = dot(H, aL);
	float HdotV = dot(H, aV);

	// Calculate fresnel term.
	float F = aF0 + (1.0 - aF0) * pow(1.0 - NdotH, 5.0);

	// Calculate geometric attenuation.
	float G = min(1.0, min(2.0 * NdotH * NdotV / HdotV, 2.0 * NdotH * NdotL / HdotV));

	// Calculate the distribution term.
	float D = exp((NdotH * NdotH - 1.0) / (aAlpha * aAlpha * NdotH * NdotH)) / (aAlpha * aAlpha * NdotH * NdotH * NdotH * NdotH);

	return F * G * D / (4.0 * NdotL * NdotV);	
}

vec4 final_color(vec2 aUV) {
	// Sampled color from material and texture.
	vec4 SampledColor = mix(texture(MaterialColor, aUV), vec4(Material.Color, Material.Opacity), Material.ColorWeight);

	// Calculates the distance between the light source and the pixel.
	vec3 l = normalize(LightPosition - PixelPosition.xyz);
	vec3 v = normalize(Camera3D.Position - PixelPosition.xyz);
	float r = length(PixelPosition.xyz - LightPosition);
	vec3 h = (l + v) / length(l + v);

	// 
	float CosTheta = max(dot(l, PixelNormal.xyz), 0.0);
	
	// Calculates the final color of the pixel based on ambient lighting and light source.
	vec4 FinalColor = SampledColor * (AmbientLight + LightAmplitude * CosTheta * LightColor / (r * r)); 

	return FinalColor;
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

	// Calculate UV coordinates after applying the height map.
	vec2 UV = TextureCoordinate.xy; //bisection_parallax(TextureCoordinate.xy, TBN);

	vec3 Emissive = texture(MaterialEmissive, UV).rgb*Material.TextureEmissiveWeight + Material.Emissive*Material.EmissiveWeight;
	float Specular = texture(MaterialSpecular, UV).r*Material.TextureSpecularWeight + Material.Specular.r*Material.SpecularWeight;
	float Shininess = texture(MaterialShininess, UV).r*Material.TextureShininessWeight + Material.Shininess*Material.ShininessWeight;
	float AmbientOcclusion = texture(MaterialAmbientOcclusion, UV).r*Material.TextureAmbientOcclusionWeight + Material.AmbientOcclusion*Material.AmbientOcclusionWeight;
	float Metallic = texture(MaterialMetallic, UV).r*Material.TextureMetallicWeight + Material.Metallic*Material.MetallicWeight;
	float Roughness = texture(MaterialRoughness, UV).r*Material.TextureRoughnessWeight + Material.Roughness*Material.RoughnessWeight;
	// vec3 SheenColor = texture(MaterialSheen, UV).rgb*Material.TextureSheenWeight + Material.SheenColor*Material.SheenWeight;
	// float SheenRoughness = Material.SheenRoughness;
	// float ClearCoat = texture(MaterialClearCoat, UV).r*Material.TextureClearCoatWeight + Material.ClearCoat*Material.ClearCoatWeight;
	// float ClearCoatRoughness = Material.ClearCoatRoughness;

	// Get Texture Normal, z should be 1.0 if directly normal to surface.
	vec3 TextureNormal = normalize(2.0*texture(MaterialNormalMap, UV).rgb - 1.0);
	PixelNormal = vec4(n, 1.0)*Material.VertexNormalWeight + vec4(TBN*TextureNormal, 1.0)*Material.TextureNormalWeight;

	// Determine World Space Position of the pixel. Maybe modify later to do based on interpolated surface normal?
	PixelPosition = vec4(WorldPosition, 1.0) + PixelNormal*texture(MaterialHeightMap, UV).r;

	// Copy over material properties to the output.
	PixelEmissive = vec4(Emissive, 1.0);
	PixelSS = vec4(Specular, Shininess, 0.0, 0.0);
	PixelORM = vec4(AmbientOcclusion, Metallic, Roughness, 0.0);

	// Calculates Albedo based on weights of the texture, vertex color, and material color.
	PixelColor = InterpolatedVertexColor*Material.VertexColorWeight + texture(MaterialColor, UV)*Material.TextureColorWeight + vec4(Material.Color, Material.Opacity)*Material.ColorWeight;

}