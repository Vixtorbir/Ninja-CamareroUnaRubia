#include "Parallax.h"
#include "Entity.h"
#include "Engine.h"
#include "Textures.h"
#include "Log.h"

Parallax::Parallax() {
    layers.resize(5);
}

bool Parallax::ChangeTextures() {
    std::vector<float> speeds = { 0.09f, 0.3f, 0.6f, 0.5f, 0.7f }; 

    for (size_t i = 0; i < layers.size(); ++i) {
        std::string texturePath = "Assets/Backgrounds/Map" + std::to_string(i) + ".png";
        layers[i].texture = Engine::GetInstance().textures->Load(texturePath.c_str());
        layers[i].speed = speeds[i];
    }
    return true;
}

bool Parallax::Start() {
    return ChangeTextures();
}

bool Parallax::Update(float dt) {
    auto& engine = Engine::GetInstance();
    float cameraX = static_cast<float>(engine.render->camera.x);

    int fixedYPosition = 1080 - sizeH;

    for (auto& layer : layers) {
        float layerPositionX = -cameraX * layer.speed;

        if (layerPositionX > sizeW) {
            layerPositionX -= sizeW;
        }
        else if (layerPositionX < -sizeW) {
            layerPositionX += sizeW;
        }

        for (int i = -1; i <= 1; ++i) {
            SDL_QueryTexture(layer.texture, NULL, NULL, &sizeW, &sizeH);

            int drawX = static_cast<int>(layerPositionX + i * sizeW) + 1920 - sizeW;
            int drawY = fixedYPosition;

            SDL_Rect rect = { 0, 0, sizeW, sizeH };
            engine.render->DrawTexture(layer.texture, -drawX, drawY, &rect, SDL_FLIP_NONE);
        }
    }

    return true;
}


bool Parallax::CleanUp() {
    LOG("CleanUp Parallax");
    for (auto& layer : layers) {
        Engine::GetInstance().textures->UnLoad(layer.texture);
    }
    return true;
}