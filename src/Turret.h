#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "Timer.h"
#include <vector>

struct SDL_Texture;

struct Shuriken2 {

    PhysBody* body;
    Timer timer;

    bool operator==(const Shuriken2& other) const {
        return body == other.body;
    }
};

class Turret : public Entity
{
public:
    Turret();
    virtual ~Turret();

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

   void ShootShuriken();


public:

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


    SDL_Texture* texture;
    SDL_Texture* shurikenTexture;

    std::vector<Shuriken2> activeShurikens;

    int texW, texH;
    pugi::xml_node parameters;

    Animation* currentAnimation = nullptr;
    Animation idle;
    Animation attackAnimation;
    PhysBody* pbody;
    Pathfinding* pathfinding;

    Timer shootTimer;

    bool dead = false;
};

