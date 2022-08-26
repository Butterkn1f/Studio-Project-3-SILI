/**
 Rays
 @brief A class that handles the rendering of flashlight's rays
 */
#include "Rays.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

#include "Primitives/MeshBuilder.h"

//To get player position to translate the flashlight
#include "Player2D.h"

// Mouse controller to rotate flashlight according to mouse position
#include "Inputs\MouseController.h"


/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
Rays::Rays(void)
	: camera(NULL)
{
	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
Rays::~Rays(void)
{
	camera = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool Rays::Init(void)
{
	// Get handler for camera
	camera = Camera::GetInstance();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Load the player texture 
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/flashlight.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/flashlight.png" << endl;
		return false;
	}

	// Create the quad mesh for the rays
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//Default values for rays
	//TODO: Maybe a get function then -= when intersect
	rays[0].length = 0.26f;
	rays[0].angle = 0.f;
	rays[1].length = 0.24f;
	rays[1].angle = 20.f;
	rays[2].length = 0.24f;
	rays[2].angle = -20.f;

	return true;
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void Rays::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use("Shader2D");
}

/**
 @brief Render this instance
 */
void Rays::Render(void)
{
	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");

	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	//CS: Render the rays
	for (int i = 0; i < (sizeof(rays) / sizeof(rays[0])); i++)
	{
		quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), 0.0025f, rays[i].length);

		glm::mat4 MVP = camera->GetMVP();
		glm::mat4 transformMVP;
		transformMVP = MVP; // make sure to initialize matrix to identity matrix first

		float xTranslate = CPlayer2D::GetInstance()->vec2UVCoordinate.x;
		float yTranslate = CPlayer2D::GetInstance()->vec2UVCoordinate.y;

		float dy = CMouseController::GetInstance()->GetMousePositionY() - cSettings->iWindowHeight * 0.5;
		float dx = CMouseController::GetInstance()->GetMousePositionX() - cSettings->iWindowWidth * 0.5;
		float overallRotate = atan2(dx, dy) + glm::radians(180.f);

		switch (CPlayer2D::GetInstance()->dir)
		{
		//LEFT
		case 0:
			yTranslate += rays[i].angle * 0.00015;

			//Limit rotation of flashlight
			if (overallRotate > glm::radians(180.f) && overallRotate < glm::radians(270.f))
				overallRotate = glm::radians(180.f);
			else if (overallRotate < glm::radians(360.f) && overallRotate > glm::radians(270.f))
				overallRotate = glm::radians(360.f);
			break;

		//RIGHT
		case 1:
			yTranslate -= rays[i].angle * 0.00015;

			//Limit rotation of flashlight
			if (overallRotate > glm::radians(0.f) && overallRotate < glm::radians(90.f))
				overallRotate = glm::radians(0.f);
			else if (overallRotate < glm::radians(180.f) && overallRotate > glm::radians(90.f))
				overallRotate = glm::radians(180.f);
			break;

		//UP
		case 2:
			xTranslate += rays[i].angle * 0.00018;
			//Limit rotation of flashlight
			if (overallRotate > glm::radians(90.f) && overallRotate < glm::radians(180.f))
				overallRotate = glm::radians(90.f);
			else if (overallRotate < glm::radians(270.f) && overallRotate > glm::radians(180.f))
				overallRotate = glm::radians(-90.f);

			//overallRotate += glm::radians(rays[i].angle);
			break;

		//DOWN
		case 3:
			xTranslate -= rays[i].angle * 0.00018;
			//Limit rotation of flashlight
			if (overallRotate > glm::radians(270.f) && overallRotate < glm::radians(360.f))
				overallRotate = glm::radians(270.f);
			else if (overallRotate < glm::radians(90.f) && overallRotate > glm::radians(0.f))
				overallRotate = glm::radians(90.f);

			//overallRotate += glm::radians(rays[i].angle);
			break;
		default:
			break;
		}

		// Update the shaders with the latest transform
		transformMVP = glm::translate(transformMVP, glm::vec3(xTranslate,
			yTranslate,
			0.0f));
		transformMVP = glm::rotate(transformMVP, overallRotate, glm::vec3(0, 0, 1));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformMVP));
		glUniform4fv(colorLoc, 1, glm::value_ptr(glm::vec4(1.f, 1.f, 1.f, 1.f)));

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		// Get the texture to be rendered
		glBindTexture(GL_TEXTURE_2D, iTextureID);

		glBindVertexArray(VAO);
		quadMesh->Render();
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void Rays::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}