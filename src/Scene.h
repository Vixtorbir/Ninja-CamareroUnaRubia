
#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include <vector>
#include "GuiControlButton.h"
#include "Dialogue.h"
#include "DialogueManager.h"
#include "Parallax.h"
#include "GuiImage.h"
#include "Item.h"

#include "Npc.h"

struct SDL_Texture;

enum class GameState {
	MAIN_MENU,
	PLAYING,
	PAUSED,
	GAME_OVER,
	LOGO
};

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

	void LoadMap1();
	void LoadMap2();
	// Called before all Updates
	bool PostUpdate();

	void FadeTransition(SDL_Renderer* renderer, bool fadeIn, float duration);

	// Called before quitting
	bool CleanUp();

	// Return the player position
	Vector2D GetPlayerPosition();

	//L15 TODO 1: Implement the Load function
	void LoadState();
	void SafeLoadMap(const char* mapName, Vector2D playerPos);
	//L15 TODO 2: Implement the Save function
	void SaveState();

	// Handles multiple Gui Event methods
	bool OnGuiMouseClickEvent(GuiControl* control);

	void FadeTransition(SDL_Renderer* renderer, SDL_Texture* texture, float duration);
	
	void LoadTextures();

	void SetState(GameState newState);
	GameState GetState() const;


	void UpdateMainMenu(float dt);
	void UpdatePlaying(float dt);
	void UpdatePaused(float dt);
	void UpdateGameOver(float dt);
	void UpdateLogo(float dt);


public:
	// Get tilePosDebug value
	std::string GetTilePosDebug() {
		return tilePosDebug;
	}

public:
	SDL_Texture* mouseTileTex = nullptr;
	SDL_Texture* textureBuffer = nullptr;
	SDL_Texture* Hanzo = nullptr;
	SDL_Texture* MenuBackgroundImage = nullptr;

	std::string tilePosDebug = "[0,0]";
	bool once = false;

	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	std::vector<Enemy*> enemyList;

	NPC* npcMentor;
	NPC* npcIsamu;
	NPC* npcKaede;
	NPC* npcHanzo;
	std::vector<NPC*> npcs;
	std::vector<Item*> items;
	// L16: TODO 2: Declare a GUI Control Button 
	GuiControlButton* guiBt;

	Dialogue* dialogue;
	DialogueManager* dialogueManager;

	bool watchtitle = false;

	GameState currentState = GameState::LOGO;

	void HandleInput();

	GuiControlButton* startButton = nullptr;
	GuiControlButton* optionsButton = nullptr;
	GuiControlButton* exitButton = nullptr;
	GuiControlButton* returnButton = nullptr;
	GuiControlButton* returntomenuButton = nullptr;

	GuiImage* menuBackgroundImage = nullptr;

	float logoTimer = 0.0f;
	float fadeDuration = 3.0f; // Duración del fade in y fade out en segundos
	float opacity = 0.0f;
	SDL_Texture* logo = nullptr;
	int logoFxId;

	int levelIndex = 0;
	bool showingMap = false;
	bool canToggleMap = true;
private:		
	
	int curPopupMusic;

	SDL_Texture* mapBackgroundUI = nullptr;
	GuiImage* mapBackgroundUIImage = nullptr;

	Parallax* parallax = nullptr;
	//music
	int mainMenuMusicId;
	int gameplayMusicId;
	
};
