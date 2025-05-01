#include "Turret.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"
#include "EntityManager.h"
#include "Timer.h"	

Turret::Turret() : Entity(EntityType::ENEMY) 
{
    
}

Turret::~Turret() {


}

bool Turret::Awake() {
    return true;
}

bool Turret::Start() {
    // Cargar textura de la torreta
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    shurikenTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/goldCoin.png");

    // Configurar posición inicial
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    idle.LoadAnimations(parameters.child("animations").child("idle"));
 
    currentAnimation = &idle;

    // Crear el cuerpo físico de la torreta
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), texW, texH, bodyType::STATIC);
    pbody->ctype = ColliderType::TURRET; // Asignar el tipo de colisionador
    pbody->listener = this;

    
    shootTimer.Start();

    return true;
}

bool Turret::Update(float dt) {

    
    if (dead)
    {

        if (pbody != nullptr)
        {
            pbody->body->DestroyFixture(pbody->body->GetFixtureList());
            pbody = nullptr;
        }

		//haz que tambien se elimine los shurikens

		for (auto& shuriken : activeShurikens) {
			Engine::GetInstance().physics.get()->DeletePhysBody(shuriken.body);
		}

        LOG("Enemy is dead, removing from scene.");

        active = false;

        return true;
    }
    if (!dead)

    {

        if (Engine::GetInstance().scene.get()->currentState == GameState::PAUSED)
        {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            b2Transform pbodyPos = pbody->body->GetTransform();
            position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 6);
            position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 6);

            Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
            currentAnimation->Update();
            return true;
        }

        if (Engine::GetInstance().scene.get()->currentState != GameState::PLAYING)
        {
            return true;
        }

        Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
        Vector2D enemyPos = GetPosition();
        Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
        Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

        // Si el jugador está fuera del rango máximo, el enemigo no hace nada
        if (abs(playerTilePos.getX() - enemyTilePos.getX()) > 105)
        {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            b2Transform pbodyPos = pbody->body->GetTransform();
            position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 6);
            position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 6);

            Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
            currentAnimation->Update();
            return true;
        }

    // Disparar shurikens cada 3 segundos
    if (shootTimer.ReadSec() >= 5.0f) {
        ShootShuriken();
        shootTimer.Start(); // Reiniciar el temporizador
    }

    // Dibujar la torreta
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 6);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 6);

    // Dibujar textura y animación
    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
    currentAnimation->Update();

    for (auto it = activeShurikens.begin(); it != activeShurikens.end(); ) {
        if (it->timer.ReadSec() >= 2.0f) { 
            Engine::GetInstance().physics.get()->DeletePhysBody(it->body); 
            it = activeShurikens.erase(it); 
        }
        else {
            ++it; 
        }
    }

    // Renderizar los shurikens
    for (const auto& shuriken : activeShurikens) {
        int x, y;
        shuriken.body->GetPosition(x, y);
        Engine::GetInstance().render.get()->DrawTexture(shurikenTexture, x + 25, y + 25);
    }
  
    }
    return true;
	
}

void Turret::ShootShuriken() {
    // Crear el shuriken como un sensor físico
    PhysBody* shuriken = Engine::GetInstance().physics.get()->CreateRectangleSensor(
        (int)position.getX() + texW / 2, (int)position.getY() + texH / 2, 40, 40, bodyType::DYNAMIC
    );

    // Configurar propiedades del shuriken
    shuriken->ctype = ColliderType::ENEMY; // El shuriken es un ataque enemigo
    shuriken->listener = this;
    shuriken->body->SetBullet(true);
    shuriken->body->SetFixedRotation(true);
    shuriken->body->SetGravityScale(0.0f);

    // Aplicar impulso horizontal hacia la derecha
    float shurikenSpeed = 5.0f;
    b2Vec2 impulse = b2Vec2(shurikenSpeed, 0);
    shuriken->body->ApplyLinearImpulseToCenter(impulse, true);

    Shuriken2 newShuriken = { shuriken, Timer() };
    newShuriken.timer.Start();
    activeShurikens.push_back(newShuriken);    

    LOG("Turret shot a shuriken!");
}

void Turret::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER_ATTACK:
        // Si el jugador golpea la torreta, marcarla como muerta
        LOG("Turret hit by player attack!");
        dead = true;
        break;

    case ColliderType::PLAYER:
        // Si el jugador colisiona con un shuriken, aplicar daño
        if (physA->ctype == ColliderType::ENEMY) {
            Engine::GetInstance().scene.get()->player->TakeDamage(1);
            LOG("Player hit by turret shuriken!");
        }
        break;
    }
}

void Turret::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        LOG("Collision player");
        break;
    }
}

void Turret::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Turret::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Turret::ResetPath() {
    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
    pathfinding->ResetPath(tilePos);
}

void Turret::LoadEnemyFx()
{
    //centinel
    ninjaWalk1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    ninjaMeleAttack1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    ninjaMeleAttack2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    ninjaRangeAttack1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    ninjaRangeAttack2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    ninjaJump1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    ninjaJump2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    ninjaDieFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    ninjaHit1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    ninjaHit2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    //skull
    skullFlyFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    skullAttack1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    skullAttack2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    skullHit1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    skullHit2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    skulldieFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    //bat
    batFlyFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    batAttack1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    batAttack2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    batHit1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    batHit2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    batdieFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
    //spirit
    //ghost soldier
}

bool Turret::CleanUp() {
    Engine::GetInstance().textures.get()->UnLoad(texture);
    Engine::GetInstance().textures.get()->UnLoad(shurikenTexture);
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    active = false;
    return true;
}