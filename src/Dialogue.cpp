#include "Dialogue.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"

Dialogue::Dialogue(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::BUTTON, id)
{
    this->bounds = bounds;
    this->text = text;

    canClick = true;
    drawBasic = false;
}

Dialogue::~Dialogue()
{
}

bool Dialogue::Update(float dt)
{
    if (state != GuiControlState::DISABLED)
    {
        if (!fullTextDisplayed)
        {
            timer += dt;
            if (timer >= textSpeed && charIndex < text.length())
            {
                timer = 0.0f;
                charIndex++;
                displayText = text.substr(0, charIndex);

                if (charIndex >= text.length())
                {
                    fullTextDisplayed = true;
                }
            }
        }

        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
        {
            displayText = text; 
            charIndex = text.length();
            fullTextDisplayed = true;
        }

        Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();
        if (mousePos.getX() > bounds.x && mousePos.getX() < bounds.x + bounds.w &&
            mousePos.getY() > bounds.y && mousePos.getY() < bounds.y + bounds.h)
        {
            state = GuiControlState::FOCUSED;

            if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
            {
                state = GuiControlState::PRESSED;
            }

            if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
            {
                NotifyObserver();
            }
        }
        else
        {
            state = GuiControlState::NORMAL;
        }

        switch (state)
        {
        case GuiControlState::DISABLED:
            Engine::GetInstance().render->DrawRectangle(bounds, 200, 200, 200, 255, true, false);
            break;
        case GuiControlState::NORMAL:
            Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 0, 255, true, false);
            break;
        case GuiControlState::FOCUSED:
            Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 20, 255, true, false);
            break;
        case GuiControlState::PRESSED:
            Engine::GetInstance().render->DrawRectangle(bounds, 0, 255, 0, 255, true, false);
            break;
        }

        Engine::GetInstance().render->DrawText(displayText.c_str(), bounds.x, bounds.y, bounds.w, bounds.h);
    }

    return false;
}
