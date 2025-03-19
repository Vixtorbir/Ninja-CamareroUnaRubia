#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class GuiControlButton : public GuiControl
{

public:

	GuiControlButton(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiControlButton();

	bool Start();

	// Called each loop iteration
	bool Update(float dt);
	void Render();


private:

	bool canClick = true;
	bool drawBasic = false;
	SDL_Texture* texture;
	const char* iftext;
};

#pragma once