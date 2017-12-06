#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 modelViewProjectionMatrix;

void main(){
 gl_Position =  modelViewProjectionMatrix * vec4(vertexPosition_modelspace,1);
}