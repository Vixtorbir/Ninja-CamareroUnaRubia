#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class Dialogue : public GuiControl
{

public:

	Dialogue(int id, SDL_Rect bounds, const char* text);
	virtual ~Dialogue();

	// Called each loop iteration
	bool Update(float dt);

private:

	bool canClick = true;
	bool drawBasic = false;
};

#pragma once