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

struct SpotLight
{
    vec3 position;
    vec4 color;
    float radius;
    float soft;
    vec3 dir;
    float spotCutOff;
};

uniform vec4 color;
uniform sampler2D texture;

uniform vec4 ambientColor;

uniform OmniLight light0;
uniform SpotLight light1;

varying vec3 position;
varying vec3 normal;
varying vec2 texcoord;

void main()
{
    vec4 diffuseColor = color * texture2D(texture, texcoord);
    vec3 resultLight = ambientColor.rgb;
    vec3 delta = position - light0.position;
    float distance = max(length(delta), 0.0005);
    vec3 rNormal = normalize(normal);
    if (distance < light0.radius) {
        delta /= distance;
        float fade = pow(1.0 - distance / light0.radius, light0.soft);
        float diffuseFactor = max(0.0, dot(delta, rNormal));
        resultLight += light0.color.rgb * (min(diffuseFactor * fade, 1.0) * light0.color.a);
    }
    delta = light1.position - position;
    distance = max(length(delta), 0.0005);
    if (distance < light1.radius) {
        delta /= distance;
        if (dot(delta, light1.dir) > light1.spotCutOff) {
            float fade = pow(1.0 - distance / light1.radius, light1.soft);
            float diffuseFactor = max(0.0, dot(delta, rNormal));
            resultLight += light1.color.rgb * (min(diffuseFactor * fade, 1.0) * light1.color.a);
        }
    }
    gl_FragColor = vec4(diffuseColor.rgb * resultLight, diffuseColor.a);
}
