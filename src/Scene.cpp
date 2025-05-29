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
#include "Boss.h"

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

	for (pugi::xml_node itemNode = configParameters.child("entities").child("items").child("testItem"); itemNode; itemNode = itemNode.next_sibling("testItem"))
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

	for (pugi::xml_node bossNode = configParameters.child("entities").child("enemies").child("boss"); bossNode; bossNode = bossNode.next_sibling("boss"))
	{
		Boss* boss = (Boss*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BOSS);
		boss->SetParameters(bossNode);
		bossList.push_back(boss);
	}
	
	// L16: TODO 2: Instantiate a new GuiControlButton in the Scene
	SDL_Rect btPos = { -10000, 350, 120,20 };
	guiBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "MyButton", btPos, this);


	//SDL_Rect btPos2 = { 0, 700, 1920,300 };
	//dialogue = (Dialogue*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::DIALOGUE, 1, "dialogo moment", btPos2, this);
	//

	dialogueManager->SetModule(this);
	for (Enemy* enemy : enemyList)
	{
		enemy->sceneModule = this;
	}
	player->sceneModule = this;

	return ret;

}

// Called before the first frame
bool Scene::Start()
{
	curPopupMusic = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Music/ScreamAngry.ogg");

	logo = Engine::GetInstance().textures->Load("Assets/UI/logo.png");
	logoFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/ExtraFx/logoFx.ogg");
	MenuBackgroundImage = Engine::GetInstance().textures.get()->Load("Assets/UI/TitleScreenWTitle.png");
	BackgroundImage = Engine::GetInstance().textures.get()->Load("Assets/UI/TitleScreen.png");
	//load cinematic parts
	testCinematicBg = Engine::GetInstance().textures.get()->Load("Assets/Textures/npcMENTORSHIP.png");
	//
	mapBackgroundUI = Engine::GetInstance().textures.get()->Load("Assets/UI/MapBackgroundUI.png");
	SDL_Rect btPos00 = { 0, 0, 0,0 };
	mapBackgroundUIImage = (GuiImage*)Engine::GetInstance().guiManager->CreateGuiImage(GuiControlType::IMAGE, 1, "MyButton", btPos00, this, mapBackgroundUI);
	mapBackgroundUIImage->visible = false;

	//Load sound fx
	birdFxID = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/BgSounds/bird chirp.ogg");
	birdTimer = 500;
	cricketFxID = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/BgSounds/crickets.ogg");
	cricketTimer = 400;
	bushFxID = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/BgSounds/bush ratle.ogg");
	bushTimer = 300;
	windFxID = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/BgSounds/wind blow.ogg");
	windTimer = 360;
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



	SDL_Rect startButtonPos = { 1100, 300, 400, 300 };
	SDL_Rect optionsButtonPos = { 1100, 550, 400, 300 };
	SDL_Rect exitButtonPos = { 1100, 800, 400, 300 };
	SDL_Rect vsyncCheckboxPos = { 1100, 550, 400, 300 };
	SDL_Rect fullscreenCheckboxPos = { 1100, 300, 400, 300 };
	SDL_Rect returntomenuButtonPos = { 1100, 800, 400, 300 };
	SDL_Rect returnButtonPos = { 1100, 550, 400, 300 };

	startButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 1, "                        Start Game", startButtonPos, this);
	startButton->Start();
	startButton->visible = true;

	optionsButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 2, "Options", optionsButtonPos, this);
	optionsButton->Start();
	optionsButton->visible = true;

	exitButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 3, "Exit", exitButtonPos, this);
	exitButton->Start();
	exitButton->visible = true;

	//returntomenuButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
	//	GuiControlType::BUTTON, 1, "Return To Menu", returntomenuButtonPos, this);
	//returntomenuButton->Start();
	//returntomenuButton->visible = false;

	vsyncCheckbox = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 1, "Vsync enabled", vsyncCheckboxPos, this);
	vsyncCheckbox->Start();
	vsyncCheckbox->visible = false;

	returntomenuButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 7, "Return", returntomenuButtonPos, this);
	returntomenuButton->Start();
	returntomenuButton->visible = false;


	fullscreenCheckbox = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 1, "FullScreen", fullscreenCheckboxPos, this);
	fullscreenCheckbox->Start();
	fullscreenCheckbox->visible = false;


	returnButton = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
		GuiControlType::BUTTON, 4, "Return", returnButtonPos, this);
	returnButton->Start();
	returnButton->visible = false;

	MenuBackgroundImage = Engine::GetInstance().textures.get()->Load("Assets/UI/TitleScreenWTitle.png");
	menuBackgroundImage->visible = true;


 
	for (Enemy* enemy : enemyList)
	{
		enemy->SetPlayer(player);
	}



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
	for (Enemy* enemy : enemyList)
	{
		if (enemy->aggressive)
		{
			anyAggressiveNow = true;
		}
	}
	if (anyAggressiveNow) player->detectedbool = true; else player->detectedbool = false;

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

	//apply item effects
	for (InventoryItem item : player->inventory)
	{
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN && item.name == "Health Potion") {
			player->HP += 20;
		} else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_DOWN && item.name == "Mana Potion") {
			player->speed += 20;
		}
	}


	HandleInput();

	switch (currentState)
	{
	case GameState::MAIN_MENU:
		UpdateMainMenu(dt);
		break;
	case GameState::CINEMATIC:
		UpdateCinematic(dt);
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
	case GameState::INVENTORY: 
		UpdateInventory(dt);
		break;
	case GameState::OPTIONS:
		UpdateOptions(dt);
		break;
	default:
		break;
	}

	// fx
	switch (player->currentLevel)
	{
	case 1:
		if (windTimer == 0)
		{
			Engine::GetInstance().audio.get()->VolumeFx(windFxID, 100);
			Engine::GetInstance().audio.get()->PlayFx(windFxID);
			
			windTimer = 360;
		}
		else
		{
			windTimer--;
		}

		if (cricketTimer == 0)
		{
			Engine::GetInstance().audio.get()->PlayFx(cricketFxID);
			cricketTimer = 600;
		}
		else
		{
			cricketTimer--;
		}
		
		break;
	case 2:
		if (birdTimer == 0)
		{
			Engine::GetInstance().audio.get()->PlayFx(birdFxID);
			
			birdTimer = 1200;
		}
		else
		{
			birdTimer--;
		}

		if (cricketTimer == 0)
		{
			Engine::GetInstance().audio.get()->PlayFx(cricketFxID);
			cricketTimer = 600;
		}
		else
		{
			cricketTimer--;
		}
		if (bushTimer == 0)
		{
			Engine::GetInstance().audio.get()->PlayFx(bushFxID);
			bushTimer = 420;
		}
		else
		{
			bushTimer--;
		}
		break;
	case 3:
		break;
	case 4:
		break;
	default: 
		break;
	}

	return true;
}

void Scene::loadThingsOfMaps(int index)
{

	switch (index)
	{
	case(0):
		SafeLoadMap("MapTemplate1_64x64.tmx", Vector2D(13552, 3940)); // Posición 
		levelIndex = 0;
		parallax->ChangeTextures(levelIndex);
		Engine::GetInstance().scene.get()->player->loadLevel1 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 1;
		break;
	case(1):
		SafeLoadMap("MapTemplate2_64x64.tmx", Vector2D(1783, 3172)); // Posición 
		levelIndex = 1;
		LoadEntities(2);
		parallax->ChangeTextures(levelIndex);
		Engine::GetInstance().scene.get()->player->loadLevel2 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 2;
		break;
	case(2):
		SafeLoadMap("Cave.tmx", Vector2D(6269, 1860)); // Posición 
		levelIndex = 2;
		LoadEntities(3);
		parallax->ChangeTextures(levelIndex);
		Engine::GetInstance().scene.get()->player->loadLevel3 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 3;

		break;
	case(3):
		SafeLoadMap("BossArena.tmx", Vector2D(2623, 1726));
		levelIndex = 3;
		LoadEntities(4);
		parallax->ChangeTextures(levelIndex);
		Engine::GetInstance().scene.get()->player->loadLevel4 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 4;

		break;

	}
}
// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
		SafeLoadMap("MapTemplate1_64x64.tmx", Vector2D(13552, 3940)); // Posición 
		levelIndex = 0;
		parallax->ChangeTextures(levelIndex);
        Engine::GetInstance().scene.get()->player->loadLevel1 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 1;
	}
	   
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {
		SafeLoadMap("MapTemplate2_64x64.tmx", Vector2D(1783, 3172)); // Posición 
		levelIndex = 1;
		LoadEntities(2);
		parallax->ChangeTextures(levelIndex);
		Engine::GetInstance().scene.get()->player->loadLevel2 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 2;
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F4) == KEY_DOWN) {
		SafeLoadMap("Cave.tmx", Vector2D(6269, 1860)); // Posición 
		levelIndex = 2;
		LoadEntities(3);
		parallax->ChangeTextures(levelIndex);
		Engine::GetInstance().scene.get()->player->loadLevel3 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 3;

		
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN) {
		SafeLoadMap("BossArena.tmx", Vector2D(2623, 1726));
		levelIndex = 3;
		LoadEntities(4);
		parallax->ChangeTextures(levelIndex);
		Engine::GetInstance().scene.get()->player->loadLevel4 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 4;
	

	}
		
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		LoadState();

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		SaveState();

	if (Engine::GetInstance().scene.get()->player->loadLevel1) {

		FadeTransition(Engine::GetInstance().render.get()->renderer, false, 1.0f);
		Engine::GetInstance().map->CleanUp(); // Esto solo limpia recursos antiguos

		SafeLoadMap("MapTemplate1_64x64.tmx", Vector2D(16280, 4720));

		Engine::GetInstance().scene.get()->player->loadLevel1 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 1;
		

	}

	if (Engine::GetInstance().scene.get()->player->loadLevel2) {
	
		FadeTransition(Engine::GetInstance().render.get()->renderer, false, 1.0f);
		Engine::GetInstance().map->CleanUp(); // Esto solo limpia recursos antiguos

		SafeLoadMap("MapTemplate2_64x64.tmx", Vector2D(1783, 3172));

		Engine::GetInstance().scene.get()->player->loadLevel2 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 2;

	}

	if (Engine::GetInstance().scene.get()->player->loadLevel3) {

		FadeTransition(Engine::GetInstance().render.get()->renderer, false, 1.0f);
		Engine::GetInstance().map->CleanUp(); // Esto solo limpia recursos antiguos

		SafeLoadMap("Cave.tmx", Vector2D(6269, 1860));

		Engine::GetInstance().scene.get()->player->loadLevel3 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 3;

	}

	if (Engine::GetInstance().scene.get()->player->loadLevel4) {

		FadeTransition(Engine::GetInstance().render.get()->renderer, false, 1.0f);
		Engine::GetInstance().map->CleanUp(); // Esto solo limpia recursos antiguos

		SafeLoadMap("BossArena.tmx", Vector2D(2623, 1726));

		Engine::GetInstance().scene.get()->player->loadLevel4 = false;
		Engine::GetInstance().scene.get()->player->currentLevel = 4;

	}

	if (Engine::GetInstance().scene.get()->player->loadLevel2back) {

		FadeTransition(Engine::GetInstance().render.get()->renderer, false, 1.0f);
		Engine::GetInstance().map->CleanUp(); // Esto solo limpia recursos antiguos

		SafeLoadMap("MapTemplate2_64x64.tmx", Vector2D(20656, 2394));

		Engine::GetInstance().scene.get()->player->loadLevel2back = false;
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

void Scene::LoadEntities(int sceneIndex)
{
	// Limpia las listas actuales de enemigos, torretas, bosses, items y npcs
	for (auto enemy : enemyList) enemy->CleanUp();
	enemyList.clear();
	for (auto turret : turretList) turret->CleanUp();
	turretList.clear();
	for (auto boss : bossList) boss->CleanUp();
	bossList.clear();


	pugi::xml_node enemiesNode;
	pugi::xml_node itemsNode;
	pugi::xml_node npcsNode;

	if (sceneIndex == 1) {
		enemiesNode = configParameters.child("entities").child("enemies");
		itemsNode = configParameters.child("entities").child("items");
		npcsNode = configParameters.child("entities");
	}
	else if (sceneIndex == 2) {
		enemiesNode = configParameters.child("entitiesbamboo").child("enemies");
		itemsNode = configParameters.child("entitiesbamboo").child("items");
		npcsNode = configParameters.child("entitiesbamboo");
	}
	else if (sceneIndex == 3) {
		enemiesNode = configParameters.child("entitiescave").child("enemies");
		itemsNode = configParameters.child("entitiescave").child("items");
		npcsNode = configParameters.child("entitiescave");
	}
	else if (sceneIndex == 4) {
		enemiesNode = configParameters.child("entitiesboss").child("enemies");
		itemsNode = configParameters.child("entitiesboss").child("items");
		npcsNode = configParameters.child("entitiesboss");
	}
	else {
		LOG("Invalid scene index for loading entities.");
		return;
	}

	// Enemigos, torretas y bosses
	if (enemiesNode) {
		for (pugi::xml_node enemyNode = enemiesNode.child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
			Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
			enemy->SetParameters(enemyNode);
			enemy->Start();
			enemyList.push_back(enemy);
		}
		for (pugi::xml_node turretNode = enemiesNode.child("turret"); turretNode; turretNode = turretNode.next_sibling("turret")) {
			Turret* turret = (Turret*)Engine::GetInstance().entityManager->CreateEntity(EntityType::TURRET);
			turret->SetParameters(turretNode);
			turret->Start();
			turretList.push_back(turret);
		}
		for (pugi::xml_node bossNode = enemiesNode.child("boss"); bossNode; bossNode = bossNode.next_sibling("boss")) {
			Boss* boss = (Boss*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BOSS);
			boss->SetParameters(bossNode);
			boss->Start();
			bossList.push_back(boss);
		}
	}

	// Items normales y testItems
	if (itemsNode) {
		for (pugi::xml_node itemNode = itemsNode.child("item"); itemNode; itemNode = itemNode.next_sibling("item")) {
			Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
			item->SetParameters(itemNode);
			item->Start(); 
			item->Puzzle();
			items.push_back(item);
		}
		for (pugi::xml_node testItemNode = itemsNode.child("testItem"); testItemNode; testItemNode = testItemNode.next_sibling("testItem")) {
			Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
			item->SetParameters(testItemNode);
			item->Start();
			items.push_back(item);
		}
	}

	// NPCs (busca todos los nodos npcX)
	if (npcsNode) {
		for (pugi::xml_node npcNode = npcsNode.child("npcMENTORSHIP"); npcNode; npcNode = npcNode.next_sibling("npcMENTORSHIP")) {
			NPC* npc = (NPC*)Engine::GetInstance().entityManager->CreateNamedCharacter(EntityType::NPC, DialogueEngine::MENTORSHIP);
			npc->SetParameters(npcNode);
			npc->Start();
			npcs.push_back(npc);
		}
		for (pugi::xml_node npcNode = npcsNode.child("npcISAMU"); npcNode; npcNode = npcNode.next_sibling("npcISAMU")) {
			NPC* npc = (NPC*)Engine::GetInstance().entityManager->CreateNamedCharacter(EntityType::NPC, DialogueEngine::ISAMU);
			npc->SetParameters(npcNode);
			npc->Start();
			npcs.push_back(npc);
		}
		for (pugi::xml_node npcNode = npcsNode.child("npcKAEDE"); npcNode; npcNode = npcNode.next_sibling("npcKAEDE")) {
			NPC* npc = (NPC*)Engine::GetInstance().entityManager->CreateNamedCharacter(EntityType::NPC, DialogueEngine::KAEDE);
			npc->SetParameters(npcNode);
			npc->Start();
			npcs.push_back(npc);
		}
		for (pugi::xml_node npcNode = npcsNode.child("npcHANZO"); npcNode; npcNode = npcNode.next_sibling("npcHANZO")) {
			NPC* npc = (NPC*)Engine::GetInstance().entityManager->CreateNamedCharacter(EntityType::NPC, DialogueEngine::HANZO);
			npc->SetParameters(npcNode);
			npc->Start();
			npcs.push_back(npc);
		}
	}

	for (Enemy* enemy : enemyList)
	{
		enemy->SetPlayer(player);
		enemy->sceneModule = this;
	}
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

	for (const auto turret : turretList) {
		turret->CleanUp();
	}

	turretList.clear();

	for (const auto boss : bossList) {
		boss->CleanUp();
	}

	bossList.clear();


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
	if (std::string(mapName) == "MapTemplate2_64x64.tmx") {
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

		MenuBackgroundImage = Engine::GetInstance().textures.get()->Load("Assets/UI/TitleScreenWTitle.png");
	}
	

	menuBackgroundImage->visible=true;
	startButton->visible = true;
	optionsButton->visible = true;
	exitButton->visible = true;

	menuBackgroundImage->Update(dt);
	startButton->Update(dt);
	optionsButton->Update(dt);
	exitButton->Update(dt);
	 
	player->inGame = false;


	// Render the main menu text


	

	 
}
void Scene::UpdateCinematic(float dt)
{
	
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		fadeDuration += dt;
	    FadeTransition(Engine::GetInstance().render.get()->renderer,testCinematicBg, 2.0f);

		SetState(GameState::PLAYING);
	}
	Engine::GetInstance().render.get()->DrawTexture(testCinematicBg, 0, 0);
	

}
void Scene::HandleInput()
{
	if (startButton != nullptr) {
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN || startButton->isClicked == true)
		{
			if (currentState == GameState::MAIN_MENU)
			{

				SetState(GameState::CINEMATIC);
				Engine::GetInstance().audio.get()->StopMusic();
				Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/gameplaySongPlaceholder.ogg");
				startButton->visible=false;
				optionsButton->visible=false;
				exitButton->visible = false;
				menuBackgroundImage->visible=false;
				startButton->isClicked = false;
			
				

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
				returnButton->visible = false;
				menuBackgroundImage->visible = false;
				exitButton->visible = false;
				returnButton->isClicked = false;
			}
		}
		if (currentState == GameState::GAME_OVER) {

			if (returnButton->isClicked == true) {
				SetState(GameState::PLAYING);
				LoadState();
				returnButton->isClicked = false;
			}
		}
		returnButton->visible=false;



	}
	if (returntomenuButton != nullptr) {
		if (currentState == GameState::GAME_OVER || currentState == GameState::OPTIONS) {
			if (returntomenuButton->isClicked == true) {
				
				returntomenuButton->visible = false;
				fullscreenCheckbox->visible = false;
				vsyncCheckbox->visible = false;
				SetState(GameState::MAIN_MENU);
				returntomenuButton->isClicked = false;

			}
		}
		//returntomenuButton->CleanUp();
	}
	if (exitButton != nullptr) {
		if (exitButton->isClicked == true&&( currentState ==GameState::PAUSED||currentState==GameState::GAME_OVER)) {
			SetState(GameState::MAIN_MENU);
			exitButton->visible = false;
			returnButton->visible = false;
			menuBackgroundImage->visible = false;
			exitButton->isClicked = false;
		}

	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_P) == KEY_DOWN)
	{
		if (currentState == GameState::PLAYING)
		{
			SetState(GameState::PAUSED);
		}
	
	}
	if (optionsButton != nullptr) {

		if (optionsButton->isClicked == true) {

			SetState(GameState::OPTIONS);
			startButton->visible=false;
			optionsButton->visible = false;
			exitButton->visible = false;
			menuBackgroundImage->visible = false;
			optionsButton->isClicked = false;	
			


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

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_I) == KEY_DOWN) {
		if (currentState == GameState::PLAYING) {
			SetState(GameState::INVENTORY);
		}
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_G) == KEY_DOWN) {
		if (currentState == GameState::PLAYING) {
			SetState(GameState::GAME_OVER);
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
	

}

void Scene::UpdatePaused(float dt) {
	exitButton->isClicked = false;
	menuBackgroundImage->visible = true;	
	returnButton->visible = true;
	exitButton->visible = true;
	returnButton->Update(dt);
	exitButton->Update(dt);

	Engine::GetInstance().render.get()->DrawText("PAUSED: Press enter to start", 600, 400, 750, 255);
}

void Scene::UpdateGameOver(float dt) {
	exitButton->isClicked = false;
	menuBackgroundImage->visible = false;
	Engine::GetInstance().render.get()->DrawText("GAME OVER", 600, 200, 750, 255);
	exitButton->visible = true;
	exitButton->Update(dt);



}
void Scene::UpdateLogo(float dt) {
	// Manejar el fade in

	
	fadeDuration += dt;
	
	FadeTransition(Engine::GetInstance().render.get()->renderer, logo, .1f);
	
	SetState(GameState::MAIN_MENU);
	//Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/titleSongPlaceholder.ogg");
	//Engine::GetInstance().audio.get()->musicVolume(50);
	

}
void Scene::UpdateOptions(float dt)
{
	Engine::GetInstance().render.get()->DrawWhiteText("OPTIONS", 600, 200, 750, 255);

	
	if (!menuBackgroundImage) {

		MenuBackgroundImage = Engine::GetInstance().textures.get()->Load("Assets/UI/TitleScreenWTitle.png");
	}
	if (!fullscreenCheckbox) {
		SDL_Rect fullscreenCheckboxPos = { 800, 500, 200, 50 };
		fullscreenCheckbox = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::BUTTON, 1, "FullScreen", fullscreenCheckboxPos, this);
		
	}
	if (!vsyncCheckbox) {
		SDL_Rect vsyncCheckboxPos = { 800, 250, 200, 50 };
		vsyncCheckbox = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(
			GuiControlType::BUTTON, 2, "Vsync enabled", vsyncCheckboxPos, this);
		
	}


	menuBackgroundImage->visible = true;	
	returntomenuButton->visible = true;
	fullscreenCheckbox->visible = true;
	vsyncCheckbox->visible = true;
	returntomenuButton->Update(dt);
	menuBackgroundImage->Update(dt);
	fullscreenCheckbox->Update(dt);
	vsyncCheckbox->Update(dt);
	
	//fxSlider->Update(dt);
	//musicSlider->Update(dt);



	//Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/gameplaySongPlaceholder.wav");
	//Engine::GetInstance().audio.get()->musicVolume(50);



}

void Scene::UpdateInventory(float dt) {
	Engine::GetInstance().render.get()->DrawText("INVENTORY", 600, 50, 750, 255);

	int xOffset = 600;
	int yOffset = 300;
	int iconSize = 64; // Tamaño del ícono
	int squareSize = 74; // Tamaño del cuadrado amarillo (iconSize + 10)
	int spacing = 20;

	static bool showItemInfo = false; // Variable para controlar si se muestra la información del objeto
	static std::string itemName = "";
	static std::string itemDescription = "";

	// Navegación del inventario
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN) {
		selectedItemIndex = (selectedItemIndex + 1) % player->inventory.size();
		showItemInfo = false; // Ocultar información al cambiar de selección
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN) {
		selectedItemIndex = (selectedItemIndex - 1 + player->inventory.size()) % player->inventory.size();
		showItemInfo = false; // Ocultar información al cambiar de selección
	}

	// Dibujar los íconos del inventario
	for (size_t i = 0; i < player->inventory.size(); ++i) {
		const InventoryItem& item = player->inventory[i];

		// Calcular la posición del cuadrado amarillo
		int x = xOffset + i * (squareSize + spacing);
		int y = yOffset;

		// Dibujar un cuadrado amarillo alrededor del objeto seleccionado
		if (i == selectedItemIndex) {
			SDL_Rect selectionRect = { x, y, squareSize, squareSize };
			SDL_SetRenderDrawColor(Engine::GetInstance().render.get()->renderer, 255, 255, 0, 255);
			SDL_RenderDrawRect(Engine::GetInstance().render.get()->renderer, &selectionRect);
		}

		// Calcular la posición del ícono para centrarlo dentro del cuadrado amarillo
		int iconX = x + (squareSize - iconSize) / 2;
		int iconY = y + (squareSize - iconSize) / 2;


		// Dibujar el ícono del objeto
		Engine::GetInstance().render.get()->DrawTexturedRectangle(item.icon, iconX, iconY, iconSize, iconSize, false);
	}

	// Mostrar el nombre y la descripción del objeto seleccionado al presionar Enter
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) {
		if (!player->inventory.empty()) {
			const InventoryItem& selectedItem = player->inventory[selectedItemIndex];
			itemName = "" + selectedItem.name;
			itemDescription = "" + selectedItem.description;
			showItemInfo = true; 
		}
	}

	// Dibujar la información del objeto si está activa
	if (showItemInfo) {
		int textSize = 500; 
		int nameY = 250;    
		int descriptionY = nameY + 250; 

		// Definir el color rojo
		SDL_Color redColor = { 255, 0, 0, 255 };

		// Dibujar el nombre y la descripción con el color rojo
		Engine::GetInstance().render.get()->DrawTextColor(itemName.c_str(), 700, nameY, textSize, 105, redColor);
		Engine::GetInstance().render.get()->DrawTextColor(itemDescription.c_str(), 700, descriptionY, textSize, 105, redColor);
	}

	// Regresar al estado de juego
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_U) == KEY_DOWN) {
		SetState(GameState::PLAYING);
		showItemInfo = false; // Ocultar información al salir del inventario
	}
}





void Scene::CreateItemLvl2(const char* mapName)
{
	if (std::string(mapName) == "MapTemplate2_64x64.tmx") {
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
			item->Puzzle();
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