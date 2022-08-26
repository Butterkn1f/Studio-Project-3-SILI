// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

#include "JumpscareState.h"

// Include CGameStateManager
#include "GameStateManager.h"

// Include Mesh Builder
#include "Primitives/MeshBuilder.h"
// Include ImageLoader
#include "System\ImageLoader.h"
// Include Shader Manager
#include "RenderControl\ShaderManager.h"

 // Include shader
#include "RenderControl\shader.h"

// Include CSettings
#include "GameControl/Settings.h"

// Include CKeyboardController
#include "Inputs/KeyboardController.h"

#include "../SoundController/SoundController.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
JumpscareState::JumpscareState(void)
	: cSoundController(NULL)
	//: background(NULL)
{

}

/**
 @brief Destructor
 */
JumpscareState::~JumpscareState(void)
{
	if (cSoundController)
		cSoundController = NULL;
}

/**
 @brief Init this class instance
 */
bool JumpscareState::Init(void)
{
	cout << "JumpscareState::Init()\n" << endl;

	CShaderManager::GetInstance()->Use("Shader2D");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	cSoundController = CSoundController::GetInstance();

	// Load the images for buttons
	CImageLoader* il = CImageLoader::GetInstance();
	ContinuteButtonData.fileName = "Image\\GUI\\ContinueButton.png";
	ContinuteButtonData.textureID = il->LoadTextureGetID(ContinuteButtonData.fileName.c_str(), false);
	OptionsButtonData.fileName = "Image\\GUI\\OptionsButton.png";
	OptionsButtonData.textureID = il->LoadTextureGetID(OptionsButtonData.fileName.c_str(), false);
	MenuButtonData.fileName = "Image\\GUI\\MenuButton.png";
	MenuButtonData.textureID = il->LoadTextureGetID(MenuButtonData.fileName.c_str(), false);

	return true;
}

/**
 @brief Update this class instance
 */
bool JumpscareState::Update(const double dElapsedTime)
{
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoNav;

	float buttonWidth = 256;
	float buttonHeight = 128;

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		// Create a window called "Hello, world!" and append into it.
		ImGui::Begin("Main Menu", NULL, window_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth/2.0 - buttonWidth, 
			CSettings::GetInstance()->iWindowHeight/3.0));				// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));

		//Added rounding for nicer effect
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 200.0f;

		// Display the FPS
		ImGui::TextColored(ImVec4(1, 1, 1, 1), "In-Game Menu");

		ImGui::End();
	}

	//For keyboard controls
	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE) && CGameStateManager::GetInstance()->OptionsGameStateClosed())
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "UnLoading PauseState" << endl;
		CGameStateManager::GetInstance()->SetPauseGameState(nullptr);
		return true;
	}

	return true;
}

/**
 @brief Render this class instance
 */
void JumpscareState::Render(void)
{
	// Clear the screen and buffer
	glClearColor(0.282, 0.239, 0.545, 1.00f);

	//cout << "JumpscareState::Render()\n" << endl;
}

/**
 @brief Destroy this class instance
 */
void JumpscareState::Destroy(void)
{
	// cout << "JumpscareState::Destroy()\n" << endl;
}