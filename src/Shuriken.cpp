#include "Shuriken.h"
#include "Engine.h"
#include "Render.h"
#include "Physics.h"

Shuriken::Shuriken(float x, float y, float velocityX) {
    texture = Engine::GetInstance().textures.get()->Load("Assets/Sprites/shuriken.png");

    // Crear el cuerpo f�sico del shuriken
    pbody = Engine::GetInstance().physics.get()->CreateCircle(x, y, 5, DYNAMIC);
    pbody->body->SetBullet(true);  // Mejora la detecci�n de colisiones r�pidas
    pbody->body->SetGravityScale(0);  // Sin gravedad para que se mantenga recto
    pbody->body->SetLinearVelocity(b2Vec2(velocityX, 0));  // Movimiento en l�nea recta
    pbody->listener = this;
    pbody->ctype = ColliderType::SHURIKEN;
}

void Shuriken::Update(float dt) {
    // Actualizar posici�n del sprite con la del cuerpo f�sico
    b2Vec2 pos = pbody->body->GetPosition();
    position.setX(METERS_TO_PIXELS(pos.x));
    position.setY(METERS_TO_PIXELS(pos.y));

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());
}

void Shuriken::OnCollision(PhysBody* physA, PhysBody* physB) {
    Engine::GetInstance().physics.get()->DeletePhysBody(physA); // Eliminar el shuriken al colisionar
}
