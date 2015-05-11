#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform vec4 color;
uniform sampler2D texture0;
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
    localSpherePoint = sphereMatrixRotation * localSpherePoint;
    vec2 uv = vec2(atan(localSpherePoint.y, localSpherePoint.x) / M_2_PI + 0.5,
                   0.5 - atan(localSpherePoint.z, length(localSpherePoint.xy)) / M_PI);
    gl_FragColor = color * texture2D(texture0, uv);
}
