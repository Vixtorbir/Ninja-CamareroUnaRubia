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

	// Called each loop iteration
	bool Update(float dt);
	void Render();

	SDL_Texture* texture;
private:

	bool canClick = true;
	bool drawBasic = false;
	
	//texture = Engine::GetInstance().textures.get()->Load("Assets/UI/individualUIsprites/textName.png");
	const char* iftext;
};

#pragma once