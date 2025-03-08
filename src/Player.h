#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "Animation.h"
#include "Shurikens.h"
#include <vector>

struct SDL_Texture;


class Player : public Entity
{
public:

	Player();
	
	virtual ~Player();

	bool Awake();

	bool Start();

	bool Update(float dt);

	float Lerp(float start, float end, float factor);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

//	void ShootShuriken();

public:

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;
	int camX, camY;
	float smoothFactor = 0.1f;
	//Audio fx
	int pickCoinFxId;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	float jumpForce = 2.5f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping
	int hasAlreadyJumpedOnce = 0;
	bool isRolling = false;

	bool canDash = true;
	bool isDashing = false;
	float dashDuration = 2;
	float dashCooldown = 1; 
	float dashTimer = 0.0f;
	float dashSpeed = 10;
	float dashElapsedTime = 0;
	float dashLerpSpeed = 10.0f;
	float currentDashSpeed = 0.0f;
	float targetDashVelocity = 0;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;
	Animation walk;

	std::vector<Shuriken*> shurikens;
	EntityDirections playerDirection = EntityDirections::RIGHT;
};