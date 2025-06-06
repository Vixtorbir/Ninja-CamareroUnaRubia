#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "Timer.h"
#include "Player.h"
#include "GuiControl.h"
#include "GuiImage.h"
#include <vector>


struct SDL_Texture;

struct BloodSplat {
    int x, y;
    float timer;
};


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
    void SetPlayer(Player* player);
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
    int lives = 3;

    bool IsNextTileCollidable();
    bool IsPlayerInRange();
    void LoadEnemyFx();
    bool aggressive = false;
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
    bool isDying = false;
    float deathDuration = .1f; 
    bool IsPlayerInAttackRange();
    void CheckAttackCollision();
    bool IsPlayerInLineOfSight();
    void ApplyKnockbackFrom(b2Vec2 sourcePosition, float strength);
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
    SDL_Texture* redTexture = nullptr;
    GuiImage* redImage = nullptr;

    Player* player;
    bool flashRed = false;
    Timer flashTimer;
    float flashDuration = 0.1f;
    Timer attackTimer;          
    bool isAttacking = false;   
    bool isCooldown = false;    
    const float attackDuration = .5; 
    const float attackCooldown = .5; 
    PhysBody* attackBody = nullptr;
    Module* sceneModule = nullptr;
	bool startDying = false;

    std::vector<BloodSplat> bloodSplats;

};

