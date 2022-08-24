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

#include "MenuState.h"

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
CMenuState::CMenuState(void)
	: background(NULL)
	, cSoundController(NULL)
	, cSettings(NULL)
{

}

/**
 @brief Destructor
 */
CMenuState::~CMenuState(void)
{
	if (cSoundController)
	{
		cSoundController = NULL;
	}

	if (cSettings)
	{
		cSettings = NULL;
	}

}

/**
 @brief Init this class instance
 */
bool CMenuState::Init(void)
{
	cout << "CMenuState::Init()\n" << endl;

	CShaderManager::GetInstance()->Use("Shader2D");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	//Create Background Entity
	background = new CBackgroundEntity("Image/MainMenu.png");
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
	startButtonData.fileName = "Image\\GUI\\ButtonPlay.png";
	startButtonData.textureID = il->LoadTextureGetID(startButtonData.fileName.c_str(), false);
	
	optionsButtonData.fileName = "Image\\GUI\\ButtonOptions.png";
	optionsButtonData.textureID = il->LoadTextureGetID(optionsButtonData.fileName.c_str(), false);

	exitButtonData.fileName = "Image\\GUI\\ButtonExit.png";
	exitButtonData.textureID = il->LoadTextureGetID(exitButtonData.fileName.c_str(), false);

	logoData.fileName = "Image\\GUI\\MenuLogo.png";
	logoData.textureID = il->LoadTextureGetID(logoData.fileName.c_str(), false);

	// Enable the cursor
	if (CSettings::GetInstance()->bDisableMousePointer == true)
		glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Get handler for sound controller
	cSoundController = CSoundController::GetInstance();

	cSettings = CSettings::GetInstance();

	return true;
}

/**
 @brief Update this class instance
 */
bool CMenuState::Update(const double dElapsedTime)
{
	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_SPACE))
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading PlayGamesState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		return true;
	}
	else if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE))
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Quitting the game from MenuState" << endl;
		return false;
	}

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
	window_flags |= ImGuiWindowFlags_NoResize;

	float buttonWidth = 256;
	float buttonHeight = 128;

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGuiWindowFlags logowindow_flags = 0;
		logowindow_flags |= ImGuiWindowFlags_NoTitleBar;
		logowindow_flags |= ImGuiWindowFlags_NoScrollbar;
		logowindow_flags |= ImGuiWindowFlags_NoBackground;
		logowindow_flags |= ImGuiWindowFlags_NoMove;
		logowindow_flags |= ImGuiWindowFlags_NoCollapse;
		logowindow_flags |= ImGuiWindowFlags_NoNav;

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.4f));
	

		ImGui::Begin("Logo", NULL, logowindow_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - buttonWidth * 3, 10));				// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));
		ImGui::Image((ImTextureID)logoData.textureID, ImVec2(buttonWidth * 6, buttonHeight * 3), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
		ImGui::End();



		// Create a window called "Hello, world!" and append into it.
		ImGui::Begin("Main Menu", NULL, window_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth/2.0 - buttonWidth, 
			CSettings::GetInstance()->iWindowHeight/2.0 - 20));				// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));

		//Added rounding for nicer effect
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 200.0f;
		
		// Add codes for Start button here
		if (ImGui::ImageButton((ImTextureID)startButtonData.textureID, 
			ImVec2(buttonWidth * 2, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			cSoundController->PlaySoundByID(2);
			cSoundController->StopSoundByID(1);
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			// Load the menu state
			cout << "Loading PlayGameState" << endl;
			CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		}
		if (ImGui::ImageButton((ImTextureID)optionsButtonData.textureID,
			ImVec2(buttonWidth * 2, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			cSoundController->PlaySoundByID(2);
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			// Load the menu state
			CGameStateManager::GetInstance()->SetOptionsGameState("OptionsState");
		}

		// Add codes for Exit button here
		if (ImGui::ImageButton((ImTextureID)exitButtonData.textureID,
			ImVec2(buttonWidth * 2, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			cSoundController->PlaySoundByID(2);
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			// Load the menu state
			cout << "Quitting the game from MenuState" << endl;

			return false;
		}
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	//For keyboard controls
	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_SPACE))
	{
		cSoundController->StopSoundByID(1);
		cSoundController->PlaySoundByID(2);
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading PlayGameState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		return true;
	}
	else if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE))
	{
		cSoundController->PlaySoundByID(2);
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Quitting the game from MenuState" << endl;
		return false;
	}

	return true;
}

/**
 @brief Render this class instance
 */
void CMenuState::Render(void)
{
	// Clear the screen and buffer
	glClearColor(0.0f, 0.55f, 1.00f, 1.00f);

	//Render Background
	background->Render();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//cout << "CMenuState::Render()\n" << endl;
}

/**
 @brief Destroy this class instance
 */
void CMenuState::Destroy(void)
{
	// Disable the cursor
	if (CSettings::GetInstance()->bDisableMousePointer == true)
		glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		// Hide the cursor
		if (CSettings::GetInstance()->bShowMousePointer == false)
			glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

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

	cout << "CMenuState::Destroy()\n" << endl;
}
