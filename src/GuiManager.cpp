#include "GuiManager.h"
#include "Engine.h"
#include "Textures.h"

#include "GuiControlButton.h"
#include "Dialogue.h"
#include "Portrait.h"

#include "Audio.h"

GuiManager::GuiManager() :Module()
{
	name = "guiManager";
}

GuiManager::~GuiManager() {}

bool GuiManager::Start()
{
	return true;
}

// Create a new GUI control and add it to the list of controls
GuiControl* GuiManager::CreateGuiControl(GuiControlType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds)
{
	GuiControl* guiControl = nullptr;

	// Call the constructor according to the GuiControlType
	switch (type)
	{
	case GuiControlType::BUTTON:
		guiControl = new GuiControlButton(id, bounds, text);
		break;
	case GuiControlType::DIALOGUE:
		guiControl = new Dialogue(id, bounds, text);
		break;
	case GuiControlType::PORTRAIT:
		guiControl = new Portrait(id, bounds, text);
		break;
	}

	// Set the observer
	guiControl->observer = observer;

	// Add the created GuiControl to the list of controls
	guiControlsList.push_back(guiControl);

	return guiControl;
}

// Clear all GUI controls of a specific type
void GuiManager::ClearControlsOfType(GuiControlType type)
{
	auto it = guiControlsList.begin();
	delete* it;
	it = guiControlsList.erase(it);

}

bool GuiManager::Update(float dt)
{
	for (const auto& control : guiControlsList)
	{
		control->Update(dt);
	}

	return true;
}

bool GuiManager::CleanUp()
{
	for (const auto& control : guiControlsList)
	{
		delete control;
	}
	guiControlsList.clear();

	return true;
}