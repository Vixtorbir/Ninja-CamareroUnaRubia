#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Physics.h"

class Shuriken : public Entity {
public:
    Shuriken(float speed);
    virtual ~Shuriken();

    bool Start();
    bool Update(float dt);
    bool CleanUp();

    void OnCollision(PhysBody* physA, PhysBody* physB);

public:
    SDL_Texture* texture;
    float speed;
    PhysBody* pbody;
    Animation* currentAnimation;
    Animation spinning;
};