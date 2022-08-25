/**
 Rays
 @brief A class that handles the rendering of flashlight's rays
 */
#pragma once

// Include Singleton template
#include "DesignPatterns\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

#include "Camera.h"

struct RenderRay {
	float angle;
	float length;
};

class Rays : public CSingletonTemplate<Rays>, public CEntity2D
{
	friend CSingletonTemplate<Rays>;
public:
	// Init
	bool Init(void);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	void SetRay(int rayNo, float angle, float length);

protected:
	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	Camera* camera;

	RenderRay rays[7];

	// Flashlight's colour, TODO: Allow flickering of flashlight when running out of light using this in the future. Maybe. If have time.
	//glm::vec4 runtimeColour;
	
	// Constructor
	Rays(void);

	// Destructor
	virtual ~Rays(void);
};

