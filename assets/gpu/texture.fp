#version 330 core 
//precision mediump float;
in vec2 tex;
in vec4 color;

uniform sampler2D uTexture;

out vec4 fragColor;

//-----------------------------------------------------------------------------

void main() {
  fragColor  = texture(uTexture, tex);
}
