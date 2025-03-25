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
	trailTexture = Engine::GetInstance().textures.get()->Load(parameters.attribute("trail_texture").as_string());

	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	// Cargar animaciones
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	attackAnimation.LoadAnimations(parameters.child("animations").child("attack"));
	currentAnimation = &idle;

	// Agregar física al enemigo
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	// Asignar tipo de colisionador
	pbody->ctype = ColliderType::ENEMY;
	pbody->listener = this;

	// Establecer la gravedad del cuerpo
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	// Inicializar pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	return true;
}


bool Enemy::Update(float dt)
{
	//ZoneScoped;
	// 
	if (Engine::GetInstance().scene.get()->currentState == GameState::PAUSED) {
	
		pbody->body->SetLinearVelocity(b2Vec2(0, 0));

		b2Transform pbodyPos = pbody->body->GetTransform();
		position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
		position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
		currentAnimation->Update();

		return true;
	
	}

	if (Engine::GetInstance().scene.get()->currentState != GameState::PLAYING) return true;


	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap((int)enemyPos.getX(), (int)enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap((int)playerPos.getX(), (int)playerPos.getY());

	if (IsPlayerInLineOfSight()) {
		state = EnemyState::AGGRESSIVE;
	}
	else if (IsPlayerInRange()) {
		if (IsPlayerInAttackRange() && attackCooldownTimer.ReadSec() >= 3) {
			state = EnemyState::ATTACK;
			attackDurationTimer.Start();
		}
		else {
			state = EnemyState::AGGRESSIVE;
		}
	}
	else {
		state = EnemyState::PATROL;
	}

	switch (state) {
	case EnemyState::PATROL:
		// Movimiento de patrulla
		if (!IsNextTileCollidable() || tilesMovedInSameDirection >= 150) {
			direction = (direction == 0) ? 1 : 0;
			tilesMovedInSameDirection = 0;
		}

		if (direction == 0) {
			pbody->body->SetLinearVelocity(b2Vec2(-2, 0));
		}
		else {
			pbody->body->SetLinearVelocity(b2Vec2(2, 0));
		}
		tilesMovedInSameDirection++;
		break;

	case EnemyState::AGGRESSIVE:
		// Movimiento agresivo
		if (playerTilePos.getX() < enemyTilePos.getX() && IsNextTileCollidable()) {
			pbody->body->SetLinearVelocity(b2Vec2(-2, 0));
			direction = 0;
		}
		else if (playerTilePos.getX() > enemyTilePos.getX() && IsNextTileCollidable()) {
			pbody->body->SetLinearVelocity(b2Vec2(2, 0));
			direction = 1;
		}
		else {
			pbody->body->SetLinearVelocity(b2Vec2(0, 0));
		}
		break;

	case EnemyState::ATTACK:
		// Lógica de ataque
		PerformAttack();

		if (attackDurationTimer.ReadSec() >= 1) {
			// Elimina la hitbox del ataque después de 1 segundo
			Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
			attackHitbox = nullptr;
			attackCooldownTimer.Start();
			state = EnemyState::AGGRESSIVE;
		}
		break;
	}

	// Dibuja la línea de visión del enemigo
	DrawLineOfSight();

	// Propagate the pathfinding algorithm using A* with the selected heuristic
	ResetPath();
	while (pathfinding->pathTiles.empty())
	{
		pathfinding->PropagateAStar(SQUARED);
	}

	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	// Draw pathfinding 

	if (Engine::GetInstance().physics.get()->debug) pathfinding->DrawPath();

	return true;
}




bool Enemy::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
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
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		
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
	return distance <= 10;
}

void Enemy::PerformAttack() {
	
	// Detiene al enemigo
	pbody->body->SetLinearVelocity(b2Vec2(0, 0));
	
	if (attackHitbox == nullptr) {
		// Crea la hitbox del ataque
		attackHitbox = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX()+2, (int)position.getY(), texW, texH, bodyType::STATIC);
		attackHitbox->ctype = ColliderType::ENEMY;
		attackHitbox->listener = this;
	}
		

	Engine::GetInstance().render.get()->DrawTexture(attackTexture, (int)position.getX(), (int)position.getY(), &attackAnimation.GetCurrentFrame());
	attackAnimation.Update();

	// Agrega la posición actual al trazo de la espada
	swordTrail.push_back(position);
	if (swordTrail.size() > maxTrailLength) {
		swordTrail.pop_front();
	}

	for (const auto& trailPos : swordTrail) {
		Engine::GetInstance().render.get()->DrawTexture(trailTexture, (int)trailPos.getX(), (int)trailPos.getY());
	}

	// Verifica colisiones con el jugador
	CheckAttackCollision();
}

void Enemy::CheckAttackCollision() {
	if (attackHitbox != nullptr) {
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
		}
	}
}


bool Enemy::IsPlayerInAttackRange() {
	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

	int distance = abs(playerTilePos.getX() - enemyTilePos.getX());
	return distance <= 3;
}

bool Enemy::IsPlayerInLineOfSight() {
	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap((int)enemyPos.getX(), (int)enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap((int)playerPos.getX(), (int)playerPos.getY());

	int distance = abs(playerTilePos.getX() - enemyTilePos.getX());

	// Verifica si el jugador está dentro de la línea de visión de 5 tiles
	if (distance <= 5) {
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





