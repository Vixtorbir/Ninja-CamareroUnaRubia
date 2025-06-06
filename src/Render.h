#pragma once

#include "Module.h"
#include "Vector2D.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

class Render : public Module
{
public:

	Render();

	// Destructor
	virtual ~Render();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	void SetMinimapSize();

	void RenderMinimap();

	// Called each loop iteration
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();

			// Use colliders and playerX/Y instead of scene/player
		// Drawing
	bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section = NULL, float speed = 1.0f, double angle = 0, int pivotX = INT_MAX, int pivotY = INT_MAX) const;
	bool DrawEntity(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, bool direction) const;
	bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, bool direction) const;
	bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool useCamera = true) const;
	bool DrawTexturedRectangle(SDL_Texture* texture, int posX, int posY, int width, int height, bool use_camera) const;
	
	bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	bool DrawCircle(int x1, int y1, int redius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	bool DrawText(const char* text, int posX, int posY, int w, int h) const;

	bool DrawTextColor(const char* text, int posx, int posy, int w, int h, SDL_Color color) const;

	bool DrawNumbers(const char* text, int posx, int posy, int w, int h) const;

	bool DrawWhiteText(const char* text, int posx, int posy, int w, int h) const;

	void EnableMinimap(bool enable);


	bool DrawTextWhite(const char* text, int posx, int posy, int w, int h) const;
	

	// Set background color
	void SetBackgroundColor(SDL_Color color);

public:
	SDL_Texture* sceneTexture = nullptr;
	SDL_Texture* minimapTexture;
	SDL_Rect minimapRect;
	float minimapZoom;
	bool minimapEnabled = false;
	float tileSizeScale;
	SDL_Renderer* renderer;
	SDL_Rect camera;
	SDL_Rect viewport;
	SDL_Color background;
	TTF_Font* font;	
	TTF_Font* fontNumbers;

};

