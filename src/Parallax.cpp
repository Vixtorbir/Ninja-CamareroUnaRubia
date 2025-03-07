#include "Parallax.h"
#include "Entity.h"
#include "Engine.h"
#include "Textures.h"
#include "Log.h"

Parallax::Parallax() {
    layers.resize(5);
}

bool Parallax::ChangeTextures() {
    std::vector<float> speeds = { 0.09f, 0.1f, 0.3f, 0.5f, 0.7f }; 

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

    for (auto& layer : layers) {
        float layerPosition = -cameraX * layer.speed;

        if (layerPosition > sizeW) {
            layerPosition -= sizeW;
        }
        else if (layerPosition < -sizeW) {
            layerPosition += sizeW;
        }

        for (int i = -1; i <= 1; ++i) {
            SDL_QueryTexture(layer.texture, NULL, NULL, &sizeW, &sizeH);

            //RESOLUTION TO FIX
             int drawX = static_cast<int>(layerPosition + i * sizeW) + 1920 - sizeW;

            SDL_Rect rect = { 0, 0, sizeW, sizeH };
            engine.render->DrawTexture(layer.texture, drawX, (int)engine.render->camera.y, &rect, SDL_FLIP_NONE);
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