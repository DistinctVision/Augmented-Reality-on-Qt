#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texturePart;
uniform vec4 color;
uniform vec4 backColor;

uniform vec2 partParam;

varying vec2 v_texcoord;

void main()
{
    float value = clamp(1.0 - (partParam.x - texture2D(texturePart, v_texcoord).r) / partParam.y, 0.0, 1.0);
    gl_FragColor = backColor * value + color * (1.0 - value);
}

