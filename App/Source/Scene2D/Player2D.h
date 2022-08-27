/**
 CPlayer2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include Singleton template
#include "DesignPatterns\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

#include "..\SoundController\SoundController.h"

// Include Physics2D
#include "Physics2D.h"

// Include Inventory Manager
#include "InventoryManager.h"

#include "GameManager.h"

#include "Camera.h"

class CPlayer2D : public CSingletonTemplate<CPlayer2D>, public CEntity2D
{
	friend CSingletonTemplate<CPlayer2D>;
public:
	int dir; //Store current direction for animation, as well as interaction with enemy

	// Init
	bool Init(void);

	// Reset
	bool Reset(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	//To be used by enemy classes, damage player, set iframe, and play sound
	void DamagePlayer(int eDirection = 0);

	//Damage enemies
	bool isAttacking(void);

	void setEBox(bool pressE);
	bool getEBox();

	glm::vec2 getOldVec();
	void setOldVec(glm::vec2 newVector);
	
	bool getCollected();
	void setCollected(bool collect);

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	glm::vec2 vec2OldIndex;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	//CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	// Physics
	CPhysics2D cPhysics2D;

	// Inventory Manager
	CInventoryManager* cInventoryManager;

	// InventoryItem
	CInventoryItem* cInventoryItem;

	CGameManager* cGameManager;

	Camera* camera;

	// Player's colour
	glm::vec4 runtimeColour;

	// Count the number of jumps
	int iJumpCount;

	// time of last attack
	double lastAttackElapsed;
	// ensure that player can only break one item per attack press
	int iAttackCount;
	// Short iframe after taking damage
	double iframeElapsed;
	//Pause game for 0.5s when dead to play animation
	double deadElapsed;
	//Focus delay, resets each time trying to focus, and auto ends after a couple seconds
	double focusElapsed;


	//******************* SP3 stuff ************************************
	//Short cooldown for the box to be pushed cos im an a hole. 
	double boxElapsed;
	//If all collectibles are collected, make this shiz true
	bool AllNumbersCollected;

	// Constructor
	CPlayer2D(void);

	// Destructor
	virtual ~CPlayer2D(void);

	// Constraint the player's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	// Check if a position is possible to move into
	bool CheckPosition(DIRECTION eDirection);

	// Update Jump or Fall

	// Check if the player is in mid-air

	// Let player interact with the map
	void InteractWithMap(void);

	// Update Health and Lives
	void UpdateHealthLives(void);

	// Update breakable objects when player attacks

	bool eBox;

	//old position of player
	glm::vec2 tempOldVec;


	bool collected;//if player just collected a paper

};

