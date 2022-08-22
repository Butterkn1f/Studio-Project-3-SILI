/**
 Mesh.h
 By: Lim Chian Song
 Date: Apr 2021
 */
#ifndef MESH_H
#define MESH_H

#include <includes/glm.hpp>
#include <vector>
#include "Material.h"

 /**
	 Vertex Class
	 Added normals :D
 */
struct Vertex
{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec3 normal;
	glm::vec2 texCoord;
	Vertex()
	{
		position = glm::vec3();
		color = glm::vec4(1.0);
		normal = glm::vec3();
		texCoord = glm::vec2();
	}
};

/**
	Mesh
	Used for generating mesh for rendering
	Contains the vertex buffer and index buffer
*/
class CMesh
{
public:
	enum DRAW_MODE
	{
		DRAW_TRIANGLES, //default mode
		DRAW_TRIANGLE_STRIP,
		DRAW_LINES,
		DRAW_MODE_LAST,
	};

	unsigned vertexBuffer;
	unsigned indexBuffer;
	unsigned indexSize;

	std::vector<Material> materials;
	static unsigned locationKa;
	static unsigned locationKd;
	static unsigned locationKs;
	static unsigned locationNs;

	Material material;

	DRAW_MODE mode;

	static void SetMaterialLoc(unsigned kA, unsigned kD, unsigned kS,
		unsigned nS);

	// Constructor
	CMesh(void);
	// Destructor
	~CMesh(void);
	virtual void Render();
};

#endif