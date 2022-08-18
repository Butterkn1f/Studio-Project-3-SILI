/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Player2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
#include "Primitives/MeshBuilder.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::CPlayer2D(void)
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
	, animatedSprites(NULL)
	, runtimeColour(glm::vec4(1.0f))
	, cSoundController(NULL)
	, camera(NULL)
{
	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::~CPlayer2D(void)
{

	if (animatedSprites)
	{
		delete animatedSprites;
		animatedSprites = NULL;
	}

	if (cSoundController)
	{
		cSoundController = NULL;
	}

	// We won't delete this since it was created elsewhere
	cInventoryManager = NULL;

	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	camera = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CPlayer2D::Init(void)
{
	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	// Get handler for camera
	camera = Camera::GetInstance();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	// Load the player texture 
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/scene2d_player.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/scene2d_player.png" << endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation
	//NOTE, TODO: FOR DOUBLE SPRITE HEIGHT JUST DO CSETTINGS->TILE_HEIGHT * 2
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(7, 6, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("right", 0, 2);
	animatedSprites->AddAnimation("left", 3, 5);
	animatedSprites->AddAnimation("atkRight", 6, 9);
	animatedSprites->AddAnimation("atkLeft", 10, 13);
	animatedSprites->AddAnimation("jumpRight", 14, 17);
	animatedSprites->AddAnimation("fallRight", 18, 20);
	animatedSprites->AddAnimation("jumpLeft", 21, 24);
	animatedSprites->AddAnimation("fallLeft", 25, 27);
	animatedSprites->AddAnimation("idleRight", 28, 29);
	animatedSprites->AddAnimation("idleLeft", 30, 31);
	animatedSprites->AddAnimation("death", 32, 37);
	animatedSprites->AddAnimation("focusRight", 38, 39);
	animatedSprites->AddAnimation("focusLeft", 40, 41);
	//CS: Play the "idle" animation as default
	// PlayAnimation(animName, loopCount, every ? seconds). Eg, "down", 5, 1.0f means loop anim 5 times every 1s. -1 loops infinitely/
	animatedSprites->PlayAnimation("idleRight", -1, 0.8f);


	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Create the quad mesh for the player
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Set the physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	iJumpCount = 0;
	lastAttackElapsed = 0;
	iAttackCount = 0;
	iframeElapsed = 1.0;
	deadElapsed = 0;
	focusElapsed = 0;
	dir = DIRECTION::RIGHT;
	
	//Variables
	AllNumbersCollected = false;

	// Get the handler to the CINventoryManager instance
	cInventoryManager = CInventoryManager::GetInstance();

	// Add a Health icon as one of the inventory items
	cInventoryItem = cInventoryManager->Add("Health", "Image/GUI_Health.png", 5, 5);
	cInventoryItem->vec2Size = glm::vec2(40, 40);

	cInventoryItem = cInventoryManager->Add("Soul", "Image/GUI_Soul.png", 100, 0);
	cInventoryItem->vec2Size = glm::vec2(200, 100);

	//Inventory item Papers
	cInventoryItem = cInventoryManager->Add("Paper", "Image/paper.png", 10, 0);
	cInventoryItem->vec2Size = glm::vec2(40, 40);

	// Get handler for sound controller
	cSoundController = CSoundController::GetInstance();

	return true;
}

/**
 @brief Reset this instance
 */
bool CPlayer2D::Reset()
{
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	//Set it to fall upon entering new level
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	//CS: Reset double jump
	iJumpCount = 0;

	//CS: Play the "down" animation as default
	// PlayAnimation(animName, loopCount, every ? seconds). Eg, "down", 5, 1.0f means loop anim 5 times every 1s. -1 loops infinitely/
	animatedSprites->PlayAnimation("idleRight", -1, 0.8f);

	//Reset attack
	iAttackCount = 0;
	lastAttackElapsed = 0;
	iframeElapsed = 1.0;
	deadElapsed = 0;
	
	//Reset all inventory items
	cInventoryManager->GetItem("Health")->Add(5);
	cInventoryManager->GetItem("Soul")->Remove(100);
	cInventoryManager->GetItem("Geo")->Remove(10000);

	return true;
}

/**
 @brief Update this instance
 */
void CPlayer2D::Update(const double dElapsedTime)
{
	// Store the old position
	vec2OldIndex = vec2Index;
	cSoundController->SetListenerPosition(vec2Index.x, vec2Index.y, 0.0f);

	iframeElapsed += 0.01;
	if (iframeElapsed < 1.0)
	{
		runtimeColour = glm::vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
	}

	// Get keyboard updates, disable movement while focusing or dead
	if (cPhysics2D.GetStatus() != CPhysics2D::STATUS::FOCUS && deadElapsed == 0)
	{
		if (cKeyboardController->IsKeyDown(GLFW_KEY_LEFT))
		{
			dir = DIRECTION::LEFT;
			/*cSoundController->PlaySoundByID(4);*/
			// Calculate the new position to the left
			if (vec2Index.x >= 0)
			{
				vec2NumMicroSteps.x--;
				if (vec2NumMicroSteps.x < 0)
				{
					vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
					vec2Index.x--;
				}
			}
			// Constraint the player's position within the screen boundary
			Constraint(LEFT);

			// If the new position is not feasible, then revert to old position
			if (CheckPosition(LEFT) == false)
			{
				vec2Index = vec2OldIndex;
				vec2NumMicroSteps.x = 0;
			}

			//CS: Play the "left" animation
			animatedSprites->PlayAnimation("left", -1, 0.2f);
			
		}
		else if (cKeyboardController->IsKeyDown(GLFW_KEY_RIGHT))
		{
			dir = DIRECTION::RIGHT;
			/*cSoundController->PlaySoundByID(4);*/
			// Calculate the new position to the right
			if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
			{
				vec2NumMicroSteps.x++;

				if (vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
				{
					vec2NumMicroSteps.x = 0;
					vec2Index.x++;
				}
			}
			// Constraint the player's position within the screen boundary
			Constraint(RIGHT);

			// If the new position is not feasible, then revert to old position
			if (CheckPosition(RIGHT) == false)
			{
				vec2NumMicroSteps.x = 0;
			}

			//CS: Play the "right" animation
			animatedSprites->PlayAnimation("right", -1, 0.2f);
		}
		else {
			//CS: Play the "idle" animation by default, if not jumping/falling as well
			if (dir == DIRECTION::LEFT)
				animatedSprites->PlayAnimation("idleLeft", -1, 0.8f);
			else
				animatedSprites->PlayAnimation("idleRight", -1, 0.8f);
		}

		if (cKeyboardController->IsKeyDown(GLFW_KEY_UP))
		{
			dir = DIRECTION::UP;
			/*cSoundController->PlaySoundByID(4);*/
			// Calculate the new position to the left
			if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
			{
				vec2NumMicroSteps.y++;

				if (vec2NumMicroSteps.y >= cSettings->NUM_STEPS_PER_TILE_YAXIS)
				{
					vec2NumMicroSteps.y = 0;
					vec2Index.y++;
				}
			}
		
			// Constraint the player's position within the screen boundary
			Constraint(UP);

			// If the new position is not feasible, then revert to old position
			if (CheckPosition(UP) == false)
			{
				/*vec2Index = vec2OldIndex;*/
				vec2NumMicroSteps.y = 0;
			}

			//CS: Play the "left" animation
			animatedSprites->PlayAnimation("left", -1, 0.2f);
		}
		else if (cKeyboardController->IsKeyDown(GLFW_KEY_DOWN))
		{
			dir = DIRECTION::DOWN;
			/*cSoundController->PlaySoundByID(4);*/
			// Calculate the new position to the right
			if (vec2Index.y >= 0)
			{
				vec2NumMicroSteps.y--;
				if (vec2NumMicroSteps.y < 0)
				{
					vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
					vec2Index.y--;
				}
			}
			// Constraint the player's position within the screen boundary
			Constraint(DOWN);

			// If the new position is not feasible, then revert to old position
			if (CheckPosition(DOWN) == false)
			{
				vec2Index.y = vec2OldIndex.y;
				vec2NumMicroSteps.y = 0;
			}

			//CS: Play the "right" animation
			animatedSprites->PlayAnimation("right", -1, 0.2f);
		}
		else {
			//CS: Play the "idle" animation by default, if not jumping/falling as well
			if (dir == DIRECTION::DOWN)
				animatedSprites->PlayAnimation("idleLeft", -1, 0.8f);
			else
				animatedSprites->PlayAnimation("idleRight", -1, 0.8f);
		}
	}

	//If stopped walking, stop the walking sound and go back to idle state
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_LEFT) || cKeyboardController->IsKeyReleased(GLFW_KEY_RIGHT))
	{
		cSoundController->StopSoundByID(4);
	}

	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
	{
		if (cKeyboardController->IsKeyPressed(GLFW_KEY_A) && cInventoryManager->GetItem("Soul")->GetCount() >= 25 && cInventoryManager->GetItem("Health")->GetCount() < 5)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FOCUS);
			focusElapsed = 0;

			//Play the focus sound
			cSoundController->PlaySoundByID(11);
		}

		if (cKeyboardController->IsKeyPressed(GLFW_KEY_Z))
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 2.f));
			iJumpCount += 1;
			//Play sound for jump
			cSoundController->PlaySoundByID(5);
		}
		else if (cKeyboardController->IsKeyPressed(GLFW_KEY_X))
		{
			lastAttackElapsed = 0;
			iAttackCount = 0;
			cPhysics2D.SetStatus(CPhysics2D::STATUS::ATTACK);

			//Play the attack sound
			cSoundController->PlaySoundByID(8);
		}
	}

	if (cKeyboardController->IsKeyReleased(GLFW_KEY_A) || focusElapsed >= 0.6)
	{
		cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);

		//Stop the focus sound
		cSoundController->StopSoundByID(11);
		focusElapsed = 0;
	}




	// Override animation if jump/falling/attacking
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP)
	{
		//CS: Play the "jump" animation
		if (dir == DIRECTION::LEFT)
			animatedSprites->PlayAnimation("jumpLeft", -1, 0.8f);
		else
			animatedSprites->PlayAnimation("jumpRight", -1, 0.8f);
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FALL)
	{
		//CS: Play the "fall" animation
		if (dir == DIRECTION::LEFT)
			animatedSprites->PlayAnimation("fallLeft", -1, 0.2f);
		else
			animatedSprites->PlayAnimation("fallRight", -1, 0.2f);

		//Play the fall sound
		/*cSoundController->PlaySoundByID(6);*/
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::ATTACK)
	{
		//CS: Play the "attack" animation
		if (dir == DIRECTION::LEFT)
			animatedSprites->PlayAnimation("atkLeft", -1, 0.3f);
		else
			animatedSprites->PlayAnimation("atkRight", -1, 0.3f);
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FOCUS)
	{
		//CS: Play the "focus" animation
		if (dir == DIRECTION::LEFT)
			animatedSprites->PlayAnimation("focusLeft", -1, 0.2f);
		else
			animatedSprites->PlayAnimation("focusRight", -1, 0.2f);
	}


	if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 1, 110))
		UpdateBox(glm::vec2(vec2Index.y, vec2Index.x - 1));
	if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1, 110))
		UpdateBox(glm::vec2(vec2Index.y, vec2Index.x + 1));
	if (cMap2D->GetMapInfo(vec2Index.y+1, vec2Index.x, 110))
		UpdateBox(glm::vec2(vec2Index.y + 1, vec2Index.x));
	if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x, 110))
		UpdateBox(glm::vec2(vec2Index.y - 1, vec2Index.x));


	//Win condition
	cInventoryItem = cInventoryManager->GetItem("Paper");
	if (cInventoryItem->GetCount() == cInventoryItem->GetMaxCount())
		CGameManager::GetInstance()->bLevelCompleted = true;

	UpdateHealthLives();

	// Update Jump or Fall
	//CS: Will cause error when debugging. Set to default elapsed time


	// Interact with the Map
	InteractWithMap();

	//CS: Update the animated sprite
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x*cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y*cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CPlayer2D::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CPlayer2D::Render(void)
{
	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	glm::mat4 MVP = camera->GetMVP();
	glm::mat4 transformMVP;
	//transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	//transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
	//												vec2UVCoordinate.y,
	//												0.0f));
	// 
	transformMVP = MVP; // make sure to initialize matrix to identity matrix first
	transformMVP = glm::translate(transformMVP, glm::vec3(vec2UVCoordinate.x,
		vec2UVCoordinate.y,
		0.0f));
	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformMVP));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	//CS: Render the animated sprite
	glBindVertexArray(VAO);
	animatedSprites->Render();
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//// bind textures on corresponding texture units
	//glActiveTexture(GL_TEXTURE0);
	//// Get the texture to be rendered
	//glBindTexture(GL_TEXTURE_2D, iTextureID);
	//	//Render the Player sprite
	//	glBindVertexArray(VAO);
	//	quadMesh->Render();
	//	glBindVertexArray(0);
	//glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CPlayer2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

/**
 @brief Constraint the player's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CPlayer2D::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (vec2Index.x < 0)
		{
			vec2Index.x = 0;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y < 0)
		{
			vec2Index.y = 0;
			vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CPlayer2D::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CPlayer2D::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.x >= cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2NumMicroSteps.x = 0;
			return true;
		}

		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}

	}
	else if (eDirection == UP)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.y >= cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2NumMicroSteps.y = 0;
			return true;
		}

		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

// Update Jump or Fall
void CPlayer2D::UpdateJumpFall(const double dElapsedTime)
{
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.SetTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		int iIndex_YAxis_OLD = vec2Index.y;
		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->MICRO_STEP_YAXIS);
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			vec2NumMicroSteps.y += iDisplacement_MicroSteps;
			if (vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				vec2NumMicroSteps.y -= cSettings->NUM_STEPS_PER_TILE_YAXIS;
				if (vec2NumMicroSteps.y < 0)
					vec2NumMicroSteps.y = 0;
				vec2Index.y++;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(UP);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop jump if so.
		int iIndex_YAxis_Proposed = vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i <= iIndex_YAxis_Proposed; i++)
		{
			// Change the player's index to the current i value
			vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(UP) == false)
			{
				// Align with the row
				vec2NumMicroSteps.y = 0;
				// Set the Physics to fall status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
				break;
			}
		}

		// If the player is still jumping and the initial velocity has reached zero or below zero, 
		// then it has reach the peak of its jump
		if ((cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP) && (cPhysics2D.GetDisplacement().y <= 0.0f))
		{
			// Set status to fall
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FALL)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.SetTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current vec2Index.y
		int iIndex_YAxis_OLD = vec2Index.y;

		// Translate the displacement from pixels to indices
		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->MICRO_STEP_YAXIS);

		if (vec2Index.y >= 0)
		{
			//Slower fall speed
			vec2NumMicroSteps.y -= fabs(iDisplacement_MicroSteps) * 0.3;
			if (vec2NumMicroSteps.y < 0)
			{
				vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(DOWN);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop fall if so.
		int iIndex_YAxis_Proposed = vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i >= iIndex_YAxis_Proposed; i--)
		{
			// Change the player's index to the current i value
			vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(DOWN) == false)
			{
				// Revert to the previous position
				if (i != iIndex_YAxis_OLD)
					vec2Index.y = i + 1;
				// Set the Physics to idle status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
				iJumpCount = 0;
				vec2NumMicroSteps.y = 0;
				
				//Play land sound
				cSoundController->StopSoundByID(6);
				cSoundController->PlaySoundByID(7);
				break;
			}
		}
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::ATTACK)
	{
		lastAttackElapsed += 0.01;
		if (lastAttackElapsed >= 0.3)
			cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
		if (vec2NumMicroSteps.x == 0 && iAttackCount == 0)
		{
			if (dir == DIRECTION::LEFT)
				UpdateBreakables(glm::vec2(vec2Index.y, vec2Index.x - 1));
			else
				UpdateBreakables(glm::vec2(vec2Index.y, vec2Index.x + 1));
		}
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FOCUS)
	{
		focusElapsed += 0.01;
		if (focusElapsed >= 0.6)
		{
			cInventoryManager->GetItem("Soul")->Remove(25);
			cInventoryManager->GetItem("Health")->Add(1);
		}
	}
}

// Check if the player is in mid-air
bool CPlayer2D::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
		return false;

	// Check if the tile below the player's current position is empty
	if ((vec2NumMicroSteps.x == 0) &&
		((cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 100) || (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) > 300)))
	{
		return true;
	}

	return false;
}

/**
 @brief Let player interact with the map. You can add collectibles such as powerups and health here.
 */
void CPlayer2D::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 99:
		DamagePlayer();
		break;
	case 97:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
		break;
	case 75:
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
		break;
	default:
		break;
	}
}

/**
 @brief Update the health and lives
 */
void CPlayer2D::UpdateHealthLives(void)
{
	// Update health and lives
	cInventoryItem = cInventoryManager->GetItem("Health");
	// Check if a life is lost
	if (cInventoryItem->GetCount() <= 0)
	{
		cSoundController->PlaySoundByID(10);
		animatedSprites->PlayAnimation("death", 1, 3.0f);
		runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
		iframeElapsed = 0;
		deadElapsed += 0.01;

		// Player loses the game
		if (deadElapsed >= 1.5)
			CGameManager::GetInstance()->bPlayerLost = true;
		//Lose condition
		/*CGameManager::GetInstance()->bPlayerLost = true;*/
	}

}

void CPlayer2D::UpdateBreakables(glm::vec2 pos)
{
	switch (cMap2D->GetMapInfo(pos.x, pos.y))
	{
	//blocks
	case 103:
		cMap2D->SetMapInfo(pos.x, pos.y, 104, true);
		cSoundController->StopSoundByID(12); //restart sound if it's playing before
		cSoundController->PlaySoundByID(12);
		break;
	case 104:
		cMap2D->SetMapInfo(pos.x, pos.y, 0, true);
		cSoundController->StopSoundByID(12);
		cSoundController->PlaySoundByID(12);
		break;

	//geo
	case 106:
		cMap2D->SetMapInfo(pos.x, pos.y, 107, true);
		cSoundController->StopSoundByID(13);
		cSoundController->PlaySoundByID(13);
		break;
	case 107:
		cMap2D->SetMapInfo(pos.x, pos.y, 0, true);
		cSoundController->StopSoundByID(13);
		cSoundController->PlaySoundByID(13);
		cInventoryManager->GetItem("Geo")->Add(300);
		break;
	}
	iAttackCount = 1;
}

void CPlayer2D::UpdateBox(glm::vec2 pos)
{
	switch (cMap2D->GetMapInfo(pos.y, pos.x))
	{
	case 110:
		if (dir == LEFT)
		{
			//Set box into new position
			cMap2D->SetMapInfo(pos.y, pos.x - 1, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(pos.y, pos.x, 0);
		}
		if (dir == RIGHT)
		{
			//Set box into new position
			cMap2D->SetMapInfo(pos.y, pos.x + 1, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(pos.y, pos.x, 0);
		}
		if (dir == UP)
		{
			//Set box into new position
			cMap2D->SetMapInfo(pos.y + 1, pos.x, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(pos.y, pos.x, 0);
		}
		if (dir == DOWN)
		{
			//Set box into new position
			cMap2D->SetMapInfo(pos.y -1, pos.x, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(pos.y, pos.x, 0);
		}
		break;
	}
}

void CPlayer2D::DamagePlayer(int eDirection)
{
	//Decrease the health by 1 if not in iframe
	if (iframeElapsed > 1.0)
	{
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		iframeElapsed = 0;
		cSoundController->PlaySoundByID(9);

		////Knockback player a bit depending on direction
		//if (eDirection == DIRECTION::LEFT)
		//{
		//	// Calculate the new position to the left
		//	if (vec2Index.x >= 0)
		//	{
		//		vec2NumMicroSteps.x -= 5;
		//		if (vec2NumMicroSteps.x < 0)
		//		{
		//			vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
		//			vec2Index.x--;
		//		}
		//	}
		//	// Constraint the player's position within the screen boundary
		//	Constraint(LEFT);

		//	// If the new position is not feasible, then revert to old position
		//	if (CheckPosition(LEFT) == false)
		//	{
		//		vec2Index = vec2OldIndex;
		//		vec2NumMicroSteps.x = 0;
		//	}

		//	if (IsMidAir() == true)
		//	{
		//		if (cPhysics2D.GetStatus() != CPhysics2D::STATUS::JUMP)
		//			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		//	}
		//}
		//else
		//{
		//	// Calculate the new position to the right
		//	if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		//	{
		//		vec2NumMicroSteps.x += 5;

		//		if (vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
		//		{
		//			vec2NumMicroSteps.x = 0;
		//			vec2Index.x++;
		//		}
		//	}
		//	// Constraint the player's position within the screen boundary
		//	Constraint(RIGHT);

		//	// If the new position is not feasible, then revert to old position
		//	if (CheckPosition(RIGHT) == false)
		//	{
		//		vec2NumMicroSteps.x = 0;
		//	}

		//	if (IsMidAir() == true)
		//	{
		//		if (cPhysics2D.GetStatus() != CPhysics2D::STATUS::JUMP)
		//			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		//	}
		//}
	}
}

bool CPlayer2D::isAttacking(void)
{
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::ATTACK)
		return true;
	else
		return false;
}
