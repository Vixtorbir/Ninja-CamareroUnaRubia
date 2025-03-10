#pragma once

#include "Module.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_rect.h"
#include "Vector2D.h"
#include <unordered_map>
#include <vector>

#define NUM_MOUSE_BUTTONS 5

enum EventWindow
{
	WE_QUIT = 0,
	WE_HIDE = 1,
	WE_SHOW = 2,
	WE_COUNT
};

enum KeyState
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};
enum InputAction
{
	HORIZONTAL_LEFT,
	HORIZONTAL_RIGHT,
	JUMP,
	ATTACK,
	ACTION_COUNT
};

class Input : public Module
{

public:

	Input();

	// Destructor
	virtual ~Input();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();

	// Called before quitting
	bool CleanUp();

	// Check key states (includes mouse and joy buttons)
	KeyState GetKey(int id) const
	{
		return keyboard[id];
	}

	KeyState GetMouseButtonDown(int id) const
	{
		return mouseButtons[id - 1];
	}

	// Check if a certain window event happened
	bool GetWindowEvent(EventWindow ev);

	// Get mouse / axis position
	Vector2D GetMousePosition();
	Vector2D GetMouseMotion();


	SDL_GameController* FindController();
	KeyState GetControllerButton(SDL_GameControllerButton button);

	Sint16 GetControllerAxis(SDL_GameControllerAxis axis);




private:
	bool windowEvents[WE_COUNT];
	KeyState*	keyboard;
	KeyState mouseButtons[NUM_MOUSE_BUTTONS];
	int	mouseMotionX;
	int mouseMotionY;
	int mouseX;
	int mouseY;

	SDL_GameController* controller;
	KeyState controllerButtons[SDL_CONTROLLER_BUTTON_MAX];
	Sint16 controllerAxes[SDL_CONTROLLER_AXIS_MAX];





};

