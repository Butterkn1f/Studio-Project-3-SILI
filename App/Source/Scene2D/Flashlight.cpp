#include "Flashlight.h"

// Include ImageLoader
#include "System\ImageLoader.h"

#include "Player2D.h"

Flashlight::Flashlight()
	: camera(NULL)
	, cSettings(NULL)
{
}

Flashlight::~Flashlight()
{
	camera = NULL;
	cSettings = NULL;
}

void Flashlight::Init()
{
	// Get handler for camera
	camera = Camera::GetInstance();
	// Get handler for cSettings
	cSettings = CSettings::GetInstance();

	// Should make this not hardcoded
	projectionMatrix = glm::perspective(
		glm::radians(45.f),
		(float)(cSettings->iWindowWidth / cSettings->iWindowHeight),
		0.1f,
		1.f);
	// Set to identity
	viewMatrix = glm::mat4(1.0f);
}

glm::vec3 Flashlight::getCurrentRay()
{
	glm::normalize(currentRay);
	return currentRay;
}

glm::vec3 Flashlight::getDirectionalVector(glm::vec3 pos)
{
	glm::vec3 directional = glm::vec3(pos.x - cSettings->iWindowWidth * 0.5, cSettings->iWindowHeight * 0.5 - pos.y, 0.f);
	glm::vec3 worldRay = glm::normalize(directional);
	return glm::normalize(worldRay);
}

void Flashlight::Update()
{
	viewMatrix = glm::lookAt(
		glm::vec3(camera->position.x, camera->position.y, camera->position.z),
		glm::vec3(camera->target.x, camera->target.y, camera->target.z),
		glm::vec3(camera->up.x, camera->up.y, camera->up.z)
	);

	//std::cout << camera->position.x << ", " << camera->position.y << ", " << camera->position.z << std::endl;
	currentRay = calculateMouseRay(viewMatrix);
}

// Calculate directional vector
glm::vec3 Flashlight::calculateMouseRay(glm::mat4 viewMatrix)
{
	float mouseX, mouseY;
	mouseX = CMouseController::GetInstance()->GetMousePositionX();
	mouseY = CMouseController::GetInstance()->GetMousePositionY();

	float dy = mouseY - cSettings->iWindowHeight * 0.5;
	float dx = mouseX - cSettings->iWindowWidth * 0.5;
	float overallRotate = atan2(dx, dy) + glm::radians(180.f);

	//Lock flashlight to only -90 to 90 degree rotation from current player's direction
	switch (CPlayer2D::GetInstance()->dir)
	{
	//LEFT
	case 0:
		if ((overallRotate > glm::radians(180.f) && overallRotate < glm::radians(270.f)) ||
			(overallRotate < glm::radians(360.f) && overallRotate > glm::radians(270.f)))
		{
			mouseX = cSettings->iWindowWidth * 0.5;
		}

		break;

	//RIGHT
	case 1:
		if ((overallRotate > glm::radians(0.f) && overallRotate < glm::radians(90.f)) ||
			(overallRotate < glm::radians(180.f) && overallRotate > glm::radians(90.f)))
		{
			mouseX = cSettings->iWindowWidth * 0.5;
		}

		break;

	//UP
	case 2:
		if ((overallRotate > glm::radians(90.f) && overallRotate < glm::radians(180.f)) ||
			(overallRotate < glm::radians(270.f) && overallRotate > glm::radians(180.f)))
		{
			mouseY = cSettings->iWindowHeight * 0.5;
		}

		break;

	//DOWN
	case 3:
		if ((overallRotate > glm::radians(270.f) && overallRotate < glm::radians(360.f)) ||
			(overallRotate < glm::radians(90.f) && overallRotate > glm::radians(0.f)))
		{
			mouseY = cSettings->iWindowHeight * 0.5;
		}

		break;
	default:
		break;
	}

	glm::vec3 directional = glm::vec3(mouseX - cSettings->iWindowWidth * 0.5, cSettings->iWindowHeight * 0.5 - mouseY, 0.f);
	glm::vec3 worldRay = glm::normalize(directional);
	//std::cout << worldRay.x << ", " << worldRay.y << ", " << worldRay.z << std::endl;
	return worldRay;
}

glm::vec3 Flashlight::getNormalizedDeviceCoords(float mouseX, float mouseY)
{
	float x = (2.f * mouseX) / cSettings->iWindowWidth - 1.f;
	float y = (2.f * mouseY) / cSettings->iWindowHeight - 1.f;
	return glm::vec3(x, y, 1.f);
}

glm::vec4 Flashlight::toEyeCoords(glm::vec4 clipCoords)
{
	glm::mat4 invertedProjection = glm::inverse(projectionMatrix);
	glm::vec4 eyeCoords = invertedProjection * clipCoords;
	// Note: Can set Z direction of where flashlight is pointing here. For now, set to same Z level as camera (0). If want to point into screen, -1.
	return glm::vec4(eyeCoords.x, eyeCoords.y, 0.f, 0.f);
}

glm::vec3 Flashlight::toWorldCoords(glm::vec4 eyeCoords, glm::mat4 viewMatrix)
{
	glm::mat4 invertedView = glm::inverse(viewMatrix);
	glm::vec4 rayWorld = invertedView * eyeCoords;
	glm::vec3 mouseRay = glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z);
	glm::normalize(mouseRay);
	return mouseRay;
}

bool Flashlight::TestRayOBBIntersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 aabb_min, glm::vec3 aabb_max, glm::mat4 ModelMatrix, float& intersection_distance)
{
	float tMin = 0.f;
	float tMax = 100000.f;

	glm::vec3 OBBposition_worldspace(ModelMatrix[3][0], ModelMatrix[3][1], ModelMatrix[3][2]);
	glm::vec3 delta = OBBposition_worldspace - ray_origin;


	//Test intersection with the 2 perpendicular planes to the OBB's X axis
	{
		glm::vec3 xaxis(ModelMatrix[0][0], ModelMatrix[0][1], ModelMatrix[0][2]);
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray_direction, xaxis);


		if (fabs(f) > 0.001f) //standard case
		{
			float t1 = (e + aabb_min.x) / f; //Intersect with 'left' plane
			float t2 = (e + aabb_max.x) / f; //Intersection with the 'right' plane
			//t1 and t2 now contain dist btw ray origin & ray plane intersections

			//t1 should be nearest intersection,
			//so if it's not the case, swap t1 and t2
			if (t1 > t2)
			{
				float w = t1;
				t1 = t2;
				t2 = w;
			}

			//tMax is the nearest 'far' intersection (amongst X/Y/Z planes pairs)
			if (t2 < tMax)
				tMax = t2;
			//tMin is the farthest 'near' intersection (amongst X/Y/Z planes pairs)
			if (t1 > tMin)
				tMin = t1;

			//If 'far' is closer than 'near', then NO intersection
			if (tMax < tMin)
				return false;
		}
		else { //Rare case: ray is almost parallel to the planes, so don't have any intersections
			if (-e + aabb_min.x > 0.0f || -e + aabb_max.x < 0.0f)
				return false;
		}
	}

	//Repeat the above method for Y axes
	{
		glm::vec3 yaxis(ModelMatrix[1][0], ModelMatrix[1][1], ModelMatrix[1][2]);
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray_direction, yaxis);

		if (fabs(f) > 0.001f) //standard case
		{
			float t1 = (e + aabb_min.y) / f;
			float t2 = (e + aabb_max.y) / f;

			if (t1 > t2)
			{
				float w = t1;
				t1 = t2;
				t2 = w;
			}

			if (t2 < tMax)
				tMax = t2;

			if (t1 > tMin)
				tMin = t1;

			if (tMax < tMin)
				return false;
		}
		else { //Rare case: ray is almost parallel to the planes, so don't have any intersections
			if (-e + aabb_min.y > 0.0f || -e + aabb_max.y < 0.0f)
				return false;
		}
	}

	//{
	//	glm::vec3 zaxis(ModelMatrix[2][0], ModelMatrix[2][1], ModelMatrix[2][2]);
	//	float e = glm::dot(zaxis, delta);
	//	float f = glm::dot(ray_direction, zaxis);

	//	if (fabs(f) > 0.001f) //standard case
	//	{
	//		float t1 = (e + aabb_min.z) / f;
	//		float t2 = (e + aabb_max.z) / f;

	//		if (t1 > t2)
	//		{
	//			float w = t1;
	//			t1 = t2;
	//			t2 = w;
	//		}

	//		if (t2 < tMax)
	//			tMax = t2;

	//		if (t1 > tMin)
	//			tMin = t1;

	//		if (tMax < tMin)
	//			return false;
	//	}
	//	else { //Rare case: ray is almost parallel to the planes, so don't have any intersections
	//		if (-e + aabb_min.z > 0.0f || -e + aabb_max.z < 0.0f)
	//			return false;
	//	}
	//}
	intersection_distance = tMin;
	return true;
}
