// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLFW
#include <GLFW/glfw3.h>

#include "PlayGameState.h"

// Include CGameStateManager
#include "GameStateManager.h"

// Include CKeyboardController
#include "Inputs/KeyboardController.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CPlayGameState::CPlayGameState(void)
	: cScene2D(NULL)
{

}

/**
 @brief Destructor
 */
CPlayGameState::~CPlayGameState(void)
{

}

/**
 @brief Init this class instance
 */
bool CPlayGameState::Init(void)
{
	cout << "CPlayGameState::Init()\n" << endl;

	// Initialise the cScene2D instance
	cScene2D = CScene2D::GetInstance();
	if (cScene2D->Init() == false)
	{
		cout << "Failed to load Scene2D" << endl;
		return false;
	} 

	screentimer = 20;
	return true;
}

/**
 @brief Update this class instance
 */
bool CPlayGameState::Update(const double dElapsedTime)
{

	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE) && CGameStateManager::GetInstance()->OptionsGameStateClosed())
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading PauseState" << endl;
		CGameStateManager::GetInstance()->SetPauseGameState("PauseState");
	}

	if (CGameManager::GetInstance()->bPLayerJumpscared == true && CGameManager::GetInstance()->bPlayerLost == false)
	{
		screentimer--;
		cout << "screentimer:" << screentimer << endl;
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading Jumpscarestate" << endl;
		CGameStateManager::GetInstance()->SetJumpscareState("JumpscareState");
		if (screentimer <= 0)
		{
			CGameStateManager::GetInstance()->OffJumpscareState();
			CGameManager::GetInstance()->bPLayerJumpscared = false;
			CSoundController::GetInstance()->StopSoundByID(27);
			screentimer = 20;
		}
	}

	// Call the cScene2D's Update method
	cScene2D->Update(dElapsedTime);

	return true;
}

/**
 @brief Render this class instance
 */
void CPlayGameState::Render(void)
{
	//cout << "CPlayGameState::Render()\n" << endl;

	// Call the cScene2D's Pre-Render method
	cScene2D->PreRender();

	// Call the cScene2D's Render method
	cScene2D->Render();

	// Call the cScene2D's PostRender method
	cScene2D->PostRender();
}

/**
 @brief Destroy this class instance
 */
void CPlayGameState::Destroy(void)
{
	cout << "CPlayGameState::Destroy()\n" << endl;

	// Destroy the cScene2D instance
	if (cScene2D)
	{
		cScene2D->Destroy();
		cScene2D = NULL;
	}
}