#pragma once

#include "Entity.h"
#include "Animation.h"
#include "Physics.h"
#include "GuiControl.h"
#include "GuiImage.h"
#include "GuiSlider.h"
#include "GuiPopup.h"
#include "Text.h"
#include "SDL2/SDL.h"


enum class PlayerState {
    IDLE,
    WALKING,
    JUMPING,
    DASHING,
    CROUCHING,
    WALL_SLIDING
};

enum class GuiPopups {
    E_Interact
};

class Player : public Entity
{
public:
    Player();
    virtual ~Player();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    float Lerp(float start, float end, float factor);
    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;

    void SetPosition(Vector2D pos);
    Vector2D GetPosition();
    void ChangeHitboxSize(float width, float height);

    void GuiPOPup(GuiPopups guiPopup);
    void LoadPlayerFx();
    void TakeDamage(int damage);
    void Die();

    // Movement controls
    void ShootShuriken();
    void HandleMovement(float dt);
    void HandleJump(float dt);
    void HandleDash(float dt);
    void HandleCrouch();

    void SetParameters(pugi::xml_node parameters) {
        this->parameters = parameters;
    }

public:
    // Physics and movement
    PhysBody* pbody;
    float speed = 0.55f;
    float jumpForce = 125;
    float wallJumpForce = 90.5f;
    float wallJumpPush = 102.0f;
    float wallClimbSpeed = -2.0f;

    // States
    PlayerState currentState;
    EntityDirections playerDirection = EntityDirections::RIGHT;
    bool isJumping = false;
    bool isDashing = false;
    bool isWalking = false;
    bool crouched = false;
    bool touchingWall = false;
    bool inBubble = false;
    bool godMode = false;
    bool inGame = false;

    // Jump mechanics
    int hasAlreadyJumpedOnce = 0;
    const float maxHoldTime = 1500.0f;
    const float minJumpMultiplier = 1.2f;
    const float maxJumpMultiplier = 2.0f;
    float jumpHoldTimer = 0.0f;
    bool isHoldingJump = false;

    // Dash mechanics
    bool canDash = true;
    float dashDuration = 2.0f;
    float dashCooldown = 1.0f;
    float dashTimer = 0.0f;
    float dashSpeed = 10.0f;
    float dashElapsedTime = 0.0f;
    float targetDashVelocity = 0.0f;

    // Health system
    int HP = MAXHP;
    const int MAXHP = 100;
    float damageCooldown = 3.0f;
    float timeSinceLastDamage = 0.0f;
    bool canTakeDamage = true;

    // Visual elements
    SDL_Texture* texture = nullptr;
    int texW, texH;
    Animation* currentAnimation = nullptr;
    Animation idle;
    Animation walk;
    Animation jump;
    Animation dash;
    Animation crouch;

    // Camera control
    int camX, camY;
    float smoothFactor = 0.1f;

    // Level management
    int currentLevel = 1;
    bool loadLevel1 = false;
    bool loadLevel2 = false;

    // UI Elements
    GuiPopup* popup = nullptr;
    GuiImage* backgroundSliderImage = nullptr;
    GuiImage* foregroundSliderImage = nullptr;
    GuiSlider* HP_Slider = nullptr;
    GuiImage* orbUi = nullptr;
    Text* orbCount = nullptr;

    SDL_Rect btPos = { 960, 520, 40, 40 };
    SDL_Rect hpPos = { 150, 100, 200, 15 };
    SDL_Rect OrbPos = { 1300, 500, 100, 100 };
    SDL_Rect OrbCountPos = { 1400, 500, 100, 100 };

    // Audio FX IDs
    int jump1FxId, jump2FxId, jump3FxId;
    int doubleJump1FxId, doubleJump2FxId;
    int walkFxId;
    int crouchFxId;
    int dash1FxId, dash2FxId, dash3FxId;
    int throwShuriken1FxId, throwShuriken2FxId, throwShuriken3FxId;
    int weakKatana1FxId, weakKatana2FxId, weakKatana3FxId;
    int strongKatana1FxId, strongKatana2FxId;
    int dieFxId;
    int hit1FxId, hit2FxId;
    int pickUpItemFxId;
    int step1, step2, step3, step4, step5;

    // Footstep system
    float footstepTimer = 0.0f;
    const float footstepDelay = 0.4f;

    // Textures
    SDL_Texture* hpIconTexture = nullptr;
    SDL_Texture* BackgroundSliderHP = nullptr;
    SDL_Texture* ForeGroundSliderHP = nullptr;
    SDL_Texture* orbUiTexture = nullptr;

    // Game items
    int Orbs = 0;    
    pugi::xml_node parameters;
    Module* sceneModule = nullptr;
private:

};




