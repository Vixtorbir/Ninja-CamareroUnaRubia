#pragma once
#include "Entity.h"
#include "Physics.h"

class Projectile : public Entity {
public:
    Projectile(float x, float y, float velocityX);
    virtual ~Projectile() {}

    virtual bool Update(float dt) override;
    virtual void OnCollision(PhysBody* physA, PhysBody* physB) override;

protected:
    PhysBody* pbody;
    float speed;
};
