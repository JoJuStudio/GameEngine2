#version 300 es
precision highp float;
precision highp int;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in uvec4 aJoints;
layout(location = 4) in vec4 aWeights;

uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform mat4 uBones[100]; // Reduced size for compatibility

flat out vec3 vNormal;

void main() {
    // Blend bone transforms
    mat4 skinMat =
        aWeights.x * uBones[aJoints.x] +
        aWeights.y * uBones[aJoints.y] +
        aWeights.z * uBones[aJoints.z] +
        aWeights.w * uBones[aJoints.w];

    // Skinned position in model space
    vec4 skinnedPos = skinMat * vec4(aPosition, 1.0);

    // Correct normal transformation for skinning
    mat3 skinNormalMatrix = transpose(inverse(mat3(skinMat)));
    vec3 skinnedNormal = normalize(skinNormalMatrix * aNormal);

    // Apply model and view-projection matrices
    vec4 worldPos = uModel * skinnedPos;
    gl_Position = uViewProjection * worldPos;

    // Correct normal transformation for model matrix
    mat3 modelNormalMatrix = transpose(inverse(mat3(uModel)));
    vNormal = normalize(modelNormalMatrix * skinnedNormal);
}
