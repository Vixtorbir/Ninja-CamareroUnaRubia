#include "Shurikens.h"
#include "Engine.h"
#include "Textures.h"
#include "Render.h"
#include "Log.h"

Shuriken::Shuriken(float speed) : Entity(EntityType::SHURIKENS), speed(speed) {
    

    name = "shuriken";
    texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/goldCoin.png");

    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), 5, bodyType::DYNAMIC);
    pbody->listener = this;
    pbody->ctype = ColliderType::SHURIKEN;

    pbody->body->SetLinearVelocity(b2Vec2(speed, 0));



}

Shuriken::~Shuriken() {}

bool Shuriken::Start() {
    

    return true;
}

bool Shuriken::Update(float dt) {
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x));
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y));

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());
    return true;
}

bool Shuriken::CleanUp() {
    Engine::GetInstance().textures.get()->UnLoad(texture);
    return true;
}

void Shuriken::OnCollision(PhysBody* physA, PhysBody* physB) {
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    pbody = nullptr;
}