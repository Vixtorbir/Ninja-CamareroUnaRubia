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
	//The enemy has to move to the right automatically modify to make the enemy move left and right after 5 seconds

	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

	// Cambia de estado si el jugador está en rango
	if (IsPlayerInRange()) {
		if (IsPlayerInAttackRange()) {
			state = EnemyState::ATTACK;
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
			tilesMovedInSameDirection = 0; // Reinicia el contador de tiles recorridos en la misma dirección
		}

		if (direction == 0) {
			pbody->body->SetLinearVelocity(b2Vec2(-2, 0));
		}
		else {
			pbody->body->SetLinearVelocity(b2Vec2(2, 0));
		}
		tilesMovedInSameDirection++; // Incrementa el contador de tiles recorridos en la misma dirección
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
		break;
	}


	// Pathfinding testing inputs
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_R) == KEY_DOWN) {
		Vector2D pos = GetPosition();
		Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(),pos.getY());
		pathfinding->ResetPath(tilePos);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
		pathfinding->PropagateBFS();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateBFS();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_DOWN) {
		pathfinding->PropagateDijkstra();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateDijkstra();
	}

	// L13: TODO 3:	Add the key inputs to propagate the A* algorithm with different heuristics (Manhattan, Euclidean, Squared)

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_B) == KEY_DOWN) {
		pathfinding->PropagateAStar(MANHATTAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_B) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(MANHATTAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_N) == KEY_DOWN) {
		pathfinding->PropagateAStar(EUCLIDEAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_N) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(EUCLIDEAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_DOWN) {
		pathfinding->PropagateAStar(SQUARED);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(SQUARED);
	}

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

	if (direction == 0) { 
		tilePos.setX(tilePos.getX() - 1);
	}
	else { 
		tilePos.setX(tilePos.getX() + 1);
	}

	return Engine::GetInstance().map.get()->IsTileCollidable(tilePos.getX(), tilePos.getY() + 1); 
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
	
	Engine::GetInstance().render.get()->DrawTexture(attackTexture, (int)position.getX(), (int)position.getY(), &attackAnimation.GetCurrentFrame());
	attackAnimation.Update();

	
	swordTrail.push_back(position);
	if (swordTrail.size() > maxTrailLength) {
		swordTrail.pop_front();
	}

	for (const auto& trailPos : swordTrail) {
		Engine::GetInstance().render.get()->DrawTexture(trailTexture, (int)trailPos.getX(), (int)trailPos.getY());
	}
}

bool Enemy::IsPlayerInAttackRange() {
	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

	int distance = abs(playerTilePos.getX() - enemyTilePos.getX());
	return distance <= 5;
}

