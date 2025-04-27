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

    currentAnimation = &idle;

    // Physics setup
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), texW, texH, bodyType::DYNAMIC);
    pbody->listener = this;
    pbody->ctype = ColliderType::PLAYER;
    pbody->body->SetFixedRotation(true);
    pbody->body->SetGravityScale(5);

    // UI elements
    popup = (GuiPopup*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::POPUP, 1, "Press E", btPos, sceneModule);
    orbCount = (Text*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::TEXT, 1, "0", OrbCountPos, sceneModule);
    backgroundSliderImage = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, " ", btPos, sceneModule, BackgroundSliderHP);
    foregroundSliderImage = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, " ", btPos, sceneModule, ForeGroundSliderHP);
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
    HP_Slider->SetSliderBarInnerSize(HP * 4, 50);
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN) HP += 20;
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_DOWN) HP -= 20;
    if (HP >= MAXHP) HP = MAXHP; else if (HP <= 0) HP = 0;

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
    if (movingLeft) {
        velocity.x = -speed * 16;
        playerDirection = EntityDirections::LEFT;
    }
    if (movingRight) {
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
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && hasAlreadyJumpedOnce == 0 && !inBubble && !crouched) {
        isHoldingJump = true;
        jumpHoldTimer = 0.0f;
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT && isHoldingJump && !inBubble) {
        jumpHoldTimer += dt;
        if (jumpHoldTimer >= maxHoldTime) {
            float jumpStrength = jumpForce * maxJumpMultiplier;
            pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpStrength), true);
            hasAlreadyJumpedOnce++;
            isHoldingJump = false;
            isJumping = true;
            currentState = PlayerState::JUMPING;
            int jumpId = Engine::GetInstance().audio.get()->randomFx(jump1FxId, jump3FxId);
            Engine::GetInstance().audio.get()->PlayFx(jumpId);
        }
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_UP && isHoldingJump && !inBubble) {
        float holdPercentage = jumpHoldTimer / maxHoldTime;
        float jumpMultiplier = minJumpMultiplier + (holdPercentage * (maxJumpMultiplier - minJumpMultiplier));
        float jumpStrength = jumpForce * jumpMultiplier;
        pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpStrength), true);
        hasAlreadyJumpedOnce++;
        isHoldingJump = false;
        isJumping = true;
        currentState = PlayerState::JUMPING;
    }

    // Double jump
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && hasAlreadyJumpedOnce == 1 && !inBubble) {
        pbody->body->SetLinearVelocity(b2Vec2(pbody->body->GetLinearVelocity().x, 0));
        pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
        hasAlreadyJumpedOnce++;
        isJumping = true;
        currentState = PlayerState::JUMPING;
        int doubleJumpId = Engine::GetInstance().audio.get()->randomFx(doubleJump1FxId, doubleJump2FxId);
        Engine::GetInstance().audio.get()->PlayFx(doubleJumpId);
    }

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
        currentState = PlayerState::JUMPING;
    }

    // Wall slide
    if (touchingWall && Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) {
        pbody->body->SetLinearVelocity(b2Vec2(0, wallClimbSpeed));
        currentState = PlayerState::WALL_SLIDING;
    }

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
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW - 20);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Render
    Engine::GetInstance().render.get()->DrawEntity(texture, (int)position.getX(), (int)position.getY(),
        &currentAnimation->GetCurrentFrame(), 1, 0, 0, 0, (int)playerDirection);
    currentAnimation->Update();

    // Camera follow
    if (!Engine::GetInstance().scene.get()->watchtitle) {
        camX = -(float)position.getX() + (Engine::GetInstance().render.get()->camera.w / 2);
        camY = -(float)position.getY() + (Engine::GetInstance().render.get()->camera.h / 2);
    }
    else {
        camX = 1920 + (Engine::GetInstance().render.get()->camera.w / 2);
        camY = -(float)position.getY() + (Engine::GetInstance().render.get()->camera.h / 2);
    }

    Engine::GetInstance().render.get()->camera.x += (camX - Engine::GetInstance().render.get()->camera.x) * smoothFactor;
    Engine::GetInstance().render.get()->camera.y += (camY - Engine::GetInstance().render.get()->camera.y) * smoothFactor;

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
    
    if (isCooldown) {
        if (attackTimer.ReadSec() >= attackCooldown) {
            isCooldown = false; 
            LOG("Cooldown ended, player can attack again.");
        }
    }

    
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_DOWN && !isAttacking && !isCooldown) {
        PerformAttack();         
        isAttacking = true;      
        attackTimer.Start();     
        LOG("Attack started.");
    }

    if (isAttacking && attackTimer.ReadSec() >= attackDuration) {
        isAttacking = false; 
        isCooldown = true;   
        attackTimer.Start(); 

        
        if (katanaAttack != nullptr) {
            Engine::GetInstance().physics.get()->DeletePhysBody(katanaAttack);
            katanaAttack = nullptr;
        }

        LOG("Attack ended, cooldown started.");
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
		isJumping = false;
		hasAlreadyJumpedOnce = 0;
       /* if (physA->ctype == ColliderType::PLAYER_ATTACK) {
            
            activeShurikens.erase(std::remove(activeShurikens.begin(), activeShurikens.end(), physA), activeShurikens.end());
            Engine::GetInstance().physics.get()->DeletePhysBody(physA);
        }*/
		break;
	case ColliderType::NPC:
		inBubble = true;
		GuiPOPup(GuiPopups::E_Interact);
		break;
	case ColliderType::WALL:
		touchingWall = true;
        /* if (physA->ctype == ColliderType::PLAYER_ATTACK) {

             activeShurikens.erase(std::remove(activeShurikens.begin(), activeShurikens.end(), physA), activeShurikens.end());
             Engine::GetInstance().physics.get()->DeletePhysBody(physA);
         }*/
		break;
	case ColliderType::ITEM:
		Engine::GetInstance().audio.get()->PlayFx(pickUpItemFxId);
		Orbs++;
        Engine::GetInstance().map.get()->DeleteCollisionBodies();
		Engine::GetInstance().physics.get()->DeletePhysBody(physB);

		break;
	case ColliderType::UNKNOWN:
		break;

    case ColliderType::ENEMY:
        if (physA->ctype == ColliderType::PLAYER_ATTACK) {
            
            Enemy* enemy = static_cast<Enemy*>(physB->listener);
            if (enemy != nullptr) {
				enemy->dead = true;
				Engine::GetInstance().audio.get()->PlayFx(weakKatana1FxId);
				
            }

            
            activeShurikens.erase(
                std::remove_if(activeShurikens.begin(), activeShurikens.end(),
                    [physA](const Shuriken& shuriken) { return shuriken.body == physA; }),
                activeShurikens.end()
            );

            Engine::GetInstance().physics.get()->DeletePhysBody(physA);
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
            Die(); 
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

void Player::PerformAttack()
{
    if (playerDirection == EntityDirections::RIGHT)
    {
        katanaAttack = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            (int)position.getX() + 220, (int)position.getY() + 100, 80, 250, bodyType::STATIC
        );
    }
    else
    {
        katanaAttack = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            (int)position.getX() - 5, (int)position.getY() + 100, 80, 250, bodyType::STATIC
        );
    }

    katanaAttack->ctype = ColliderType::PLAYER_ATTACK; 
    katanaAttack->listener = this; 
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
	// Destroy the current fixture
	b2Fixture* fixture = pbody->body->GetFixtureList();
	while (fixture != nullptr) {
		b2Fixture* next = fixture->GetNext();
		pbody->body->DestroyFixture(fixture);
		fixture = next;
	}

	// Create a new fixture with the new size
	if (height == texH / 2) {
	pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX()+120, (int)position.getY()+170, width, height, bodyType::DYNAMIC);
	}
	else {
		pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX()+120, (int)position.getY()+100, width, height, bodyType::DYNAMIC);
	}
	pbody->listener = this;
	pbody->ctype = ColliderType::PLAYER;
	pbody->body->SetFixedRotation(true);
    pbody->body->SetGravityScale(5);
}

