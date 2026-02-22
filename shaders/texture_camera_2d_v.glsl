#version 300 es
precision mediump float;
layout(location = 0) in vec2 aVertexPosition;
layout(location = 1) in vec2 aTexturePosition;

uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uProjectionMatrix;
uniform bool toFramebuffer;

out vec2 vTextureCoordinate;
out vec2 vPosition;

void main() {
	vec2 pos = vec2(aVertexPosition.x, aVertexPosition.y);
	gl_Position = uViewMatrix * uModelMatrix * vec4(pos, 0, 1);
	//if (toFramebuffer) {
	//	gl_Position.y *= -1.0;
	//}
	vTextureCoordinate = aTexturePosition;
	vPosition = vec2(gl_Position.x, gl_Position.y);
}