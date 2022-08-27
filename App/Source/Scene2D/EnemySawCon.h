/**
 CEnemySawCon
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

 // Include shader
#include "RenderControl\shader.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Settings
#include "GameControl\Settings.h"

// Include Physics2D
#include "Physics2D.h"

// Include Player2D
#include "Player2D.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

#include "Camera.h"

#include "../SoundController/SoundController.h"

#include "InventoryManager.h"

#include "Rays.h"

class CEnemySawCon : public CEntity2D
{
public:
	// Constructor
	CEnemySawCon(void);

	// Destructor
	virtual ~CEnemySawCon(void);

	// Init
	bool Init(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	// Set the indices of the EnemyFalseKnight
	void Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis);

	// Set the number of microsteps of the EnemyFalseKnight
	void Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis);

	// Set the UV coordinates of the EnemyFalseKnight
	void Setvec2UVCoordinates(const float fUVCoordinate_XAxis, const float fUVCoordinate_YAxis);

	// Get the indices of the EnemyFalseKnight
	glm::vec2 Geti32vec2Index(void) const;

	// Get the number of microsteps of the EnemyFalseKnight
	glm::vec2 Geti32vec2NumMicroSteps(void) const;

	// Set the UV coordinates of the EnemyFalseKnight
	glm::vec2 Getvec2UVCoordinates(void) const;

	// Set the handle to cPlayer to this class instance
	void SetPlayer2D(CPlayer2D* cPlayer2D);

	// boolean flag to indicate if this enemy is active
	bool bIsActive;

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	enum FSM
	{
		IDLE = 0,
		PATROL = 1,
		ATTACK = 2,
		COOLDOWN = 3,
		SCARED = 4,
		INVESTIGATE = 5,
		NUM_FSM
	};

	glm::vec2 i32vec2OldIndex;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// The vec2 which stores the indices of the EnemyFalseKnight in the Map2D
	glm::vec2 i32vec2Index;

	// The vec2 variable which stores The number of microsteps from the tile indices for the EnemyFalseKnight. 
	// A tile's width or height is in multiples of these microsteps
	glm::vec2 i32vec2NumMicroSteps;

	// The vec2 variable which stores the UV coordinates to render the EnemyFalseKnight
	glm::vec2 vec2UVCoordinate;

	// The vec2 which stores the indices of the destination for EnemySawCon in the Map2D
	glm::vec2 i32vec2Destination;
	// The vec2 which stores the direction for EnemySawCon movement in the Map2D
	glm::vec2 i32vec2Direction;
	//store the indices of a spot
	glm::vec2 spotDestination;
	//store the direction for enemysawcon to spot
	glm::vec2 spotDirection;

	// Settings
	CSettings* cSettings;

	CInventoryManager* cInventoryManager;

	// Physics
	CPhysics2D cPhysics2D;

	// Current color
	glm::vec4 runtimeColour;

	// Handle to the CPlayer2D
	CPlayer2D* cPlayer2D;
	
	CSoundController* cSoundController;

	//CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	Ray* rays;

	// Current FSM
	FSM sCurrentFSM;

	// FSM counter - count how many frames it has been in this FSM
	int iFSMCounter;

	// Max count in a state
	const int iMaxFSMCounter = 60;

	// Health for player attack
	int health;
	// Armour
	int armour;

	// Short iframe after taking damage
	double iframeElapsed;

	double lastAttackElapsed;


	// Constraint the EnemyFalseKnight's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	// Check if a position is possible to move into
	bool CheckPosition(DIRECTION eDirection);

	// Let EnemyFalseKnight interact with the player
	bool InteractWithPlayer(void);

	// Update direction
	void UpdateDirection(void);

	//// Update direction to a spot
	//void UpdateDirectionSpot(void);

	// Flip horizontal direction. For patrol use only
	void FlipHorizontalDirection(void);

	// Flip verticle direction. For patrol use only
	void FlipVerticleDirection(void);
	// Update position
	void UpdatePosition(void);
	int dir; //Store current direction for animation

	float chaseRange;//range for the enemy to chase the player
	float atkrange;//range for the enemy to atk the player
	double movementspeed;//change via the microsteps 
	double increasespeed;//the more collectable, the more faster enemy gets

	int AtkCounter; // atk counter - count how many frames it has been in this FSM
	const int MaxAtkCounter = 150;// Max count in a state
	int ScaredCounter; // atk counter - count how many frames it has been in this FSM
	const int MaxScaredCounter = 60;// Max count in a state
	int InvestigateCounter; // investigate counter - count how many frames it has been in this FSM
	const int MaxInvestigateCounter = 120;// Max count in a state

	bool playerInteractWithBox;//interact with box







	bool shun;//player shun light onto enemy
	bool sawPlayer;//player saw enemy












	bool playerNewlyVec(glm::vec2 oldvec);//check if the player got a new collectable, if so, get the pos of that collectable
	void UpdatePositionPatrol(void);
	void RandDirection(void);
	void UpdateDirectionRun(void);
	void EnemySpeedUp(double &movementspeed);
	bool displaytest;//toggle on couts
	bool pathtest;//cout for testing enemy a* pathing to check whether it goes to the latest spotdestination
	bool statetest;//cout for testing enemy state

	Camera* camera;
};

