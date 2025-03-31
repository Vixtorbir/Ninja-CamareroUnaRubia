#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "Timer.h"

#define SOUND_RADIUS 4

struct SDL_Texture;

enum class EnemyState {
    PATROL,
    AGGRESSIVE,
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

private:

    bool IsNextTileCollidable();
    bool IsPlayerInRange();
    void LoadEnemyFx();

	//fx
    //centinel
	int centinelWalk1FxId;
	int centinelWalk2FxId;
	int centinelMeleAttack1FxId;
	int centinelMeleAttack2FxId;
	int centinelRangeAttack1FxId;
	int centinelRangeAttack2FxId;
	int centinelJump1FxId;
	int centinelJump2FxId;
	int centinelDieFxId;
	int centinelHit1FxId;
	int centinelHit2FxId;
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

    bool IsPlayerInAttackRange();
    void CheckAttackCollision();
    bool IsPlayerInLineOfSight();
    void DrawLineOfSight();

    SDL_Texture* texture;
    SDL_Texture* attackTexture;
    SDL_Texture* trailTexture;
    const char* texturePath;
    int texW, texH;
    pugi::xml_node parameters;
    Animation* currentAnimation = nullptr;
    Animation idle;
    Animation attackAnimation;
    PhysBody* pbody;
    Pathfinding* pathfinding;
    Timer timer;
    int direction = 0;
    int tilesMovedInSameDirection = 0;
    EnemyState state = EnemyState::PATROL; // Estado inicial
    std::deque<Vector2D> swordTrail;
    const int maxTrailLength = 10;
    PhysBody* attackHitbox = nullptr;
    PhysBody* soundRange = nullptr;

    Timer attackCooldownTimer;
    Timer attackDurationTimer;

    EntityDirections enemyDirections = EntityDirections::RIGHT;

};

