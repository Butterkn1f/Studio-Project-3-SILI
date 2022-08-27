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
	cMap2D->SetMapInfo(uiRow, uiCol, 2);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	// Load the player texture 
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/scene2d_player2.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/scene2d_player.png" << endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation
	//NOTE, TODO: FOR DOUBLE SPRITE HEIGHT JUST DO CSETTINGS->TILE_HEIGHT * 2
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(8, 20, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("right", 0, 19);
	animatedSprites->AddAnimation("left", 20, 39);
	animatedSprites->AddAnimation("up", 40, 59);
	animatedSprites->AddAnimation("down", 60, 79);
	animatedSprites->AddAnimation("idleRight", 80, 99);
	animatedSprites->AddAnimation("idleLeft", 100, 119);
	animatedSprites->AddAnimation("idleDown", 120, 139);
	animatedSprites->AddAnimation("idleUp", 140, 159);
	//CS: Play the "idle" animation as default
	// PlayAnimation(animName, loopCount, every ? seconds). Eg, "down", 5, 1.0f means loop anim 5 times every 1s. -1 loops infinitely/
	animatedSprites->PlayAnimation("idleUp", -1, 0.8f);


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
	//*********** SP3 STUFF ************
	boxElapsed = 0;
	dir = DIRECTION::UP;
	
	//Variables
	AllNumbersCollected = false;

	// Get the handler to the CINventoryManager instance
	cInventoryManager = CInventoryManager::GetInstance();

	// Add a Health icon as one of the inventory items
	cInventoryItem = cInventoryManager->Add("Health", "Image/Heart.png", 3, 3);
	cInventoryItem->vec2Size = glm::vec2(40, 40);

	//cInventoryItem = cInventoryManager->Add("Soul", "Image/GUI_Soul.png", 100, 0);
	//cInventoryItem->vec2Size = glm::vec2(200, 100);

	//Inventory item Papers
	cInventoryItem = cInventoryManager->Add("Paper", "Image/passcode.png", 10, 0);
	cInventoryItem->vec2Size = glm::vec2(40, 40);

	// Get handler for sound controller
	cSoundController = CSoundController::GetInstance();

	eBox = false;
	tempOldVec = glm::vec2(0, 0);
	collected = false;
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
	animatedSprites->PlayAnimation("idleRight", -1, 1.f);

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


	if (cKeyboardController->IsKeyDown(GLFW_KEY_A))
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
		cSoundController->PlaySoundByID(10);
			
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_D))
	{
		dir = DIRECTION::RIGHT;
		//vec2Index.x++;
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
		animatedSprites->PlayAnimation("right", -1, 0.1f);
		cSoundController->PlaySoundByID(10);
	}

	if (cKeyboardController->IsKeyDown(GLFW_KEY_W))
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
		animatedSprites->PlayAnimation("up", -1, 0.2f);
		cSoundController->PlaySoundByID(10);
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_S))
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
		animatedSprites->PlayAnimation("down", -1, 0.2f);
		cSoundController->PlaySoundByID(10);
	}

	
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_A) || cKeyboardController->IsKeyReleased(GLFW_KEY_D) || cKeyboardController->IsKeyReleased(GLFW_KEY_W) || cKeyboardController->IsKeyReleased(GLFW_KEY_S))
	{
		//CS: Play the "idle" animation by default, if not jumping/falling as well
		if (dir == DIRECTION::LEFT)
			animatedSprites->PlayAnimation("idleLeft", -1, 1.f);
		else if (dir == DIRECTION::RIGHT)
			animatedSprites->PlayAnimation("idleRight", -1, 1.f);
		else if (dir == DIRECTION::UP)
			animatedSprites->PlayAnimation("idleUp", -1, 1.f);
		else if (dir == DIRECTION::DOWN)
			animatedSprites->PlayAnimation("idleDown", -1, 1.f);
	}
	
	//If stopped walking, stop the walking sound and go back to idle state
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_LEFT) || cKeyboardController->IsKeyReleased(GLFW_KEY_RIGHT))
	{
		cSoundController->StopSoundByID(10);
	}

	//ilan box nonsense hihihihi
	boxElapsed += 0.01;
	int offsetX = 8;	//The offset for X microsteps for the player to be in the middle of two tiles, i.e player looks like hes above a object but his index is one lesser/higher than the object.
	int offsetY = 6;	//The offset for Y microsteps for the player to be in the middle of two tiles, i.e player looks like hes to the right of an object but his index is one lesser/higher than the object.
	//cout <<"X: "<< vec2Index.x << endl;
	//cout <<"Y: " << vec2Index.y << endl;
	if (cKeyboardController->IsKeyDown(GLFW_KEY_E) && boxElapsed > 0.5)
	{
		//If box is on the left of character
		if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 1) == 110 &&				//if box is directly towards the left of player
			cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 2) == 2)
		{
			cout << "player push box left" << endl;
			cSoundController->PlaySoundByID(6);
			setEBox(true);
			//Set box into new position
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 2, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x - 1, 2);
			boxElapsed = 0;
		}
		else if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x - 1) == 110 &&		//if player is halfway below
			vec2NumMicroSteps.y >= offsetY &&
			cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x - 2) == 2)
		{
			cout << "player push box left" << endl;
			cSoundController->PlaySoundByID(6);
			setEBox(true);
			//Set box into new position
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x - 2, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x - 1, 2);
			boxElapsed = 0;
		}

		//If box is on the right of character
		else if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) == 110 &&			//if box is directly towards the right of player
			cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 2) == 2)
		{
			cout << "player push box right" << endl;
			cSoundController->PlaySoundByID(6);
			setEBox(true);
			//Set box into new position
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 2, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x + 1, 2);
			boxElapsed = 0;
		}
		else if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) == 110 &&		//if player is halfway below
			vec2NumMicroSteps.y >= offsetY &&
			cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 2) == 2)
		{
			cout << "player push box right" << endl;
			cSoundController->PlaySoundByID(6);
			setEBox(true);
			//Set box into new position
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x + 2, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x + 1, 2);
			boxElapsed = 0;
		}

		//If box is below player
		else if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 110 &&		//if box directly above
			vec2NumMicroSteps.x <= offsetX &&
			cMap2D->GetMapInfo(vec2Index.y - 2, vec2Index.x) == 2)
		{
			cout << "player push box down" << endl;
			cSoundController->PlaySoundByID(6);
			setEBox(true);
			//Set box into new position
			cMap2D->SetMapInfo(vec2Index.y - 2, vec2Index.x, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(vec2Index.y - 1, vec2Index.x, 2);
			boxElapsed = 0;
		}
		else if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x + 1) == 110 &&		//if player is on the top left
			vec2NumMicroSteps.x >= offsetX &&
			cMap2D->GetMapInfo(vec2Index.y - 2, vec2Index.x + 1) == 2)
		{
			cout << "player push box down" << endl;
			cSoundController->PlaySoundByID(6);
			setEBox(true);
			//Set box into new position
			cMap2D->SetMapInfo(vec2Index.y - 2, vec2Index.x + 1, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(vec2Index.y - 1, vec2Index.x + 1, 2);
			boxElapsed = 0;
		}


		//If box is above player
		else if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) == 110 &&			//if box directly above player
			vec2NumMicroSteps.x <= offsetX &&
			cMap2D->GetMapInfo(vec2Index.y + 2, vec2Index.x) == 2)
		{
			cout << "player push box up" << endl;
			setEBox(true);
			//Set box into new position
			cMap2D->SetMapInfo(vec2Index.y + 2, vec2Index.x, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x, 2);
			boxElapsed = 0;
			cSoundController->PlaySoundByID(6);
		}
		else if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) == 110 &&			//if player is on bottom left of the box
			vec2NumMicroSteps.x >= offsetX &&
			cMap2D->GetMapInfo(vec2Index.y + 2, vec2Index.x + 1) == 2)
		{
			cout << "player push box up" << endl;
			setEBox(true);
			//Set box into new position
			cMap2D->SetMapInfo(vec2Index.y + 2, vec2Index.x + 1, 110);
			//Remove box from original position
			cMap2D->SetMapInfo(vec2Index.y + 1, vec2Index.x + 1, 2);
			boxElapsed = 0;
			cSoundController->PlaySoundByID(6);
		}

	}


	//Win condition
	cInventoryItem = cInventoryManager->GetItem("Paper");
	if (cInventoryItem->GetCount() == cInventoryItem->GetMaxCount())
		cMap2D->SetMapInfo(43, 1, 91);

	UpdateHealthLives();

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
	//unsigned int MVLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "MV");
	//unsigned int inverseLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "MV_inverse_transpose");
	//unsigned int ambientLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "material.kAmbient");
	//unsigned int diffuseLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "material.kDiffuse");
	//unsigned int specularLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "material.kSpecular");
	//unsigned int shininessLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "material.kShininess");

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

	//glm::mat4 MV = camera->GetMV();
	//glm::mat4 transformMV = MV; // init to original matrix
	//transformMV = glm::translate(transformMV, glm::vec3(vec2UVCoordinate.x,
	//	vec2UVCoordinate.y,
	//	0.0f));
	//glUniformMatrix4fv(MVLoc, 1, GL_FALSE, &transformMV[0][0]);

	//glm::mat4 MV_inverse_transpose = glm::transpose(glm::inverse(transformMVP));
	//glUniformMatrix4fv(inverseLoc, 1, GL_FALSE, &MV_inverse_transpose[0][0]);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	//glUniform3fv(ambientLoc, 1, &quadMesh->material.kAmbient.r);
	//glUniform3fv(diffuseLoc, 1, &quadMesh->material.kDiffuse.r);
	//glUniform3fv(specularLoc, 1, &quadMesh->material.kSpecular.r);
	//glUniform1f(shininessLoc, quadMesh->material.kShininess);

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

	case 77:
	case 76:
	case 75:
		setOldVec(vec2Index);
		cout << "player get x" << tempOldVec.x << "y" << tempOldVec.y << endl;
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);	
		cInventoryItem = cInventoryManager->GetItem("Paper");
		cInventoryItem->Add(1);
		cSoundController->PlaySoundByID(13);
		collected = true;
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
	if (cInventoryItem->GetCount() < 0)
	{
		animatedSprites->PlayAnimation("death", 1, 3.0f);
		runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
		iframeElapsed = 0;
		deadElapsed += 0.01;

		CGameManager::GetInstance()->bPlayerLost = true;
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

void CPlayer2D::setEBox(bool pressE)
{
	eBox = pressE;
}

bool CPlayer2D::getEBox()
{
	return eBox;
}

glm::vec2 CPlayer2D::getOldVec()
{
	return tempOldVec;
}

void CPlayer2D::setOldVec(glm::vec2 newVector)
{
	tempOldVec = newVector;
}

bool CPlayer2D::getCollected()
{
	return collected;
}

void CPlayer2D::setCollected(bool collect)
{
	collected = collect;
}
