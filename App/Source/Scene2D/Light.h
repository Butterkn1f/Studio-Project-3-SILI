#pragma once
#include "Primitives/Mesh.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

struct Light
{
	glm::vec3 position;
	glm::vec4 color;
	float power;
	float kC, kL, kQ;
	enum LIGHT_TYPE
	{
		LIGHT_POINT = 0,
		LIGHT_DIRECTIONAL = 1,
		LIGHT_SPOT,
	};
	LIGHT_TYPE type;
	glm::vec3 spotDirection;
	float cosCutoff;
	float cosInner;
	float exponent;
	Light();
};

