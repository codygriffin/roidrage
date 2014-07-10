#version 150
//precision mediump float;

in vec2 tex;
in vec4 color;
out vec4 fragColor;

uniform sampler2D uTexture;
void main() {
  fragColor  = texture(uTexture, tex);
  fragColor *= color;
}
