#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;
uniform vec4 color;

varying vec2 v_texcoord;

void main()
{
    gl_FragColor = texture2D(texture, v_texcoord) * color;
}

