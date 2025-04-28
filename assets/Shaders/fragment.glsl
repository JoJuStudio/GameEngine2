// romfs:/Shaders/fragment.glsl

precision mediump float;

varying vec3 vNormal;
varying vec2 vTexCoord;

// Add all the textures your Material binds!
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_MetallicRoughnessMap;
uniform sampler2D u_NormalMap;

// Base material factors
uniform vec4 u_BaseColorFactor;
uniform float u_Metallic;
uniform float u_Roughness;

// Simple constant light
const vec3 LightDir = normalize(vec3(0.5, 1.0, 0.3));

void main() {
    vec3 normal = normalize(vNormal);

    // Sample textures
    vec4 baseColor = texture2D(u_AlbedoMap, vTexCoord);

    // Fake simple lighting
    float lightIntensity = max(dot(normal, LightDir), 0.2);

    vec3 color = baseColor.rgb * lightIntensity;

    gl_FragColor = vec4(color, baseColor.a);
}
