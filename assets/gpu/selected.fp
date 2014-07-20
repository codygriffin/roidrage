#version 150
//precision mediump float;
in vec2 tex;
in vec4 color;
out vec4 fragColor;

//-----------------------------------------------------------------------------

void main() {
  float r = length(2 * tex - vec2(1.0, 1.0));
  //float r = length(tex);
  
  if (r > 0.90 && r < 1.0) {
    fragColor  = color;
  } else {
    fragColor  = vec4(0.0, 0.0, 0.0, 0.0);
  }
}
