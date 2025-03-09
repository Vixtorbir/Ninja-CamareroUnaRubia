#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Title.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"
#include "Enemy.h"
#include "GuiControl.h"
#include "GuiManager.h"
#include "DialogueManager.h"

Title::Title() : Module()
{
	name = "scene";
}

// Destructor
Title::~Title()
{}

// Called before render is available
bool Title::Awake()
{

	LOG("Loading Title");
	bool ret = true;

	SDL_Rect btPos = { 520, 350, 120,20 };
	guiBt = (GuiControlButton*) Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "ni", btPos, this);
	
	
	return ret;
}

// Called before the first frame
bool Title::Start()
{
	//L06 TODO 3: Call the function to load the map. 

	return true;
}

// Called each loop iteration
bool Title::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Title::Update(float dt)
{



	return true;
}

// Called each loop iteration
bool Title::PostUpdate()
{
	bool ret = true;


	return ret;
}

// Called before quitting
bool Title::CleanUp()
{
	LOG("Freeing Title");
	return true;
}


// L15 TODO 1: Implement the Load function

bool Title::OnGuiMouseClickEvent(GuiControl* control)
{
	// L15: DONE 5: Implement the OnGuiMouseClickEvent method
	LOG("Press Gui Control: %d", control->id);

	return true;
}

