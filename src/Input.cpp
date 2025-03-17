#include "Engine.h"
#include "Input.h"
#include "Window.h"
#include "Log.h"

#define MAX_KEYS 300

Input::Input() : Module()
{
	name = "input";

	keyboard = new KeyState[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KeyState) * MAX_KEYS);
	memset(mouseButtons, KEY_IDLE, sizeof(KeyState) * NUM_MOUSE_BUTTONS);




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
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        LOG("SDL_GAMECONTROLLER could not initialize! SDL_Error: %s", SDL_GetError());
        ret = false;
    }
	return ret;
}
SDL_GameController* Input::FindController()
{
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            LOG("Controller found at index %d: %s", i, SDL_GameControllerNameForIndex(i));
            return SDL_GameControllerOpen(i);
        }
    }
    LOG("No compatible game controller found");
    return nullptr;
}
// Called before the first frame
bool Input::Start()
{
	SDL_StopTextInput();

    controller = FindController();
    if (controller != nullptr) {
        LOG("Game controller connected: %s", SDL_GameControllerName(controller));
    }
    else {
        LOG("No game controller detected at startup");
    }

	return true;
}

// Called each loop iteration
bool Input::PreUpdate()
{
 

    static SDL_Event event;


    const Uint8* keys = SDL_GetKeyboardState(NULL);

    for (int i = 0; i < MAX_KEYS; ++i) {
        if (keys[i] == 1) {
            if (keyboard[i] == KEY_IDLE)
                keyboard[i] = KEY_DOWN;
            else
                keyboard[i] = KEY_REPEAT;
        }
        else {
            if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
                keyboard[i] = KEY_UP;
            else
                keyboard[i] = KEY_IDLE;
        }
    }

    // Mouse button input
    for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i) {
        
        if (mouseButtons[i] == KEY_DOWN)
            mouseButtons[i] = KEY_REPEAT;

        if (mouseButtons[i] == KEY_UP)
            mouseButtons[i] = KEY_IDLE;
    }

    if (controller != nullptr) {
        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
            bool isPressed = SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)i);
            if (isPressed) {
                if (controllerButtons[i] == KEY_IDLE) {
                    controllerButtons[i] = KEY_DOWN;
                }
                else {
                    controllerButtons[i] = KEY_REPEAT;
                }
            }
            else {
                if (controllerButtons[i] == KEY_REPEAT || controllerButtons[i] == KEY_DOWN) {
                    controllerButtons[i] = KEY_UP;
                }
                else {
                    controllerButtons[i] = KEY_IDLE;
                }
            }
        }

        for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; ++i) {
            Sint16 axisValue = SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis)i);
            if (axisValue != controllerAxes[i]) {
                controllerAxes[i] = axisValue;
            }
        }
    }

    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT:
            windowEvents[WE_QUIT] = true;
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_HIDDEN:
            case SDL_WINDOWEVENT_MINIMIZED:
            case SDL_WINDOWEVENT_FOCUS_LOST:
                windowEvents[WE_HIDE] = true;
                break;

					//case SDL_WINDOWEVENT_ENTER:
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
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);

    if (controller != nullptr) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
        LOG("Game controller disconnected during cleanup");
    }
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
KeyState Input::GetControllerButton(SDL_GameControllerButton button)
{
    return controllerButtons[button];
}

Sint16 Input::GetControllerAxis(SDL_GameControllerAxis axis)
{
    return controllerAxes[axis];
}