#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 matrix_wvp;

attribute vec4 vertex_position;
attribute vec2 vertex_texcoord;

void main()
{
    gl_Position = matrix_wvp * vertex_position;
}

