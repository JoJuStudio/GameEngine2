// romfs:/Shaders/vertex.glsl

// #version 300 es
// precision mediump float;
// precision highp int;
//
// Attributes
// layout(location = 0) in vec3 aPos;
// layout(location = 1) in vec3 aNormal;
// layout(location = 2) in vec2 aTexCoord;
// layout(location = 3) in uvec4 aJoints;
// layout(location = 4) in vec4 aWeights;
//
// Uniforms
// uniform mat4 uModel;
// uniform mat4 uView;
// uniform mat4 uProj;
// uniform mat4 uBones[100]; // must match CPU-side max bones
//
// Varyings
// out vec3 vNormal;
// out vec2 vTexCoord;
//
// void main() {
//     mat4 skinMatrix =
//           aWeights.x * uBones[aJoints.x] +
//           aWeights.y * uBones[aJoints.y] +
//           aWeights.z * uBones[aJoints.z] +
//           aWeights.w * uBones[aJoints.w];
//
//     vec4 skinnedPos = skinMatrix * vec4(aPos, 1.0);
//     vec3 skinnedNormal = mat3(skinMatrix) * aNormal;
//
//     vNormal = normalize(mat3(uModel) * skinnedNormal);
//     vTexCoord = aTexCoord;
//     gl_Position = uProj * uView * uModel * skinnedPos;
// }


// #version 300 es
// precision mediump float;
//
// layout(location = 0) in vec3 aPos;
// layout(location = 1) in vec3 aNormal;
// layout(location = 2) in vec2 aTexCoord;
//
// uniform mat4 uModel;
// uniform mat4 uView;
// uniform mat4 uProj;
//
// out vec3 vFragPos;
// out vec3 vNormal;
// out vec2 vTexCoord;
//
// void main() {
//     vec4 worldPos = uModel * vec4(aPos, 1.0);
//     vFragPos = worldPos.xyz;
//     vNormal = mat3(transpose(inverse(uModel))) * aNormal;
//     vTexCoord = aTexCoord;
//
//     gl_Position = uProj * uView * worldPos;
// }


#version 300 es
precision mediump float;

layout(location = 0) in vec3 aPosition;

uniform mat4 uMVP;

void main() {
    gl_Position = uMVP * vec4(aPosition, 1.0);
}



