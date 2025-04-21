#include "GuiCheckbox.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Textures.h"
#include "Scene.h"

GuiCheckbox::GuiCheckbox(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::BUTTON, id)
{
	this->bounds = bounds;
	this->text = text;

	canClick = true;
	drawBasic = false;
}

GuiCheckbox::~GuiCheckbox()
{

}
bool GuiCheckbox::Start()
{
	CheckboxTexture = Engine::GetInstance().textures.get()->Load("Assets/UI/individualUIsprites/textName.png");
	CheckedTexture = Engine::GetInstance().textures.get()->Load("Assets/UI/individualUIsprites/selectedButton.png");

	return false;
}
bool GuiCheckbox::Update(float dt)
{
	if (Engine::GetInstance().scene.get()->currentState != GameState::PLAYING) return true;


	if (isActive)
	{
		if (state != GuiControlState::DISABLED)
		{
			// L16: TODO 3: Update the state of the GUiButton according to the mouse position
			Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

			//If the position of the mouse if inside the bounds of the button 
			if (mousePos.getX() > bounds.x && mousePos.getX() < bounds.x + bounds.w && mousePos.getY() > bounds.y && mousePos.getY() < bounds.y + bounds.h) {

				state = GuiControlState::FOCUSED;

				if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
					state = GuiControlState::PRESSED;
				}

				if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
					NotifyObserver();
				}
			}
			else {
				state = GuiControlState::NORMAL;
			}

			//L16: TODO 4: Draw the button according the GuiControl State
			switch (state)
			{
			case GuiControlState::DISABLED:
				Engine::GetInstance().render->DrawTexturedRectangle(CheckboxTexture, bounds.x - 200, bounds.y - 100, 600, 400, false);
				break;
			case GuiControlState::NORMAL:
				Engine::GetInstance().render->DrawTexturedRectangle(CheckboxTexture, bounds.x - 200, bounds.y - 100, 600, 400, false);
				break;
			case GuiControlState::FOCUSED:
				Engine::GetInstance().render->DrawTexturedRectangle(CheckedTexture, bounds.x - 200, bounds.y - 100, 600, 400, false);
				break;
			case GuiControlState::PRESSED:
				Engine::GetInstance().render->DrawTexturedRectangle(CheckedTexture, bounds.x - 200, bounds.y - 100, 600, 400, false);
				isClicked = true;
				break;
			}

			Engine::GetInstance().render->DrawWhiteText(text.c_str(), bounds.x, bounds.y, bounds.w, bounds.h);


		}
	}


	return false;
}

