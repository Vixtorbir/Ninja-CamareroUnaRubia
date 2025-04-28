#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"
#include "Enemy.h"
#include "GuiControl.h"
#include "GuiImage.h"

#include "GuiManager.h"
#include "DialogueManager.h"
#include "NPC.h"
#include "Turret.h"

Scene::Scene() : Module()
{
	name = "scene";
}

// Destructor
Scene::~Scene()
{
}

// Called before render is available
bool Scene::Awake()
{
	parallax = Engine::GetInstance().parallax.get();

	LOG("Loading Scene");
	bool ret = true;
	dialogueManager = new DialogueManager();
	//L04: TODO 3b: Instantiate the player using the entity manager
	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	player->SetParameters(configParameters.child("entities").child("player"));
	
	npcMentor = (NPC*)Engine::GetInstance().entityManager->CreateNamedCharacter(EntityType::NPC, DialogueEngine::MENTORSHIP);
	npcMentor->SetParameters(configParameters.child("entities").child("npcMENTORSHIP"));
	npcs.push_back(npcMentor);

	npcIsamu = (NPC*)Engine::GetInstance().entityManager->CreateNamedCharacter(EntityType::NPC, DialogueEngine::ISAMU);
	npcIsamu->SetParameters(configParameters.child("entities").child("npcISAMU"));
	npcs.push_back(npcIsamu);

	npcKaede = (NPC*)Engine::GetInstance().entityManager->CreateNamedCharacter(EntityType::NPC, DialogueEngine::KAEDE);
	npcKaede->SetParameters(configParameters.child("entities").child("npcKAEDE"));
	npcs.push_back(npcKaede);

	npcHanzo = (NPC*)Engine::GetInstance().entityManager->CreateNamedCharacter(EntityType::NPC, DialogueEngine::HANZO);
	npcHanzo->SetParameters(configParameters.child("entities").child("npcHANZO"));
	npcs.push_back(npcHanzo);
	//L08 Create a new item using the entity manager and set the position to (200, 672) to test
	for (pugi::xml_node itemNode = configParameters.child("entities").child("items").child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
	{
		Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
		item->SetParameters(itemNode);
		items.push_back(item);
	}

	// Create a enemy using the entity manager 
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
		enemy->SetParameters(enemyNode);
		enemyList.push_back(enemy);
	}

	//Crea un enemigo tipo torreta en la posicion 1000, 1000
	for (pugi::xml_node turretNode = configParameters.child("entities").child("enemies").child("turret"); turretNode; turretNode = turretNode.next_sibling("turret"))
	{
		Turret* turret = (Turret*)Engine::GetInstance().entityManager->CreateEntity(EntityType::TURRET);
		turret->SetParameters(turretNode);
		turretList.push_back(turret);
	}
	
	// L16: TODO 2: Instantiate a new GuiControlButton in the Scene
	SDL_Rect btPos = { -10000, 350, 120,20 };
	guiBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "MyButton", btPos, this);


	//SDL_Rect btPos2 = { 0, 700, 1920,300 };
	//dialogue = (Dialogue*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::DIALOGUE, 1, "dialogo moment", btPos2, this);
	//

	dialogueManager->SetModule(this);
	player->sceneModule = this;

	return ret;

}

// Called before the first frame
bool Scene::Start()
{
	curPopupMusic = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Music/ScreamAngry.ogg");

	logo = Engine::GetInstance().textures->Load("Assets/UI/logo.png");
	logoFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/ExtraFx/logoFx.ogg");
	MenuBackgroundImage = Engine::GetInstance().textures.get()->Load("Assets/UI/Menu.png");
	mapBackgroundUI = Engine::GetInstance().textures.get()->Load("Assets/UI/MapBackgroundUI.png");
	SDL_Rect btPos00 = { 0, 0, 0,0 };
	mapBackgroundUIImage = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, "MyButton", btPos00, this, mapBackgroundUI);
	mapBackgroundUIImage->visible = false;
	// Inicializar el estado de la pantalla de presentaci�n

	SetState(GameState::LOGO);
	Engine::GetInstance().audio.get()->PlayFx(curPopupMusic);


	SDL_Rect btPosMenu = { 0, 0, 0, 0 };

	menuBackgroundImage = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, " ", btPosMenu, this, MenuBackgroundImage);


	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());

	// Texture to highligh mouse position 
	mouseTileTex = Engine::GetInstance().textures.get()->Load("Assets/Maps/MapMetadata.png");


	// Initalize the camera position
	int w, h;
	Engine::GetInstance().window.get()->GetWindowSize(w, h);
	Engine::GetInstance().render.get()->camera.x = 0;
	Engine::GetInstance().render.get()->camera.y = 0;



	SDL_Rect startButtonPos = { 800, 300, 200, 50 };
	SDL_Rect optionsButtonPos = { 800, 550, 200, 50 };
	SDL_Rect exitButtonPos = { 800, 800, 200, 50 };

	startButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 1, "Start Game", startButtonPos, this);


	optionsButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 2, "Options", optionsButtonPos, this);


	exitButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 3, "Exit", exitButtonPos, this);
	



	

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	//L03 TODO 3: Make the camera movement independent of framerate
	float camSpeed = 1;
	Engine::GetInstance().render->RenderMinimap(); // No parameters needed now
	/*if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.y -= ceil(camSpeed * dt);

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.y += ceil(camSpeed * dt);

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.x -= ceil(camSpeed * dt);

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.x += ceil(camSpeed * dt);
*/

	//Get mouse position and obtain the map coordinate
	int scale = Engine::GetInstance().window.get()->GetScale();
	Vector2D mousePos = Engine::GetInstance().input.get()->GetMousePosition();
	Vector2D mouseTile = Engine::GetInstance().map.get()->WorldToMap(mousePos.getX() - Engine::GetInstance().render.get()->camera.x / scale,
		mousePos.getY() - Engine::GetInstance().render.get()->camera.y / scale);

	//Render a texture where the mouse is over to highlight the tile, use the texture 'mouseTileTex'
	Vector2D highlightTile = Engine::GetInstance().map.get()->MapToWorld(mouseTile.getX(), mouseTile.getY());
	SDL_Rect rect = { 0,0,32,32 };
	Engine::GetInstance().render.get()->DrawTexture(mouseTileTex,
		highlightTile.getX(),
		highlightTile.getY(),
		&rect);

	// saves the tile pos for debugging purposes
	if (mouseTile.getX() >= 0 && mouseTile.getY() >= 0 || once) {
		tilePosDebug = "[" + std::to_string((int)mouseTile.getX()) + "," + std::to_string((int)mouseTile.getY()) + "] ";
		once = true;
	}

	//If mouse button is pressed modify enemy position
	if (Engine::GetInstance().scene.get()->player->godMode && Engine::GetInstance().input.get()->GetMouseButtonDown(1) == KEY_REPEAT && currentState == GameState::PLAYING) {
		
		player->SetPosition(Vector2D(highlightTile.getX(), highlightTile.getY()));
		
	}

	//Dialogue things
	dialogueManager->Update();
	for (NPC* npc : npcs)
	{
		if (npc->showcaseDialogue)
		{
		dialogueManager->CastDialogue(npc->dialogueName);
			npc->showcaseDialogue = false;
		}
	}


	HandleInput();

	switch (currentState)
	{
	case GameState::MAIN_MENU:
		UpdateMainMenu(dt);
		break;
	case GameState::PLAYING:
		UpdatePlaying(dt);
		break;
	case GameState::PAUSED:
		UpdatePaused(dt);
		break;
	case GameState::GAME_OVER:
		UpdateGameOver(dt);
		break;
	case GameState::LOGO:
		UpdateLogo(dt);
		break;
	default:
		break;
	}


	return true;
}


// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
		SafeLoadMap("MapTemplate1.tmx", Vector2D(17280, 4224)); // Posición específica Mapa 1
		levelIndex = 0;
		parallax->ChangeTextures(levelIndex);

	}
		//Engine::GetInstance().scene.get()->player->currentLevel = 1;
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {
		SafeLoadMap("MapTemplate2.tmx", Vector2D(193, 3845)); // Posición específica Mapa 2
		levelIndex = 1;
		parallax->ChangeTextures(levelIndex);
	}
		//Engine::GetInstance().scene.get()->player->currentLevel = 2;
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		LoadState();

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		SaveState();

	if (Engine::GetInstance().scene.get()->player->loadLevel1) {

		FadeTransition(Engine::GetInstance().render.get()->renderer, false, 1.0f);
		Engine::GetInstance().map->CleanUp(); // Esto solo limpia recursos antiguos

		SafeLoadMap("MapTemplate1.tmx", Vector2D(22480, 4304));

		Engine::GetInstance().scene.get()->player->loadLevel1 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 1;

	}

	if (Engine::GetInstance().scene.get()->player->loadLevel2) {
	
		FadeTransition(Engine::GetInstance().render.get()->renderer, false, 1.0f);
		Engine::GetInstance().map->CleanUp(); // Esto solo limpia recursos antiguos

		SafeLoadMap("MapTemplate2.tmx", Vector2D(1504, 3888));

		Engine::GetInstance().scene.get()->player->loadLevel2 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 2;

	}

	return ret;
}

void Scene::FadeTransition(SDL_Renderer* renderer, bool fadeIn, float duration)
{
	Uint32 startTime = SDL_GetTicks();
	Uint8 alpha = fadeIn ? 255 : 0;
	Uint8 targetAlpha = fadeIn ? 0 : 255;

	SDL_Rect screenRect = { 0, 0, 1920, 1080 };
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	while (true)
	{
		Uint32 elapsedTime = SDL_GetTicks() - startTime;
		float progress = (float)elapsedTime / (duration * 1000);

		if (progress > 1.0f)
			break;

		alpha = fadeIn
			? (Uint8)(255 * (1.0f - progress))
			: (Uint8)(255 * progress);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
		SDL_RenderFillRect(renderer, &screenRect);
		SDL_RenderPresent(renderer);

		SDL_Delay(16);
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, targetAlpha);
	SDL_RenderFillRect(renderer, &screenRect);
	SDL_RenderPresent(renderer);
}


// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");
	return true;
}

// Return the player position
Vector2D Scene::GetPlayerPosition()
{
	return player->GetPosition();
}




void Scene::LoadTextures()
{
	Hanzo = Engine::GetInstance().textures.get()->Load("Assets/Portraits/Hanzo.png");
	dialogueManager->Hanzo = Hanzo;

}

void Scene::SetState(GameState newState) {
	Engine::GetInstance().guiManager->ClearControlsOfType(GuiControlType::BUTTON);

	currentState = newState;
}


GameState Scene::GetState() const {
	return currentState;
}

void Scene::SaveState() {

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}

	pugi::xml_node sceneNode = loadFile.child("config").child("scene");

	//Save info to XML 

	//Player position
	sceneNode.child("entities").child("player").attribute("x").set_value(player->GetPosition().getX());
	sceneNode.child("entities").child("player").attribute("y").set_value(player->GetPosition().getY());

	pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");

	// Ensure enemies node exists
	if (!enemiesNode) {
		enemiesNode = sceneNode.child("entities").append_child("enemies");
	}

	// Loop through existing enemies and update their positions
	for (pugi::xml_node enemyNode = enemiesNode.child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
		// Find the corresponding enemy in your list
		for (auto& enemy : enemyList) {
			 {
				enemyNode.attribute("x").set_value(enemy->GetPosition().getX());
				enemyNode.attribute("y").set_value(enemy->GetPosition().getY());
			}
		}
	}

	pugi::xml_node ItemsNode = sceneNode.child("entities").child("items");
	if (!ItemsNode) {
		ItemsNode = sceneNode.child("entities").append_child("items");
	}



	//Saves the modifications to the XML 
	loadFile.save_file("config.xml");

}

void Scene::LoadState() {

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}

	pugi::xml_node sceneNode = loadFile.child("config").child("scene");

	//Read XML and restore information

	//Player position
	Vector2D playerPos = Vector2D(sceneNode.child("entities").child("player").attribute("x").as_int(),
		(sceneNode.child("entities").child("player").attribute("y").as_int()) - 32);
	player->SetPosition(playerPos);

	pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");
	if (enemiesNode) {
		for (pugi::xml_node enemyNode = enemiesNode.child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
			float enemyX = enemyNode.child("position").attribute("x").as_float();
			float enemyY = enemyNode.child("position").attribute("y").as_float();

			bool isAlive = enemyNode.attribute("alive").as_bool(true);

		}

	}
}
void Scene::SafeLoadMap(const char* mapName, Vector2D playerPos) {
	Engine::GetInstance().map->CleanUp(); // Esto solo limpia recursos antiguos

	for (const auto enemy : enemyList) {
		enemy->CleanUp();
	}

	enemyList.clear();

	for (const auto npc : npcs) {
		npc->CleanUp();
	}

	npcs.clear();

	for (const auto item : items) {
		item->CleanUp();
	}

	items.clear();

	std::string path = configParameters.child("map").attribute("path").as_string();
	if (!Engine::GetInstance().map->Load(path.c_str(), mapName)) {
		LOG("Error cargando %s", mapName);
		return; // Si falla, conservamos el mapa anterior
	}

	// Reposicionar jugador y cámara
	player->SetPosition(playerPos);
	Engine::GetInstance().render->camera.x = 0;
	Engine::GetInstance().render->camera.y = 0;

	// Crear ítem solo si es el nivel 2
	if (std::string(mapName) == "MapTemplate2.tmx") {
		CreateItemLvl2(mapName);
	}

	LOG("Mapa cambiado a %s", mapName);
}


void Scene::UpdateMainMenu(float dt) {
	
	if (!startButton) {
		SDL_Rect startButtonPos = { 800, 300, 200, 50 };
		startButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::BUTTON, 1, "Start Game", startButtonPos, this);
	}

	if (!optionsButton) {
		SDL_Rect optionsButtonPos = { 800, 550, 200, 50 };
		optionsButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::BUTTON, 2, "Options", optionsButtonPos, this);
	}

	if (!exitButton) {
		SDL_Rect exitButtonPos = { 800, 800, 200, 50 };
		exitButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::BUTTON, 3, "Exit", exitButtonPos, this);
	}
	if (!menuBackgroundImage) {

		MenuBackgroundImage = Engine::GetInstance().textures.get()->Load("Assets/UI/Menu.png");
	}
	


	startButton->Start();
	optionsButton->Start();
	exitButton->Start();

	menuBackgroundImage->Update(dt);
	startButton->Update(dt);
	optionsButton->Update(dt);
	exitButton->Update(dt);

	player->inGame = false;


	// Render the main menu text
	Engine::GetInstance().render.get()->DrawText("MAIN MENU", 600, 40, 750, 255);

	/*if (startButton->isClicked == true)
	{
		SetState(GameState::PLAYING);
		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/gameplaySongPlaceholder.wav");
		Engine::GetInstance().audio.get()->musicVolume(50);
		startButton->CleanUp();
		startButton = nullptr;
		optionsButton->CleanUp();
		optionsButton = nullptr;
		exitButton->CleanUp();
		exitButton = nullptr;
		menuBackgroundImage->CleanUp();
	}*/


}
void Scene::HandleInput()
{
	if (startButton != nullptr) {
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN || startButton->isClicked == true)
		{
			if (currentState == GameState::MAIN_MENU)
			{

				SetState(GameState::PLAYING);
				Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/gameplaySongPlaceholder.wav");
				Engine::GetInstance().audio.get()->musicVolume(50);
				startButton->CleanUp();
				optionsButton->CleanUp();
				exitButton->CleanUp();
				menuBackgroundImage->CleanUp();
				startButton = nullptr;
				optionsButton = nullptr;
				exitButton = nullptr;

			}
			/*else if (currentState == GameState::GAME_OVER)
			{
				SetState(GameState::MAIN_MENU);
			}*/
		}
	}
	if (returnButton != nullptr) {

		if (currentState == GameState::PAUSED) {

			if (returnButton->isClicked == true) {
				SetState(GameState::PLAYING);
			}
		}
		if (currentState == GameState::GAME_OVER) {

			if (returnButton->isClicked == true) {
				SetState(GameState::PLAYING);
				LoadState();
			}
		}
		returnButton->CleanUp();



	}
	if (returntomenuButton != nullptr) {
		if (currentState == GameState::GAME_OVER || currentState == GameState::OPTIONS) {
			if (returntomenuButton->isClicked == true) {
				SetState(GameState::MAIN_MENU);

			}
		}
		returntomenuButton->CleanUp();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_P) == KEY_DOWN)
	{
		if (currentState == GameState::PLAYING)
		{
			SetState(GameState::GAME_OVER);
		}
		else if (currentState == GameState::PAUSED)
		{
			SetState(GameState::PLAYING);
		}
	}
	if (optionsButton != nullptr) {

		if (optionsButton->isClicked == true) {
			SetState(GameState::OPTIONS);
			startButton->CleanUp();
			optionsButton->CleanUp();
			exitButton->CleanUp();
			menuBackgroundImage->CleanUp();
			startButton = nullptr;
			optionsButton = nullptr;
			exitButton = nullptr;


		}


	}

	if (fullscreenCheckbox != nullptr) {
		if (fullscreenCheckbox->isClicked == true) {

			SDL_Window* window = Engine::GetInstance().window.get()->GetSDLWindow();
			Uint32 flags = SDL_GetWindowFlags(window);

			// Alternar entre pantalla completa y modo ventana
			if (flags & SDL_WINDOW_FULLSCREEN)
			{
				SDL_SetWindowFullscreen(window, 0); // Salir de pantalla completa
			}
			else
			{
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN); // Activar pantalla completa
			}
		}




	}

	if (vsyncCheckbox != nullptr) {
		if (vsyncCheckbox->isClicked == true) {


			if (SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"))
			{
				LOG("VSync activado correctamente.");
			}
			else
			{
				LOG("Error al activar VSync.");
			}
		}
	}

}



void Scene::UpdatePlaying(float dt) {
	player->inGame = true;

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_TAB) == KEY_DOWN)
	{
		if (canToggleMap)
		{
			showingMap = !showingMap;
			canToggleMap = false;
		}
	}
	else
	{
		// Reset the toggle when the key is no longer pressed
		canToggleMap = true;
	}

	// Control visibility based on showingMap
	mapBackgroundUIImage->visible = showingMap;

	player->Update(dt);
	for (auto& enemy : enemyList) {
		enemy->Update(dt);
	}

	for (auto& turret : turretList) {
		turret->Update(dt);
	}
}

void Scene::UpdatePaused(float dt) {
	SDL_Rect returnButtonPos = { 800, 550, 200, 50 };
	returnButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 4, "Return", returnButtonPos, this);
	returnButton->Start();
	returnButton->Update(dt);
	exitButton->Update(dt);

	Engine::GetInstance().render.get()->DrawText("PAUSED: Press enter to start", 600, 400, 750, 255);
}

void Scene::UpdateGameOver(float dt) {
	
	Engine::GetInstance().render.get()->DrawText("GAME OVER", 600, 200, 750, 255);
	if (!returntomenuButton) {
		
		SDL_Rect returntomenuButtonPos = { 800, 550, 200, 50 };
		returntomenuButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::BUTTON, 5, "Return To Menu", returntomenuButtonPos, this);
	}
	if (!menuBackgroundImage) {

		MenuBackgroundImage = Engine::GetInstance().textures.get()->Load("Assets/UI/Menu.png");
	}
	returntomenuButton->Start();
	returntomenuButton->Update(dt);
	menuBackgroundImage->Update(dt);

	Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/gameplaySongPlaceholder.wav");
	Engine::GetInstance().audio.get()->musicVolume(50);

}
void Scene::UpdateLogo(float dt) {
	// Manejar el fade in

	
	fadeDuration += dt;
	
	FadeTransition(Engine::GetInstance().render.get()->renderer, logo, .1f);
	
	SetState(GameState::MAIN_MENU);
	Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/titleSongPlaceholder.wav");
	Engine::GetInstance().audio.get()->musicVolume(50);
	

}
void Scene::UpdateOptions(float dt)
{
	Engine::GetInstance().render.get()->DrawText("OPTIONS", 600, 200, 750, 255);
	Engine::GetInstance().render.get()->DrawText("FullScreen", 700, 500, 750, 255);
	Engine::GetInstance().render.get()->DrawText("VSYNC", 700, 400, 750, 255);
	if (!returntomenuButton) {
		SDL_Rect returntomenuButtonPos = { 800, 550, 200, 50 };
		returntomenuButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::BUTTON, 5, "Return To Menu", returntomenuButtonPos, this);
	}
	if (!menuBackgroundImage) {

		MenuBackgroundImage = Engine::GetInstance().textures.get()->Load("Assets/UI/Menu.png");
	}
	if (!fullscreenCheckbox) {
		SDL_Rect fullscreenCheckboxPos = { 900, 500, 200, 50 };
		fullscreenCheckbox = (GuiCheckbox*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::CHECKBOX, 1, "", fullscreenCheckboxPos, this);
	}
	if (!vsyncCheckbox) {
		SDL_Rect vsyncCheckboxPos = { 900, 400, 200, 50 };
		vsyncCheckbox = (GuiCheckbox*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::CHECKBOX, 2, "", vsyncCheckboxPos, this);
	}
	if (!fxSlider) {
		SDL_Rect fxSliderPos = { 800, 550, 200, 50 };
		fxSlider = (GuiSlider*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::SLIDER, 4, "", fxSliderPos, this);


	}
	if (!musicSlider) {
		SDL_Rect musicSliderPos = { 800, 550, 200, 50 };
		musicSlider = (GuiSlider*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::SLIDER, 5, "", musicSliderPos, this);


	}


	returntomenuButton->Start();
	returntomenuButton->Update(dt);
	menuBackgroundImage->Update(dt);
	fullscreenCheckbox->Start();
	fullscreenCheckbox->Update(dt);
	vsyncCheckbox->Start();
	vsyncCheckbox->Update(dt);
	
	fxSlider->Update(dt);
	musicSlider->Update(dt);



	Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/gameplaySongPlaceholder.wav");
	Engine::GetInstance().audio.get()->musicVolume(50);



}
void Scene::CreateItemLvl2(const char* mapName)
{
	if (std::string(mapName) == "MapTemplate2.tmx") {
		// Verificar si ya existe un ítem en el nivel 2
		if (!items.empty()) {
			LOG("El ítem ya existe en el nivel 2. No se creará otro.");
			return;
		}

		for (pugi::xml_node itemNode = configParameters.child("entities").child("items").child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
		{
			Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
			item->SetParameters(itemNode);
			items.push_back(item);
			// L08 TODO 4: Add a physics to an item - initialize the physics body
			item->Start();
		}
	}
}

bool Scene::OnGuiMouseClickEvent(GuiControl* control)
{
	// L15: DONE 5: Implement the OnGuiMouseClickEvent method
	LOG("Press Gui Control: %d", control->id);

	if (control->id == 1) // ID del startButton
	{
		SetState(GameState::PLAYING);
		startButton->CleanUp();
		optionsButton->CleanUp();
		exitButton->CleanUp();
	}
	if (control->id == 4) {

		SetState(GameState::PLAYING);
		returnButton->CleanUp();

	}

	return true;
}
void Scene::FadeTransition(SDL_Renderer* renderer, SDL_Texture* texture, float duration)
{
	Uint32 startTime = SDL_GetTicks();
	Uint8 alpha = 0;
	bool fadeIn = true;

	SDL_Rect screenRect = { 0, 0, 1920, 1080 };
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	while (true)
	{
		Uint32 elapsedTime = SDL_GetTicks() - startTime;
		float progress = (float)elapsedTime / (duration * 1000);

		if (progress > 1.0f)
		{
			if (fadeIn)
			{
				fadeIn = false;
				startTime = SDL_GetTicks();
				progress = 0.0f;
			}
			else
			{
				break;
			}
		}

		alpha = fadeIn
			? (Uint8)(255 * progress)
			: (Uint8)(255 * (1.0f - progress));

		SDL_SetTextureAlphaMod(texture, alpha);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, &screenRect);
		SDL_RenderPresent(renderer);

		SDL_Delay(16);
	}
}