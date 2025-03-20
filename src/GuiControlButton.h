#pragma once

#include "GuiControl.h"
#include "Vector2D.h"
#include "Engine.h"

class GuiControlButton : public GuiControl
{

public:

	GuiControlButton(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiControlButton();

	bool Start();
	bool Update(float dt);
	void CleanUp();

	SDL_Texture* texture;
	bool isClicked = false;

private:
	
	bool canClick = true;
	bool drawBasic = false;

	
};

#pragma once