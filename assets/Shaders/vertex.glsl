#version 300 es
precision mediump float;

// Attributes
layout(location = 0) in vec3 aPosition;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 aJoints;
layout(location = 4) in vec4 aWeights;

// Uniforms
uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform mat4 uBones[100]; // Match max bones in your model

// Varyings
out vec2 vTexCoord;

void main() {
    vec4 totalPosition = vec4(0.0);

    for(int i = 0; i < 4; i++) {
        int joint = aJoints[i];
        float weight = aWeights[i];
        if(weight > 0.0 && joint >= 0) {
            totalPosition += weight * (uBones[joint] * vec4(aPosition, 1.0));
        }
    }

    gl_Position = uViewProjection * uModel * totalPosition;
    vTexCoord = aTexCoord;
}
