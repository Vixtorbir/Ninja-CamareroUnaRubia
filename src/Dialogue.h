#pragma once

#include "GuiControl.h"
#include "Vector2D.h"
#include <vector>
#include <string>
#include <sstream> // Add this at the top for istringstream
class Dialogue : public GuiControl
{
public:
    Dialogue(int id, SDL_Rect bounds, const char* text);
    virtual ~Dialogue();

    bool Start();

    bool Update(float dt);

    std::vector<std::string> SplitLines(const std::string& text);

    std::string WrapText(const std::string& text, int maxWidth, TTF_Font* font);


private:
    bool canClick = true;
    bool drawBasic = false;
    int textureWidth, textureHeight;
    int textureWidthOverlay, textureHeightOverlay;
    int textureWidthOverlay2, textureHeightOverlay2;

    SDL_Rect portraitPos;

    bool started = false;
    bool isName = false;
    std::string text;
    std::string displayText;
    int charIndex = 0;
    float textSpeed = 0.0001f;
    float timer = 0.0f;
    bool fullTextDisplayed = false;
    
    SDL_Texture* Overlay;
    SDL_Texture* OverlayPortrait;

    int screenWidth;
    int screenHeight;

    SDL_Texture* Hanzo;
    SDL_Texture* Mikado;
    SDL_Texture* Mentor;

    pugi::xml_document config;

};
