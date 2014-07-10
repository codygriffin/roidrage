#version 330 core
//precision mediump float;
in vec4 color;
in vec2 tex;

out vec4 fragColor;

uniform int time;

//-----------------------------------------------------------------------------
/* 
 *   Calculate a standing wave
 *
 *   f = frequency
 *   t = time in ms
 *   k = wavenumber 
 *   x = position along standing wave 
 *  
 */
float standingWave(float f, float k, float x) {
  float wt  = 2.0 * 3.14159 * f * 0.001 * float(time);
  float kx  = k * x;
  return sin(kx + wt) + sin(kx - wt);
}

//-----------------------------------------------------------------------------

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

//-----------------------------------------------------------------------------

void main() {
  if (tex.x > 0.005 && tex.x < 0.995  && 
      tex.y > 0.005 && tex.y < 0.995 ) {

    float x = tex.x - 0.5;
    float y = tex.y - 0.5;

    float s = 30.0;
    //vec2 q = vec2((floor(x * s) + 1.0)/s, (floor(y * s) + 1.0)/s);
    x = (floor(x * s) + 1.0)/s; 
    y = (floor(y * s) + 1.0)/s;

    float d = sqrt(y * y + x * x);

    // Ring standing wave
    lowp float m;
    m  = standingWave(0.097, 13.0, d + x)
       + standingWave(0.191, 23.0, d * x * y)
       + standingWave(0.059, 19.0, y - d);

    // normalize
    m  = (m * 0.3) + 0.5;

    // quantize
    m  = (floor(m*5.0) + 1.0)/5.0;

  
    fragColor = mix(color, vec4(0.07, 0.07, 0.13, 0.1), m);
  } else {
    fragColor = vec4(0.8, 0.5, 1.0, 1.0);
  }
}
