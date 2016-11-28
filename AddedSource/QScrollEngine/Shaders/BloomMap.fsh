#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D screenTexture;
uniform float threshold;
uniform float gain;

varying vec2 v_texcoord;

//const vec3 average = vec3(0.3333, 0.3333, 0.3333);

void main()
{
    vec3 final_color = texture2D(screenTexture, v_texcoord).rgb;
    vec3 color = max(final_color.rgb, threshold) - vec3(threshold);
    if (dot(color, vec3(1.0)) > 0.0) {
        gl_FragColor.rgb = color * gain;
    } else {
        gl_FragColor.rgb = vec3(0.0);
    }
    gl_FragColor.a = 1.0;
}
