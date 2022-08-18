/**
 CEnemyWarrior
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "EnemyWarrior.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"

// Include GLEW
#include <GL/glew.h>

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include math.h
#include <math.h>

#include "System\filesystem.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CEnemyWarrior::CEnemyWarrior(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::IDLE)
	, quadMesh(NULL)
	, camera(NULL)
	, animatedSprites(NULL)
	, cSoundController(NULL)
	, cInventoryManager(NULL)
{
	transform = glm::mat4(1.0f);	// make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	i32vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	i32vec2Destination = glm::i32vec2(0, 0);	// Initialise the iDestination
	i32vec2Direction = glm::i32vec2(0, 0);		// Initialise the iDirection
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CEnemyWarrior::~CEnemyWarrior(void)
{
	// Delete the quadMesh
	if (quadMesh)
	{
		delete quadMesh;
		quadMesh = NULL;
	}

	if (animatedSprites)
	{
		delete animatedSprites;
		animatedSprites = NULL;
	}

	// We won't delete this since it was created elsewhere
	cPlayer2D = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	camera = NULL;

	cSoundController = NULL;

	cInventoryManager = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool CEnemyWarrior::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Get handler for camera
	camera = Camera::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();

	cInventoryManager = CInventoryManager::GetInstance();

	cSoundController = CSoundController::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(301, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Load the EnemyWarrior texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/warrior.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/warrior.png" << endl;
		return false;
	}

	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(4, 6, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("left", 0, 2);
	animatedSprites->AddAnimation("right", 3, 5);
	animatedSprites->AddAnimation("atkLeft", 6, 8);
	animatedSprites->AddAnimation("atkRight", 9, 11);
	animatedSprites->AddAnimation("shieldLeft", 12, 14);
	animatedSprites->AddAnimation("shieldRight", 15, 17);
	animatedSprites->AddAnimation("death", 18, 20);
	animatedSprites->PlayAnimation("right", -1, 0.5f);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	iframeElapsed = 0.3;
	health = 80;
	iFSMCounter = 0;
	lastAttackElapsed = 0;

	// Create the quad mesh for the player
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	dir = DIRECTION::RIGHT;

	return true;
}

/**
 @brief Update this instance
 */
void CEnemyWarrior::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return;
	lastAttackElapsed += 0.01;
	iframeElapsed += 0.01;

	//cSoundController->LoadSound(FileSystem::getPath("Sounds\\CreepyLaugh.wav"), 26, true, false, CSoundInfo::SOUNDTYPE::_3D, vec3df(0, 0, 0));

	////change volume based on distance of enemy to player
	//if (cPlayer2D->vec2Index.y >= vec2Index.y && cPlayer2D->vec2Index.y <= vec2Index.y + 3 ||
	//	cPlayer2D->vec2Index.y <= vec2Index.y && cPlayer2D->vec2Index.y >= vec2Index.y - 3)
	//{
	//	if (cPlayer2D->vec2Index.x >= vec2Index.x - 3 && cPlayer2D->vec2Index.x < vec2Index.x)
	//		cSoundController->SetVolume(26, 1);
	//	//Player is to the right of the enemy
	//	else if (cPlayer2D->vec2Index.x > vec2Index.x && cPlayer2D->vec2Index.x <= vec2Index.x + 3)
	//		cSoundController->SetVolume(26, 1);

	//	//If enemy is below or above player
	//	if (cPlayer2D->vec2Index.x == vec2Index.x)
	//		cSoundController->SetSoundType(26, 2);
	//	else
	//		cSoundController->SetSoundType(26, 3);
	//}
	//else
	//	cSoundController->SetVolume(26, 0.5);

	////Update sound position based on enemy position
	//if (cSoundController->GetSoundType(26) == 3)
	//{
	//	cSoundController->SetSoundPosition(vec2Index.x, vec2Index.y, 0, 26);

	//	//Set player as the listener position
	//	cSoundController->SetListenerPosition(cPlayer2D->vec2Index.x, cPlayer2D->vec2Index.y, 0);
 //		//cout << "3";
	//}

	////if (cSoundController->GetSoundType(26) == 2)
	////	cout << "2" << endl;

	////if (cPlayer2D->vec2Index.x < vec2Index.x)
	////	cSoundController->SetListenerDirection(vec2Index.x - cPlayer2D->vec2Index.x,vec2Index.y, 0);
	////else if (cPlayer2D->vec2Index.x > vec2Index.x)
	////	cSoundController->SetListenerDirection(cPlayer2D->vec2Index.x - vec2Index.x,vec2Index.y, 0);
	////else
	////	cSoundController->SetListenerDirection(vec2Index.x, vec2Index.y, 0);

	//cout << "Sound:" << cSoundController->getCurrentVolume(26) << endl;
	//cSoundController->PlaySoundByID(26);
	
	if (iframeElapsed < 0.3)
	{
		runtimeColour = glm::vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
	}

	if (health <= 0)
	{
		cSoundController->PlaySoundByID(15);

		cInventoryManager->GetItem("Soul")->Add(30);
		cInventoryManager->GetItem("Geo")->Add(150);
		bIsActive = false;
		cSoundController->StopSoundByID(26);
	}

	switch (sCurrentFSM)
	{
	case IDLE:
		break;
	case PATROL:
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 8.0f)
		{
			sCurrentFSM = ATTACK;
		}
		else
		{
			// Patrol around
			// Update the EnemyWarrior's position for patrol
			UpdatePosition();
		}
		break;
	case ATTACK:
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 8.0f)
		{
			// Attack
			// Update direction to move towards for attack
			//UpdateDirection();
			auto path = cMap2D->PathFind(vec2Index,
				cPlayer2D->vec2Index,
				heuristic::euclidean,
				10);

			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				if (bFirstPosition == true)
				{
					// Set a destination
					i32vec2Destination = coord;
					// Calculate the direction between EnemyWarrior and this destination
					i32vec2Direction = i32vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - i32vec2Destination) == i32vec2Direction)
					{
						//Set a destination
						i32vec2Destination = coord;
					}
					else
						break;
				}
			}
			// Update the EnemyWarrior's position for attack
			UpdatePosition();
		}
		else {
			//Return to patrol state when out of range
			sCurrentFSM = PATROL;
		}
		break;

	case DEFEND:
		// Interact with the Player
		InteractWithPlayer();

		iFSMCounter++;
		if (iFSMCounter > 100)
		{
			sCurrentFSM = ATTACK;
		}
		break;

	case STUN:
		//do not move under stun
		iFSMCounter++;
		if (iFSMCounter > 20)
		{
			sCurrentFSM = PATROL;
		}
		break;
	default:
		break;
	}

	// Update the animated sprite
	animatedSprites->Update(dElapsedTime);

	// Update Jump or Fall
	UpdateJumpFall(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x*cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y*cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CEnemyWarrior::PreRender(void)
{
	if (!bIsActive)
		return;

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CEnemyWarrior::Render(void)
{
	if (!bIsActive)
		return;

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

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CEnemyWarrior::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Set the indices of the EnemyWarrior
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void CEnemyWarrior::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the EnemyWarrior
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void CEnemyWarrior::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void CEnemyWarrior::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}


/**
 @brief Constraint the EnemyWarrior's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CEnemyWarrior::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (vec2Index.x < 0)
		{
			vec2Index.x = 0;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y < 0)
		{
			vec2Index.y = 0;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CEnemyWarrior::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CEnemyWarrior::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
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
			i32vec2NumMicroSteps.x = 0;
			return true;
		}

		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
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
			i32vec2NumMicroSteps.y = 0;
			return true;
		}

		// If the new position is fully within a column, then check this column only
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
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
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
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
		cout << "CEnemyWarrior::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

// Check if the EnemyWarrior is in mid-air
bool CEnemyWarrior::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
		return false;

	// Check if the tile below the player's current position is empty
	if ((i32vec2NumMicroSteps.x == 0) &&
		(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 100))
	{
		return true;
	}

	return false;
}

// Update Jump or Fall
void CEnemyWarrior::UpdateJumpFall(const double dElapsedTime)
{
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.SetTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current vec2Index.y
		int iIndex_YAxis_OLD = vec2Index.y;

		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->MICRO_STEP_YAXIS); //DIsplacement divide by distance for 1 microstep
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			i32vec2NumMicroSteps.y += iDisplacement_MicroSteps;
			if (i32vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				i32vec2NumMicroSteps.y -= cSettings->NUM_STEPS_PER_TILE_YAXIS;
				if (i32vec2NumMicroSteps.y < 0)
					i32vec2NumMicroSteps.y = 0;
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
				i32vec2NumMicroSteps.y = 0;
				// Set the Physics to fall status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
				break;
			}
		}

		// If the player is still jumping and the initial velocity has reached zero or below zero, 
		// then it has reach the peak of its jump
		if ((cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP) && (cPhysics2D.GetInitialVelocity().y <= 0.0f))
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
			i32vec2NumMicroSteps.y -= fabs(iDisplacement_MicroSteps);
			if (i32vec2NumMicroSteps.y < 0)
			{
				i32vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
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
				i32vec2NumMicroSteps.y = 0;
				break;
			}
		}
	}
}

/**
 @brief Let EnemyWarrior interact with the player.
 */
bool CEnemyWarrior::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
	if (cPlayer2D->isAttacking() && iframeElapsed >= 0.3)
	{
		if (((vec2Index.x >= i32vec2PlayerPos.x - 1.0) &&
			(vec2Index.x <= i32vec2PlayerPos.x))
			&&
			((vec2Index.y >= i32vec2PlayerPos.y - 1.0) &&
				(vec2Index.y <= i32vec2PlayerPos.y + 1.0))
			&&
			cPlayer2D->dir == DIRECTION::LEFT
			)
		{
			//If player attacks, and is facing opposite direction as warrior, go to defense mode
			if (dir == DIRECTION::RIGHT)
			{
				cSoundController->PlaySoundByID(16);
				animatedSprites->PlayAnimation("shieldRight", 1, 0.3f);
				iFSMCounter = 0;
				sCurrentFSM = DEFEND;

				return true;
			}

			//Attacked by player
			health -= 10;
			cSoundController->PlaySoundByID(14);

			if (sCurrentFSM != DEFEND)
			{
				sCurrentFSM = STUN;
				iFSMCounter = 0;
			}
			iframeElapsed = 0;
		}
		else if (((vec2Index.x >= i32vec2PlayerPos.x) &&
			(vec2Index.x <= i32vec2PlayerPos.x + 1.0))
			&&
			((vec2Index.y >= i32vec2PlayerPos.y - 1.0) &&
				(vec2Index.y <= i32vec2PlayerPos.y + 1.0))
			&&
			cPlayer2D->dir == DIRECTION::RIGHT
			&&
			lastAttackElapsed >= 0.3
			)
		{
			//If player attacks, and is facing opposite direction as warrior, go to defense mode
			if (dir == DIRECTION::LEFT)
			{
				cSoundController->PlaySoundByID(16);
				animatedSprites->PlayAnimation("shieldLeft", 1, 0.3f);
				iFSMCounter = 0;
				sCurrentFSM = DEFEND;
				return true;
			}

			health -= 10;
			cSoundController->PlaySoundByID(14);

			if (sCurrentFSM != DEFEND)
			{
				iFSMCounter = 0;
				sCurrentFSM = STUN;
			}
			iframeElapsed = 0;
		}
	}


	// Check if the EnemyWarrior is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) &&
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&& 
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
		(vec2Index.y <= i32vec2PlayerPos.y + 0.5))
		&& 
		sCurrentFSM == ATTACK
		&& 
		lastAttackElapsed >= 1.0)
	{
		if (dir == DIRECTION::LEFT)
		{
			animatedSprites->PlayAnimation("atkLeft", 1, 0.5f);
		}
		else
		{
			animatedSprites->PlayAnimation("atkRight", 1, 0.5f);
		}
		cSoundController->PlaySoundByID(17);
		lastAttackElapsed = 0;
		cPlayer2D->DamagePlayer(dir);
		return true;
	}
	return false;
}

/**
 @brief Update the enemy's direction.
 */
void CEnemyWarrior::UpdateDirection(void)
{
	// Set the destination to the player
	i32vec2Destination = cPlayer2D->vec2Index;

	// Calculate the direction between EnemyWarrior and player2D
	i32vec2Direction = i32vec2Destination - vec2Index;

	// Calculate the distance between EnemyWarrior and player2D
	float fDistance = cPhysics2D.CalculateDistance(vec2Index, i32vec2Destination);
	if (fDistance >= 1.f)
	{
		// Calculate direction vector.
		// We need to round the numbers as it is easier to work with whole numbers for movements
		i32vec2Direction.x = (int)round(i32vec2Direction.x / fDistance);
		i32vec2Direction.y = (int)round(i32vec2Direction.y / fDistance);
	}
	else
	{
		// Since we are not going anywhere, set this to 0.
		i32vec2Direction = glm::i32vec2(0);
	}
}

/**
 @brief Flip horizontal direction. For patrol use only
 */
void CEnemyWarrior::FlipHorizontalDirection(void)
{
	i32vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void CEnemyWarrior::UpdatePosition(void)
{
	// Store the old position
	i32vec2OldIndex = vec2Index;

	// if the player is to the left or right of the EnemyWarrior, then charge at player
	if (i32vec2Direction.x < 0)
	{
		animatedSprites->PlayAnimation("left", -1, 0.5f);
		dir = DIRECTION::LEFT;
		// Move left
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x >= 0)
		{
			i32vec2NumMicroSteps.x-= 0.5;
			if (i32vec2NumMicroSteps.x < 0)
			{
				i32vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}

		// Constraint the EnemyWarrior's position within the screen boundary
		Constraint(LEFT);

		// Find a feasible position for the EnemyWarrior's current position
		if (CheckPosition(LEFT) == false)
		{
			FlipHorizontalDirection();
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Check if EnemyWarrior is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		// Interact with the Player
		InteractWithPlayer();
	}
	else if (i32vec2Direction.x > 0)
	{
		animatedSprites->PlayAnimation("right", -1, 0.5f);
		dir = DIRECTION::RIGHT;
		// Move right
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		{
			i32vec2NumMicroSteps.x+= 0.5;

			if (i32vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
			{
				i32vec2NumMicroSteps.x = 0;
				vec2Index.x++;
			}
		}

		// Constraint the EnemyWarrior's position within the screen boundary
		Constraint(RIGHT);

		// Find a feasible position for the EnemyWarrior's current position
		if (CheckPosition(RIGHT) == false)
		{
			FlipHorizontalDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Check if EnemyWarrior is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		// Interact with the Player
		InteractWithPlayer();
	}
}
