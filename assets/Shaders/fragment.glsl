#version 300 es
precision mediump float;

// Mirror the declaration so the linker keeps it
uniform mat4 uBones[100];

flat in vec3 vNormal;
out vec4 FragColor;

void main() {
    // dummy read so fragment stage references it too
    if (uBones[0][0][0] > 1.0e10) {
        discard;
    }

    // debug‐normal coloring
    vec3 c = normalize(vNormal) * 0.5 + 0.5;
    FragColor = vec4(c, 1.0);
}
