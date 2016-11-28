#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 matrix_wvp;
uniform mat4 matrix_w;

uniform vec3 light0_position;
uniform vec3 light1_position;

attribute vec4 vertex_position;
attribute vec2 vertex_texcoord;
attribute vec3 vertex_normal;

varying vec3 position;
varying vec3 normal;
varying vec2 texcoord;

void main()
{
    normal = normalize(
           vec3(dot(vertex_normal, vec3(matrix_w[0][0], matrix_w[1][0], matrix_w[2][0])),
                dot(vertex_normal, vec3(matrix_w[0][1], matrix_w[1][1], matrix_w[2][1])),
                dot(vertex_normal, vec3(matrix_w[0][2], matrix_w[1][2], matrix_w[2][2]))));
    vec4 p = matrix_w * vertex_position;
    position = p.xyz;
    texcoord = vertex_texcoord;
    gl_Position = matrix_wvp * vertex_position;
}

