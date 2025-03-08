#include "Engine.h"
#include "Input.h"
#include "Window.h"
#include "Log.h"
#include <unordered_map>
#include <vector>


#define MAX_KEYS 300

Input::Input() : Module()
{
	name = "input";

	keyboard = new KeyState[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KeyState) * MAX_KEYS);
	memset(mouseButtons, KEY_IDLE, sizeof(KeyState) * NUM_MOUSE_BUTTONS);



	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
	{
		LOG("SDL GameController could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
}

// Destructor
Input::~Input()
{
	delete[] keyboard;
}

// Called before render is available
bool Input::Awake()
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

// Called before the first frame
bool Input::Start()
{
	SDL_StopTextInput();
	return true;
}

// Called each loop iteration
bool Input::PreUpdate()
{
    // Update action states before processing events
    for (auto& state : actionStates)
    {
        if (state.second == KEY_DOWN)
            state.second = KEY_REPEAT;
        else if (state.second == KEY_UP)
            state.second = KEY_IDLE;
    }

    static SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        switch (event.type)
        {
        case SDL_CONTROLLERBUTTONDOWN:
            for (auto& pair : joystickButtonBindings)
            {
                if (std::find(pair.second.begin(), pair.second.end(), event.cbutton.button) != pair.second.end())
                {
                    actionStates[pair.first] = KEY_DOWN;
                }
            }
            break;

        case SDL_CONTROLLERBUTTONUP:
            for (auto& pair : joystickButtonBindings)
            {
                if (std::find(pair.second.begin(), pair.second.end(), event.cbutton.button) != pair.second.end())
                {
                    actionStates[pair.first] = KEY_UP;
                }
            }
            break;

        case SDL_CONTROLLERAXISMOTION:
            for (auto& pair : joystickAxisBindings)
            {
                if (pair.second == event.caxis.axis)
                {
                    if (event.caxis.value < -8000)
                    {
                        actionStates[pair.first] = KEY_DOWN;
                    }
                    else if (event.caxis.value > 8000)
                    {
                        actionStates[pair.first] = KEY_DOWN;
                    }
                    else
                    {
                        if (actionStates[pair.first] == KEY_DOWN || actionStates[pair.first] == KEY_REPEAT)
                            actionStates[pair.first] = KEY_UP;
                    }
                }
            }
            break;


        case SDL_QUIT:
            windowEvents[WE_QUIT] = true;
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_HIDDEN:
            case SDL_WINDOWEVENT_MINIMIZED:
            case SDL_WINDOWEVENT_FOCUS_LOST:
                windowEvents[WE_HIDE] = true;
                break;

            case SDL_WINDOWEVENT_SHOWN:
            case SDL_WINDOWEVENT_FOCUS_GAINED:
            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESTORED:
                windowEvents[WE_SHOW] = true;
                break;
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            mouseButtons[event.button.button - 1] = KEY_DOWN;
            break;

        case SDL_MOUSEBUTTONUP:
            mouseButtons[event.button.button - 1] = KEY_UP;
            break;

        case SDL_MOUSEMOTION:
            int scale = Engine::GetInstance().window.get()->GetScale();
            mouseMotionX = event.motion.xrel / scale;
            mouseMotionY = event.motion.yrel / scale;
            mouseX = event.motion.x / scale;
            mouseY = event.motion.y / scale;
            break;
        }
    }

    // Handle keyboard state updates
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < MAX_KEYS; ++i)
    {
        if (keys[i] == 1)
        {
            if (keyboard[i] == KEY_IDLE)
                keyboard[i] = KEY_DOWN;
            else
                keyboard[i] = KEY_REPEAT;
        }
        else
        {
            if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
                keyboard[i] = KEY_UP;
            else
                keyboard[i] = KEY_IDLE;
        }
    }

    // Update mouse button states
    for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
    {
        if (mouseButtons[i] == KEY_DOWN)
            mouseButtons[i] = KEY_REPEAT;

        if (mouseButtons[i] == KEY_UP)
            mouseButtons[i] = KEY_IDLE;
    }

    return true;
}


// Called before quitting
bool Input::CleanUp()
{
	LOG("Quitting SDL event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}


bool Input::GetWindowEvent(EventWindow ev)
{
	return windowEvents[ev];
}

Vector2D Input::GetMousePosition()
{
	return Vector2D(mouseX, mouseY);
}

Vector2D Input::GetMouseMotion()
{
	return Vector2D(mouseMotionX, mouseMotionY);
}
void Input::BindAction(InputAction action, int key, int joystickButton, int axis)
{
	if (key != -1)
		keyBindings[action].push_back(key);

	if (joystickButton != -1)
		joystickButtonBindings[action].push_back(joystickButton);

	if (axis != -1)
		joystickAxisBindings[action] = axis;
}
KeyState Input::GetActionState(InputAction action)
{
		auto it = actionStates.find(action);
		if (it != actionStates.end())
			return it->second;

		return KEY_IDLE;
	
}