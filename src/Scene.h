
#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include <vector>
#include "GuiControlButton.h"
#include "GuiCheckbox.h"
#include "GuiSlider.h"
#include "Dialogue.h"
#include "DialogueManager.h"
#include "Parallax.h"
#include "GuiImage.h"
#include "Item.h"

#include "Npc.h"
#include "Turret.h"
#include "Boss.h"

struct SDL_Texture;

enum class GameState {
	MAIN_MENU,
	OPTIONS,
	PLAYING,
	PAUSED,
	INVENTORY,
	GAME_OVER,
	LOGO,
	CINEMATIC
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

	void loadThingsOfMaps(int index);

	void LoadMap1();
	void LoadMap2();
	// Called before all Updates
	bool PostUpdate();

	void FadeTransition(SDL_Renderer* renderer, bool fadeIn, float duration);

	void LoadEntities(int sceneIndex);

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
	void UpdateOptions(float dt);
	void UpdateInventory(float dt);
	void UpdateCinematic(float dt);

	void CreateItemLvl2(const char* mapName);


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
	SDL_Texture* BackgroundImage = nullptr;
	bool anyAggressiveNow = false;
	std::string tilePosDebug = "[0,0]";
	bool once = false;

	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	std::vector<Enemy*> enemyList;
	std::vector<Turret*> turretList;
	std::vector<Boss*> bossList;

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
	GuiControlButton* fullscreenCheckbox = nullptr;
	GuiControlButton* vsyncCheckbox = nullptr;
	GuiSlider* fxSlider = nullptr;
	GuiSlider* musicSlider = nullptr;


	GuiImage* menuBackgroundImage = nullptr;
	GuiImage* backgroundImage = nullptr;
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

	//cinematic scenes
	SDL_Texture* testCinematicBg = nullptr;
	//fx timers

	int birdTimer;
	int cricketTimer;
	int windTimer;
    int bushTimer;

	int birdFxID;
	int cricketFxID;
	int windFxID;
	int bushFxID;
	
	int selectedItemIndex = 0;

};
