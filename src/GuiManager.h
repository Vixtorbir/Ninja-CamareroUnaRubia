#pragma once

#include "Module.h"
#include "GuiControl.h"

#include <list>

class GuiManager : public Module
{
public:

	// Constructor
	GuiManager();

	// Destructor
	virtual ~GuiManager();

	// Called before the first frame
	 bool Start();

	 void ClearControlsOfType(GuiControlType type);

	 // Called each loop iteration
	 bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	// Additional methods
	GuiControl* CreateGuiControl(GuiControlType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds = { 0,0,0,0 });

	GuiControl* CreateGuiImage(GuiControlType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Texture* texture);

public:

	std::list<GuiControl*> guiControlsList;
	SDL_Texture* texture;

	//Audio
	

};

