#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "Animation.h"
#include "DialogueManager.h"
#include <vector>

struct SDL_Texture;



class NPC : public Entity
{
public:

	NPC();

	NPC(DialogueEngine name);
	
	virtual ~NPC();

	bool Awake();

	bool Start();

	bool Update(float dt);

	float Lerp(float start, float end, float factor);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();


public:

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;
	int camX, camY;
	float smoothFactor = 0.1f;
	//Audio fx

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	bool showcaseDialogue = false;
	bool once = true;

	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;

	DialogueEngine dialogueName;

	EntityDirections npcDirection = EntityDirections::RIGHT;
};