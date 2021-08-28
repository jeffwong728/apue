#version 330

uniform vec4 cColor;
out vec4 outputColor;

void main() {
  float lerpVal = gl_FragCoord.y / 400.0f;
  outputColor = cColor;
}
