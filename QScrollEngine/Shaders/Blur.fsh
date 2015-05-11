#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D screenTexture;

uniform vec3 texelParam;

varying vec2 v_texcoord;

float invTotalScale = 1.0 / ((texelParam.z + 1.0) * (texelParam.z + 1.0));

void main()
{
    float scale = texelParam.z + 1.0;
    vec4 value = texture2D(screenTexture, v_texcoord) * scale;
    vec2 d = vec2(0.0);
    while (scale > 0.0)
    {
        d += texelParam.xy;
        scale -= 1.0;
        value += (texture2D(screenTexture, v_texcoord - d) +
                texture2D(screenTexture, v_texcoord + d)) * scale;
    }
    gl_FragColor = value * invTotalScale;
}
