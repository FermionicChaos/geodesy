#version 450 core

// Object Data
layout (set = 0, binding = 0) uniform UBOBaseObject {
    vec3    Position; // cm
    mat4    Orientation;
} Object;

// Render Target Data
layout (set = 1, binding = 0) uniform UBOWindow {
    vec2    Size; // cm
    uvec2   Resolution; // px
} Window;

layout (location = 0) in vec3   VertexPosition;
layout (location = 1) in vec3   VertexNormal;
layout (location = 2) in vec3   VertexTangent;
layout (location = 3) in vec3   VertexBitangent;
layout (location = 4) in vec3   VertexTextureCoordinate;
layout (location = 5) in vec4   VertexColor;

layout (location = 0) out vec2 InterpolatedTextureCoordinate;
layout (location = 1) out vec4 InterpolatedColor;

void main() {
    vec4 v = Object.Orientation * vec4(VertexPosition, 1.0f);
    v = v + vec4(Object.Position, 1.0f);
    // mat4 a = mat4(
    //     1.0f / Window.Size.x,   0.0f,                   0.0f,               0.0f,
    //     0.0f,                   1.0f / Window.Size.y,   0.0f,               0.0f,
    //     0.0f,                   0.0f,                   1.0f,               0.0f,
    //     0.0f,                   0.0f,                   0.0f,               1.0f
    // );

    gl_Position = v;
    InterpolatedTextureCoordinate = VertexTextureCoordinate.xy;
    InterpolatedColor = VertexColor;
}
