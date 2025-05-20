#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "Timer.h"

struct SDL_Texture;

enum class EnemyState {
    PATROL,
    AGGRESSIVE,
    WAIT,
    ATTACK
};

class Enemy : public Entity
{
public:
    Enemy();
    virtual ~Enemy();

    bool Awake();
    bool Start();
    bool Update(float dt);
    bool CleanUp();
    void SetParameters(pugi::xml_node parameters) {
        this->parameters = parameters;
    }
    void SetPosition(Vector2D pos);
    Vector2D GetPosition();
    void ResetPath();
    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

public:

    bool IsNextTileCollidable();
    bool IsPlayerInRange();
    void LoadEnemyFx();

	//fx
    //centinel
	int ninjaWalk1FxId;
	int ninjaMeleAttack1FxId;
	int ninjaMeleAttack2FxId;
	int ninjaRangeAttack1FxId;
	int ninjaRangeAttack2FxId;
	int ninjaJump1FxId;
	int ninjaJump2FxId;
	int ninjaDieFxId;
	int ninjaHit1FxId;
	int ninjaHit2FxId;
    //skull
    int skullFlyFxId;
    int skullAttack1FxId;
    int skullAttack2FxId;
    int skullHit1FxId;
    int skullHit2FxId;
    int skulldieFxId;
    //bat
    int batFlyFxId;
    int batAttack1FxId;
    int batAttack2FxId;
    int batHit1FxId;
    int batHit2FxId;
    int batdieFxId;
   
    void PerformAttack();
    Timer deathTimer;
    bool isDying = false; // Controls if the death animation is playing
    float deathDuration = 1.5f; // You can tweak this
    bool IsPlayerInAttackRange();
    void CheckAttackCollision();
    bool IsPlayerInLineOfSight();
    void DrawLineOfSight();

    SDL_Texture* texture;
    SDL_Texture* attackTexture;
    
    const char* texturePath;
    int texW, texH;
    pugi::xml_node parameters;
    Animation* currentAnimation = nullptr;
    Animation idle;
    Animation walkAnimation;
    Animation attackAnimation;
    Animation deathAnimation;
    PhysBody* pbody;
    Pathfinding* pathfinding;
    Timer timer;
    int direction = 0;
    int tilesMovedInSameDirection = 0;
    EnemyState state = EnemyState::PATROL; // Estado inicial
    

    Timer attackTimer;          
    bool isAttacking = false;   
    bool isCooldown = false;    
    const float attackDuration = 1.0f; 
    const float attackCooldown = 3.0f; 
    PhysBody* attackBody = nullptr;

	bool startDying = false;
};

