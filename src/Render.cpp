#include "Engine.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"
#include "Player.h"
//#include "tracy/Tracy.hpp"
#include "Scene.h"
#define VSYNC true

Render::Render() : Module()
{
	name = "render";
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render()
{}

// Called before render is available
bool Render::Awake()
{
	LOG("Create SDL rendering context");
	bool ret = true;

	Uint32 flags = SDL_RENDERER_ACCELERATED;

	//L05 TODO 5 - Load the configuration of the Render module
	if (configParameters.child("vsync").attribute("value").as_bool() == true) {
		flags |= SDL_RENDERER_PRESENTVSYNC;
		LOG("Using vsync");
	}
	int scale = Engine::GetInstance().window.get()->GetScale();
	float camera_zoom = Engine::GetInstance().window.get()->GetCameraZoom();
	SDL_Window* window = Engine::GetInstance().window.get()->window;
	renderer = SDL_CreateRenderer(window, -1, flags);

	if(renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		camera.w = Engine::GetInstance().window.get()->width * scale;
		camera.h = Engine::GetInstance().window.get()->height * scale;
		camera.x = 6668;
		camera.y = 3959;
	}

	//initialise the SDL_ttf library
	TTF_Init();

	//load a font into memory
	font = TTF_OpenFont("Assets/Fonts/Ancizar/AncizarSerif-Regular.ttf", 40);
	fontNumbers = TTF_OpenFont("Assets/Fonts/arial/arial.ttf", 50);

	return ret;
}

// Called before the first frame
bool Render::Start()
{
	minimapTexture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		256, 256  // Minimap resolution (adjust as needed)
	);
	minimapRect = { 1920 - 266, 10, 256, 256 };  // Top-right corner
	minimapZoom = 0.05f;  // Show 5x more area than the main camera
	minimapEnabled = true;

	camera.x = 6668;
	camera.y = 3959;
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	return true;
}
void Render::RenderMinimap() {
	if (!minimapEnabled) return;

	// Get data
	Scene* scene = Engine::GetInstance().scene.get();
	Vector2D playerPos = scene->GetPlayerPosition();
	Map* map = Engine::GetInstance().map.get();

	// Set render target
	SDL_SetRenderTarget(renderer, minimapTexture);
	SDL_RenderClear(renderer);

	// Calculate view area
	SDL_Rect minimapView = {
		(int)(playerPos.getX() - (minimapRect.w / (2 * minimapZoom))),
		(int)(playerPos.getY() - (minimapRect.h / (2 * minimapZoom))),
		(int)(minimapRect.w / minimapZoom),
		(int)(minimapRect.h / minimapZoom)
	};

	int mapWidth, mapHeight;
	map->GetMapDimensions(mapWidth, mapHeight);
	MapLayer* navLayer = map->GetNavigationLayer();

	if (navLayer) {  // Only proceed if we found the layer
		for (int y = 0; y < mapHeight; y++) {
			for (int x = 0; x < mapWidth; x++) {
				int tileIndex = y * mapWidth + x;
				if (navLayer->tiles[tileIndex] != 0) {  // Access tiles through the layer
					SDL_Rect tileRect = {
						(int)((x * map->mapData.tileWidth - minimapView.x) * minimapZoom),
						(int)((y * map->mapData.tileHeight - minimapView.y) * minimapZoom),
						(int)(map->mapData.tileWidth * minimapZoom),
						(int)(map->mapData.tileHeight * minimapZoom)
					};
					DrawRectangle(tileRect, 0, 0, 0, 255, true, false);
				}
			}
		}
	}
	// Draw colliders (from previous implementation)
	for (size_t i = 0; i < map->collisionBodies.size(); i++) {
		PhysBody* body = map->collisionBodies[i];
		int x, y;
		body->GetPosition(x, y);
		SDL_Rect rect = {
			(int)((x - minimapView.x) * minimapZoom),
			(int)((y - minimapView.y) * minimapZoom),
			(int)(body->width * minimapZoom * 2 + 2),
			(int)(body->height * minimapZoom *2 + 2)
		};
		// Draw colliders in dark gray
		DrawRectangle(rect, 255, 255, 255, 255, true, false);
	}
	MapLayer* tileLayer = nullptr;
	const char* layerNames[] = { "Floor", "Walkable", "Navigation", "Tile Layer 1" };
	for (const char* name : layerNames) {
		tileLayer = map->GetLayer(name);
		if (tileLayer) break;
	}

	if (tileLayer) {
		for (int y = 0; y < tileLayer->height; y++) {
			for (int x = 0; x < tileLayer->width; x++) {
				int gid = tileLayer->Get(x, y);
				if (gid > 0) {
					SDL_Rect dest = {
						(int)((x * 128 - minimapView.x) * minimapZoom),
						(int)((y * 128 - minimapView.y) * minimapZoom),
						(int)(128 * minimapZoom),
						(int)(128 * minimapZoom)
					};

					// Draw walkable tiles as light gray
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderFillRect(renderer, &dest);
				}
			}
		}
	}
	// Draw player (red dot)
	SDL_Rect playerRect = {
		minimapRect.w / 2 - 2,
		minimapRect.h / 2 - 2,
		4, 4
	};
	DrawRectangle(playerRect, 255, 0, 0, 255, true, false);

	// Reset render target
	SDL_SetRenderTarget(renderer, nullptr);
}// Called each loop iteration
bool Render::PreUpdate()
{
	//ZoneScoped;
	// Code you want to profile

	SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	SDL_RenderSetLogicalSize(renderer, 1920, 1080);
	return true;
}

bool Render::PostUpdate()
{
	//ZoneScoped;
	// Code you want to profile
	if (minimapEnabled) {
		// Draw the minimap texture
		SDL_RenderCopy(renderer, minimapTexture, nullptr, &minimapRect);

		// Optional: Add a border
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &minimapRect);
	}
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);
	

	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

// Blit to screen
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_Rect rect;
	rect.x = (int)(camera.x * speed) + x * scale;
	rect.y = (int)(camera.y * speed) + y * scale;

	if(section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w *= scale;
	rect.h *= scale;

	SDL_Point* p = NULL;
	SDL_Point pivot;

	if(pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = pivotX;
		pivot.y = pivotY;
		p = &pivot;
	}

	if(SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, SDL_FLIP_NONE) != 0)
	{
		//LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}
bool Render::DrawEntity(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, bool direction) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_Rect rect;
	rect.x = (int)(camera.x * speed) + x * scale;
	rect.y = (int)(camera.y * speed) + y * scale;

	if (section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w *= scale;
	rect.h *= scale;

	SDL_Point* p = NULL;
	SDL_Point pivot;

	if (pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = pivotX;
		pivot.y = pivotY;
		p = &pivot;
	}
	SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL;
	if (direction == 0)
	{
		flip = SDL_FLIP_NONE;
	}
	else {
		flip = SDL_FLIP_HORIZONTAL;
	}
	if (SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}
bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = (int)(camera.x + rect.x * scale);
		rec.y = (int)(camera.y + rect.y * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}
bool Render::DrawTexturedRectangle(SDL_Texture* texture, int posX, int posY, int width, int height, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_Rect rect;
	rect.x = posX;
	rect.y = posY;
	rect.w = width;
	rect.h = height;

	if (use_camera)
	{
		rect.x = (int)(camera.x + posX * scale);
		rect.y = (int)(camera.y + posY * scale);
		rect.w *= scale;
		rect.h *= scale;
	}

	if (SDL_RenderCopy(renderer, texture, NULL, &rect) != 0)
	{
		//LOG("Cannot draw textured rectangle to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer, camera.x + x1 * scale, camera.y + y1 * scale, camera.x + x2 * scale, camera.y + y2 * scale);
	else
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	for(int i = 0; i < 360; ++i)
	{
		points[i].x = (int)(x * scale + camera.x) + (int)(radius * cos(i * factor));
		points[i].y = (int)(y * scale + camera.y) + (int)(radius * sin(i * factor));
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawText(const char* text, int posx, int posy, int w, int h) const
{

	SDL_Color color = { 0, 0, 0 };
	SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
	SDL_Rect dstrect = { posx, posy, w, h };

	SDL_RenderCopy(renderer, texture, NULL, &dstrect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	return true;
}

bool Render::DrawTextColor(const char* text, int posx, int posy, int w, int h, SDL_Color color) const
{

	SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
	SDL_Rect dstrect = { posx, posy, w, h };

	SDL_RenderCopy(renderer, texture, NULL, &dstrect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	return true;
}


bool Render::DrawNumbers(const char* text, int posx, int posy, int w, int h) const
{

	SDL_Color color = { 0, 0, 0 };
	SDL_Surface* surface = TTF_RenderText_Solid(fontNumbers, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
	SDL_Rect dstrect = { posx, posy, w, h };

	SDL_RenderCopy(renderer, texture, NULL, &dstrect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	return true;
}
bool Render::DrawWhiteText(const char* text, int posx, int posy, int w, int h) const
{

	SDL_Color color = { 255, 255, 255 };
	SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
	SDL_Rect dstrect = { posx, posy, w, h };

	SDL_RenderCopy(renderer, texture, NULL, &dstrect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	return true;
}

bool Render::DrawTextWhite(const char* text, int posx, int posy, int w, int h) const
{
	SDL_Color color = { 255, 255, 255 };
	SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
	SDL_Rect dstrect = { posx, posy, w, h };

	SDL_RenderCopy(renderer, texture, NULL, &dstrect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	return true;
}

