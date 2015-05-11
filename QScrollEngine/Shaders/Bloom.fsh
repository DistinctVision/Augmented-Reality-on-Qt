#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D screenTexture;
uniform sampler2D bloomMapTexture;

uniform vec3 texelParam;

vec2 dx = vec2(texelParam.x, 0.0);
vec2 dy = vec2(0.0, texelParam.y);
vec2 dxdy = vec2(texelParam.x, texelParam.y);
vec2 dxdy_ = vec2(-texelParam.x, texelParam.y);

varying vec2 v_texcoord;

void main()
{
    vec3 addedColor = (texture2D(bloomMapTexture, v_texcoord.xy).rgb +
                       texture2D(bloomMapTexture, v_texcoord.xy + dx).rgb +
                       texture2D(bloomMapTexture, v_texcoord.xy - dx).rgb +
                       texture2D(bloomMapTexture, v_texcoord.xy + dy).rgb +
                       texture2D(bloomMapTexture, v_texcoord.xy - dy).rgb +
                       texture2D(bloomMapTexture, v_texcoord.xy + dxdy).rgb +
                       texture2D(bloomMapTexture, v_texcoord.xy - dxdy).rgb +
                       texture2D(bloomMapTexture, v_texcoord.xy + dxdy_).rgb +
                       texture2D(bloomMapTexture, v_texcoord.xy - dxdy_).rgb) * 0.1111111;
    gl_FragColor = vec4(texture2D(screenTexture, v_texcoord).rgb + (addedColor * texelParam.z), 1.0);
}
