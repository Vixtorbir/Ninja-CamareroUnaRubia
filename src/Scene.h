#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include <vector>
#include "GuiControlButton.h"
#include "Dialogue.h"
#include "DialogueManager.h"
#include "Parallax.h"
#include "Npc.h"

struct SDL_Texture;

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Return the player position
	Vector2D GetPlayerPosition();

	//L15 TODO 1: Implement the Load function
	void LoadState();
	//L15 TODO 2: Implement the Save function
	void SaveState();

	// Handles multiple Gui Event methods
	bool OnGuiMouseClickEvent(GuiControl* control);

	void LoadTextures();

public:
	// Get tilePosDebug value
	std::string GetTilePosDebug() {
		return tilePosDebug;

	}

public:
	SDL_Texture* mouseTileTex = nullptr;
	SDL_Texture* textureBuffer = nullptr;

	std::string tilePosDebug = "[0,0]";
	bool once = false;

	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	std::vector<Enemy*> enemyList;

	NPC* npc;

	// L16: TODO 2: Declare a GUI Control Button 
	GuiControlButton* guiBt;

	Dialogue* dialogue;
	DialogueManager* dialogueManager;

private:
	Parallax* parallax = nullptr;

};

