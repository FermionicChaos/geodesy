#version 450 core
#extension GL_ARB_separate_shader_objects : require

#define MAX_BONE_COUNT 256

layout (location = 0) out vec3 WorldPosition;
layout (location = 1) out vec3 WorldNormal;
layout (location = 2) out vec3 WorldTangent;
layout (location = 3) out vec3 WorldBitangent;
layout (location = 4) out vec3 TextureCoordinate;

layout (location = 0) in vec3   VertexPosition;
layout (location = 1) in vec3   VertexNormal;
layout (location = 2) in vec3   VertexTangent;
layout (location = 3) in vec3   VertexBitangent;
layout (location = 4) in vec3   VertexTextureCoordinate;
layout (location = 5) in vec4   VertexColor;
layout (location = 6) in uvec4  VertexBoneID;
layout (location = 7) in vec4   VertexBoneWeight;

layout (set = 0, binding = 0) uniform Camera3DUBO {
	vec3 Position;
    mat4 Rotation;
    mat4 Projection;
} Camera3D;

layout (set = 0, binding = 1) uniform ObjectUBO {
    vec3 Position;
    mat4 Orientation;
} Object;

layout (set = 0, binding = 2) uniform MeshUBO {
    mat4 Transform;
    mat4 BoneTransform[MAX_BONE_COUNT];
    mat4 OffsetTransform[MAX_BONE_COUNT];
} Mesh;

void main() {
    vec4 v = vec4(VertexPosition, 1.0);
    vec4 n = vec4(VertexNormal, 1.0);
	vec4 t = vec4(VertexTangent, 1.0);
	vec4 b = vec4(VertexBitangent, 1.0);
	vec4 vd = vec4(0.0, 0.0, 0.0, 1.0);

    mat4 mt = mat4(0.0f);
    if (VertexBoneID[0] < MAX_BONE_COUNT) {
        for (int i = 0; i < 4; i++) {
            mt += Mesh.BoneTransform[VertexBoneID[i]] * Mesh.OffsetTransform[VertexBoneID[i]] * VertexBoneWeight[i];
        }
    } else {
        mt = Mesh.Transform;
    }

    v = mt * v;
    n = mt * n;
	t = mt * t;
	b = mt * b;

    mat4 ot = mat4(
        1.0f, 0.0f, 0.0f, Object.Position.x,
        0.0f, 1.0f, 0.0f, Object.Position.y,
        0.0f, 0.0f, 1.0f, Object.Position.z,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    v = ot * Object.Orientation * v;
    n = ot * Object.Orientation * n;
	t = ot * Object.Orientation * t;
	b = ot * Object.Orientation * b;

    WorldPosition   = v.xyz;
    WorldNormal     = n.xyz;
	WorldTangent	= t.xyz;
	WorldBitangent	= b.xyz;

    TextureCoordinate = VertexTextureCoordinate;

	mat4 ct = mat4(
		1.0f, 0.0f, 0.0f, Camera3D.Position.x,
		0.0f, 1.0f, 0.0f, Camera3D.Position.y,
		0.0f, 0.0f, 1.0f, Camera3D.Position.z,
		0.0f, 0.0f, 0.0f, 1.0f
	);
    gl_Position = Camera3D.Projection * Camera3D.Rotation * ct * v;
}