#pragma once

#include "GuiControl.h"
#include "Vector2D.h"
#include "Engine.h"
#include "Textures.h"

class GuiImage : public GuiControl
{

public:

	GuiImage(int id, SDL_Rect bounds, const char* text, SDL_Texture* texture);
	virtual ~GuiImage();

	bool Start();
	bool Update(float dt);
	void CleanUp();

	SDL_Texture* texture;
	SDL_Texture* textureSelected;
	bool isClicked = false;
	bool isOptionA = false;
	bool isOptionB = false;
private:
	SDL_Rect imagePos;
	int textX;
	int textY;

	bool canClick = true;
	bool drawBasic = false;

	
};

#pragma once