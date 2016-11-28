#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

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
uniform float specularIntensity;
uniform float specularPower;
uniform vec3 viewPosition;

uniform vec4 ambientColor;

uniform SpotLight light0;
uniform SpotLight light1;

varying vec3 position;
varying vec3 normal;
varying vec2 texcoord;

void main()
{
    vec4 diffuseColor = color * texture2D(texture, texcoord);
    vec3 resultLight = ambientColor.rgb, lightDir;
    vec3 delta = position - light0.position;
    vec3 viewDir = normalize(viewPosition - position);
    vec3 rNormal = normalize(normal);
    float distance = max(length(delta), 0.0005);
    if (distance < light0.radius) {
        delta /= distance;
        if (dot(delta, light0.dir) > light0.spotCutOff) {
            float fade = pow(1.0 - distance / light0.radius, light0.soft);
            float diffuseFactor = max(0.0, dot(delta, rNormal));
            float specularFactor = max(0.0, pow(max(0.0, dot(viewDir, normalize(reflect(delta, rNormal)))), specularPower) * specularIntensity);
            resultLight += light0.color.rgb * (min((diffuseFactor + specularFactor) * fade, 1.0) * light0.color.a);
            diffuseColor.a += light0.color.a * specularFactor;
        }
    }
    delta = position - light1.position;
    distance = max(length(delta), 0.0005);
    if (distance < light1.radius) {
        delta /= distance;
        if (dot(delta, light1.dir) > light1.spotCutOff) {
            float fade = pow(1.0 - distance / light1.radius, light1.soft);
            float diffuseFactor = max(0.0, dot(delta, rNormal));
            float specularFactor = max(0.0, pow(max(0.0, dot(viewDir, normalize(reflect(delta, rNormal)))), specularPower) * specularIntensity);
            resultLight += light1.color.rgb * (min((diffuseFactor + specularFactor) * fade, 1.0) * light1.color.a);
            diffuseColor.a += light1.color.a * specularFactor;
        }
    }
    gl_FragColor = vec4(diffuseColor.rgb * resultLight, diffuseColor.a);
}
