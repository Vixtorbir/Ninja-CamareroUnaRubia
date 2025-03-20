#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "Animation.h"

#include "GuiPopup.h"
#include "GuiControl.h"
#include "Module.h"
#include <vector>


struct SDL_Texture;

enum class GuiPopups
{
	E_Interact
};

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

	void GuiPOPup(GuiPopups guiPopup);

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void ShootShuriken();
	
	//sound
	void LoadPlayerFx();

	//damage
	void TakeDamage(int damage);
	void Die();

	void ChangeHitboxSize(float width, float height);

	


public:

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;
	int camX, camY;
	float smoothFactor = 0.1f;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	float jumpForce = 65.0f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping
	int hasAlreadyJumpedOnce = 0;
	bool isRolling = false;

	const float maxHoldTime = 1500.0f;  
	const float minJumpMultiplier = 0.5f;
	const float maxJumpMultiplier = 1.2f;
	float jumpHoldTimer = 0.0f;
	bool isHoldingJump = false;

	bool touchingWall = false;  
	float wallJumpForce = 40.5f; 
	float wallJumpPush = 2.0f;  
	float wallClimbSpeed = -1.5f;


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

	
	GuiPopup* popup;
	SDL_Rect btPos = { 960, 520, 40, 40};
	Module* sceneModule = nullptr;
	EntityDirections playerDirection = EntityDirections::RIGHT;
	
private:
		//Audio fx
		int jump1FxId;
		int jump2FxId;
		int jump3FxId;
		int doubleJump1FxId;
		int doubleJump2FxId;
		int walk1FxId;
		int walk2FxId;
		int walk3FxId;
		int walk4FxId;
		int walk5FxId;
		int dash1FxId;
		int dash2FxId;
		int dash3FxId;
		int throwShuriken1FxId;
		int throwShuriken2FxId;
		int throwShuriken3FxId;
		int weakKatana1FxId;
		int weakKatana2FxId;
		int weakKatana3FxId;
		int strongKatana1FxId;
		int strongKatana2FxId;
		int dieFxId;
		int hit1FxId;
		int hit2FxId;
		int pickUpItemFxId;
		

	int hp; 
	const int maxHp = 3;

	float damageCooldown = 3.0f; 
	float timeSinceLastDamage = 0.0f; 
	bool canTakeDamage = true;

	SDL_Texture* hpIconTexture;

	bool crouched;
};