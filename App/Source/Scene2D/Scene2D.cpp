/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Scene2D.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"
#include "../GameStateManagement/GameStateManager.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CScene2D::CScene2D(void)
	: cMap2D(NULL)
	, cPlayer2D(NULL)
	, cGUI_Scene2D(NULL)
	, cKeyboardController(NULL)
	, cGameManager(NULL)
	, cSoundController(NULL)
	, camera(NULL)
{
}

/**
 @brief Destructor
 */
CScene2D::~CScene2D(void)
{
	if (cKeyboardController)
	{
		// We won't delete this since it was created elsewhere
		cKeyboardController = NULL;
	}

	if (cMap2D)
	{
		cMap2D->Destroy();
		cMap2D = NULL;
	}

	if (cPlayer2D)
	{
		cPlayer2D->Destroy();
		cPlayer2D = NULL;
	}

	if (cGUI_Scene2D)
	{
		cGUI_Scene2D->Destroy();
		cGUI_Scene2D = NULL;
	}

	if (cGameManager)
	{
		cGameManager->Destroy();
		cGameManager = NULL;
	}

	if (cSoundController)
	{
		//Won't delete this since it was created elsewhere
		cSoundController = NULL;
	}

	//Destroy the enemies
	for (int i = 0; i < enemyVector.size(); i++)
	{
		delete enemyVector[i];
		enemyVector[i] = NULL;
	}
	enemyVector.clear();

	if (camera)
	{
		camera = NULL;
	}
}

/**
@brief Init Initialise this instance
*/ 
bool CScene2D::Init(void)
{
	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");

	// Get camera
	camera = Camera::GetInstance();
	camera->Init(glm::vec3(-1, 0, 1), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0));

	// Get uniform location of lighting from shader
	m_programID = CShaderManager::GetInstance()->activeShader->ID;

	//LIGHT 1
	//m_parameters[U_LIGHT0_POSITION] =
	//	glGetUniformLocation(m_programID,
	//		"lights[0].position_cameraspace");
	//m_parameters[U_LIGHT0_COLOR] =
	//	glGetUniformLocation(m_programID, "lights[0].color");
	//m_parameters[U_LIGHT0_POWER] =
	//	glGetUniformLocation(m_programID, "lights[0].power");
	//m_parameters[U_LIGHT0_KC] = glGetUniformLocation(m_programID,
	//	"lights[0].kC");
	//m_parameters[U_LIGHT0_KL] = glGetUniformLocation(m_programID,
	//	"lights[0].kL");
	//m_parameters[U_LIGHT0_KQ] = glGetUniformLocation(m_programID,
	//	"lights[0].kQ");
	//m_parameters[U_LIGHTENABLED] =
	//	glGetUniformLocation(m_programID, "lightEnabled");
	//m_parameters[U_LIGHT0_TYPE] =
	//	glGetUniformLocation(m_programID, "lights[0].type");
	//m_parameters[U_LIGHT0_SPOTDIRECTION] =
	//	glGetUniformLocation(m_programID, "lights[0].spotDirection");
	//m_parameters[U_LIGHT0_COSCUTOFF] =
	//	glGetUniformLocation(m_programID, "lights[0].cosCutoff");
	//m_parameters[U_LIGHT0_COSINNER] =
	//	glGetUniformLocation(m_programID, "lights[0].cosInner");
	//m_parameters[U_LIGHT0_EXPONENT] =
	//	glGetUniformLocation(m_programID, "lights[0].exponent");

	//// Set num of lights in shader to 1
	//glUniform1i(m_parameters[U_NUMLIGHTS], 1);
	//// Enable light, to turn off, set to 0
	//glUniform1i(m_parameters[U_LIGHTENABLED], 0);

	//light[0].type = Light::LIGHT_SPOT;
	//light[0].position = glm::vec3(0, 0, 0);
	//light[0].color = glm::vec4(1, 1, 1, 1);
	//light[0].power = 2;
	//light[0].kC = 0.1f;
	//light[0].kL = 0.00001f;
	//light[0].kQ = 0.000001f;
	//light[0].cosCutoff = cos(glm::radians(30.f));
	//light[0].cosInner = cos(glm::radians(25.f));
	//light[0].exponent = 3.f;
	//light[0].spotDirection = glm::vec3(0.f, 1.f, 0.f);
	//// Pass uniform parameters
	//glUniform1i(m_parameters[U_LIGHT0_TYPE], light[0].type);
	//glUniform3fv(m_parameters[U_LIGHT0_COLOR], 1, &light[0].color.r);
	//glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
	//glUniform1f(m_parameters[U_LIGHT0_KC], light[0].kC);
	//glUniform1f(m_parameters[U_LIGHT0_KL], light[0].kL);
	//glUniform1f(m_parameters[U_LIGHT0_KQ], light[0].kQ);
	//glUniform1f(m_parameters[U_LIGHT0_COSCUTOFF], light[0].cosCutoff);
	//glUniform1f(m_parameters[U_LIGHT0_COSINNER], light[0].cosInner);
	//glUniform1f(m_parameters[U_LIGHT0_EXPONENT], light[0].exponent);

	// Create and initialise the Map 2D
	cMap2D = CMap2D::GetInstance();
	// Set a shader to this class
	cMap2D->SetShader("Shader2D");
	// Initialise the instance
	if (cMap2D->Init(2, CSettings::GetInstance()->NUM_TILES_YAXIS,
						CSettings::GetInstance()->NUM_TILES_XAXIS) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}

	// Load the map into an array
	if (cMap2D->LoadMap("Maps/Maze_Level_01.csv") == false)
	{
		// The loading of a map has failed. Return false
		return false;
	}

	if (cMap2D->LoadMap("Maps/DM2213_Map_Level_02.csv", 1) == false)
	{
		// The loading of a map has failed. Return false
		return false;
	}

	cMap2D->SetCurrentLevel(0);

	//Load Scene2DColour into ShaderManager
	CShaderManager::GetInstance()->Use("Shader2D_Colour");
	//Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();
	//Pass shader to cPlayer2D
	cPlayer2D->SetShader("Shader2D_Colour");
	//Initialise the instance
	if (cPlayer2D->Init() == false)
	{
		cout << "Failed to load CPlayer2D" << endl;
		return false;
	}

	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();

	// Store the cGUI_Scene2D singleton instance here
	cGUI_Scene2D = CGUI_Scene2D::GetInstance();
	cGUI_Scene2D->Init();

	// Game Manager
	cGameManager = CGameManager::GetInstance();
	cGameManager->Init();

	enemyVector.clear();
	//for sawcon 
	while (true)
	{
		CEnemySawCon* cEnemySawCon = new CEnemySawCon();
		//Pass shader to cEnemyWarrior
		cEnemySawCon->SetShader("Shader2D_Colour");
		//Initalise the instance
		if (cEnemySawCon->Init() == true)
		{
			cEnemySawCon->SetPlayer2D(cPlayer2D);
			enemyVector.push_back(cEnemySawCon);
		}
		else
		{
			//Break out of this loop if the enemy has all been loaded
			break;
		}
	}
	while (true)
	{
		CCrate* cCrate = new CCrate();
		//Pass shader to cEnemyWarrior
		cCrate->SetShader("Shader2D_Colour");
		//Initalise the instance
		if (cCrate->Init() == true)
		{
			cCrate->SetPlayer2D(cPlayer2D);
			enemyVector.push_back(cCrate);
		}
		else
		{
			//Break out of this loop if the enemy has all been loaded
			break;
		}
	}

	// Load sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	//Game BGM
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\GameBGM.wav"), 5, true, true);

	cSoundController->LoadSound(FileSystem::getPath("Sounds\\CreepyLaugh.wav"), 26, true, false, CSoundInfo::SOUNDTYPE::_3D, vec3df(0, 0, 0));

	//Set BGM and SFX volume to be same as menu state's, since they haven't been set since they haven't been loaded yet
	cSoundController->SetBGMVolume(cSoundController->getCurrentVolume(1));
	cSoundController->SetSFXVolume(cSoundController->getCurrentVolume(2));

	cSoundController->StopSoundByID(1);
	cSoundController->PlaySoundByID(5);

	return true;
}

/**
@brief Update Update this instance
*/
bool CScene2D::Update(const double dElapsedTime)
{
	// Call the cPlayer2D's update method before Map2D
	// as we want to capture the inputs before map2D update
	cPlayer2D->Update(dElapsedTime);

	//Call all of the cEnemyCrawlid's update method before Map2D as we want to capture the updates before map2D update
	for (int i = 0; i < enemyVector.size(); i++)
	{
		enemyVector[i]->Update(dElapsedTime);
	}

	// Call the Map2D's update method
	cMap2D->Update(dElapsedTime);

	if (cKeyboardController->IsKeyDown(GLFW_KEY_F6))
	{
		//Save the current game to a save file
		try {
			if (cMap2D->SaveMap("Maps/DM2213_Map_Level_01_SAVEGAMEtest.csv") == false)
			{
				throw runtime_error("Unable to save the current game to a file");
			}
		}
		catch (runtime_error e)
		{
			cout << "Runtime error: " << e.what();
			return true;
		}
	}

	// Call the cGUI_Scene2D's update method
	cGUI_Scene2D->Update(dElapsedTime);

	// Move light with player
	light[0].position = glm::vec3(cPlayer2D->vec2Index.x, cPlayer2D->vec2Index.y, 0.0f);

	// Check if the game should go to the next level
	if (cGameManager->bLevelCompleted == true)
	{
		cMap2D->SetCurrentLevel(cMap2D->GetCurrentLevel() + 1);
		//Reset enemies
		//Destroy the enemies
		for (int i = 0; i < enemyVector.size(); i++)
		{
			delete enemyVector[i];
			enemyVector[i] = NULL;
		}
		enemyVector.clear();


		while (true)
		{
			CEnemySawCon* cEnemySawCon = new CEnemySawCon();
			//Pass shader to cEnemyWarrior
			cEnemySawCon->SetShader("Shader2D_Colour");
			//Initalise the instance
			if (cEnemySawCon->Init() == true)
			{
				cEnemySawCon->SetPlayer2D(cPlayer2D);
				enemyVector.push_back(cEnemySawCon);
			}
			else
			{
				//Break out of this loop if the enemy has all been loaded
				break;
			}
		}
	}

	// Check if the game has been won by the player
	if (cGameManager->bPlayerWon == true)
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading WinState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("WinState");
	}
	else if (cGameManager->bPlayerLost == true)
	{
		cMap2D->SetCurrentLevel(cMap2D->GetCurrentLevel());
		cPlayer2D->Reset();

		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading LoseState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("LoseState");
	}
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CScene2D::PreRender(void)
{
	// Reset the OpenGL rendering environment
	glLoadIdentity();

	// Clear the screen and buffer
	glClearColor(0.282, 0.239, 0.345, 1.00f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable 2D texture rendering
	glEnable(GL_TEXTURE_2D);
}

/**
 @brief Render Render this instance
 */
void CScene2D::Render(void)
{

	// Hacky, but it works `\("/)/`
	// Would be better to target camera to middle of player, but due to time contraints decided to go to roundabout way of reverse engineering. Sorry D:

	// Min. player coords are (2, 4), Max. player coords are (77, 43).
	// After some trial and error, to center player in camera in both Min. and Max.,
	// the translation required is Min. (-0.93, -0.8), Max. (0.93, 0.95).
	// The calculations is working backwards to arrive at the required translation values, given the player coords.
	float xTranslate = (cPlayer2D->vec2Index.x + (cPlayer2D->vec2NumMicroSteps.x / 15) - 2) * 0.0248 - 0.93;
	float yTranslate = (cPlayer2D->vec2Index.y + (cPlayer2D->vec2NumMicroSteps.y / 15) - 4) * 0.045 - 0.8;

	camera->Update(glm::vec3(xTranslate, yTranslate, 0.5f),
		glm::vec3(xTranslate, yTranslate, 0.f),
		glm::vec3(0.f, 1, 0.f)
	);

	/*glm::mat4 viewMatrix;
	viewMatrix = glm::lookAt(
		glm::vec3(xTranslate, yTranslate, 0.5f),
		glm::vec3(xTranslate, yTranslate, 0.f),
		glm::vec3(0.f, 1, 0.f)
	);

	if (light[0].type == Light::LIGHT_SPOT)
	{
		glm::vec3 lightPosition_cameraspace = viewMatrix * glm::vec4(light[0].position.x, light[0].position.y, light[0].position.z, 0);
		glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, &lightPosition_cameraspace.x);
		glm::vec3 spotDirection_cameraspace = viewMatrix * glm::vec4(light[0].spotDirection.x, light[0].spotDirection.y, light[0].spotDirection.z, 0);
		glUniform3fv(m_parameters[U_LIGHT0_SPOTDIRECTION], 1, &spotDirection_cameraspace.x);
		glUniform3fv(m_parameters[U_LIGHT0_COLOR], 1, &light[0].color.r);
		glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
	}*/

	cMap2D->PreRender();
	cMap2D->Render();
	cMap2D->PostRender();

	cPlayer2D->PreRender();
	cPlayer2D->Render();
	cPlayer2D->PostRender();

	for (int i = 0; i < enemyVector.size(); i++)
	{
		// Call the CEnemyCrawlid's preRender()
		enemyVector[i]->PreRender();
		enemyVector[i]->Render();
		enemyVector[i]->PostRender();
	}

	// Call the cGUI_Scene2D's PreRender()
	cGUI_Scene2D->PreRender();
	// Call the cGUI_Scene2D's Render()
	cGUI_Scene2D->Render();
	// Call the cGUI_Scene2D's PostRender()
	cGUI_Scene2D->PostRender();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CScene2D::PostRender(void)
{
}