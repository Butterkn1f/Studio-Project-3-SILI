/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_Scene2D.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::CGUI_Scene2D(void)
	: cSettings(NULL)
	, window_flags(0)
	, m_fProgressBar(0.0f)
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::~CGUI_Scene2D(void)
{
	if (cInventoryManager)
	{
		cInventoryManager->Destroy();
		cInventoryManager = NULL;
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// We won't delete this since it was created elsewhere
	cSettings = NULL;
}

/**
  @brief Initialise this instance
  */
bool CGUI_Scene2D::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Store the CFPSCounter singleton instance here
	cFPSCounter = CFPSCounter::GetInstance();

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

	// Define the window flags
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	//// Show the mouse pointer
	//glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Initialise the cInventoryManager
	cInventoryManager = CInventoryManager::GetInstance();
	// Add a Geo as one of the inventory items
	cInventoryItem = cInventoryManager->Add("Geo", "Image/GUI_Geo.png", 10000, 0);
	cInventoryItem->vec2Size = glm::vec2(40, 40);

	// These variables are for IMGUI demo only
	show_demo_window = true;
	show_another_window = true;
	clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	m_fProgressBar = 0.0f;

	return true;
}

/**
 @brief Update this instance
 */
void CGUI_Scene2D::Update(const double dElapsedTime)
{
	// Calculate the relative scale to our default windows width
	const float relativeScale_x = cSettings->iWindowWidth / 1366.0f;
	const float relativeScale_y = cSettings->iWindowHeight / 768.0f;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Create an invisible window which covers the entire OpenGL window
	ImGui::Begin("Invisible window", NULL, window_flags);
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);

	// Display the FPS
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %d", cFPSCounter->GetFrameRate());

	// Render a progress bar
	/*m_fProgressBar += 0.001f;
	if (m_fProgressBar > 1.0f)
		m_fProgressBar = 0.0f;
	ImVec4 col = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, col);
	col = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, col);
		ImGui::ProgressBar(cInventoryItem->GetCount() /
			(float)cInventoryItem->GetMaxCount(), ImVec2(100.0f * relativeScale_x, 20.0f * relativeScale_y));
		ImGui::PopStyleColor();
	ImGui::PopStyleColor();*/

	// Render Soul
	ImGuiWindowFlags soulWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Soul", NULL, soulWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.03f,
		cSettings->iWindowHeight * 0.03f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	cInventoryItem = cInventoryManager->GetItem("Soul");
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
			cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();

	ImGui::Begin("SoulProgress", NULL, soulWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.11f,
		cSettings->iWindowHeight * 0.1f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.000, 0.961, 0.933, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.412, 0.412, 0.412, 1.0f));
	ImGui::ProgressBar(cInventoryItem->GetCount() /
		(float)cInventoryItem->GetMaxCount(), ImVec2(300.0f *
			relativeScale_x, 20.0f * relativeScale_y), "Soul");
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::End();

	// Render Health
	ImGuiWindowFlags healthWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Health", NULL, healthWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.12f,
		cSettings->iWindowHeight * 0.05f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	cInventoryItem = cInventoryManager->GetItem("Health");

	for (int i = 0; i < cInventoryItem->GetCount(); i++)
	{
		ImGui::SameLine();
		ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
			cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	}
	ImGui::End();

	//// Render the inventory items
	cInventoryItem = cInventoryManager->GetItem("Geo");
	ImGuiWindowFlags inventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Image", NULL, inventoryWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.05f, cSettings->iWindowHeight * 0.16f));
	ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x, cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(2.f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "%d", cInventoryItem->GetCount());
	ImGui::End();


	//// Render the inventory items
	cInventoryItem = cInventoryManager->GetItem("Paper");
	ImGuiWindowFlags PaperWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Image", NULL, PaperWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.05f, cSettings->iWindowHeight * 0.8f));
	ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x, cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(2.f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "Paper: %d / %d", cInventoryItem->GetCount() , cInventoryItem->GetMaxCount());
	ImGui::SameLine();
	ImGui::End();

	ImGui::End();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! 
// You can browse its code to learn more about Dear ImGui!).
	//if (show_demo_window)
	//	ImGui::ShowDemoWindow(&show_demo_window);

	//// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	//{
	//	static float f = 0.0f;
	//	static int counter = 0;

	//	// Create a window called "Preview IMGUI" and append into it.
	//	ImGui::Begin("Preview IMGUI", NULL);
	//	ImGui::SetWindowPos(ImVec2(10.0f, 10.0f));				// Set the top-left of the window at (10,10)
	//	ImGui::SetWindowSize(ImVec2(300.0f, 200.0f));			// Set the window size to (300,200)

	//	ImGui::Text("Toggle IMGUI demo.");						// Display some text (you can use a format strings too)
	//	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	//	ImGui::Checkbox("Another Window", &show_another_window);

	//	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	//	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	//	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
	//		counter++;
	//	ImGui::SameLine();
	//	ImGui::Text("counter = %d", counter);

	//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
	//		1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//	ImGui::End();
	//}

	//// 3. Show another simple window.
	//if (show_another_window)
	//{
	//	ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	//	ImGui::Text("Hello from another window!");
	//	if (ImGui::Button("Close Me"))
	//		show_another_window = false;
	//	ImGui::End();
	//}
}


/**
 @brief Set up the OpenGL display environment before rendering
 */
void CGUI_Scene2D::PreRender(void)
{
}

/**
 @brief Render this instance
 */
void CGUI_Scene2D::Render(void)
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CGUI_Scene2D::PostRender(void)
{
}
