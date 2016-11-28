#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 matrix_wvp;
uniform mat4 matrix_w;

attribute vec4 vertex_position;
varying vec3 v_position;

void main()
{
    vec4 v = matrix_w * vertex_position;
    v_position = v.xyz;
    gl_Position = matrix_wvp * vertex_position;
}

