#version 450 core
#extension GL_ARB_separate_shader_objects : require

// Geometry Buffer Ouput. Pixel Position is the world space
// position of the pixel, PixelNormal is the world space normal
// for that pixel. These world space positions and normals will
// be used for deferred lighting calculations.
layout (location = 0) out vec4 PixelColor;
layout (location = 1) out vec4 PixelPosition;
layout (location = 2) out vec4 PixelNormal;
// layout (location = 3) out vec4 PixelSpecular;

// Transformed vertex data, interpolated from the vertex shader.
layout (location = 0) in vec3 WorldPosition;
layout (location = 1) in vec3 WorldNormal;
layout (location = 2) in vec3 WorldTangent;
layout (location = 3) in vec3 WorldBitangent;
layout (location = 4) in vec3 TextureCoordinate;

// Camera3D information.
layout (set = 0, binding = 0) uniform Camera3DUBO {
    mat4 Projection;
    mat4 Rotation;
	vec3 Position;
} Camera3D;

// Material texture inputs.
layout (set = 2, binding = 0) uniform sampler2D SurfaceColor; 		// Color of the Surface
layout (set = 2, binding = 1) uniform sampler2D SurfaceNormalMap; 	// Normal Map of the Surface.
layout (set = 2, binding = 2) uniform sampler2D SurfaceHeightMap; 	// Bump Map of the surface.
layout (set = 2, binding = 4) uniform sampler2D SurfaceEmission; 	// Light Emission of surface.
layout (set = 2, binding = 5) uniform sampler2D SurfaceOpacity; 	// Opacity of the Surface.
layout (set = 2, binding = 6) uniform sampler2D SurfaceAOC; 		// Opacity of the Surface.

// PBR Specific
layout (set = 2, binding = 7) uniform sampler2D SurfaceMetallicRoughness;

layout (set = 2, binding = 8) uniform MaterialUBO {
	float ParallaxScale;
	int ParallaxIterations;
} Material;

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
    for (int i = 0; i < Material.ParallaxIterations; i++) {

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

void main() {

    // Construct the TBN matrix to transform from world space to surface tangent space.
    mat3 TBN = mat3(normalize(WorldTangent), normalize(WorldBitangent), normalize(WorldNormal));

	// Calculate UV coordinates after applying the height map.
	vec2 UV = bisection_parallax(TextureCoordinate.xy, TBN);

	// Determin Modified World Space Normals of surface applying material properties.
    // Acquire the surface normals of the normal map with the modified UV coordinates.
    vec3 TangentSurfaceNormal = texture(SurfaceNormalMap, UV).rgb;
    // Convert normal from [0, 1] range to [-1, 1]
    TangentSurfaceNormal = normalize(TangentSurfaceNormal * 2.0 - 1.0);
	// Convert tangent surface normals to world space.
	PixelNormal = vec4(normalize(TBN * TangentSurfaceNormal), 1.0);

	// Determine World Space Position of the pixel. Maybe modify later to do based on interpolaed surface normal?
	PixelPosition = vec4(WorldPosition, 1.0) + PixelNormal * texture(SurfaceHeightMap, UV).r;

	// Simple Tranmsmission of the surface color.
	PixelColor = texture(SurfaceColor, UV);

}