/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

 //Include Singleton Template
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

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include vector
#include <vector>

// Include Keyboard controller
//#include "Inputs\KeyboardController.h"

//Include the Map2D as we will use it to check the player's movements and actions
//#include "Map2D.h"
////Include CPlayer2D
//#include "Player2D.h"
////Include GUI_Scene2D
//#include "GUI_Scene2D.h"
//// Game Manager
//#include "GameManager.h"

class Camera : public CSingletonTemplate<Camera>
{
	friend CSingletonTemplate<Camera>;
public:
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	void Init(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up);
	glm::mat4 GetMVP();
	glm::mat4 GetMV();
	std::vector<glm::mat4> GetViewStack();
	void Reset();
	void Update(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up);

protected:
	//CMap2D* cMap2D;
	////Handler containing the instance of CPlayer2D
	//CPlayer2D* cPlayer2D;
	//// Keyboard Controller singleton instance
	//CKeyboardController* cKeyboardController;

	std::vector<glm::mat4> modelStack;
	std::vector<glm::mat4> viewStack;
	std::vector<glm::mat4> projectionStack;
	glm::mat4 MVP;
	glm::mat4 MV;

	// Constructor
	Camera(void);
	// Destructor
	virtual ~Camera(void);
};

