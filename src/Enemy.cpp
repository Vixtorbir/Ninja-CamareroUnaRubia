#include "Enemy.h"
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
//#include "tracy/Tracy.hpp"

Enemy::Enemy() : Entity(EntityType::ENEMY)
{

}

Enemy::~Enemy() {
	delete pathfinding;
}

bool Enemy::Awake() {
	return true;
}

bool Enemy::Start() {
	// Inicializar texturas
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	attackTexture = Engine::GetInstance().textures.get()->Load(parameters.attribute("attack_texture").as_string());
	

	attackTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/enemyAttack.png"); 


	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	// Cargar animaciones
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	attackAnimation.LoadAnimations(parameters.child("animations").child("attack"));
	walkAnimation.LoadAnimations(parameters.child("animations").child("walk"));
	deathAnimation.LoadAnimations(parameters.child("animations").child("die"));

	currentAnimation = &idle;

	// Agregar física al enemigo
	pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX()+256, (int)position.getY(), texW, texH-200, bodyType::DYNAMIC);

	// Asignar tipo de colisionador
	pbody->ctype = ColliderType::ENEMY;
	pbody->listener = this;

	// Asignar gravedad al enemigo
	pbody->body->SetFixedRotation(true);

	// Establecer la gravedad del cuerpo
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	// Inicializar pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	return true;
}


bool Enemy::Update(float dt)
{

	if (isDying)
	{
		currentAnimation = &deathAnimation;
		// When timer is done, destroy enemy
		if (deathTimer.ReadSec() >= deathDuration)
		{
			if (attackBody != nullptr) {
				Engine::GetInstance().physics.get()->DeletePhysBody(attackBody);
				attackBody = nullptr;
			}

			if (pbody != nullptr) {
				pbody->body->DestroyFixture(pbody->body->GetFixtureList());
				pbody = nullptr;
			}

			LOG("Enemy death animation finished. Removing from scene.");
			active = false;
		}

		return true; // Skip further update while dying
	}


	if (!startDying)

	{

	if (Engine::GetInstance().scene.get()->currentState == GameState::PAUSED)
	{
		pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		b2Transform pbodyPos = pbody->body->GetTransform();
		position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 6);
		position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 6);

		Engine::GetInstance().render.get()->DrawEntity(
			texture,
			(int)position.getX(),
			(int)position.getY(),
			&currentAnimation->GetCurrentFrame(),
			1, 0, 0, 0, -(int)direction
		);
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
	if (abs(playerTilePos.getX() - enemyTilePos.getX()) > 75)
	{
		pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		b2Transform pbodyPos = pbody->body->GetTransform();
		position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 6);
		position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 6);

		currentAnimation->Update();
		return true;
	}

	// Determinar el estado del enemigo
	if (state != EnemyState::WAIT && state != EnemyState::ATTACK) // No cambiar el estado si está en espera o atacando
	{
		if (IsPlayerInRange())
		{
			if (IsPlayerInAttackRange())
			{
				state = EnemyState::ATTACK;
			}
			else
			{
				state = EnemyState::AGGRESSIVE;
			}
		}
		else
		{
			state = EnemyState::PATROL;
		}
	}

	// Manejar los estados del enemigo
	switch (state)
	{
	case EnemyState::PATROL:
		currentAnimation = &walkAnimation;
		aggressive = false;
		// Movimiento de patrulla
		if (!IsNextTileCollidable() || tilesMovedInSameDirection >= 350)
		{
			direction = (direction == 0) ? 1 : 0;
			tilesMovedInSameDirection = 0;
		}

		if (direction == 1)
		{
			pbody->body->SetLinearVelocity(b2Vec2(-2, 0));
		}
		else
		{
			pbody->body->SetLinearVelocity(b2Vec2(2, 0));
		}
		tilesMovedInSameDirection++;
		break;

	case EnemyState::AGGRESSIVE:
		currentAnimation = &walkAnimation;
		aggressive = true;
		// Movimiento agresivo hacia el jugador
		if (playerTilePos.getX() < enemyTilePos.getX() && IsNextTileCollidable())
		{
			pbody->body->SetLinearVelocity(b2Vec2(-2, 0));
			direction = 0;
		}
		else if (playerTilePos.getX() > enemyTilePos.getX() && IsNextTileCollidable())
		{
			pbody->body->SetLinearVelocity(b2Vec2(2, 0));
			direction = 1;
		}
		else
		{
			pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		}
		break;

	case EnemyState::ATTACK:
		// El enemigo permanece quieto mientras ataca
		pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		currentAnimation = &attackAnimation;

		if (!isAttacking && !isCooldown)
		{
			//PerformAttack();
			isAttacking = true;
			attackTimer.Start(); // Inicia el temporizador para el ataque
		}
		break;

	case EnemyState::WAIT:
		// El enemigo se queda quieto durante 2 segundos
		pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		if (attackTimer.ReadSec() >= 2.0f) // Espera 2 segundos
		{
			state = EnemyState::PATROL; // Cambia al estado de patrulla después de esperar
			LOG("Enemy finished waiting.");
		}
		break;
	}

	// Lógica de temporizadores (fuera del switch)
	if (isAttacking && attackTimer.ReadSec() >= attackDuration)
	{
		// Termina el ataque
		isAttacking = false;
		isCooldown = true;
		attackTimer.Start(); // Inicia el temporizador para el cooldown

		// Eliminar la hitbox del ataque
		if (attackBody != nullptr)
		{
			Engine::GetInstance().physics.get()->DeletePhysBody(attackBody);
			attackBody = nullptr;
			LOG("Attack hitbox removed.");
		}

		LOG("Enemy attack ended, entering WAIT state.");
		state = EnemyState::WAIT; // Cambia al estado de espera
	}

	if (isCooldown && attackTimer.ReadSec() >= attackCooldown)
	{
		// Termina el cooldown
		isCooldown = false;
		LOG("Enemy cooldown ended, can attack again.");
	}

	if (isAttacking && attackBody != nullptr) {
		int x, y;
		attackBody->GetPosition(x, y);

		int textureWidth = 104; 
		int textureHeight = 128; 
		int renderX = x - textureWidth / 2;
		int renderY = y - textureHeight / 2;

		Engine::GetInstance().render.get()->DrawTexture(attackTexture, renderX, renderY);
	}

	// Dibuja la línea de visión del enemigo
	DrawLineOfSight();

	// Actualizar posición del enemigo desde la física
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 6);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 6);

	// Dibujar textura y animación
	Engine::GetInstance().render.get()->DrawEntity(
		texture,
		(int)position.getX()-230,
		(int)position.getY(),
		&currentAnimation->GetCurrentFrame(),
		1, 0, 0, 0, -(int)direction
	);	currentAnimation->Update();

	// Dibujar el pathfinding si está en modo debug
	if (Engine::GetInstance().physics.get()->debug)
	{
		pathfinding->DrawPath();
	}
}
	return true;
}

void Enemy::SetPlayer(Player* player)
{
	this->player = player;
}

bool Enemy::CleanUp() {
	Engine::GetInstance().textures.get()->UnLoad(texture);
	Engine::GetInstance().textures.get()->UnLoad(attackTexture); 

	if (pbody != nullptr) {
		Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	}
	active = false;
	return true;
}


void Enemy::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Enemy::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Enemy::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::PLAYER_ATTACK:
		if (!isDying) {
			LOG("Enemy hit by player attack!");
			isDying = true;
			deathTimer.Start();
			currentAnimation = &deathAnimation;
			pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		}
		break;

	case ColliderType::PLAYER:
		// Si el jugador colisiona con un shuriken, aplicar daño
		if (physA->ctype == ColliderType::ENEMY) {
			if (!isDying) {
				Engine::GetInstance().scene.get()->player->TakeDamage(1);
				LOG("Player hit by enemy attack!");
			}
		
		}
		break;
	}
}


void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collision player");
		break;
	}
}

bool Enemy::IsNextTileCollidable() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());

	// Calcular la posición de la siguiente tile en la dirección actual
	if (direction == 0) { // Izquierda
		tilePos.setX(tilePos.getX() - 1);
	}
	else { // Derecha
		tilePos.setX(tilePos.getX() + 1);
	}

	// Verificar si la siguiente tile es colisionable (pared o plataforma)
	bool nextTileCollidable = Engine::GetInstance().map.get()->IsTileCollidable(tilePos.getX(), tilePos.getY());
	bool nextObjectGroupCollidable = Engine::GetInstance().map.get()->IsObjectGroupCollidable(tilePos.getX(), tilePos.getY());

	// Verificar si hay suelo debajo de la siguiente tile
	bool floorBelowNextTile = Engine::GetInstance().map.get()->IsTileCollidable(tilePos.getX(), tilePos.getY() + 1) ||
		Engine::GetInstance().map.get()->IsObjectGroupCollidable(tilePos.getX(), tilePos.getY() + 1);

	// Si la siguiente tile es colisionable o no hay suelo debajo, no se puede mover
	return nextTileCollidable || nextObjectGroupCollidable || !floorBelowNextTile;
}



bool Enemy::IsPlayerInRange() {
	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

	int distance = abs(playerTilePos.getX() - enemyTilePos.getX());
	bool isInRange = distance <= 20;

	// Verificar si el enemigo está mirando en la dirección del jugador
	bool isFacingPlayer = (direction == 0 && playerTilePos.getX() < enemyTilePos.getX()) ||
		(direction == 1 && playerTilePos.getX() > enemyTilePos.getX());

	return isInRange && isFacingPlayer;
}


void Enemy::PerformAttack() {
	// Detiene al enemigo
	pbody->body->SetLinearVelocity(b2Vec2(0, 0));
	currentAnimation = &attackAnimation;

	if (attackBody == nullptr) {
		// Determina la dirección del ataque
		int attackOffsetX = (direction == 0) ? -100 : 210; // Ajusta según la dirección del enemigo

		// Crea la hitbox del ataque
		attackBody = Engine::GetInstance().physics.get()->CreateRectangleSensor(
			(int)position.getX() + attackOffsetX,
			(int)position.getY() + 90,
			104, 128, // Tamaño de la hitbox
			bodyType::STATIC
		);
		attackBody->ctype = ColliderType::ENEMY;
		attackBody->listener = this;
	}

	// Dibujar la animación de ataque
	Engine::GetInstance().render.get()->DrawTexture(attackTexture, (int)position.getX(), (int)position.getY(), &attackAnimation.GetCurrentFrame());
	attackAnimation.Update();

	LOG("Enemy is attacking!");
}




void Enemy::CheckAttackCollision() {
	if (attackBody != nullptr) {
		Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
		SDL_Rect playerHitbox = {
			(int)playerPos.getX(),
			(int)playerPos.getY(),
			Engine::GetInstance().scene.get()->player->texW,
			Engine::GetInstance().scene.get()->player->texH
		};

		SDL_Rect attackRect = {
			(int)position.getX(),
			(int)position.getY(),
			texW,
			texH
		};

		if (SDL_HasIntersection(&attackRect, &playerHitbox)) {
			// Reduce la HP del jugador
			Engine::GetInstance().scene.get()->player->TakeDamage(1);
			LOG("Player hit by enemy attack!");
		}
	}
}


bool Enemy::IsPlayerInAttackRange() {
	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

	int distance = abs(playerTilePos.getX() - enemyTilePos.getX());
	return distance <= 14;
}

void Enemy::LoadEnemyFx()
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

bool Enemy::IsPlayerInLineOfSight() {
	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap((int)enemyPos.getX(), (int)enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap((int)playerPos.getX(), (int)playerPos.getY());

	int distance = abs(playerTilePos.getX() - enemyTilePos.getX());

	// Verifica si el jugador está dentro de la línea de visión de 5 tiles
	if (distance <= 10) {
		if (direction == 0 && playerTilePos.getX() < enemyTilePos.getX()) { // Izquierda
			for (int i = 1; i <= 5; ++i) {
				if (Engine::GetInstance().map.get()->IsTileCollidable(enemyTilePos.getX() - i, enemyTilePos.getY())) {
					return false; // Hay una colisión en la línea de visión
				}
				if (playerTilePos.getX() == enemyTilePos.getX() - i) {
					return true; // El jugador está en la línea de visión
				}
			}
		}
		else if (direction == 1 && playerTilePos.getX() > enemyTilePos.getX()) { // Derecha
			for (int i = 1; i <= 5; ++i) {
				if (Engine::GetInstance().map.get()->IsTileCollidable(enemyTilePos.getX() + i, enemyTilePos.getY())) {
					return false; // Hay una colisión en la línea de visión
				}
				if (playerTilePos.getX() == enemyTilePos.getX() + i) {
					return true; // El jugador está en la línea de visión
				}
			}
		}
	}

	return false;
}



void Enemy::DrawLineOfSight() {
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap((int)enemyPos.getX(), (int)enemyPos.getY());

	SDL_Renderer* renderer = Engine::GetInstance().render.get()->renderer;
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Color rojo para la línea de visión

	for (int i = 1; i <= 5; ++i) {
		int tileX = (direction == 0) ? enemyTilePos.getX() - i : enemyTilePos.getX() + i;
		int tileY = enemyTilePos.getY();

		if (Engine::GetInstance().map.get()->IsTileCollidable(tileX, tileY)) {
			break; // Detén el dibujo si hay una colisión en la línea de visión
		}

		Vector2D tilePos = Engine::GetInstance().map.get()->MapToWorld(tileX, tileY);
		SDL_Rect rect = { (int)tilePos.getX(), (int)enemyPos.getY(), Engine::GetInstance().map.get()->GetTileWidth(), Engine::GetInstance().map.get()->GetTileHeight() };
		SDL_RenderDrawRect(renderer, &rect);
	}
}





