#include "GuiManager.h"
#include "Engine.h"
#include "Textures.h"

#include "GuiControlButton.h"
#include "Dialogue.h"
#include "Portrait.h"
#include "GuiPopup.h"
#include "GuiSlider.h"
#include "GuiCheckbox.h"
#include "Audio.h"
#include "GuiImage.h"
#include "Textures.h"
#include "Text.h"
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
	case GuiControlType::CHECKBOX:
		guiControl = new GuiCheckbox(id, bounds, text);
		break;
	case GuiControlType::DIALOGUE:
		guiControl = new Dialogue(id, bounds, text);
		break;
	case GuiControlType::PORTRAIT:
		guiControl = new Portrait(id, bounds, text);
		break;
	case GuiControlType::POPUP:
		guiControl = new GuiPopup(id, bounds, text);
		break;
	case GuiControlType::OPTIONA:
		guiControl = new GuiControlButton(id, bounds, text, true, false);
		break;
	case GuiControlType::OPTIONB:
		guiControl = new GuiControlButton(id, bounds, text, false, true);
		break;
	case GuiControlType::HPSLIDER:
		guiControl = new GuiSlider(id, bounds, text, 0, 100, 100);
		break;
	case GuiControlType::TEXT:
		guiControl = new Text(id, bounds, text);
		break;
	}

	// Set the observer
	guiControl->observer = observer;

	guiControlsList.push_back(guiControl);

	return guiControl;
}
GuiControl* GuiManager::CreateGuiImage(GuiControlType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Texture* texture)
{
	GuiControl* guiControl = nullptr;

	// Call the constructor according to the GuiControlType
	switch (type)
	{
	case GuiControlType::IMAGE:
		guiControl = new GuiImage(id, bounds, text, texture);
		break;
	}

	// Set the observer
	guiControl->observer = observer;

	guiControlsList.push_back(guiControl);

	return guiControl;
}
void GuiManager::ClearControlsOfType(GuiControlType type)
{
	auto it = guiControlsList.begin();
	while (it != guiControlsList.end())
	{
		GuiControl* control = *it;

		if (!control->unkillable)
		{
			delete control;
			it = guiControlsList.erase(it);
		}
		else
		{
			++it;
		}
	}
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