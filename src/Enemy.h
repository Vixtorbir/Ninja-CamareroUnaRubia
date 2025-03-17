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

    void PerformAttack();

    bool IsPlayerInAttackRange();
    void CheckAttackCollision();

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
    SDL_Rect attackHitbox;
};

