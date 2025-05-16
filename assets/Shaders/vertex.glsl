// Vertex Shader
#version 300 es
precision mediump float;

// Attributes
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 aJoints;
layout(location = 4) in vec4 aWeights;

// Uniforms
uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform mat4 uBones[100];

// Varyings
flat out vec3 vNormal;

void main() {
    // ─── NO-OP uBones usage ───
    // prevent the driver from optimizing uBones[] away
    vec4 _dummy = uBones[0] * vec4(0.0);

    // ─── compute face normal ───
    // handle non-uniform scale via inverse-transpose
    mat3 normalMatrix = transpose(inverse(mat3(uModel)));
    vNormal = normalize(normalMatrix * aNormal);

    // ─── position ───
    gl_Position = uViewProjection * uModel * vec4(aPosition, 1.0);
}
