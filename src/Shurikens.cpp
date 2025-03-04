#include "Shurikens.h"
#include "Engine.h"
#include "Render.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Textures.h"

Shuriken::Shuriken(float x, float y, float velocityX)
    : Projectile(x, y, velocityX)
{
    texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/goldCoin.png");
}

bool Shuriken::Update(float dt) {

    Projectile::Update(dt);

    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), nullptr);

    return true;
}

void Shuriken::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + 32 / 2);
    pos.setY(pos.getY() + 32 / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Shuriken::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Shuriken::SetDirection(float direction)
{
	pbody->body->SetLinearVelocity(b2Vec2(speed * direction, 0));

}


