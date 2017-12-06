#version 330



in vec2 uvCoords;
in vec3 ecPosition3;
in vec3 outNormal;
out vec4 fragColor;


void main()
{


	fragColor = vec4 (uvCoords,0.6f,1.0f);

}