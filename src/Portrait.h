#pragma once

#include "GuiControl.h"
#include "Vector2D.h"


class Portrait : public GuiControl
{

public:

	Portrait(int id, SDL_Rect bounds, const char* text);
	virtual ~Portrait();

	// Called each loop iteration
	bool Update(float dt);

private:

	SDL_Texture* Hanzo = NULL;

	bool canClick = true;
	bool drawBasic = false;
};

#pragma once