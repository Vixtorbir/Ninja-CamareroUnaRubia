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
    showStartButton = true; // Initialize the button as visible
}

// Destructor
Title::~Title()
{
}

// Called before render is available
bool Title::Awake()
{
    LOG("Loading Title");
    bool ret = true;

    return ret;
}

// Called before the first frame
bool Title::Start()
{

    if (showStartButton)
    {
        SDL_Rect btPos = { 520, 350, 120, 20 };
        guiBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "", btPos, this);
        LOG("Start button created with ID: %d", guiBt->id); // Debug log
    }

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
    // Hide the button when the "Enter" key is pressed
    if (showStartButton && Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_DOWN)
    {
        LOG("Enter key pressed, hiding Start button"); // Debug log
        showStartButton = false;
        Engine::GetInstance().guiManager->ClearControlsOfType(GuiControlType::BUTTON);
    }

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

// Handle GUI click events
bool Title::OnGuiMouseClickEvent(GuiControl* control)
{
    LOG("GUI Control Clicked: ID = %d", control->id); // Debug log

    // Check if the clicked control is the "Start" button
    if (control->id == 1) // Assuming the "Start" button has ID 1
    {
        LOG("Start button clicked, hiding button"); // Debug log
        showStartButton = false; // Hide the button
        Engine::GetInstance().guiManager->ClearControlsOfType(GuiControlType::BUTTON); // Remove the button from the GUI
    }

    return true;
}