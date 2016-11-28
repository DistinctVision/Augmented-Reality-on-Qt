#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 matrix_wvp;
uniform mat4 matrix_w;
uniform mat2 matrix_screen;

attribute vec3 vertex_position;
attribute vec3 vertex_normal;

varying vec3 vertexWorldPosition;
varying vec3 vertexWorldNormal;
varying vec3 texProj;

void main()
{
    gl_Position = matrix_wvp * vec4(vertex_position, 1.0);
    vec4 t = matrix_w * vec4(vertex_position, 1.0);
    vertexWorldPosition = t.xyz;
    t = normalize(matrix_w * vec4(vertex_normal, 0.0));
    vertexWorldNormal = t.xyz;
    vec2 screen_t = matrix_screen * gl_Position.xy;
    texProj.x = 0.5 * (gl_Position.w + screen_t.x);
    texProj.y = 0.5 * (gl_Position.w - screen_t.y);
    texProj.z = gl_Position.w;
}
