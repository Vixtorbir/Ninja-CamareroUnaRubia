#pragma once

#include "Module.h"
#include "SDL2/SDL_Video.h"
#include "SDL2/SDL.h"

class Window : public Module
{
public:

	Window();

	// Destructor
	virtual ~Window();

	// Called before render is available
	bool Awake();

	// Called before quitting
	bool CleanUp();

	// Changae title
	void SetTitle(const char* title);

	// Retrive window size
	void GetWindowSize(int& width, int& height) const;

	// Retrieve window scale
	int GetScale() const;

	int GetCameraZoom() const;

	SDL_Window* GetSDLWindow() const { return sdlWindow; }

public:
	// The window we'll be rendering to
	SDL_Window* window;

	std::string title;
	int width = 1920;
	int height = 1080;
	int scale = 3;

	float zoom;
	SDL_Window* sdlWindow; // Puntero a la ventana SDL

};