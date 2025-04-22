#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class GuiCheckbox : public GuiControl
{

public:

	GuiCheckbox(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiCheckbox();

	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	bool isActive = false;

	SDL_Texture* CheckboxTexture;
	SDL_Texture* CheckedTexture;
	bool isClicked = false;

	int buttonSelectedFxId;
	int buttonPressedFxId;

	bool fxPlayed = false;

private:
	bool canClick = true;
	bool drawBasic = false;

};

#pragma once