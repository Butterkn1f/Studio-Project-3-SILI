#version 330 core
out vec4 FragColour;

in vec2 TexCoord;
in vec4 Colour;
in vec3 Pos;
in vec3 Normal;

// texture samplers
uniform sampler2D imageTexture;
uniform vec4 runtimeColour;

struct Light {
	int type;
	vec3 position_cameraspace;
	vec4 color;
	float power;
	float kC;
	float kL;
	float kQ;
	vec3 spotDirection;
	float cosCutoff;
	float cosInner;
	float exponent;
};

struct Material {
	vec4 kAmbient;
	vec4 kDiffuse;
	vec4 kSpecular;
	float kShininess;
};

float getAttenuation(Light light, float distance) {
	//directional light
	if (light.type == 1)
		return 1;
	else
		return 1 / max(1, light.kC + light.kL * distance + light.kQ * distance * distance);
}

float getSpotlightEffect(Light light, vec3 lightDirection) {
	vec3 S = normalize(light.spotDirection);
	vec3 L = normalize(lightDirection);
	float cosDirection = dot(L, S);

	if (cosDirection < light.cosCutoff)
		return 0;
	else
		return 1; //pow(cosDirection, light.exponent);
}

// Constant values
const int MAX_LIGHTS = 8;

// Values that stay constant for the whole mesh
uniform bool lightEnabled;
uniform Light lights[MAX_LIGHTS];
uniform Material material;
uniform int numLights;

void main()
{
	if (lightEnabled == true)
	{
		// Material properties
		vec4 materialColor;
		materialColor = texture(imageTexture, TexCoord);

		// Vectors
		vec3 eyeDirection_cameraspace = -Pos;
		vec3 E = normalize(eyeDirection_cameraspace);
		vec3 N = normalize(Normal);

		FragColour =
			// Ambient: simulates indirect lighting
			materialColor * material.kAmbient;

		for (int i = 0; i < numLights; ++i)
		{
			// Light direction
			float spotlightEffect = 1;
			vec3 lightDirection_cameraspace;
			if (lights[i].type == 1) {
				lightDirection_cameraspace = lights[i].position_cameraspace;
			}
			else if (lights[i].type == 2) {
				lightDirection_cameraspace = lights[i].position_cameraspace - Pos;
				spotlightEffect = getSpotlightEffect(lights[i], lightDirection_cameraspace);
			}
			else {
				lightDirection_cameraspace = lights[i].position_cameraspace - Pos;
			}
			// Distance to the light
			float distance = length(lightDirection_cameraspace);

			// Light attenuation
			float attenuationFactor = getAttenuation(lights[i], distance);

			vec3 L = normalize(lightDirection_cameraspace);
			float cosTheta = clamp(dot(N, L), 0, 1);

			vec3 R = reflect(-L, N);
			float cosAlpha = clamp(dot(E, R), 0, 1);

			FragColour +=
				// Diffuse: "color" of the object
				materialColor * material.kDiffuse * lights[i].color * lights[i].power * cosTheta * attenuationFactor * spotlightEffect +

				// Specular: reflective highlight
				material.kSpecular * lights[i].color * lights[i].power * pow(cosAlpha, material.kShininess) * attenuationFactor * spotlightEffect;
		}
	}
	else
	{
		FragColour = texture(imageTexture, TexCoord);
		FragColour *= runtimeColour;
	}
	//FragColour = texture(imageTexture, TexCoord);
	//FragColour *= Colour;
	//FragColour *= runtimeColour;
}