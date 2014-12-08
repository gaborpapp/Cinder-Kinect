#version 150

uniform sampler2D depthTex;

uniform mat4 ciModelViewProjection;

in vec4 ciPosition;
in vec2 ciTexCoord0;

out vec4 vVertex;
out float depth;

void main()
{
	vVertex				= ciPosition;
	depth				= texture( depthTex, ciTexCoord0.st ).b;
	vVertex.z			+= depth * 1000.0;
	gl_Position			= ciModelViewProjection * vVertex;
}
