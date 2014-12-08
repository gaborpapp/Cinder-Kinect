#version 150

in float depth;
out vec4 oColor;

void main()
{
//	if( depth < 0.1 ) discard;

	oColor.rgb	= vec3( depth );
	oColor.a	= 1.0;
}





