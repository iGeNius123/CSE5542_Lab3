#version 330 core
// Vertex Shader � file "PassThruFarPlane.vert"

in  vec2 in_Position;
out vec4 color;

void main(void)
{
	gl_Position = vec4(in_Position, 1.0, 1.0);
	color = vec4(0.95f,0.56f,0.23f,0.1f);
}
