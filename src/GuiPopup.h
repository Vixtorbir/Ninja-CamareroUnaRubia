#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class GuiPopup : public GuiControl
{

public:

	GuiPopup(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiPopup();

	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	bool isActive = false;

private:
	bool canClick = true;
	bool drawBasic = false;
};

#pragma once