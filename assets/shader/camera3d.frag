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
	vec3 Color;
	vec3 Emissive;
	vec3 Ambient;
	vec3 Specular;
	float Opacity;
	float RefractionIndex;
	float Shininess;
	float Metallic;
	float Roughness;
	float VertexColorWeight;
	float MaterialColorWeight;
	float ParallaxScale;
	int ParallaxIterationCount;
} Material;

// Texture Data
layout (set = 1, binding = 0) uniform sampler2D SurfaceColor; 				// Color of the Surface
layout (set = 1, binding = 1) uniform sampler2D SurfaceNormalMap; 			// Normal Map of the Surface.
layout (set = 1, binding = 2) uniform sampler2D SurfaceHeightMap; 			// Bump Map of the surface.
layout (set = 1, binding = 3) uniform sampler2D SurfaceEmission; 			// Light Emission of surface.
layout (set = 1, binding = 4) uniform sampler2D SurfaceOpacity; 			// Opacity of the Surface.
layout (set = 1, binding = 5) uniform sampler2D SurfaceAOC; 				// Opacity of the Surface.
layout (set = 1, binding = 6) uniform sampler2D SurfaceMetallicRoughness;	// PBR Specific

// -------------------- OUTPUT DATA -------------------- //

// Geometry Buffer Ouput. Pixel Position is the world space
// position of the pixel, PixelNormal is the world space normal
// for that pixel. These world space positions and normals will
// be used for deferred lighting calculations.
layout (location = 0) out vec4 PixelColor;
layout (location = 1) out vec4 PixelPosition;
layout (location = 2) out vec4 PixelNormal;

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
	float hStart = texture(SurfaceHeightMap, UVStart).r;
	float hEnd = texture(SurfaceHeightMap, UVEnd).r;

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
		float hMid = texture(SurfaceHeightMap, UVMid).r;
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

const vec4 AmbientLight = vec4(0.8, 0.8, 0.6, 1.0);
const float LightAmplitude = 100.0;
const vec4 LightColor = vec4(0.0, 1.0, 1.0, 1.0);
const vec3 LightPosition = vec3(0.0, 0.0, 5.0);

vec4 final_color(vec2 aUV) {
	// Sampled color from material and texture.
	vec4 SampledColor = mix(texture(SurfaceColor, aUV), vec4(Material.Color, Material.Opacity), Material.MaterialColorWeight);

	// Calculates the distance between the light source and the pixel.
	float r = length(PixelPosition.xyz - LightPosition);

	// 
	// float CosTheta = dot(normalize(PixelNormal.xyz), normalize(LightPosition - PixelPosition.xyz));
	
	// Calculates the final color of the pixel based on ambient lighting and light source.
	vec4 FinalColor = SampledColor * (AmbientLight + LightAmplitude * LightColor / (r * r)); 

	return FinalColor;
}

void main() {

    // Construct the TBN matrix to transform from world space to surface tangent space.
    mat3 TBN = mat3(normalize(WorldTangent), normalize(WorldBitangent), normalize(WorldNormal));

	// Calculate UV coordinates after applying the height map.
	vec2 UV = TextureCoordinate.xy; //bisection_parallax(TextureCoordinate.xy, TBN);

	// Get Texture Normal, z should be 1.0 if directly normal to surface.
	vec3 TextureNormal = normalize(2.0 * texture(SurfaceNormalMap, UV).rgb - 1.0);
	PixelNormal = mix(vec4(WorldNormal, 1.0), vec4(normalize(TBN * TextureNormal), 1.0), 0.0);

	// Determine World Space Position of the pixel. Maybe modify later to do based on interpolaed surface normal?
	PixelPosition = vec4(WorldPosition, 1.0) + PixelNormal * texture(SurfaceHeightMap, UV).r;

	// Calculates Albedo based on weights of the texture, vertex color, and material color.
	PixelColor = final_color(UV);

}