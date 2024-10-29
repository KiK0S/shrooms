#version 300 es
precision mediump float;

uniform sampler2D uTexture;
uniform vec4 uColor;

in vec2 bezierPos;

out vec4 outColor;
void main() {
//	outColor = texture(uTexture, bezierPos);
  outColor = uColor;
	//outColor = vec4(0.0, 0.0, 1.0, 1.0);
}