#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class GuiSlider : public GuiControl
{
public:
    GuiSlider(int id, SDL_Rect bounds, const char* text, int minValue, int maxValue, int defaultValue);
    virtual ~GuiSlider();

    void SetSliderBarSize(int width, int height);

    void SetSliderBarInnerSize(int width, int height);

    bool Update(float dt);
    bool Draw(std::shared_ptr<Render> render);

    int GetValue() const { return currentValue; }

    bool canClick = true;
    bool drawBasic = false;
    bool visible = true;

private:
    int minValue;
    int maxValue;
    int currentValue;

    Vector2D mousePos;
    SDL_Texture* backgroundSliderHP;
    int SliderTextureW;
    int SliderTextureH;
    SDL_Rect backgroundSliderSizes;

    SDL_Rect sliderBar;
    SDL_Rect sliderBarInner;

    SDL_Rect sliderThumb;
};
