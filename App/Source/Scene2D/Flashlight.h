#ifndef FLASHLIGHT_H
#define FLAHSLIGHT

#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

#include "Camera.h"
#include "GameControl\Settings.h"

#include "Inputs\MouseController.h"

class Flashlight
{
private:
	glm::vec3 currentRay;
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	Camera* camera;
	glm::vec3 calculateMouseRay(glm::mat4 viewMatrix);
	glm::vec3 getNormalizedDeviceCoords(float mouseX, float mouseY);
	glm::vec4 toEyeCoords(glm::vec4 clipCoords);
	glm::vec3 toWorldCoords(glm::vec4 eyeCoords, glm::mat4 viewMatrix);

public:
	Flashlight();
	~Flashlight();
	//Cast invisible ray from mouse cursor to world space
	virtual void Init();
	virtual void Update();
	glm::vec3 getCurrentRay();
	bool TestRayOBBIntersection(
		glm::vec3 ray_origin, //Ray origin in world space
		glm::vec3 ray_direction, //Ray direction in world space
		glm::vec3 aabb_min, //untransformed min XYZ coords of mesh, can be scaled first
		glm::vec3 aabb_max, //untransformed max XYZ coords of mesh
		glm::mat4 ModelMatrix, //Transformation applied on mesh (Only works with Transform & Rotate)
		float& intersectionDist //output intersection dist
	);
};

#endif