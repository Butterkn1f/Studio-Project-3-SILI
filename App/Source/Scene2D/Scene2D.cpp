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
	while (true)
	{
		CEnemyCrawlid* cEnemyCrawlid = new CEnemyCrawlid();
		//Pass shader to cEnemyCrawlid
		cEnemyCrawlid->SetShader("Shader2D_Colour");
		//Initalise the instance
		if (cEnemyCrawlid->Init() == true)
		{
			cEnemyCrawlid->SetPlayer2D(cPlayer2D);
			enemyVector.push_back(cEnemyCrawlid);
		}
		else
		{
			//Break out of this loop if the enemy has all been loaded
			break;
		}
	}
	while (true)
	{
		CEnemyWarrior* cEnemyWarrior = new CEnemyWarrior();
		//Pass shader to cEnemyWarrior
		cEnemyWarrior->SetShader("Shader2D_Colour");
		//Initalise the instance
		if (cEnemyWarrior->Init() == true)
		{
			cEnemyWarrior->SetPlayer2D(cPlayer2D);
			enemyVector.push_back(cEnemyWarrior);
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

	//Game
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\CrossroadsMainBGM.wav"), 3, true, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\hero_walk.wav"), 4, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\hero_jump.wav"), 5, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\hero_falling.wav"), 6, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\hero_land.wav"), 7, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\sword.wav"), 8, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\hero_damage.wav"), 9, true);
	//TODO: Figure out how to get player's position from here
	//cSoundController->LoadSound(FileSystem::getPath("Sounds\\hero_death.wav"), 10, true, false, CSoundInfo::_3D);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\hero_death.wav"), 10, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\hero_focus.wav"), 11, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\breakable_wall.wav"), 12, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\geoHit.wav"), 13, true);

	//Enemies
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\enemy_damage.wav"), 14, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\enemy_death.wav"), 15, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\ShieldHit.wav"), 16, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Warrior_Sword.wav"), 17, true);

	//Boss
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FKnight_BGM.wav"), 18, true, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FKnight_ArmourBreak.wav"), 19, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FKnight_ArmourBreakFinal.wav"), 20, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FKnight_Damage.wav"), 21, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FKnight_Jump.wav"), 22, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FKnight_Land.wav"), 23, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FKnight_Smash.wav"), 24, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\FKnight_Death.wav"), 25, true);

	//Set BGM and SFX volume to be same as menu state's, since they haven't been set since they haven't been loaded yet
	cSoundController->SetBGMVolume(cSoundController->getCurrentVolume(1));
	cSoundController->SetSFXVolume(cSoundController->getCurrentVolume(2));

	cSoundController->StopSoundByID(1);
	cSoundController->PlaySoundByID(3);

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
			CEnemyCrawlid* cEnemyCrawlid = new CEnemyCrawlid();
			//Pass shader to cEnemyCrawlid
			cEnemyCrawlid->SetShader("Shader2D_Colour");
			//Initalise the instance
			if (cEnemyCrawlid->Init() == true)
			{
				cEnemyCrawlid->SetPlayer2D(cPlayer2D);
				enemyVector.push_back(cEnemyCrawlid);
			}
			else
			{
				//Break out of this loop if the enemy has all been loaded
				break;
			}
		}
		while (true)
		{
			CEnemyWarrior* cEnemyWarrior = new CEnemyWarrior();
			//Pass shader to cEnemyWarrior
			cEnemyWarrior->SetShader("Shader2D_Colour");
			//Initalise the instance
			if (cEnemyWarrior->Init() == true)
			{
				cEnemyWarrior->SetPlayer2D(cPlayer2D);
				enemyVector.push_back(cEnemyWarrior);
			}
			else
			{
				//Break out of this loop if the enemy has all been loaded
				break;
			}
		}

		CEnemyFalseKnight* cEnemyFalseKnight = new CEnemyFalseKnight();
		//Pass shader to cEnemyFalseKnight
		cEnemyFalseKnight->SetShader("Shader2D_Colour");
		//Initalise the instance
		if (cEnemyFalseKnight->Init() == true)
		{
			cEnemyFalseKnight->SetPlayer2D(cPlayer2D);
			enemyVector.push_back(cEnemyFalseKnight);
		}

		cGameManager->bLevelCompleted = false;
		cPlayer2D->Reset();
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