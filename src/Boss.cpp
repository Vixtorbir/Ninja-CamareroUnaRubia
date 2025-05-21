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
#include "Boss.h"

Boss::Boss() : Entity(EntityType::BOSS)
{

}

Boss::~Boss() {
	delete pathfinding;
}

bool Boss::Awake() {
	return true;
}

bool Boss::Start() {
	// Inicializar texturas
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());

	shurikenTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/goldCoin.png");
	attackTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/enemyAttack.png");


	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int()*2;
	texH = parameters.attribute("h").as_int()*2;

	// Cargar animaciones
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	attackAnimation.LoadAnimations(parameters.child("animations").child("attack"));
	currentAnimation = &idle;

	// Agregar física al enemigo
	pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), texW, texH, bodyType::DYNAMIC);

	// Asignar tipo de colisionador
	pbody->ctype = ColliderType::BOSS;
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


bool Boss::Update(float dt)
{

	if (dead)
	{
		// Si el enemigo está muerto, eliminarlo de la escena y la física
		if (attackBody != nullptr) {
			Engine::GetInstance().physics.get()->DeletePhysBody(attackBody);
			attackBody = nullptr;
			LOG("Attack hitbox removed.");
		}

		if (pbody != nullptr)
		{
			pbody->body->DestroyFixture(pbody->body->GetFixtureList());
			pbody = nullptr;
		}

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
		if (abs(playerTilePos.getX() - enemyTilePos.getX()) > 75)
		{
			pbody->body->SetLinearVelocity(b2Vec2(0, 0));
			b2Transform pbodyPos = pbody->body->GetTransform();
			position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 6);
			position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 6);

			Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
			currentAnimation->Update();
			return true;
		}

		// Determinar el estado del enemigo
		if (state != BossState::WAIT && state != BossState::ATTACK) // No cambiar el estado si está en espera o atacando
		{
			if (IsPlayerInRange())
			{
				if (IsPlayerInAttackRange())
				{
					state = BossState::ATTACK;
				}
				else
				{
					state = BossState::AGGRESSIVE;
				}
			}
			else
			{
				state = BossState::PATROL;
			}
		}

		// Manejar los estados del enemigo
		switch (state)
		{
		case BossState::PATROL:
			// Movimiento de patrulla
			if (!IsNextTileCollidable() || tilesMovedInSameDirection >= 350)
			{
				direction = (direction == 0) ? 1 : 0;
				tilesMovedInSameDirection = 0;
			}

			if (direction == 0)
			{
				pbody->body->SetLinearVelocity(b2Vec2(-2, 0));
			}
			else
			{
				pbody->body->SetLinearVelocity(b2Vec2(2, 0));
			}
			tilesMovedInSameDirection++;
			break;

		case BossState::AGGRESSIVE:
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

		case BossState::ATTACK:
			// El enemigo permanece quieto mientras ataca
			pbody->body->SetLinearVelocity(b2Vec2(0, 0));

			if (!isAttacking && !isCooldown)
			{
				PerformAttack();
				isAttacking = true;
				attackTimer.Start(); // Inicia el temporizador para el ataque
			}
			break;

		case BossState::WAIT:
			// El enemigo se queda quieto durante 2 segundos
			pbody->body->SetLinearVelocity(b2Vec2(0, 0));
			if (attackTimer.ReadSec() >= 2.0f) // Espera 2 segundos
			{
				state = BossState::PATROL; // Cambia al estado de patrulla después de esperar
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
			state = BossState::WAIT; // Cambia al estado de espera
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
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
		currentAnimation->Update();

		// Dibujar el pathfinding si está en modo debug
		if (Engine::GetInstance().physics.get()->debug)
		{
			pathfinding->DrawPath();
		}

		if (shootTimer.ReadSec() >= 6.0f) {
			ShootShuriken();
			shootTimer.Start(); // Reiniciar el temporizador
		}

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

	if (!canTakeDamage && damageCooldownTimer.ReadSec() >= damageCooldown) {
		canTakeDamage = true;
		LOG("Boss can take damage again.");
	}

	return true;
}

void Boss::ShootShuriken() {
	// Crear el shuriken como un sensor físico
	PhysBody* shuriken = Engine::GetInstance().physics.get()->CreateRectangleSensor(
		(int)position.getX() + (direction == 0 ? -texW / 2 : texW / 2), 
		(int)position.getY() + texH / 4, 
		40, 40, 
		bodyType::DYNAMIC
	);

	// Configurar propiedades del shuriken
	shuriken->ctype = ColliderType::ENEMY; 
	shuriken->listener = this;
	shuriken->body->SetBullet(true);
	shuriken->body->SetFixedRotation(true);
	shuriken->body->SetGravityScale(0.0f);

	// Determinar la dirección del disparo
	float shurikenSpeed = 5.0f;
	b2Vec2 impulse = (direction == 0) ? b2Vec2(-shurikenSpeed, 0) : b2Vec2(shurikenSpeed, 0); 
	shuriken->body->ApplyLinearImpulseToCenter(impulse, true);

	// Agregar el shuriken a la lista de shurikens activos
	Shuriken3 newShuriken = { shuriken, Timer() };
	newShuriken.timer.Start();
	activeShurikens.push_back(newShuriken);

	LOG("Boss shot a shuriken in direction: %s", (direction == 0 ? "LEFT" : "RIGHT"));
}


void Boss::TakeDamage(int damage) {
	if (canTakeDamage) {
		health -= damage;
		LOG("Boss took damage! Remaining health: %d", health);

		if (health <= 0) {
			dead = true;
			LOG("Boss is dead!");
		}

		canTakeDamage = false; 
		damageCooldownTimer.Start(); 
	}
	else {
		LOG("Boss is in damage cooldown, cannot take damage.");
	}
}


bool Boss::CleanUp() {
	Engine::GetInstance().textures.get()->UnLoad(texture);
	Engine::GetInstance().textures.get()->UnLoad(attackTexture);

	if (pbody != nullptr) {
		Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	}
	active = false;
	return true;
}


void Boss::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Boss::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Boss::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

void Boss::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {

	case ColliderType::PLAYER:

		if (physA->ctype == ColliderType::BOSS) {
			Engine::GetInstance().scene.get()->player->TakeDamage(1);
			LOG("Player hit by boss shuriken!");
		}
		break;
	}
}


void Boss::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collision player");
		break;
	}
}

bool Boss::IsNextTileCollidable() {
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



bool Boss::IsPlayerInRange() {
	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

	int distance = abs(playerTilePos.getX() - enemyTilePos.getX());
	bool isInRange = distance <= 16;

	// Verificar si el enemigo está mirando en la dirección del jugador
	bool isFacingPlayer = (direction == 0 && playerTilePos.getX() < enemyTilePos.getX()) ||
		(direction == 1 && playerTilePos.getX() > enemyTilePos.getX());

	return isInRange && isFacingPlayer;
}


void Boss::PerformAttack() {
	// Detiene al enemigo
	pbody->body->SetLinearVelocity(b2Vec2(0, 0));

	if (attackBody == nullptr) {
		// Determina la dirección del ataque
		int attackOffsetX = (direction == 0) ? -80 : 110; // Ajusta según la dirección del enemigo

		// Crea la hitbox del ataque
		attackBody = Engine::GetInstance().physics.get()->CreateRectangleSensor(
			(int)position.getX() + attackOffsetX,
			(int)position.getY() + 10,
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




void Boss::CheckAttackCollision() {
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


bool Boss::IsPlayerInAttackRange() {
	Vector2D playerPos = Engine::GetInstance().scene.get()->player->GetPosition();
	Vector2D enemyPos = GetPosition();
	Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
	Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

	int distance = abs(playerTilePos.getX() - enemyTilePos.getX());
	return distance <= 10;
}

void Boss::LoadEnemyFx()
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

bool Boss::IsPlayerInLineOfSight() {
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



void Boss::DrawLineOfSight() {
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





