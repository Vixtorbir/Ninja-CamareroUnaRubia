#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class GuiControlButton : public GuiControl
{

public:
	GuiControlButton(int id, SDL_Rect bounds, const char* text, bool optionA, bool optionB);

	GuiControlButton(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiControlButton();

	bool Start();

	// Called each loop iteration
	bool Update(float dt);
	bool isOptionA = false;
	bool isOptionB = false;
private:

	SDL_Texture* texture;
	
	bool canClick = true;
	bool drawBasic = false;
};

#pragma once