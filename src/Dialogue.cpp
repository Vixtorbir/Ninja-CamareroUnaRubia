#include "Dialogue.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Textures.h"

Dialogue::Dialogue(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::BUTTON, id)
{
    this->bounds = bounds;
    this->text = text;
    unkillable = false;
    canClick = true;
    drawBasic = false;


}

Dialogue::~Dialogue()
{
}
bool Dialogue::Start()
{
    pugi::xml_parse_result result = config.load_file("config.xml");

    if (!result)
    {
        std::cout << "XML file could not be loaded: " << result.description() << std::endl;
        return false;
    }

    screenWidth = 1920;
    screenHeight = 1080;

    Hanzo = Engine::GetInstance().textures.get()->Load("Assets/Portraits/Hanzo.png");
    Mikado = Engine::GetInstance().textures.get()->Load("Assets/Portraits/Mikado.png");
    Mentor = Engine::GetInstance().textures.get()->Load("Assets/Portraits/Mentor.png");

    Overlay = Engine::GetInstance().textures.get()->Load("Assets/UI/textBox.png");
    OverlayPortrait = Engine::GetInstance().textures.get()->Load("Assets/UI/textName.png");

    started = true;
    return true;
}

bool Dialogue::Update(float dt)
{
    if (!started) Start();
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
        int textW = 0, textH = 0;
        TTF_SizeText(Engine::GetInstance().render->font, displayText.c_str(), &textW, &textH);

        if (textW > bounds.w) textW = bounds.w;

        int textX = bounds.x + (bounds.w - textW) / 2;
        int textY = bounds.y + (bounds.h - textH) / 2;
        SDL_QueryTexture(OverlayPortrait, NULL, NULL, &textureWidthOverlay, &textureHeightOverlay);
        SDL_Rect overlayPos = { 0, 0, textureWidthOverlay, textureHeightOverlay };
        SDL_QueryTexture(Overlay, NULL, NULL, &textureWidthOverlay2, &textureHeightOverlay2);
        SDL_Rect overlayPos2 = { 0, 0, textureWidthOverlay2, textureHeightOverlay2 };



        std::string buttonText = text;

        if (buttonText == "Hanzo") {
            SDL_QueryTexture(Hanzo, NULL, NULL, &textureWidth, &textureHeight);
            SDL_Rect portraitPos = { 0, 0, textureWidth, textureHeight };

            Engine::GetInstance().render.get()->DrawTexture(Hanzo, (textX - 700) - Engine::GetInstance().render->camera.x, (textY - 550) - Engine::GetInstance().render->camera.y, &portraitPos);
            Engine::GetInstance().render.get()->DrawTexture(OverlayPortrait, screenHeight / 3 - Engine::GetInstance().render->camera.x, screenHeight / 2 - Engine::GetInstance().render->camera.y, &overlayPos);

        }
        else if (buttonText == "Mikado") {
            SDL_QueryTexture(Mikado, NULL, NULL, &textureWidth, &textureHeight);
            SDL_Rect portraitPos = { 0, 0, textureWidth, textureHeight };

            Engine::GetInstance().render.get()->DrawTexture(Mikado, (textX - 700) - Engine::GetInstance().render->camera.x, (textY - 550) - Engine::GetInstance().render->camera.y, &portraitPos);
            Engine::GetInstance().render.get()->DrawTexture(OverlayPortrait, screenHeight / 3 - Engine::GetInstance().render->camera.x, screenHeight / 2 - Engine::GetInstance().render->camera.y, &overlayPos);

        }
        else if (buttonText == "Mentor") {
            SDL_QueryTexture(Mentor, NULL, NULL, &textureWidth, &textureHeight);
            SDL_Rect portraitPos = { 0, 0, textureWidth, textureHeight };

            Engine::GetInstance().render.get()->DrawTexture(Mentor, (textX - 700) - Engine::GetInstance().render->camera.x, (textY - 550) - Engine::GetInstance().render->camera.y, &portraitPos);
            Engine::GetInstance().render.get()->DrawTexture(OverlayPortrait, screenHeight / 3 - Engine::GetInstance().render->camera.x, screenHeight / 2 - Engine::GetInstance().render->camera.y, &overlayPos);

        }
        else
        {
            Engine::GetInstance().render.get()->DrawTexture(Overlay, 480. - Engine::GetInstance().render->camera.x, 550 - Engine::GetInstance().render->camera.y, &overlayPos2);
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
            Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 0, 0, true, false);
     

            break;
        case GuiControlState::FOCUSED:
            Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 20, 255, true, false);
            break;
        case GuiControlState::PRESSED:
            Engine::GetInstance().render->DrawRectangle(bounds, 0, 255, 0, 255, true, false);
            break;
        }
   
        Engine::GetInstance().render->DrawText(displayText.c_str(), textX, textY, textW, textH);


    }

    return false;
}
