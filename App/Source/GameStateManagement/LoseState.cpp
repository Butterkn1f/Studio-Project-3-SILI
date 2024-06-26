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

#include "LoseState.h"

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
CLoseState::CLoseState(void)
	: background(NULL)
	, cSoundController(NULL)
	, cSettings(NULL)
{

}

/**
 @brief Destructor
 */
CLoseState::~CLoseState(void)
{
	if (cSoundController)
		cSoundController = NULL;

	if (cSettings)
		cSettings = NULL;
}

/**
 @brief Init this class instance
 */
bool CLoseState::Init(void)
{
	cout << "CLoseState::Init()\n" << endl;

	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");
	CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	//Create Background Entity
	background = new CBackgroundEntity("Image/SceneLose.png");
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
	loseData.fileName = "Image\\GUI\\LostMenu.png";
	loseData.textureID = il->LoadTextureGetID(loseData.fileName.c_str(), false);

	// Load sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	cSoundController->StopSoundByID(5);
	cSoundController->StopSoundByID(27);
	cSoundController->PlaySoundByID(3);
	CGameStateManager::GetInstance()->OffJumpscareState();

	cSettings = CSettings::GetInstance();

	return true;
}

/**
 @brief Update this class instance
 */
bool CLoseState::Update(const double dElapsedTime)
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

	//ImGui::Begin("Logo", NULL, window_flags);
	//ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - 750, CSettings::GetInstance()->iWindowHeight / 5.0));				// Set the top-left of the window at (10,10)
	//ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
	//ImGui::Image((ImTextureID)loseData.textureID, ImVec2(1500, 400), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
	//ImGui::End();

	//cout << "CLoseState::Update()\n" << endl;
	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_SPACE))
	{
		cSoundController->PlaySoundByID(5);
		cSoundController->StopSoundByID(3);
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading PlayGameState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		return true;
	}

	return true;
}

/**
 @brief Render this class instance
 */
void CLoseState::Render()
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
void CLoseState::Destroy(void)
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

	cout << "CLoseState::Destroy()\n" << endl;
}