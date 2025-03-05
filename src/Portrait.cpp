#include "Portrait.h"
#include "Render.h"
#include "Engine.h"
#include "Textures.h"

Portrait::Portrait(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::PORTRAIT, id)
{
	this->bounds = bounds;
	this->text = text;
	Hanzo = Engine::GetInstance().textures.get()->Load("Assets/Portraits/Hanzo.jpg");

	canClick = true;
	drawBasic = false;
}

Portrait::~Portrait()
{

}

bool Portrait::Update(float dt)
{
	state = GuiControlState::NORMAL;

	if (state != GuiControlState::DISABLED && state == GuiControlState::NORMAL)
	{
		Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 255, 255, true, false);
		//Engine::GetInstance().render->DrawTexture(Hanzo, 0,0 255, 255)
	}

	return false;
}

