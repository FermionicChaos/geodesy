#version 450 core
#extension GL_ARB_separate_shader_objects : require

// -------------------- INPUT DATA -------------------- //
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexTangent;
layout (location = 3) in vec3 VertexBitangent;
layout (location = 4) in vec3 VertexTextureCoordinate;
layout (location = 5) in vec4 VertexColor;

// -------------------- UNIFORM DATA -------------------- //
// Render Target Data
layout (set = 0, binding = 0) uniform WindowUBO {
    vec2 Size; // cm
    uvec3 Resolution; // px
} Window;

// Object Data
layout (set = 0, binding = 1) uniform ObjectUBO {
    vec3 Position;
    mat4 Orientation;
} Object;

// -------------------- OUTPUT DATA -------------------- //
layout (location = 0) out vec2 TextureCoordinate;
layout (location = 1) out vec4 InterpolatedColor;

void main() {
    // Convert Vertex to World Space.
    vec4 v = (Object.Orientation * vec4(2.0 * VertexPosition, 1.0f)) + vec4(Object.Position, 0.0f);
    gl_Position = v;
    TextureCoordinate = VertexTextureCoordinate.xy;
    InterpolatedColor = VertexColor;
}
