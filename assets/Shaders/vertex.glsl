// Attributes
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in uvec4 aJoints;
layout(location = 4) in vec4 aWeights;

// Uniforms
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uBones[100]; // make sure this matches CPU bone count

// Varyings
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    mat4 skinMatrix =
          aWeights.x * uBones[aJoints.x] +
          aWeights.y * uBones[aJoints.y] +
          aWeights.z * uBones[aJoints.z] +
          aWeights.w * uBones[aJoints.w];

    vec4 skinnedPos = skinMatrix * vec4(aPos, 1.0);
    vec3 skinnedNormal = mat3(skinMatrix) * aNormal;

    vNormal = normalize(mat3(uModel) * skinnedNormal);
    vTexCoord = aTexCoord;
    gl_Position = uProj * uView * uModel * skinnedPos;
}
