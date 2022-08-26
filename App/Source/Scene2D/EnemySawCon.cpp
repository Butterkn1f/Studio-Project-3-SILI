/**
 CEnemySawCon
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "EnemySawCon.h"

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

//To get rays
#include "Map2D.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CEnemySawCon::CEnemySawCon(void)
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
CEnemySawCon::~CEnemySawCon(void)
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
bool CEnemySawCon::Init(void)
{
	//CSettings instance
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
	if (cMap2D->FindValue(303, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 2);
	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Load the EnemySawCon texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/enemy3.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/enemy3.png" << endl;
		return false;
	}
	//Down = 0 - 7
	//idleDown = 8 - 10
	//idleUp = 12 - 14
	//Up = 16 - 23
	//Left = 24 - 31
	//idleLeft = 32 - 34
	//Right = 36 - 43
	//idleRight = 45 - 47
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(12, 4,cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("idleRight", 45, 47);
	animatedSprites->AddAnimation("idleLeft", 32, 34);
	animatedSprites->AddAnimation("idleDown", 8, 10);
	animatedSprites->AddAnimation("idleUp", 12, 14);
	animatedSprites->AddAnimation("down", 0, 7);
	animatedSprites->AddAnimation("up", 16, 23);
	animatedSprites->AddAnimation("left", 24, 31);
	animatedSprites->AddAnimation("right", 36, 43);

	animatedSprites->PlayAnimation("idleLeft", -1, 0.5f);

	//CS: Init the color to white
	runtimeColour = glm::vec4(0.f, 0.f, 0.f, 1.0);

	// Create the quad mesh for the player
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH,cSettings->TILE_HEIGHT);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
	dir = DIRECTION::LEFT;

	//set defaults
	spotDestination = glm::vec2(0, 0);
	shun = false; 
	sawPlayer = false;
	playerInteractWithBox = false;
	iFSMCounter = 0;
	AtkCounter = 0;
	InvestigateCounter = 0;
	ScaredCounter = 0;

	displaytest = false; //<<<<<<<<<<<<<<<<<,,togle on to display troubleshoot  must togle on to see others
	pathtest = true;    //<<<<<<<<<<<<<<<<<,,togle on to path troubleshoot
	statetest = true;   //<<<<<<<<<<<<<<<<<,,togle on to state troubleshoot
	
	


	//-----------------------change here----------------------------
	chaseRange = 3.5f;//how far enemy can detect u
	atkrange = .05f;//how close must the enemy be to atk u
	movementspeed = 0.9;// speed of enemy
	
	
	

	if (!displaytest)//let others know that the enemysawcon couts is dissable if its off
		cout << "toggled off cout for EnemySawCon.cpp" << endl;
	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	rays = Rays::GetInstance()->GetRays();

	return true;
}

/**
 @brief Update this instance
 */
void CEnemySawCon::Update(const double dElapsedTime)
{
	if (!displaytest)
	{
		pathtest = false;
		statetest = false;
	}
	playerInteractWithBox = cPlayer2D->getEBox();
	if (pathtest)
	{
		cout << "enemy - x " <<vec2Index.x << "y "<< vec2Index.y << endl;
		cout << "player - x " << cPlayer2D->vec2Index.x << " y " << cPlayer2D->vec2Index.y << endl;
	}
	if (!bIsActive)
		return;

	rays = Rays::GetInstance()->GetRays();

	//change volume based on distance of enemy to player
	if (cPlayer2D->vec2Index.y >= vec2Index.y && cPlayer2D->vec2Index.y <= vec2Index.y + 3 ||
		cPlayer2D->vec2Index.y <= vec2Index.y && cPlayer2D->vec2Index.y >= vec2Index.y - 3)
	{
		if (cPlayer2D->vec2Index.x >= vec2Index.x - 3 && cPlayer2D->vec2Index.x < vec2Index.x)
			cSoundController->SetVolume(26, 1);
		//Player is to the right of the enemy
		else if (cPlayer2D->vec2Index.x > vec2Index.x && cPlayer2D->vec2Index.x <= vec2Index.x + 3)
			cSoundController->SetVolume(26, 1);
		else 
			cSoundController->SetVolume(26, 0.5);
	}
	else
		//Sound is very soft as 
		cSoundController->SetVolume(26, 0.25);

	//If enemy is below or above player
	if (cPlayer2D->vec2Index.x == vec2Index.x)
		cSoundController->SetSoundType(26, 2);
	else
		cSoundController->SetSoundType(26, 3);

	//Update sound position based on enemy position
	if (cSoundController->GetSoundType(26) == 3)
	{
		cSoundController->SetSoundPosition(vec2Index.x, vec2Index.y, 0, 26);

		//Set player as the listener position
		cSoundController->SetListenerPosition(cPlayer2D->vec2Index.x, cPlayer2D->vec2Index.y, 0);
	}
	cSoundController->PlaySoundByID(26);
	

	
	//check if player collect the collectable<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	
	switch (sCurrentFSM)
	{
	case IDLE:
		//combineCheckSawPlayer();<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		 
		
		//kena shun by light
		if (shun)
		{
			ScaredCounter = 0;
			sCurrentFSM = SCARED;
		}
		//player collect a paper
		else if (playerNewlyVec(cPlayer2D->getOldVec()))
		{
			sCurrentFSM = INVESTIGATE;
			if(statetest)
				cout << "Switching to Investigate State :" << cPlayer2D->getOldVec().x << cPlayer2D->getOldVec().y << endl;
		}
		//counter thingy
		else if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			if(statetest)
				cout << "Switching to Patrol State" << endl;
		}
		iFSMCounter++;
		break;
	case PATROL:
		//player interact with the box
		if (playerInteractWithBox)
		{
			InvestigateCounter = 0;
			sCurrentFSM = INVESTIGATE;
		}
		//kena shun by light
		if (shun)
		{
			ScaredCounter = 0;
			sCurrentFSM = SCARED;
		}
		//fsmcounter
		else if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			if(statetest)
			cout << "Switching to Idle State" << endl;
		}
		//chase range
		else if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < chaseRange)
		{
			if(statetest)
				cout << "Switching to Attack State" <<endl;
			sCurrentFSM = ATTACK;
			iFSMCounter = 0;
		}
		else
		{
			UpdatePosition();
		}
		iFSMCounter++;
		break;
	case ATTACK://help check if no LOS
		//kena shun by light
		if (shun)
		{
			ScaredCounter = 0;
			sCurrentFSM = SCARED;
		}
		//close to player, atk
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < atkrange)
		{		
			CInventoryItem* cInvenytoryItem;
			cInvenytoryItem = cInventoryManager->GetItem("Health");
			if (cInvenytoryItem->GetCount() >= 0)
			{
				cInvenytoryItem->Remove(1);
				cSoundController->PlaySoundByID(27);
				CGameManager::GetInstance()->bPLayerJumpscared = true;
			}
			else
				CGameManager::GetInstance()->bPlayerLost = true;
			if (statetest)
			{
				cout << "switch to cooldown state" << endl;
				cout << "atk player" << endl;
			}
			AtkCounter = 0;
			sCurrentFSM = COOLDOWN;
		}
		//close to player, chase
		else if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < chaseRange)
		{
			if(statetest || pathtest)
				cout << "chase player" << endl;
			 /*Attack
			 Update direction to move towards for attack*/
			//UpdateDirection();
			//cout << "startpos: " << vec2Index.x << ", " << vec2Index.y << endl;

			auto path = cMap2D->PathFind(vec2Index,
				cPlayer2D->vec2Index,
				heuristic::euclidean,
				10);
			
			/*calculate new destination*/
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				if (bFirstPosition == true)
				{
					/* Set a destination*/
					i32vec2Destination = coord;
					/* Calculate the direction between EnemySawCon and this destination*/
					i32vec2Direction = i32vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - i32vec2Destination) == i32vec2Direction)
					{
						/*Set a destination*/
						i32vec2Destination = coord;
					}
					else
						break;
				}
				/*else
					break;*/
			}
			/* Update the EnemySawCon's position for attack*/
			UpdatePosition();
		}
		else
		{
			if (iFSMCounter > iMaxFSMCounter)
			{
				sCurrentFSM = PATROL;
				iFSMCounter = 0;
				if(statetest)
					cout << "ATTACK : Reset counter: " << iFSMCounter << endl;
			}
			iFSMCounter++;
		}
		break;
	case INVESTIGATE:
		//kena shun by light
		if (shun)
		{
			ScaredCounter = 0;
			sCurrentFSM = SCARED;
		}
		//close to player, go atk
		else if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < chaseRange)
		{
			if(statetest)
				cout << "Switching to Attack State" << endl;
			sCurrentFSM = ATTACK;
		}
		//player interactwithbox
		if (playerInteractWithBox)
		{
			if(statetest)
				cout << InvestigateCounter << endl;
			if (InvestigateCounter > MaxInvestigateCounter)
			{
				playerInteractWithBox = false;
				cPlayer2D->setEBox(playerInteractWithBox);
				sCurrentFSM = PATROL;
			}
			else
			{
				/*Attack
			Update direction to move towards for attack
			//UpdateDirection();
			//cout << "startpos: " << vec2Index.x << ", " << vec2Index.y << endl;*/

				auto path = cMap2D->PathFind(vec2Index,
					cPlayer2D->vec2Index,
					heuristic::euclidean,
					10);

				/*calculate new destination*/
				bool bFirstPosition = true;
				for (const auto& coord : path)
				{
					if (bFirstPosition == true)
					{
						/* Set a destination*/
						i32vec2Destination = coord;
						/* Calculate the direction between EnemySawCon and this destination*/
						i32vec2Direction = i32vec2Destination - vec2Index;
						bFirstPosition = false;
					}
					else
					{
						if ((coord - i32vec2Destination) == i32vec2Direction)
						{
							/*Set a destination*/
							i32vec2Destination = coord;
						}
						else
							break;
					}
				}
			}
			InvestigateCounter++;
		}
		//go to the spotdestination
		else
		{
			playerNewlyVec(cPlayer2D->getOldVec());
			if(pathtest)
				cout << spotDestination.x << spotDestination.y << endl;
				auto path = cMap2D->PathFind(vec2Index,
					spotDestination,
					heuristic::euclidean,
					10);

				//calculate new destination
				bool bFirstPosition = true;
				for (const auto& coord : path)
				{
					if (bFirstPosition == true)
					{
						// Set a destination
						i32vec2Destination = coord;
						// Calculate the direction between EnemySawCon and this destination
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
		}
		// Update the EnemySawCon's position
		UpdatePosition();
		break;
	case COOLDOWN:
		//kena shun by light
		if (shun)
		{
			ScaredCounter = 0;
			sCurrentFSM = SCARED;
		}
		//wait for 2 sec
		else if (AtkCounter > MaxAtkCounter)
		{
			if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < chaseRange)
			{
				if(statetest)
				cout << "Switching to Attack State" << endl;
				sCurrentFSM = ATTACK;
				AtkCounter = 0;
				break;
			}
			sCurrentFSM = PATROL;
			AtkCounter = 0;
			if(statetest)
				cout << "Switching to patrol State" << endl;
		}
		AtkCounter++;
		break;
	case SCARED:
		if (ScaredCounter > MaxScaredCounter)
		{
			shun = false;
			sCurrentFSM = IDLE;
		}
		else
		{
			cout << "stay or run away later ah.. waitin on kena shun" << endl;
		//negative direction to player
		}
		ScaredCounter++;
		break;
	default:
		break;
	}

	// Update the animated sprite
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CEnemySawCon::PreRender(void)
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
void CEnemySawCon::Render(void)
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

	if (vec2Index.y <= cPlayer2D->vec2Index.y + 10 && vec2Index.y >= cPlayer2D->vec2Index.y - 10 &&
		vec2Index.x <= cPlayer2D->vec2Index.x + 10 && vec2Index.x >= cPlayer2D->vec2Index.x - 10)
	{
		runtimeColour = cMap2D->GetMapColour(vec2Index.y, vec2Index.x);
		glm::mat4 enemyTransform;
		enemyTransform = glm::mat4(1.f);
		enemyTransform = glm::translate(enemyTransform, glm::vec3(vec2UVCoordinate.x, vec2UVCoordinate.y, 0));

		float intersectionDist = 9999;

		//Note: Doubled size of boundary box such that it is more forgiving and lights up more
		if (Rays::GetInstance()->flashlight.TestRayOBBIntersection(camera->position,
			rays[0].direction,
			glm::vec3(-cSettings->TILE_WIDTH, -cSettings->TILE_HEIGHT * 0.5, -1.f),
			glm::vec3(cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT, 1.f),
			enemyTransform,
			intersectionDist))
		{
			//Stun enemy here
			// Add if statement on itnersection dist here
		/*	runtimeColour = glm::vec4(1.f, 0.f, 0.f, 1.f);
			std::cout << "Stunned!" << std::endl;*/
		}
	}
	else
	{
		runtimeColour = glm::vec4(0.f, 0.f, 0.f, 0.f);
	}
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
void CEnemySawCon::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Set the indices of the EnemySawCon
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void CEnemySawCon::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the EnemySawCon
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void CEnemySawCon::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void CEnemySawCon::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}


/**
 @brief Constraint the EnemySawCon's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CEnemySawCon::Constraint(DIRECTION eDirection)
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
		cout << "CEnemySawCon::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CEnemySawCon::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				//cout << "left no" << endl;
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
				//cout << "left no y++" << endl;
				return false;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.x >= cSettings->NUM_TILES_XAXIS - 1)
		{
			//cout << "right no" << endl;
			i32vec2NumMicroSteps.x = 0;
			return true;
		}

		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100)
			{
				//cout << "right no micro" << endl;
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
		cout << "CEnemySawCon::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

// Update Jump or Fall

/**
 @brief Let EnemySawCon interact with the player.
 */
bool CEnemySawCon::InteractWithPlayer(void)
{

	/*	cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
	cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 3.5f));

	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;

	if (cPlayer2D->isAttacking() && iframeElapsed >= 0.4) {

		if (sCurrentFSM != VULNERABLE && 
			((vec2Index.x >= i32vec2PlayerPos.x - 1.5) &&
			(vec2Index.x <= i32vec2PlayerPos.x + 2))
			&&
			((vec2Index.y >= i32vec2PlayerPos.y - 1.5) &&
				(vec2Index.y <= i32vec2PlayerPos.y + 1.5))) 
		{
			armour -= 10;
			cSoundController->PlaySoundByID(19);
			iframeElapsed = 0;
		}

		else if (sCurrentFSM == VULNERABLE) {
			if (((vec2Index.y >= i32vec2PlayerPos.y - 1.5) &&
				(vec2Index.y <= i32vec2PlayerPos.y + 1.5)) &&
				(
				(dir == LEFT && cPlayer2D->dir == RIGHT &&
					((vec2Index.x >= i32vec2PlayerPos.x) &&
					(vec2Index.x <= i32vec2PlayerPos.x + 1.5))) 
				||
				(dir == RIGHT && cPlayer2D->dir == LEFT &&
					((vec2Index.x >= i32vec2PlayerPos.x - 1) &&
					(vec2Index.x <= i32vec2PlayerPos.x)))
				))
			{
				health -= 5;
				cInventoryManager->GetItem("Soul")->Add(10);
				cSoundController->PlaySoundByID(21);
				iframeElapsed = 0;
			}
		}
	}*/
	return false;
}

/**
 @brief Update the enemy's direction.
 */
void CEnemySawCon::UpdateDirection(void)
{
	// Set the destination to the player
	i32vec2Destination = cPlayer2D->vec2Index;

	// Calculate the direction between EnemySawCon and player2D
	i32vec2Direction = i32vec2Destination - vec2Index;

	// Calculate the distance between EnemySawCon and player2D
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
 @brief Update the enemy's direction to a spot
 */
//void CEnemySawCon::UpdateDirectionSpot(void)
//{
//	// Set the destination to the spot
//	spotDestination = glm::vec2(0, 0);
//
//	// Calculate the direction between EnemySawCon and spot
//	spotDirection = spotDestination - vec2Index;
//
//	// Calculate the distance between EnemySawCon and player2D
//	float fDistance = cPhysics2D.CalculateDistance(vec2Index, spotDestination);
//	if (fDistance >= 1.f)
//	{
//		// Calculate direction vector.
//		// We need to round the numbers as it is easier to work with whole numbers for movements
//		spotDirection.x = (int)round(spotDirection.x / fDistance);
//		spotDirection.y = (int)round(spotDirection.y / fDistance);
//	}
//	else
//	{
//		// Since we are not going anywhere, set this to 0.
//		spotDirection = glm::i32vec2(0);
//	}
//}




/**
 @brief Flip horizontal direction. For patrol use only
 */
void CEnemySawCon::FlipHorizontalDirection(void)
{
	i32vec2Direction.x *= -1;
}

/**
 @brief Flip verticle direction. For patrol use only
 */
void CEnemySawCon::FlipVerticleDirection(void)
{
	i32vec2Direction.y *= -1;
}

/**
@brief Update position.
*/
void CEnemySawCon::UpdatePosition(void)
{
	// Store the old position
	i32vec2OldIndex = vec2Index;

	// if the player is to the left or right of the EnemySawCon, then charge at player

	//left
	if (i32vec2Direction.x < 0)
	{
		animatedSprites->PlayAnimation("left", -1, 0.5f);
		dir = DIRECTION::LEFT;
		// Move left
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x >= 0)
		{
			i32vec2NumMicroSteps.x -= movementspeed;
			if (i32vec2NumMicroSteps.x < 0)
			{
				i32vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}

		// Constraint the EnemySawCon's position within the screen boundary
		Constraint(LEFT);

		// Find a feasible position for the EnemySawCon's current position
		if (CheckPosition(LEFT) == false)
		{
			FlipHorizontalDirection();
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}


		// Interact with the Player
		InteractWithPlayer();
	}

	//right
	else if (i32vec2Direction.x > 0)
	{
		animatedSprites->PlayAnimation("right", -1, 0.5f);
		dir = DIRECTION::RIGHT;
		// Move right
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		{
			i32vec2NumMicroSteps.x += movementspeed;

			if (i32vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
			{
				i32vec2NumMicroSteps.x = 0;
				vec2Index.x++;
			}
		}

		// Constraint the EnemySawCon's position within the screen boundary
		Constraint(RIGHT);

		// Find a feasible position for the EnemySawCon's current position
		if (CheckPosition(RIGHT) == false)
		{
			FlipHorizontalDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}


		// Interact with the Player
		InteractWithPlayer();
	}

	// up
	else if (i32vec2Direction.y > 0)
	{
		animatedSprites->PlayAnimation("up", -1, 0.5f);
		//dir equal to up
		dir = DIRECTION::UP;
		//move upward
		const int iOldIndex = vec2Index.y;
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			i32vec2NumMicroSteps.y += movementspeed;
			if (i32vec2NumMicroSteps.y >= cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				i32vec2NumMicroSteps.y = 0;
				vec2Index.y++;
			}
		}
		// Constraint the EnemySawCon's position within the screen boundary
		Constraint(UP);

		// Find a feasible position for the EnemySawCon's current position
		if (CheckPosition(UP) == false)
		{
			FlipVerticleDirection();
			//vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.y = 0;
		}
		InteractWithPlayer();
	}

	//down
	else if (i32vec2Direction.y < 0)
	{
	animatedSprites->PlayAnimation("down", -1, 0.5f);
		//dir equal to down
		dir = DIRECTION::DOWN;
		// Move down
		const int iOldIndex = vec2Index.y;
		if (vec2Index.y >= 0)
		{
			i32vec2NumMicroSteps.y -= movementspeed;
			if (i32vec2NumMicroSteps.y < 0)
			{
				i32vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Constraint the EnemySawCon's position within the screen boundary
		Constraint(DOWN);

		// Find a feasible position for the EnemySawCon's current position
		if (CheckPosition(DOWN) == false)
		{
			FlipVerticleDirection();
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.y = 0;
		}
		InteractWithPlayer();
	}
}


bool CEnemySawCon::playerNewlyVec(glm::vec2 oldvec)
{
	if (oldvec != spotDestination)  //meaning player get a new vec 
	{
		spotDestination = oldvec;
	return true;
	}
	else
		return false;
}





