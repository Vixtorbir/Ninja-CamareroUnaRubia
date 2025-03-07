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
	currentAnimation = &idle;

	// L08 TODO 5: Add physics to the player - initialize physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/retro-video-game-coin-pickup-38299.ogg");

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

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
		ShootShuriken();
	}
	
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
	// Move Up
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
		velocity.y = -0.2 * 16;
	}

	// Move down
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
		velocity.y = 0.2 * 16;
	}

	//Jump
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && (isJumping == false || hasAlreadyJumpedOnce <= 1)) {
		// Apply an initial upward force
		pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
		hasAlreadyJumpedOnce++;
		isJumping = true;
	}

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


	if (isJumping == true)
	{
		velocity.y = pbody->body->GetLinearVelocity().y;
	}

	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawEntity(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1, 0, 0, 0, (int)playerDirection);
	currentAnimation->Update();

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

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		isJumping = false;
		hasAlreadyJumpedOnce = 0;
		break;
	case ColliderType::ITEM:
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		Engine::GetInstance().physics.get()->DeletePhysBody(physB); // Deletes the body of the item from the physics world
		break;
	case ColliderType::UNKNOWN:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		break;
	case ColliderType::ITEM:
		break;
	case ColliderType::UNKNOWN:
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

void Player::ShootShuriken() {
	const int maxShurikens = 12;

	if (shurikens.size() >= maxShurikens) {
		return;
	}

	float shurikenSpeed = 5.0f;
	float spawnX = position.getX() + (texW / 2);
	float spawnY = position.getY() + (texH / 2);

	float direction = (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) ? -shurikenSpeed : shurikenSpeed;

	Shuriken* newShuriken = (Shuriken*)Engine::GetInstance().entityManager->CreateEntity(EntityType::SHURIKENS);
	shurikens.push_back(newShuriken);
}
