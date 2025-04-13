#pragma once

#include "GuiControl.h"
#include "Vector2D.h"
#include "Engine.h"

class Text : public GuiControl
{

public:

	Text(int id, SDL_Rect bounds, const char* text);
	Text(int id, SDL_Rect bounds, const char* text, bool optionA, bool optionB);
	virtual ~Text();

	bool Start();
	bool Update(float dt);
	void SetText(const std::string& text);
	void CleanUp();

	SDL_Texture* texture;
	SDL_Texture* textureSelected;
	bool isClicked = false;
	bool isOptionA = false;
	bool isOptionB = false;
private:
	bool canClick = true;
	bool drawBasic = false;
	std::string currentText = "";
	//Audio
	int buttonSelectedFxId;
	int buttonPressedFxId;

	bool fxPlayed = false;
	bool pressed = false;
};

#pragma once