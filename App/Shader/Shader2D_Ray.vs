#version 330 core
// Input vertex data, different for all executions of this shader
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColour;
layout(location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

// Output data, will be interpolated for each fragment.
out vec3 Pos;
out vec4 Colour;
out vec3 Normal;
out vec2 TexCoord;

// Values that stay constant for the whole mesh
uniform mat4 transform;
uniform mat4 MV;
uniform mat4 MV_inverse_transpose;
uniform bool lightEnabled;

void main()
{
	// Output position of the vertex, in clip space
	gl_Position = transform * vec4(aPos, 1.0);

	// Vector position, in camera space
	Pos = (MV * vec4(aPos, 1.0)).xyz;

	if (lightEnabled == true)
	{
		// Vertex normal, in camera space
		// Use MV if ModelMatrix does not scale the model. Use its inverse transpose otherwise
		Normal = (MV_inverse_transpose * vec4(aNormal, 0)).xyz;
	}

	// Color of each vertex will be interpolated to produce the color of each fragment
	Colour = aColour;
	// Simple pass through. The texCoord of each fragment will be interpolated from texCoord of each vertex
	TexCoord = aTexCoord;
}