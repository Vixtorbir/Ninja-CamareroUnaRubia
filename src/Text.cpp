#include "Text.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Textures.h"

#define LOG(msg) std::cerr << msg << std::endl
Text::Text(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::BUTTON, id)
{
	this->bounds = bounds;
	this->text = text;

	canClick = true;
	drawBasic = false;
	
	
}
Text::Text(int id, SDL_Rect bounds, const char* text, bool optionA, bool optionB) : GuiControl(GuiControlType::BUTTON, id)
{
	this->bounds = bounds;

	this->text = text;

	canClick = true;
	drawBasic = false;

	this->isOptionA = optionA;
	unkillable = false;
	this->isOptionB = optionB;

}
Text::~Text()
{

}
bool Text::Start()
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
bool Text::Update(float dt)
{
	Engine::GetInstance().render->DrawNumbers(currentText.c_str(), 1750, 0, 25,100);

    return false;
}
void Text::SetText(const std::string& text) {

    currentText = text;
}
void Text::CleanUp()
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





