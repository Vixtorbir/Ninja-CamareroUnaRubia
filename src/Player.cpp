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

Player::Player() : Entity(EntityType::PLAYER)
{
    name = "Player";
    crouched = false;
    godMode = false;
    currentState = PlayerState::IDLE;
}

Player::~Player() {}

bool Player::Awake() {
    return true;
}

bool Player::Start() {
    // Initialize parameters
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());

    // Load UI textures
    BackgroundSliderHP = Engine::GetInstance().textures.get()->Load("Assets/UI/lifeBarBack.png");
    ForeGroundSliderHP = Engine::GetInstance().textures.get()->Load("Assets/UI/lifeBarFront.png");
    orbUiTexture = Engine::GetInstance().textures.get()->Load("Assets/UI/OrbUi.png");
    shurikenTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/goldCoin.png");
    meleeAttackTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/meleeAttack.png");

    Hidden = Engine::GetInstance().textures.get()->Load("Assets/UI/Hidden.png");
    Detected = Engine::GetInstance().textures.get()->Load("Assets/UI/Detected.png");

    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    // Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    walk.LoadAnimations(parameters.child("animations").child("walk"));
    jump.LoadAnimations(parameters.child("animations").child("jump"));
    dash.LoadAnimations(parameters.child("animations").child("dash"));
    crouch.LoadAnimations(parameters.child("animations").child("crouch"));
    attack1.LoadAnimations(parameters.child("animations").child("attack1"));
    attack2.LoadAnimations(parameters.child("animations").child("attack2"));
    attack3.LoadAnimations(parameters.child("animations").child("attack3"));
    climb.LoadAnimations(parameters.child("animations").child("climb"));

    currentAnimation = &idle;

    // Physics setup
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), texW/2, texH-50, bodyType::DYNAMIC);
    pbody->listener = this;
    pbody->ctype = ColliderType::PLAYER;
    pbody->body->SetFixedRotation(true);
    pbody->body->SetGravityScale(5);
	//quitale la friccion al player
	pbody->body->SetLinearDamping(0.0f);
	pbody->body->SetAngularDamping(0.0f);
	


    // UI elements
    popup = (GuiPopup*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::POPUP, 1, "Press E", btPos, sceneModule);
    orbCount = (Text*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::TEXT, 1, "0", OrbCountPos, sceneModule);
    backgroundSliderImage = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, " ", btPos, sceneModule, BackgroundSliderHP);
    foregroundSliderImage = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, " ", btPos, sceneModule, ForeGroundSliderHP);
    
    detected = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, " ", btPos, sceneModule, Detected);
    hidden = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, " ", btPos, sceneModule, Hidden);

    orbUi = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, " ", btPos, sceneModule, orbUiTexture);
    HP_Slider = (GuiSlider*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::HPSLIDER, 1, " ", hpPos, sceneModule);

    HP_Slider->SetSliderBarSize(200, 15);
    HP_Slider->SetSliderBarInnerSize(100, 10);

    backgroundSliderImage->visible = false;
    foregroundSliderImage->visible = false;
    HP_Slider->visible = false;

    // Initialize other variables
    timeSinceLastDamage = damageCooldown;
    canTakeDamage = true;
    hpIconTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/hp_icon.png");

    LoadPlayerFx();
    return true;
}

bool Player::Update(float dt) {
    // Update UI elements
    orbCount->SetText(std::to_string(Orbs));
    backgroundSliderImage->visible = inGame;
    foregroundSliderImage->visible = inGame;
    HP_Slider->visible = inGame;


    if (Engine::GetInstance().scene.get()->currentState == GameState::PAUSED) {
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        b2Transform pbodyPos = pbody->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
        currentAnimation->Update();
        return true;
    }

    if (Engine::GetInstance().scene.get()->currentState != GameState::PLAYING) {
        return true;
    }

    // Handle HP
    /*HP_Slider->SetSliderBarInnerSize(HP * 4, 50);
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN) HP += 20;
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_DOWN) HP -= 20;
    if (HP >= MAXHP) HP = MAXHP; else if (HP <= 0) HP = 0;*/

    // God mode toggle
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) godMode = true;
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F11) == KEY_DOWN) godMode = false;

    // Dash cooldown
    if (!canDash) {
        dashTimer += dt / 1000;
        if (dashTimer >= dashCooldown) {
            canDash = true;
            dashTimer = 0.0f;
        }
    }

    // Physics movement
    b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
    if (!parameters.attribute("gravity").as_bool()) velocity = b2Vec2(0, 0);

    // State management
    bool movingLeft = Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT;
    bool movingRight = Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT;
    bool moving = movingLeft || movingRight;
    bool grounded = !isJumping && !isDashing && !touchingWall;
    isWalking = false;
    if (movingLeft && !still) {
        velocity.x = -speed * 16;
        playerDirection = EntityDirections::LEFT;
    }
    if (movingRight && !still) {
        velocity.x = speed * 16;
        playerDirection = EntityDirections::RIGHT;
    }

   
    bool wantsToCrouch = Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT ||
        Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_DOWN;

    if (wantsToCrouch && grounded) {  // Only allow crouching when grounded
        crouched = true;
        currentState = PlayerState::CROUCHING;
        if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_DOWN) {
            ChangeHitboxSize(texW, texH / 2);
            Engine::GetInstance().audio.get()->PlayFx(crouchFxId);
        }
    }
    else if (crouched && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_UP) {
        crouched = false;
        ChangeHitboxSize(texW, texH);
        // Don't force IDLE state here - let the state priority system handle it
    }

    // Handle jumping
    bool jumpPressed = Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN;
    bool jumpReleased = Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_UP;

    if (jumpPressed && !jumpKeyHeld && hasAlreadyJumpedOnce < 2 && !inBubble && !crouched) {
        float appliedForce = (hasAlreadyJumpedOnce == 0) ? jumpForce : doubleJumpForce;

        b2Vec2 vel = pbody->body->GetLinearVelocity();
        vel.y = 0;
        pbody->body->SetLinearVelocity(vel);

        pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -appliedForce), true);

        int fxId = (hasAlreadyJumpedOnce == 0)
            ? Engine::GetInstance().audio.get()->randomFx(jump1FxId, jump3FxId)
            : Engine::GetInstance().audio.get()->randomFx(doubleJump1FxId, doubleJump2FxId);
        Engine::GetInstance().audio.get()->PlayFx(fxId);

        hasAlreadyJumpedOnce++;
        isJumping = true;
        currentState = PlayerState::JUMPING;
        jumpKeyHeld = true;

        LOG("Jump count: %d", hasAlreadyJumpedOnce);
    }

    if (jumpReleased) {
        jumpKeyHeld = false;
    }

    /*
    // Wall jump
    if (touchingWall && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_L) == KEY_DOWN) {
        float jumpDirection = (playerDirection == EntityDirections::LEFT) ? 1.0f : -1.0f;
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        pbody->body->ApplyLinearImpulseToCenter(b2Vec2(jumpDirection * wallJumpPush, -wallJumpForce), true);
        isHoldingJump = false;
        jumpHoldTimer = 0.0f;
        playerDirection = (playerDirection == EntityDirections::LEFT) ? EntityDirections::RIGHT : EntityDirections::LEFT;
        int doubleJumpId = Engine::GetInstance().audio.get()->randomFx(doubleJump1FxId, doubleJump2FxId);
        Engine::GetInstance().audio.get()->PlayFx(doubleJumpId);
        currentAnimation == &climb;

        //currentState = PlayerState::JUMPING;
    }

    // Wall slide
    if (touchingWall && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) {
        currentAnimation == &climb;
        pbody->body->SetLinearVelocity(b2Vec2(0, wallClimbSpeed));
        currentState = PlayerState::WALL_SLIDING;
    }*/

    // Dash
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
        currentState = PlayerState::DASHING;
        currentAnimation->Reset();
        int dashId = Engine::GetInstance().audio.get()->randomFx(dash1FxId, dash3FxId);
        Engine::GetInstance().audio.get()->PlayFx(dashId);
    }

    if (isDashing) {
        float lerpFactor = .4f;
        //velocity.x = Lerp(velocity.x, targetDashVelocity, lerpFactor);
        dashElapsedTime += dt;

        if (dashElapsedTime >= dashDuration) {
            isDashing = false;
            targetDashVelocity = 0.0f;
            currentState = grounded ? PlayerState::IDLE : PlayerState::JUMPING;
        }
    }

    // Damage cooldown
    if (!canTakeDamage) {
        timeSinceLastDamage += dt / 1000;
        if (timeSinceLastDamage >= damageCooldown) {
            canTakeDamage = true;
            timeSinceLastDamage = 0.0f;
        }
    }

    // State priority system (highest priority first)
    if (isDashing) {
        currentState = PlayerState::DASHING;
    }
    else if (isJumping) {
        currentState = PlayerState::JUMPING;
    }
    else if (crouched) {
        currentState = PlayerState::CROUCHING;
    }
    else if (isAttacking) { 
        currentState = PlayerState::ATTACKING;
    }
    else if (touchingWall && !grounded) {
        currentState = PlayerState::WALL_SLIDING;
    }
    else if (moving && grounded) {
        currentState = PlayerState::WALKING;
        isWalking = true;
    }
    else {
        currentState = PlayerState::IDLE;
    }

    // Animation selection
    switch (currentState) {
    case PlayerState::DASHING:
        currentAnimation = &dash;
        break;
    case PlayerState::JUMPING:
        currentAnimation = &jump;
        break;
    case PlayerState::CROUCHING:
        currentAnimation = &crouch;
        break;
    case PlayerState::WALL_SLIDING:
        currentAnimation = &idle; // Replace with wall slide animation if available
        break;
    case PlayerState::WALKING:
        currentAnimation = &walk;
        // Footstep sounds
        footstepTimer += dt;
        if (footstepTimer >= footstepDelay) {
            int stepSounds[] = { step1, step2, step3, step4, step5 };
            int randomIndex = rand() % 5;
            Engine::GetInstance().audio.get()->PlayFx(stepSounds[randomIndex]);
            footstepTimer = 0.0f;
        }
        break;
    case PlayerState::ATTACKING: 
        still = true;
        break;
    case PlayerState::IDLE:
    default:
        currentAnimation = &idle;
        break;
    }
    // Apply velocity
    if (isJumping) velocity.y = pbody->body->GetLinearVelocity().y;
    pbody->body->SetLinearVelocity(velocity);

    // Update position
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW - 135);

    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - (texH / 2)-27);

    int renderOffsetY = 0;
    if (crouched) {
        renderOffsetY = -45; 
    }

    // Renderizar la textura del jugador
    Engine::GetInstance().render.get()->DrawEntity(
        texture,
        (int)position.getX(),
        (int)position.getY() + renderOffsetY , 
        &currentAnimation->GetCurrentFrame(),
        1, 0, 0, 0, (int)playerDirection
    );

    currentAnimation->Update();

    // Camera follow
    if (!Engine::GetInstance().scene.get()->watchtitle) {
        camX = -(float)position.getX() - 256 + (Engine::GetInstance().render.get()->camera.w / 2);
        camY = -(float)position.getY() + (Engine::GetInstance().render.get()->camera.h / 2);
    }
    else {
        camX = 2000 + (Engine::GetInstance().render.get()->camera.w / 2);
        camY = -(float)position.getY() + (Engine::GetInstance().render.get()->camera.h / 2);
    }

    Engine::GetInstance().render.get()->camera.x += ((int)camX - Engine::GetInstance().render.get()->camera.x) * smoothFactor;
    Engine::GetInstance().render.get()->camera.y += ((int)camY - Engine::GetInstance().render.get()->camera.y) * smoothFactor;

    // Draw HP icons
  /*  for (int i = 0; i < hp; ++i) {
        Engine::GetInstance().render.get()->DrawTexture(hpIconTexture, 10 + i * 40, 10);
    }*/

    // Level transitions
    if (GetPosition().getX() >= 20480 && GetPosition().getY() >= 4320 && currentLevel == 1) {
        if (loadLevel2 == false) {
            loadLevel2 = true;
        }
    }
    

    
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_DOWN && !isAttacking && !isCooldown) {
        PerformAttack();
        isAttacking = true;
        attackTimer.Start(); 
    }

    if (isAttacking && attackTimer.ReadSec() >= attackDuration) {
        isAttacking = false;
        isCooldown = true;
        attackCooldownTimer.Start(); 
        if (katanaAttack != nullptr) {
            Engine::GetInstance().physics.get()->DeletePhysBody(katanaAttack);
            katanaAttack = nullptr;
        }
        still = false;

        LOG("Attack ended, cooldown started.");
    }
    if (isCooldown && attackCooldownTimer.ReadSec() >= attackCooldown) {
        isCooldown = false;
    }
    if (isAttacking && katanaAttack != nullptr) {
        int x, y;
        katanaAttack->GetPosition(x, y);

        //Esto ya se pondra bien cuando haya una textura

        int textureWidth = 80;  
        int textureHeight = 250; 
        int renderX = x - textureWidth / 2;
        int renderY = y - textureHeight / 2;

        Engine::GetInstance().render.get()->DrawTexture(meleeAttackTexture, renderX, renderY);
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_O) == KEY_DOWN) {
        ThrowShuriken();
    }

    for (auto it = activeShurikens.begin(); it != activeShurikens.end(); ) {
        if (it->timer.ReadSec() >= 3.5f) { // Si han pasado 5 segundos
            Engine::GetInstance().physics.get()->DeletePhysBody(it->body); // Eliminar el cuerpo físico
            it = activeShurikens.erase(it); // Eliminar de la lista
        }
        else {
            ++it; // Continuar con el siguiente shuriken
        }
    }

    // Renderizar los shurikens
    for (const auto& shuriken : activeShurikens) {
        int x, y;
        shuriken.body->GetPosition(x, y);
        Engine::GetInstance().render.get()->DrawTexture(shurikenTexture, x + 25, y + 25);
    }

    if (!canShootShuriken && shurikenCooldownTimer.ReadSec() >= 1.0f) {
        canShootShuriken = true;
        LOG("Shuriken cooldown ended. Player can shoot again.");
    }

    return true;
}


float Player::Lerp(float start, float end, float factor) {
	return start + factor * (end - start);
}


bool Player::CleanUp() {
    LOG("Cleanup player");
    Engine::GetInstance().textures.get()->UnLoad(texture);
    Engine::GetInstance().textures.get()->UnLoad(meleeAttackTexture); 
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
		LOG("Grounded - reset jump count");
    isJumping = false;
    hasAlreadyJumpedOnce = 0;
    break;
	case ColliderType::NPC:
		inBubble = true;
		GuiPOPup(GuiPopups::E_Interact);
		break;
	case ColliderType::WALL:
		
		break;
    case ColliderType::ITEM: {
        Item* item = static_cast<Item*>(physB->listener);
        if (item != nullptr) {
            // Añadir el objeto al inventario
            AddItem(InventoryItem(item->name, item->quantity, item->description, item->icon));

            // Reproducir efecto de sonido de recogida
            Engine::GetInstance().audio.get()->PlayFx(pickUpItemFxId);
            Orbs++;

            // Desactivar el objeto en el mundo
			Engine::GetInstance().render.get()->DrawText(("Picked up: " + item->name).c_str(), 600, 200, 750, 255); // Mensaje en pantalla
			Engine::GetInstance().physics.get()->DeletePhysBody(physB); 
            item->active = false;
        }
        break;
    }

	case ColliderType::UNKNOWN:
		break;

    case ColliderType::ENEMY:
        if (physA->ctype == ColliderType::PLAYER_ATTACK) {
            
            Enemy* enemy = static_cast<Enemy*>(physB->listener);
            if (enemy != nullptr) {
				enemy->startDying = true;
				Engine::GetInstance().audio.get()->PlayFx(weakKatana1FxId);
				
            }

            
            activeShurikens.erase(
                std::remove_if(activeShurikens.begin(), activeShurikens.end(),
                    [physA](const Shuriken& shuriken) { return shuriken.body == physA; }),
                activeShurikens.end()
            );

            Engine::GetInstance().physics.get()->DeletePhysBody(physA);
        }

		else if (physA->ctype == ColliderType::PLAYER_KATANA) {
			Enemy* enemy = static_cast<Enemy*>(physB->listener);
			if (enemy != nullptr) {
				enemy->startDying = true;
				Engine::GetInstance().audio.get()->PlayFx(weakKatana1FxId);
			}
		}
        break;
	case ColliderType::TURRET:
		if (physA->ctype == ColliderType::PLAYER_ATTACK) {
			Turret* turret = static_cast<Turret*>(physB->listener);
			if (turret != nullptr) {
				turret->dead = true;
				Engine::GetInstance().audio.get()->PlayFx(weakKatana1FxId);
			}
			activeShurikens.erase(
				std::remove_if(activeShurikens.begin(), activeShurikens.end(),
					[physA](const Shuriken& shuriken) { return shuriken.body == physA; }),
				activeShurikens.end()
			);
		Engine::GetInstance().physics.get()->DeletePhysBody(physA);
		}

        else if (physA->ctype == ColliderType::PLAYER_KATANA) {
            Turret* turret = static_cast<Turret*>(physB->listener);
            if (turret != nullptr) {
                turret->dead = true;
                Engine::GetInstance().audio.get()->PlayFx(weakKatana1FxId);
            }
        }
		break;
        case ColliderType::BOSS:
        if (physA->ctype == ColliderType::PLAYER_ATTACK) {
            Boss* boss = static_cast<Boss*>(physB->listener);
            if (boss != nullptr && boss->canTakeDamage) { 
                boss->TakeDamage(1);
                Engine::GetInstance().audio.get()->PlayFx(weakKatana1FxId);

                activeShurikens.erase(
                    std::remove_if(activeShurikens.begin(), activeShurikens.end(),
                        [physA](const Shuriken& shuriken) { return shuriken.body == physA; }),
                    activeShurikens.end()
                );

                Engine::GetInstance().physics.get()->DeletePhysBody(physA);
            }
            else {
                LOG("Boss is in damage cooldown, shuriken has no effect.");
            }
        }
        else if (physA->ctype == ColliderType::PLAYER_KATANA) {
            Boss* boss = static_cast<Boss*>(physB->listener);
            if (boss != nullptr) {
                boss->TakeDamage(1);
                Engine::GetInstance().audio.get()->PlayFx(weakKatana1FxId);
            }
        }
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
		inBubble = false;
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
	
	 jump1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump1.ogg");
	 jump2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump2.ogg");
	 jump3FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/Jump3.ogg");
	 doubleJump1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/doubleJump1.ogg");
	 doubleJump2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/doubleJump2.ogg");
	 walkFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/playerRunLoopFx.ogg");
	 crouchFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/crouchFx.ogg");
	 dash1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/dash1.ogg");
	 dash2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/dash2.ogg");
	 dash3FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/dash3.ogg");
	 throwShuriken1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/shootShuriken1.ogg");
	 throwShuriken2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/shootShuriken2.ogg");
	 throwShuriken3FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/shootShuriken3.ogg");
	 weakKatana1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/lightSwordSlash1.ogg");
	 weakKatana2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/lightSwordSlash2.ogg");
	 weakKatana3FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/lightSwordSlash3.ogg");
	 strongKatana1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/heavySwordSlash1.ogg");
	 strongKatana2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/heavySwordSlash2.ogg");
	 dieFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/die.ogg");
	 hit1FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/hit1.ogg");
	 hit2FxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/PlayerFx/hit2.ogg");
	 pickUpItemFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/ExtraFx/pickUpItem.ogg");
	 step1 = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PlayerFx/wood-step1.ogg");
	 step2 = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PlayerFx/wood-step2.ogg");
	 step3 = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PlayerFx/wood-step3.ogg");
	 step4 = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PlayerFx/wood-step4.ogg");
	 step5 = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PlayerFx/wood-step5.ogg");

}

void Player::TakeDamage(int damage) {
    if (canTakeDamage) {
        HP -= damage;
        if (HP <= 0) {
            //Die(); 
        }
        canTakeDamage = false; 
        LOG("Player took damage! Remaining HP: %d", HP);
    }
}



void Player::Die() {
	
	LOG("Player has died");
	Engine::GetInstance().audio.get()->PlayFx(dieFxId);
	Engine::GetInstance().scene->SetState(GameState::GAME_OVER);
}

void Player::PerformAttack() {
    // Seleccionar la animación de ataque actual

    currentState = PlayerState::ATTACKING;
    
    currentAnimation = &attack1;
     

    // Configurar el área de ataque según la dirección del jugador
    if (playerDirection == EntityDirections::RIGHT) {
        katanaAttack = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            (int)position.getX() + 390, (int)position.getY() + 100, 80, 250, bodyType::KINEMATIC
        );
    }
    else {
        katanaAttack = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            (int)position.getX() +135, (int)position.getY() + 100, 80, 250, bodyType::KINEMATIC
        );
    }

    katanaAttack->ctype = ColliderType::PLAYER_KATANA;
    katanaAttack->listener = this;
    katanaAttack->body->SetGravityScale(0.0f);

    // Avanzar al siguiente ataque (cíclico)
    currentAttackIndex = (currentAttackIndex + 1) % 3;

    // Reproducir el efecto de sonido correspondiente
    int attackFxId;
    switch (currentAttackIndex) {
    case 0:
        attackFxId = weakKatana1FxId;
        break;
    case 1:
        attackFxId = weakKatana2FxId;
        break;
    case 2:
        attackFxId = weakKatana3FxId;
        break;
    default:
        attackFxId = weakKatana1FxId;
        break;
    }
    Engine::GetInstance().audio.get()->PlayFx(attackFxId);
}


void Player::ThrowShuriken() {

    // Verificar si ya hay 3 shurikens activos
    if (activeShurikens.size() >= 3) {
        LOG("Cannot throw more shurikens. Maximum limit reached.");
        return; 
    }

    // Verificar si el cooldown ha terminado
    if (!canShootShuriken) {
        LOG("Cannot throw shuriken. Cooldown active.");
        return; 
    }

    // Crear el shuriken como un sensor físico
    PhysBody* shuriken = Engine::GetInstance().physics.get()->CreateRectangleSensor(
        (int)position.getX() + (playerDirection == EntityDirections::RIGHT ? 220 : -5),
        (int)position.getY() + 100,
        40, 40,
        bodyType::DYNAMIC
    );

    // Configurar propiedades del shuriken
    shuriken->ctype = ColliderType::PLAYER_ATTACK;
    shuriken->listener = this;
    shuriken->body->SetBullet(true);
    shuriken->body->SetFixedRotation(true);
    shuriken->body->SetGravityScale(0.0f);

    // Aplicar impulso horizontal en la dirección del jugador
    float shurikenSpeed = 10.0f;
    b2Vec2 impulse = b2Vec2((playerDirection == EntityDirections::RIGHT ? shurikenSpeed : -shurikenSpeed), 0);
    shuriken->body->ApplyLinearImpulseToCenter(impulse, true);

    // Agregar el shuriken a la lista con un temporizador
    Shuriken newShuriken = { shuriken, Timer() };
    newShuriken.timer.Start();
    activeShurikens.push_back(newShuriken);

    // Iniciar el cooldown
    canShootShuriken = false;
    shurikenCooldownTimer.Start();

    // Reproducir efecto de sonido
    int shurikenFxId = Engine::GetInstance().audio.get()->randomFx(throwShuriken1FxId, throwShuriken3FxId);
    Engine::GetInstance().audio.get()->PlayFx(shurikenFxId);
}



void Player::ChangeHitboxSize(float width, float height) {
    // Obtener la posición actual del cuerpo físico
    b2Vec2 currentPosition = pbody->body->GetPosition();

    // Destruir el fixture actual
    b2Fixture* fixture = pbody->body->GetFixtureList();
    while (fixture != nullptr) {
        b2Fixture* next = fixture->GetNext();
        pbody->body->DestroyFixture(fixture);
        fixture = next;
    }

    // Si se está agachando, la hitbox es la mitad de alta y se sube para que los pies queden igual
    if (crouched) {
        width = texW / 2;
        height = (texH - 50) / 2;
        // Subir la posición la mitad de la diferencia de altura
        currentPosition.y -= PIXEL_TO_METERS(((texH - 50) / 4)-2);
    }
    else {
        width = texW / 2;
        height = texH - 50;
        // Al levantarse, bajar la posición la mitad de la diferencia de altura
        currentPosition.y += PIXEL_TO_METERS((texH - 50) / 4);
    }

    // Crear el nuevo cuerpo físico con el tamaño ajustado
    pbody = Engine::GetInstance().physics.get()->CreateRectangle(
        METERS_TO_PIXELS(currentPosition.x), METERS_TO_PIXELS(currentPosition.y), width, height, bodyType::DYNAMIC
    );

    // Configurar las propiedades del nuevo cuerpo físico
    pbody->listener = this;
    pbody->ctype = ColliderType::PLAYER;
    pbody->body->SetFixedRotation(true);
    pbody->body->SetGravityScale(5);
    pbody->body->SetLinearDamping(0.0f);
    pbody->body->SetAngularDamping(0.0f);

    // Actualizar la posición del cuerpo físico
    pbody->body->SetTransform(currentPosition, 0);
}




void Player::AddItem(const InventoryItem& item) {
    // Busca si el objeto ya existe en el inventario
    for (auto& invItem : inventory) {
        if (invItem.name == item.name) {
            invItem.quantity += item.quantity; // Incrementa la cantidad
            return;
        }
    }
    // Si no existe, añade un nuevo objeto
    inventory.push_back(item);
}

void Player::RemoveItem(const std::string& itemName, int quantity) {
    for (auto it = inventory.begin(); it != inventory.end(); ++it) {
        if (it->name == itemName) {
            it->quantity -= quantity; // Reduce la cantidad
            if (it->quantity <= 0) {
                inventory.erase(it); // Elimina el objeto si la cantidad es 0
            }
            return;
        }
    }
}

InventoryItem* Player::GetItem(const std::string& itemName) {
    for (auto& invItem : inventory) {
        if (invItem.name == itemName) {
            return &invItem; // Devuelve un puntero al objeto
        }
    }
    return nullptr; // No encontrado
}

void Player::SaveInventory(pugi::xml_node& node) {
    for (const auto& item : inventory) {
        pugi::xml_node itemNode = node.append_child("item");
        itemNode.append_attribute("name") = item.name.c_str();
        itemNode.append_attribute("quantity") = item.quantity;
    }
}

void Player::LoadInventory(pugi::xml_node& node) {
    inventory.clear();
    for (pugi::xml_node itemNode = node.child("item"); itemNode; itemNode = itemNode.next_sibling("item")) {
        std::string name = itemNode.attribute("name").as_string();
        int quantity = itemNode.attribute("quantity").as_int();
        inventory.emplace_back(name, quantity);
    }
}

