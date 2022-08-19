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

#include "IntroState.h"

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
#include "System\filesystem.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CIntroState::CIntroState(void)
	: background(NULL)
	, cSoundController(NULL)
	, cSettings(NULL)
{

}

/**
 @brief Destructor
 */
CIntroState::~CIntroState(void)
{
	if (cSoundController)
		cSoundController = NULL;

	if (cSettings)
		cSettings = NULL;
}

/**
 @brief Init this class instance
 */
bool CIntroState::Init(void)
{
	cout << "CIntroState::Init()\n" << endl;

	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");
	CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	//Create Background Entity
	background = new CBackgroundEntity("Image/Background.png");
	background->SetShader("Shader2D");
	background->Init();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(CSettings::GetInstance()->pWindow, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load the images for buttons
	CImageLoader* il = CImageLoader::GetInstance();
	logoData.fileName = "Image\\GUI\\Logo.png";
	logoData.textureID = il->LoadTextureGetID(logoData.fileName.c_str(), false);

	// Load sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	// Menu
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\MenuBGM.wav"), 1, true, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\ui_button_confirm.wav"), 2, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\LoseBGM.wav"), 3, true, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\WinBGM.wav"), 4, true, true);

	cSoundController->PlaySoundByID(1);

	cSettings = CSettings::GetInstance();

	return true;
}

/**
 @brief Update this class instance
 */
bool CIntroState::Update(const double dElapsedTime)
{
	// Calculate the relative scale to our default windows width
	const float relativeScale_x = cSettings->iWindowWidth / 1366.0f;
	const float relativeScale_y = cSettings->iWindowHeight / 768.0f;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoNav;

	ImGui::Begin("Logo", NULL, window_flags);
	ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - 750, CSettings::GetInstance()->iWindowHeight / 5.0));				// Set the top-left of the window at (10,10)
	ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
	ImGui::Image((ImTextureID)logoData.textureID, ImVec2(1500, 400), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
	ImGui::End();

	ImGuiWindowFlags helperwindow_flags = 0;
	helperwindow_flags |= ImGuiWindowFlags_NoTitleBar;
	helperwindow_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	helperwindow_flags |= ImGuiWindowFlags_NoBackground;
	helperwindow_flags |= ImGuiWindowFlags_NoMove;
	helperwindow_flags |= ImGuiWindowFlags_NoCollapse;
	helperwindow_flags |= ImGuiWindowFlags_NoNav;
	// Another window
	ImGui::Begin("Helper Text", NULL, helperwindow_flags);
	ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 4.0, CSettings::GetInstance()->iWindowHeight * 0.6));
	ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
	ImGui::SetWindowFontScale(5.f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "Press [SPACE] to begin.");
	ImGui::End();

	//cout << "CIntroState::Update()\n" << endl;
	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_SPACE))
	{
		cSoundController->PlaySoundByID(2);
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading MenuState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("MenuState");
		return true;
	}

	return true;
}

/**
 @brief Render this class instance
 */
void CIntroState::Render()
{
	// Clear the screen and buffer
	glClearColor(0.282, 0.239, 0.545, 1.00f);

	//Draw the background
 	background->Render();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief Destroy this class instance
 */
void CIntroState::Destroy(void)
{
	// Delete the background
	if (background)
	{
		delete background;
		background = NULL;
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	cout << "CIntroState::Destroy()\n" << endl;
}