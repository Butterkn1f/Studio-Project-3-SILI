#include "Camera.h"

// Include CSettings which stores information about the App
#include "GameControl\Settings.h"


Camera::Camera()
{
	Reset();
}

Camera::~Camera()
{
}

void Camera::Init(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up)
{
	MV = glm::mat4(1.0f);
	this->position = pos;
	this->target = target;
	this->up = up;
}

glm::mat4 Camera::GetMVP()
{
	glm::mat4 projection;
	projection = glm::perspective(
		glm::radians(45.f),
		(float)(CSettings::GetInstance()->iWindowWidth / CSettings::GetInstance()->iWindowHeight),
		0.f,
		1.f);
	//projection = glm::mat4(
	//	2 / (float)(cSettings->iWindowWidth - 0), 0, 0, 0,
	//	0, 2 / (float)(cSettings->iWindowHeight - 0), 0, 0,
	//	0, 0, -2 / (float)(10 - -10), 0,
	//	-(float)((cSettings->iWindowWidth + 0) / (cSettings->iWindowWidth - 0)), -(float)((cSettings->iWindowHeight + 0) / (cSettings->iWindowHeight - 0)), -(float)((10 + -10) / (10 - -10)), 1
	//);
	//projection = glm::ortho(0, (int)cSettings->iWindowWidth, 0, (int)cSettings->iWindowHeight, -10, 10);

	// Load identity
	projectionStack.push_back(glm::mat4(1.0f));
	projectionStack.back() = projection;

	//Camera matrix
	glm::mat4 viewMatrix;
	viewMatrix = glm::lookAt(
		glm::vec3(position.x, position.y, position.z),
		glm::vec3(target.x, target.y, target.z),
		glm::vec3(up.x, up.y, up.z)
	);
	// Load identity
	viewStack.push_back(glm::mat4(1.0f));
	viewStack.back() = viewStack.back() * viewMatrix;

	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.push_back(glm::mat4(1.0f));

	//unsigned int MVPLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "MVP");

	MV = glm::mat4(1.0f); //initialize matrix to identity matrix first
	MV = viewStack.back() * modelStack.back();

	MVP = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	MVP = projectionStack.back() * viewStack.back() * modelStack.back();

	//MVP[0][0] = 1;
	//MVP[1][1] = 1;
	//MVP[2][2] = 1;

	//std::cout << "[" << viewMatrix[0][0] << ", " << viewMatrix[0][1] << ", " << viewMatrix[0][2] << std::endl;
	//std::cout << viewMatrix[1][0] << ", " << viewMatrix[1][1] << ", " << viewMatrix[1][2] << std::endl;
	//std::cout << viewMatrix[2][0] << ", " << viewMatrix[2][1] << ", " << viewMatrix[2][2] << "]" << std::endl;
	//std::cout << std::endl;
	//glUniformMatrix4fv(MVPLoc, 1, GL_FALSE, glm::value_ptr(MVP));
	return MVP;
}

glm::mat4 Camera::GetMV()
{
	return MV;
}

std::vector<glm::mat4> Camera::GetViewStack()
{
	return viewStack;
}

void Camera::Reset()
{
	position = glm::vec3(1, 0, 0);
	target = glm::vec3(0, 0, 0);
	up = glm::vec3(0, 1, 0);
}

void Camera::Update(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up)
{
	this->position = pos;
	this->target = target;
	this->up = up;
}