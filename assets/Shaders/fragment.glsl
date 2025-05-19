#version 300 es
precision highp float;    // must match vertex shader’s float precision :contentReference[oaicite:3]{index=3}

uniform mat4 uBones[100]; // highp default

flat in  vec3 vNormal;    // flat qualifier to match vertex output :contentReference[oaicite:4]{index=4}
out      vec4 FragColor;

void main() {
    // dummy read to keep uBones alive
    if (uBones[0][0][0] > 1.0e10) {
        discard;
    }

    // debug‐normal coloring (unchanged)
    vec3 c = normalize(vNormal) * 0.5 + 0.5;
    FragColor = vec4(c, 1.0);
}
