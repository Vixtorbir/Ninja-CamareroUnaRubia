#include "Projectile.h"
#include "Engine.h"
#include "Render.h"
#include "Physics.h"
#include "EntityManager.h"

Projectile::Projectile(float x, float y, float velocityX)
    : Entity(EntityType::SHURIKENS), speed(velocityX)
{
    position.setX(x);
    position.setY(y);

    pbody = Engine::GetInstance().physics->CreateCircle((int)x, (int)y, 5, bodyType::DYNAMIC);
    pbody->listener = this;
    pbody->ctype = ColliderType::SHURIKEN;
    pbody->body->SetGravityScale(0);
    pbody->body->SetLinearVelocity(b2Vec2(velocityX, 0));
}

bool Projectile::Update(float dt) {

    b2Vec2 bodyPos = pbody->body->GetPosition();
    position.setX(METERS_TO_PIXELS(bodyPos.x));
    position.setY(METERS_TO_PIXELS(bodyPos.y));
    return true;
}

void Projectile::OnCollision(PhysBody* physA, PhysBody* physB) {

  //  Engine::GetInstance().entityManager->DestroyEntity(this);
}
