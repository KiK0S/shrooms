#version 300 es
precision mediump float;

uniform sampler2D uTexture;
uniform vec4 uColor;

in vec2 vTextureCoordinate;
in vec2 vPosition;

out vec4 outColor;
void main() {
	outColor = texture(uTexture, vTextureCoordinate);
	outColor *= uColor;
}