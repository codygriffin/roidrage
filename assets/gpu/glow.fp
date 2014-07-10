#version 150
//precision mediump float;
in vec2 tex;
in vec4 color;
out vec4 fragColor;

uniform sampler2D uGlowFrame;
uniform float fade;

//-----------------------------------------------------------------------------

void main()
{
  fragColor = texture(uGlowFrame, tex) * fade;
/*
  vec4 sum = vec4(0);
  int j;
  int i;

  for( i= -4 ;i < 4; i++) {
    for (j = -3; j < 3; j++) {
      sum += texture2D(uGlowFrame, tex + vec2(j, i)*0.004) * 0.25;
    }
  }
  if (texture2D(uGlowFrame, tex).r < 0.3) {
   gl_FragColor = sum*sum*0.012 + texture2D(uGlowFrame, tex);
  } else {
    if (texture2D(uGlowFrame, tex).r < 0.5) {
      gl_FragColor = sum*sum*0.009 + texture2D(uGlowFrame, tex);
    } else {
      gl_FragColor = sum*sum*0.0075 + texture2D(uGlowFrame, tex);
    }
  }
*/
}
