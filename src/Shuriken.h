#pragma once

#include "Physics.h"

class Projectile {
public:
    Projectile(float x, float y, float velocityX);
    virtual ~Projectile() {}

    virtual void Update(float dt);
    virtual void OnCollision(PhysBody* physA, PhysBody* physB);

protected:
    Vector2D position;
    PhysBody* pbody;
};

class Shuriken : public Projectile {
public:
    Shuriken(float x, float y, float velocityX);
    void Update(float dt) override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

private:
    SDL_Texture* texture;
};