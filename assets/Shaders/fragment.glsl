#version 300 es
precision mediump float;

in vec2 vTexCoord;        // Add this varying
out vec4 FragColor;

uniform sampler2D uTexture;  // Add texture uniform

void main() {
    // Sample texture (fallback to green if no texture)
    vec4 texColor = texture(uTexture, vTexCoord);
    FragColor = mix(vec4(0.2, 0.8, 0.4, 1.0), texColor, texColor.a);
}
