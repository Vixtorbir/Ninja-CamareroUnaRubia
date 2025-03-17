#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "GuiManager.h"
//#include "tracy/Tracy.hpp"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {

	return true;
}

bool Player::Start() {
	
	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	walk.LoadAnimations(parameters.child("animations").child("walk"));

	currentAnimation = &idle;

	// L08 TODO 5: Add physics to the player - initialize physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), texW, texH, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	pbody->body->SetFixedRotation(true);

	popup = (GuiPopup*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::POPUP, 1, "E", btPos, sceneModule);
	

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	//initialize audio effect
	LoadPlayerFx();

	hp = maxHp;
	timeSinceLastDamage = damageCooldown;
	canTakeDamage = true;

	hpIconTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/hp_icon.png");

	return true;
}

bool Player::Update(float dt)
{
	if (!canDash) {
		dashTimer += dt / 1000;
		if (dashTimer >= dashCooldown) {
			canDash = true; // Dash is ready again
			dashTimer = 0.0f;
		}
	}
	//ZoneScoped;
	// Code you want to profile

	
	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

	if (!parameters.attribute("gravity").as_bool()) {
		velocity = b2Vec2(0, 0);
	}

	// Move left
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -0.2 * 16;
		playerDirection = EntityDirections::LEFT;
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = 0.2 * 16;
		playerDirection = EntityDirections::RIGHT;
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT ||
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		currentAnimation = &walk;
	}
	else {
		currentAnimation = &idle;
	}
	// Move Up
	/*if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
		velocity.y = -0.2 * 16;
	}
	*/
	// Move down
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
		velocity.y = 0.2 * 16;
	}

	//Jump
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_DOWN && hasAlreadyJumpedOnce == 0) 
	{
		isHoldingJump = true;
		jumpHoldTimer = 0.0f;
		
		
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT && isHoldingJump)
	{
		jumpHoldTimer += dt;

		if (jumpHoldTimer >= maxHoldTime) {
			float jumpStrength = jumpForce * maxJumpMultiplier;
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpStrength), true);
			hasAlreadyJumpedOnce++;
			isHoldingJump = false;
			isJumping = true;
			
		}
		Engine::GetInstance().audio.get()->PlayFx(jump1FxId);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_UP && isHoldingJump) 
	{
		float holdPercentage = jumpHoldTimer / maxHoldTime;
		float jumpMultiplier = minJumpMultiplier + (holdPercentage * (maxJumpMultiplier - minJumpMultiplier));
		float jumpStrength = jumpForce * jumpMultiplier;

		pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpStrength), true);
		hasAlreadyJumpedOnce++;
		isHoldingJump = false;
		isJumping = true;
	}

	
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_DOWN && hasAlreadyJumpedOnce == 1) {
		pbody->body->SetLinearVelocity(b2Vec2(pbody->body->GetLinearVelocity().x, 0)); 
		pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
		hasAlreadyJumpedOnce++;
		isJumping = true;
		Engine::GetInstance().audio.get()->PlayFx(throwShuriken1FxId);
	}

	// Wall Jump
	if (touchingWall && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_L) == KEY_DOWN) {
		float jumpDirection = (playerDirection == EntityDirections::LEFT) ? 1.0f : -1.0f;

		pbody->body->SetLinearVelocity(b2Vec2(0, 0)); 
		pbody->body->ApplyLinearImpulseToCenter(b2Vec2(jumpDirection * wallJumpPush, -wallJumpForce), true);

		isHoldingJump = false;
		jumpHoldTimer = 0.0f;

		playerDirection = (playerDirection == EntityDirections::LEFT) ? EntityDirections::RIGHT : EntityDirections::LEFT;
	}

	if (touchingWall && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
		pbody->body->SetLinearVelocity(b2Vec2(0, wallClimbSpeed));
	}


	//Dash
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN && canDash) {
		if (playerDirection == EntityDirections::RIGHT) {
			targetDashVelocity = dashSpeed * dt;
		}
		else if (playerDirection == EntityDirections::LEFT) {
			targetDashVelocity = -dashSpeed * dt;
		}
		canDash = false;
		dashTimer = 0.0f;
		isDashing = true;
		dashElapsedTime = 0.0f;
		int dashId = Engine::GetInstance().audio.get()->randomFx(dash1FxId,dash3FxId);
		Engine::GetInstance().audio.get()->PlayFx(dashId);
	}

	if (isDashing) {
		float lerpFactor = .4f;
		velocity.x = Lerp(velocity.x, targetDashVelocity, lerpFactor);

		dashElapsedTime += dt;

		if (dashElapsedTime >= dashDuration) {
			isDashing = false;
			targetDashVelocity = 0.0f;
		}
	}

	if (!canTakeDamage) {
		timeSinceLastDamage += dt / 1000;
		if (timeSinceLastDamage >= damageCooldown) {
			canTakeDamage = true;
			timeSinceLastDamage = 0.0f;
		}
	}


	if (isJumping == true)
	{
		velocity.y = pbody->body->GetLinearVelocity().y;
	}

	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawEntity(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1, 0, 0, 0, (int)playerDirection);
	currentAnimation->Update();

	if (!Engine::GetInstance().scene.get()->watchtitle)
	{
		camX = -(float)position.getX() + (Engine::GetInstance().render.get()->camera.w / 2);
		camY = -(float)position.getY() + (Engine::GetInstance().render.get()->camera.h / 2);

		Engine::GetInstance().render.get()->camera.x += (camX - Engine::GetInstance().render.get()->camera.x) * smoothFactor;
		Engine::GetInstance().render.get()->camera.y += (camY - Engine::GetInstance().render.get()->camera.y) * smoothFactor;
	}
	else {
		camX = 1920 + (Engine::GetInstance().render.get()->camera.w / 2);
		camY = -(float)position.getY() + (Engine::GetInstance().render.get()->camera.h / 2);

		Engine::GetInstance().render.get()->camera.x += (camX - Engine::GetInstance().render.get()->camera.x) * smoothFactor;
		Engine::GetInstance().render.get()->camera.y += (camY - Engine::GetInstance().render.get()->camera.y) * smoothFactor;
	}

	Engine::GetInstance().render.get()->camera.x += (camX - Engine::GetInstance().render.get()->camera.x) * smoothFactor;
	Engine::GetInstance().render.get()->camera.y += (camY - Engine::GetInstance().render.get()->camera.y) * smoothFactor;

	for (int i = 0; i < hp; ++i) {
		Engine::GetInstance().render.get()->DrawTexture(hpIconTexture, 10 + i * 40, 10);
	}

	return true;
}
float Player::Lerp(float start, float end, float factor) {
	return start + factor * (end - start);
}
bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

void Player::GuiPOPup(GuiPopups guiPopup)
{
	switch (guiPopup)
	{
	case GuiPopups::E_Interact:
		popup->isActive = true;
		break;
	}
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		isJumping = false;
		hasAlreadyJumpedOnce = 0;
		break;
	case ColliderType::NPC:
		GuiPOPup(GuiPopups::E_Interact);
		break;
	case ColliderType::WALL:
		touchingWall = true;
		break;
	case ColliderType::ITEM:
		Engine::GetInstance().physics.get()->DeletePhysBody(physB); // Deletes the body of the item from the physics world
		break;
	case ColliderType::UNKNOWN:
		break;

	case ColliderType::ENEMY:
		TakeDamage(1); // El jugador recibe 1 punto de da�o
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		break;
	case ColliderType::NPC:
		popup->isActive = false;

		break;
	case ColliderType::UNKNOWN:
		break;
	case ColliderType::WALL:
		touchingWall = false;
		break;
	}
}

void Player::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Player::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}
void Player::LoadPlayerFx()
{
	
	 jump1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 jump2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 jump3FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 doubleJump1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 doubleJump2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 walk1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 walk2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 walk3FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 walk4FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 walk5FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 dash1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 dash2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 dash3FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 throwShuriken1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 throwShuriken2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 throwShuriken3FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 weakKatana1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 weakKatana2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 weakKatana3FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 strongKatana1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 strongKatana2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 dieFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 hit1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
	 hit2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/");
}

void Player::TakeDamage(int damage) {
	if (canTakeDamage) {
		hp -= damage;
		if (hp <= 0) {
			Die();
		}
		canTakeDamage = false;
		timeSinceLastDamage = 0.0f;
	}
}


void Player::Die() {
	
	LOG("Player has died");
	// Puedes reiniciar el nivel, mostrar una pantalla de game over, etc.
}


