#pragma once

#include "SDL2/SDL.h"
#include "Projectile.h"

class Shuriken : public Projectile {
public:
    Shuriken(float x, float y, float velocityX);
    ~Shuriken() {}

    bool Update(float dt) override;

    void SetPosition(Vector2D pos);

    Vector2D GetPosition();

    void SetDirection(float direction); 

private:
    SDL_Texture* texture;
    float direction; 
};
