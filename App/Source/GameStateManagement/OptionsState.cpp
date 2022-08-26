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

#include "OptionsState.h"

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
COptionsState::COptionsState(void)
	: cSoundController(NULL)
	//: background(NULL)
{

}

/**
 @brief Destructor
 */
COptionsState::~COptionsState(void)
{
	if (cSoundController)
		cSoundController = NULL;
}

/**
 @brief Init this class instance
 */
bool COptionsState::Init(void)
{
	cout << "COptionsState::Init()\n" << endl;

	CShaderManager::GetInstance()->Use("Shader2D");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	cSoundController = CSoundController::GetInstance();

	// Load the images for buttons
	CImageLoader* il = CImageLoader::GetInstance();
	OptionsButtonData.fileName = "Image\\GUI\\OptionsButton.png";
	OptionsButtonData.textureID = il->LoadTextureGetID(OptionsButtonData.fileName.c_str(), false);
	SaveButtonData.fileName = "Image\\GUI\\SaveButton.png";
	SaveButtonData.textureID = il->LoadTextureGetID(SaveButtonData.fileName.c_str(), false);
	return true;
}

/**
 @brief Update this class instance
 */
bool COptionsState::Update(const double dElapsedTime)
{
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoNav;
	window_flags |= ImGuiWindowFlags_NoResize;

	float buttonWidth = 256;
	float buttonHeight = 128;

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		// Create a window called "Hello, world!" and append into it.
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.282, 0.209, 0.445, 0.9f)); // Set window background to blue
		ImGui::Begin("Options Menu", NULL, window_flags);
		ImGui::SetWindowPos(ImVec2(300, 150));				// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth - 600, CSettings::GetInstance()->iWindowHeight - 300));

		//Added rounding for nicer effect
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 200.0f;

		// Options
		ImGui::Image((ImTextureID)OptionsButtonData.textureID,
			ImVec2(buttonWidth , buttonHeight * 0.5), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));

		static float masterVolume = cSoundController->getCurrentVolume(0);
		ImGui::SliderFloat("Master Volume", &masterVolume, 0, 1);

		static float bgmVolume = cSoundController->getCurrentVolume(1);
		ImGui::SliderFloat("Background Music", &bgmVolume, 0, 1);

		static float sfxVolume = cSoundController->getCurrentVolume(2);
		ImGui::SliderFloat("SFX", &sfxVolume, 0, 1);

		cSoundController->SetMasterVolume(masterVolume);
		cSoundController->SetBGMVolume(bgmVolume);
		cSoundController->SetSFXVolume(sfxVolume);

	ImGui::PopStyleColor();

	ImGuiWindowFlags savewindow_flags = 0;
	savewindow_flags |= ImGuiWindowFlags_NoTitleBar;
	savewindow_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	savewindow_flags |= ImGuiWindowFlags_NoMove;
	savewindow_flags |= ImGuiWindowFlags_NoCollapse;
	savewindow_flags |= ImGuiWindowFlags_NoNav;
	savewindow_flags |= ImGuiWindowFlags_NoResize;
	savewindow_flags |= ImGuiWindowFlags_NoBackground;

	// Another window
	ImGui::BeginChild("Options Menu", ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight), true, savewindow_flags);
	ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - buttonWidth / 2.0,
	CSettings::GetInstance()->iWindowHeight / 2.0 + 50));				// Set the top-left of the window at (10,10)

	if (ImGui::ImageButton((ImTextureID)SaveButtonData.textureID,
		ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
	{
		cSoundController->PlaySoundByID(2);
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		//Restart BGMs
		if (cSoundController->isPlaying(1))
		{
			cSoundController->StopSoundByID(1);
			cSoundController->PlaySoundByID(1);
		}
		if (cSoundController->isPlaying(3))
		{
			cSoundController->StopSoundByID(3);
			cSoundController->PlaySoundByID(3);
		}
		if (cSoundController->isPlaying(4))
		{
			cSoundController->StopSoundByID(4);
			cSoundController->PlaySoundByID(4);
		}
		if (cSoundController->isPlaying(5))
		{
			cSoundController->StopSoundByID(5);
			cSoundController->PlaySoundByID(5);
		}
		// Load the menu state
		CGameStateManager::GetInstance()->OffOptionsGameState();
	}
	ImGui::EndChild();

	ImGui::End();
	}

	//For keyboard controls
	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE) && CGameStateManager::GetInstance()->OptionsGameStateClosed())
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "UnLoading OptionsState" << endl;
		CGameStateManager::GetInstance()->SetOptionsGameState(nullptr);
		return true;
	}

	return true;
}

/**
 @brief Render this class instance
 */
void COptionsState::Render(void)
{
	// Clear the screen and buffer
	glClearColor(0.282, 0.239, 0.545, 1.00f);

	//cout << "COptionsState::Render()\n" << endl;
}

/**
 @brief Destroy this class instance
 */
void COptionsState::Destroy(void)
{
	// cout << "COptionsState::Destroy()\n" << endl;
}
