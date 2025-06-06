#pragma once

#include "Module.h"
#include "Entity.h"
#include "NPC.h"
#include "Enemy.h"
#include "Player.h"
#include "Turret.h"
#include "Boss.h"
#include <list>

class EntityManager : public Module
{
public:

	EntityManager();

	// Destructor
	virtual ~EntityManager();

	// Called before render is available
	bool Awake();

	// Called after Awake
	bool Start();

	// Called every frame
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	// Additional methods
	Entity* CreateEntity(EntityType type);

	Entity* CreateNamedCharacter(EntityType type, DialogueEngine name);

	void DestroyEntity(Entity* entity);

	void AddEntity(Entity* entity);

public:

	std::list<Entity*> entities;

};
