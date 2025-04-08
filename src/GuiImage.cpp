#include "GuiImage.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Textures.h"

GuiImage::GuiImage(int id, SDL_Rect bounds, const char* text, SDL_Texture* texture) : GuiControl(GuiControlType::IMAGE, id)
{
	this->bounds = bounds;
	this->text = text;

    this->texture = texture;

	canClick = true;
	drawBasic = false;
	

    SDL_QueryTexture(texture, NULL, NULL, &textX, &textY);
    imagePos = { 0, 0, textX, textY };
}

GuiImage::~GuiImage()
{

}

bool GuiImage::Update(float dt)
{
    if (state != GuiControlState::DISABLED)
    {
        // L16: TODO 3: Update the state of the GuiButton according to the mouse position
        Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

        // If the position of the mouse is inside the bounds of the button 
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

        // L16: TODO 4: Draw the button according to the GuiControl State
        if (visible)
        {
            switch (state)
            {
            case GuiControlState::DISABLED:
                Engine::GetInstance().render->DrawTexturedRectangle(texture, 0, -0, textX, textY, false);

                break;
            case GuiControlState::NORMAL:
                Engine::GetInstance().render->DrawTexturedRectangle(texture, 0, -0, textX, textY, false);
                break;

            case GuiControlState::FOCUSED:
                Engine::GetInstance().render->DrawTexturedRectangle(texture, 0, -0, textX, textY, false);
                break;

            case GuiControlState::PRESSED:
                Engine::GetInstance().render->DrawTexturedRectangle(texture, 0, -0, textX, textY, false);
                break;
            }
        }
       

 
    }

    return false;
}
void GuiImage::CleanUp()
{
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture);
        SDL_DestroyTexture(textureSelected);
		texture = nullptr;
	}

	// Limpiar el texto
	text = "";
}





