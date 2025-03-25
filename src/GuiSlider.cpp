#include "GuiSlider.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Textures.h"

GuiSlider::GuiSlider(int id, SDL_Rect bounds, const char* text, int minValue, int maxValue, int defaultValue)
    : GuiControl(GuiControlType::SLIDER, id), minValue(minValue), maxValue(maxValue), currentValue(defaultValue), canClick(true), drawBasic(false)
{
    this->bounds = bounds;
    this->text = text;

    sliderBar = { bounds.x, bounds.y + bounds.h / 2 - 5, bounds.w, 10 };
    sliderThumb = { bounds.x + ((defaultValue - minValue) * bounds.w / (maxValue - minValue)) - 5, bounds.y, 10, bounds.h };

    backgroundSliderHP = Engine::GetInstance().textures.get()->Load("Assets/UI/lifeBarBack.png");
    SDL_QueryTexture(backgroundSliderHP, NULL, NULL, &SliderTextureW, &SliderTextureH);
    SDL_Rect backgroundSliderSizes = { 0, 0, SliderTextureW, SliderTextureH };
}

GuiSlider::~GuiSlider()
{
}

bool GuiSlider::Update(float dt)
{
    if (state != GuiControlState::DISABLED)
    {
        mousePos = Engine::GetInstance().input->GetMousePosition();

        if (mousePos.getX() > sliderThumb.x && mousePos.getX() < sliderThumb.x + sliderThumb.w &&
            mousePos.getY() > sliderThumb.y && mousePos.getY() < sliderThumb.y + sliderThumb.h)
        {
            state = GuiControlState::FOCUSED;

            if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
            {
                state = GuiControlState::PRESSED;

                int mouseX = mousePos.getX() - bounds.x;
                mouseX = std::max(0, std::min(mouseX, bounds.w)); // Clamp within bounds
                currentValue = minValue + (mouseX * (maxValue - minValue) / bounds.w);

                sliderThumb.x = bounds.x + mouseX - sliderThumb.w / 2;

                NotifyObserver();
            }
        }
        else
        {
            state = GuiControlState::NORMAL;
        }
    }

    Draw(Engine::GetInstance().render);

    return false;
}

bool GuiSlider::Draw(std::shared_ptr<Render> render)
{
    if (visible)
    {
        render->DrawRectangle(sliderBar, 200, 200, 200, 255, true, false);

        int textX = bounds.x + (bounds.w - SliderTextureW);
        int textY = bounds.y + (bounds.h - SliderTextureH);
        Engine::GetInstance().render.get()->DrawTexture(backgroundSliderHP,Engine::GetInstance().render->camera.x,Engine::GetInstance().render->camera.y, &backgroundSliderSizes);
        std::cout << "textX: " << textX
            << ", textY: " << textY
            << ", Camera X: " << Engine::GetInstance().render->camera.x
            << ", Camera Y: " << Engine::GetInstance().render->camera.y
            << std::endl;
        switch (state)
        {
        case GuiControlState::DISABLED:
            render->DrawRectangle(sliderThumb, 150, 150, 150, 255, true, false);
            break;
        case GuiControlState::NORMAL:
            render->DrawRectangle(sliderThumb, 255, 255, 255, 255, true, false);
            break;
        case GuiControlState::FOCUSED:
            render->DrawRectangle(sliderThumb, 0, 0, 255, 255, true, false);
            break;
        case GuiControlState::PRESSED:
            render->DrawRectangle(sliderThumb, 0, 255, 0, 255, true, false);
            break;
        }

        render->DrawText(text.c_str(), bounds.x + bounds.w + 10, bounds.y, bounds.w, bounds.h);
    }

    return false;
}
