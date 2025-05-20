#include "GuiControlButton.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Textures.h"

#define LOG(msg) std::cerr << msg << std::endl
GuiControlButton::GuiControlButton(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::BUTTON, id)
{
	this->bounds = bounds;
	this->text = text;

	canClick = true;
	drawBasic = false;
	
	
}
GuiControlButton::GuiControlButton(int id, SDL_Rect bounds, const char* text, bool optionA, bool optionB) : GuiControl(GuiControlType::BUTTON, id)
{
	this->bounds = bounds;

	this->text = text;

	canClick = true;
	drawBasic = false;

	this->isOptionA = optionA;
	unkillable = false;
	this->isOptionB = optionB;

}
GuiControlButton::~GuiControlButton()
{

}
bool GuiControlButton::Start()
{
	texture = Engine::GetInstance().textures.get()->Load("Assets/UI/individualUIsprites/textName.png");
    textureSelected = Engine::GetInstance().textures.get()->Load("Assets/UI/individualUIsprites/selectedButton.png");

    //Audio
    buttonSelectedFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/ExtraFx/selectButton.ogg");
    buttonPressedFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/ExtraFx/clickButton.ogg");
   /* 
    titleFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/ExtraFx/TittleFx.ogg");*/
    return false;
}
bool GuiControlButton::Update(float dt)
{
    if (visible)
    {
        if (state != GuiControlState::DISABLED)
        {
            // L16: TODO 3: Update the state of the GuiButton according to the mouse position
            Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

            // If the position of the mouse is inside the bounds of the button 
            if (mousePos.getX() > bounds.x && mousePos.getX() < bounds.x + bounds.w && mousePos.getY() > bounds.y && mousePos.getY() < bounds.y + bounds.h)
            {
                state = GuiControlState::FOCUSED;

                if (!fxPlayed)
                {
                    Engine::GetInstance().audio.get()->PlayFx(buttonSelectedFxId);
                    fxPlayed = true;
                }

                if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
                    state = GuiControlState::PRESSED;
                    if (!pressed)
                    {
                        Engine::GetInstance().audio.get()->PlayFx(buttonPressedFxId);
                        pressed = true;
                    }

                }

                if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
                    NotifyObserver();
                }
            }
            else
            {
                fxPlayed = false;

                state = GuiControlState::NORMAL;
            }

            // L16: TODO 4: Draw the button according to the GuiControl State
            switch (state)
            {

            case GuiControlState::NORMAL:
                Engine::GetInstance().render->DrawTexturedRectangle(texture, bounds.x-200, bounds.y-100, 600, 400, false);

                if (isOptionA || isOptionB)
                {

                    Engine::GetInstance().render->DrawTextWhite(text.c_str(), bounds.x, bounds.y, 150, 100);

                }
                break;

            case GuiControlState::FOCUSED:
                Engine::GetInstance().render->DrawTexturedRectangle(textureSelected, bounds.x - 200, bounds.y - 100, 600, 400, false);

                break;


            case GuiControlState::PRESSED:
                Engine::GetInstance().render->DrawTexturedRectangle(textureSelected, bounds.x - 200, bounds.y - 100, 600, 400, false);
                isClicked = true;


                break;
            }

            // Calculate the text size
            int textW = 0, textH = 0;
            TTF_SizeText(Engine::GetInstance().render->font, text.c_str(), &textW, &textH);

            // Ensure the text width doesn't exceed button width
            if (textW > bounds.w) {
                textW = bounds.w;
            }

            // Calculate position to center text within the button
            int textX = bounds.x + (bounds.w - textW) / 2;
            int textY = (bounds.y + 80) + (bounds.h - textH) / 2;

            // Render the text
            if (!isOptionA && !isOptionB) Engine::GetInstance().render->DrawText(text.c_str(), textX, textY, textW, textH);
        }

        return false;
    }
   
}
void GuiControlButton::CleanUp()
{

		SDL_DestroyTexture(texture);
        SDL_DestroyTexture(textureSelected);
		texture = nullptr;
	

	// Limpiar el texto
	text = "";
}





