#version 300 es
precision mediump float;

uniform mat4 uViewMatrix;
uniform sampler2D uTransparency;
uniform sampler2D uBackground;
uniform vec4 uColor;
uniform vec2 uStartPosition;

in vec2 bezierPos;

out vec4 outColor;
void main() {
	int iterations = 100;
	int i;
	vec2 endPosition = bezierPos;
	vec2 positionOnLine = vec2(uViewMatrix * vec4(uStartPosition, 0, 1));
	vec2 dPos = (endPosition - positionOnLine) / float(iterations);
	int firstOccurence = 0;
	for (i = 0; i < iterations; i++) {
		positionOnLine += dPos;
		vec2 pos = (positionOnLine + vec2(1.0, 1.0)) / 2.0;
		pos = vec2(pos.x, -pos.y);
		float transparency = texture(uTransparency, pos).r;
		if (transparency > 0.5) {
			firstOccurence = 1;
		} else {
			if (firstOccurence > 0) {
				outColor = texture(uBackground, vec2(endPosition.x, endPosition.y));
				return;
			}
		}
	}
	outColor = uColor;
}