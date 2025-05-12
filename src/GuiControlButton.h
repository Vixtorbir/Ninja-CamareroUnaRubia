#pragma once

#include "GuiControl.h"
#include "Vector2D.h"
#include "Engine.h"

class GuiControlButton : public GuiControl
{

public:

	GuiControlButton(int id, SDL_Rect bounds, const char* text);
	GuiControlButton(int id, SDL_Rect bounds, const char* text, bool optionA, bool optionB);
	virtual ~GuiControlButton();

	bool Start();
	bool Update(float dt);
	void CleanUp();

	SDL_Texture* texture;
	SDL_Texture* textureSelected;
	bool isClicked = false;
	bool isOptionA = false;
	bool isOptionB = false;

	bool visible = true;

private:
	bool canClick = true;
	bool drawBasic = false;

	//Audio
	int buttonSelectedFxId;
	int buttonPressedFxId;

	bool fxPlayed = false;
	bool pressed = false;
};

#pragma once