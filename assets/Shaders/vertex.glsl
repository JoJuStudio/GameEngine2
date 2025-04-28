// Shaders/vertex.glsl

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

varying vec3 vNormal;
varying vec2 vTexCoord;

void main() {
    mat3 normalMatrix = mat3(uModel);

    vNormal = normalize(normalMatrix * aNormal);
    vTexCoord = aTexCoord;
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
}
