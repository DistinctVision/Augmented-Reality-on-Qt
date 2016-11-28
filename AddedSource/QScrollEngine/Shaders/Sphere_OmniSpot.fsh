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

uniform vec4 ambientColor;
uniform OmniLight light0;
uniform SpotLight light1;
uniform float diffuseIntensity;
uniform float specularIntensity;
uniform float specularPower;
uniform vec4 color;
uniform sampler2D texture;
uniform vec3 viewPosition;
uniform vec3 sphereCenter;
uniform float sphereRadiusSquared;
uniform mat3 sphereMatrixRotation;

varying vec3 v_position;

vec3 localSphere = sphereCenter - viewPosition;

const float M_PI = 3.14159265;
const float M_2_PI = 3.14159265 * 2.0;

void main()
{
    vec3 delta = normalize(v_position - viewPosition);
    vec3 localSpherePoint = delta * (dot(localSphere, delta)) - localSphere;
    float a = dot(localSpherePoint, localSpherePoint);
    if (a > sphereRadiusSquared) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }
    localSpherePoint -= delta * (sqrt(sphereRadiusSquared - a));
    vec3 localSpherePointTransformed = sphereMatrixRotation * localSpherePoint;
    vec2 uv = vec2(atan(localSpherePointTransformed.y, localSpherePointTransformed.x) / M_2_PI + 0.5,
                   0.5 - atan(localSpherePointTransformed.z, length(localSpherePointTransformed.xy)) / M_PI);
    vec3 normal = normalize(localSpherePoint);
    vec4 diffuseColor = color * texture2D(texture, uv);
    vec3 resultLight = ambientColor.rgb;
    vec3 globalSpherePoint = localSpherePoint + sphereCenter;
    vec3 viewDir = normalize(globalSpherePoint - viewPosition);
    delta = light0.position - globalSpherePoint;
    float distance = max(length(delta), 0.0005);
    if (distance < light0.radius) {
        delta /= distance;
        float fade = pow(1.0 - distance / light0.radius, light0.soft);
        float diffuseFactor = max(0.0, dot(delta, normal)) * diffuseIntensity;
        float specularFactor = max(0.0, pow(max(0.0, dot(viewDir, normalize(reflect(delta, normal)))),
                                            specularPower) * specularIntensity);
        resultLight += light0.color.rgb * (min(1.0, (diffuseFactor + specularFactor) * fade) * light0.color.a);
        diffuseColor.a += light0.color.a * specularFactor;
    }
    delta = light1.position - globalSpherePoint;
    distance = max(length(delta), 0.0005);
    if (distance < light1.radius) {
        delta /= distance;
        if (dot(delta, light1.dir) > light1.spotCutOff) {
            float fade = pow(1.0 - distance / light1.radius, light1.soft);
            float diffuseFactor = max(0.0, dot(delta, normal)) * diffuseIntensity;
            float specularFactor = max(0.0, pow(max(0.0, dot(viewDir, normalize(reflect(delta, normal)))),
                                                specularPower) * specularIntensity);
            resultLight += light1.color.rgb * (min(1.0, (diffuseFactor + specularFactor) * fade) * light1.color.a);
            diffuseColor.a += light1.color.a * specularFactor;
        }
    }
    gl_FragColor = vec4(resultLight * diffuseColor.rgb, diffuseColor.a);

}
