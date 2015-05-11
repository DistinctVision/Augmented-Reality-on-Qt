#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

struct OmniLight
{
    vec3 position;
    vec4 color;
    float radius;
    float soft;
};

uniform vec4 color;
uniform sampler2D texture0;

uniform vec4 ambientColor;

uniform OmniLight light0;
uniform OmniLight light1;

varying vec3 position;
varying vec3 normal;
varying vec2 texcoord;

void main()
{
    vec4 diffuseColor = color * texture2D(texture0, texcoord);
    vec3 resultLight = ambientColor.rgb;
    vec3 delta = position - light0.position;
    float distance = max(length(delta), 0.0005);
    if (distance < light0.radius) {
        delta /= distance;
        float fade = pow(1.0 - distance / light0.radius, light0.soft);
        float diffuseFactor = max(0.0, dot(delta, normal));
        resultLight += light0.color.rgb * (min(1.0, diffuseFactor * fade) * light0.color.a);
    }
    delta = position - light1.position;
    distance = max(length(delta), 0.0005);
    if (distance < light1.radius) {
        delta /= distance;
        float fade = pow(1.0 - distance / light1.radius, light1.soft);
        float diffuseFactor = max(0.0, dot(delta, normal));
        resultLight += light1.color.rgb * (min(1.0, diffuseFactor * fade) * light1.color.a);
    }
    gl_FragColor = vec4(diffuseColor.r * resultLight.r, diffuseColor.g * resultLight.g, diffuseColor.b * resultLight.b, diffuseColor.a);
}
