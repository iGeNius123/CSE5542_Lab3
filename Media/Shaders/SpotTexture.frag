#version 330
uniform sampler2D spotTexture;
uniform sampler2D normalMap;
in vec4 baseColor;
in vec2 uvCoords;
out vec4 fragColor;
in vec3 inPosition2;
in vec3 inNormal2;
in vec2 inTexCoords2;
struct LightSource
{
	vec4 position;
	vec4 color;
};

struct BasicMaterial
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};
const int maxLights = 2;
uniform vec4 lightPositions[maxLights];
uniform vec4 lightColors[maxLights];
uniform BasicMaterial frontMaterial;

uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 normalMatrix;
void PointLight2(in int i, in vec3 eye, in vec3 ecPosition3, in vec3 normal,
	inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	float nDotVP;	// normal . light direction
	float nDotHV;	// normal . half vector
	float pf;		// power factor
	float attenuation;	// computed attenuation factor
	float d;			// distance from surface to light source
	vec3 VP;			// direction from surface to light position
	vec3 halfVector;	// direction of maximum highlights

						// Compute vector from surface to light position
	vec4 lightPosInCamera = modelViewMatrix * lightPositions[i];
	VP = normalize(vec3(lightPosInCamera) - ecPosition3);

	// Compute distance between surface and light position
	d = length(VP);

	// Compute attenuation
	attenuation = 1.0;

	//halfVector = normalize(VP + eye);
	halfVector = normalize(2 * dot(normal, VP)*normal - VP);

	nDotVP = max(0.0, dot(normal, normalize(VP)));
	nDotHV = max(0.0, dot(normal, halfVector));

	if (nDotVP <= 0.0)
		pf = 0.0;
	else
		pf = pow(nDotHV, frontMaterial.shininess);

	diffuse += lightColors[i] * nDotVP * attenuation;
	specular += lightColors[i] * pf * attenuation;
}


void DirectionalLight2(in int i, in vec3 normal, in vec3 ecPosition3,
	inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	float pf;		// power factor

	vec3 lightDir = normalize((normalMatrix * lightPositions[i]).xyz);
	vec3 reflectionDir = normalize(2 * dot(normal, lightDir)*normal - lightDir);
	vec3 viewDir = normalize(ecPosition3);

	float nDotL = max(0.0, dot(normal, lightDir));
	float vDotR = max(0.0, dot(viewDir, reflectionDir));

	if (nDotL <= 0.0)
		pf = 0.0;
	else
		pf = pow(vDotR, frontMaterial.shininess);

	diffuse += lightColors[i] * nDotL;
	specular += lightColors[i] * pf;
}

void main( )
{
	vec4 baseColor2 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 ambient = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);

	// Calculate the eye position
	vec4 position = vec4(inPosition2, 1);
	vec4 ecPosition = modelViewMatrix * position;
	vec3 ecPosition3 = (vec3(ecPosition)) / ecPosition.w;
	vec3 eye = vec3(0, 0, 1);
	
	// Calculate the lighting
	vec3 normal = normalize((normalMatrix * vec4(inNormal2, 0)).xyz);
	vec4 baseColor3 = texture2D( spotTexture, uvCoords );
	//vec3 normal = normalize(inPosition);
	for (int i = 0; i <maxLights; i++)
	{
		if (lightPositions[i].w <= 0.1)
			DirectionalLight2(i, normal, ecPosition3, ambient, diffuse, specular);
		else
			PointLight2(i, eye, ecPosition3, normal, ambient, diffuse, specular);
	}
		baseColor2 = ambient* frontMaterial.ambient
		+ diffuse * frontMaterial.diffuse
		+ specular*frontMaterial.specular;

	
	fragColor = baseColor3;
} 
