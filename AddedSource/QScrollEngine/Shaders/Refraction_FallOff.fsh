#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D screenTexture;

// FallOff
uniform vec3 FallOffColor;		// FallOff Color
uniform lowp float FallOffInt;			// FallOff Intensity
uniform mediump float FallOffSoft;		// FallOff Soft

// Refract
uniform float refractPower;			// Refraction Intensity

uniform vec3 worldCameraPosition;

varying vec3 vertexWorldPosition;
varying vec3 vertexWorldNormal;
varying vec3 texProj;

void main()
{
    vec3 eyeVector = normalize(vertexWorldPosition - worldCameraPosition);
    vec3 cS = texture2D(screenTexture, (texProj.xy / texProj.z) + (refract(eyeVector, vertexWorldNormal, 0.5).xy * refractPower)).rgb;
    float cFO = pow((1.0 - max(dot(eyeVector, vertexWorldNormal), 0.0)), FallOffSoft) * FallOffInt;

    gl_FragColor = vec4(cS * (1.0 - cFO) + FallOffColor * cFO, 1.0);
}

