#pragma once

#include "Module.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_render.h"
#include <box2d/box2d.h>
#include <string>
#include <vector>

struct Layer {
	SDL_Texture* texture = nullptr;
	float speed = 0.0f;
	float offset = 0.0f;
};

class Parallax : public Module {
public:
	Parallax();
	bool ChangeTextures(int levelIndex);
	~Parallax() {}
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	
	std::vector<Layer> layers;
	float previousCamX;
	int sizeW = 1421;
	int sizeH = 480;

	std::string textureName1;
	std::string textureName2;
	std::string textureName3;
	std::string textureName4;
	std::string textureName5;

	std::string textureName1lvl1;
	std::string textureName2lvl1;
	std::string textureName3lvl1;
	std::string textureName4lvl1;
	std::string textureName5lvl1;
};
