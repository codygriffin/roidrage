#version 330 core
//precision mediump float;
in vec2 tex;
in vec4 color;
out vec4 fragColor;

//-----------------------------------------------------------------------------

void main() { 
  const float d = 0.25;

  vec2 mag = tex - vec2(0.5, 0.5);
  float  m = length(mag);

  //if (m < d) {
  //  gl_FragColor  = color;
  //} else {
  //  gl_FragColor  = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  //}
  fragColor = mix(color, vec4(0.0, 0.0, 0.0, 0.0), m*2.0);
}
