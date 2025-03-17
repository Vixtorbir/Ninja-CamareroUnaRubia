#ifndef STATEMANAGER_H
#define STATEMANAGER_H


#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "src/Scene.h"
#include <string>
#include <unordered_map>

class StateManager {
public:
    StateManager(SDL_Renderer* renderer);
    ~StateManager();

    void ChangeState(GameState newState); // Switch to a new state
    void Update(float deltaTime);         // Update the current state
    void Render();                        // Render the current state
    void HandleEvents(SDL_Event& event);  // Handle events for the current state

private:
    SDL_Renderer* renderer;               // SDL renderer for drawing
    GameState currentState;               // Current game state
    TTF_Font* font;                       // Font for rendering text

    void RenderText(const std::string& text, int x, int y, SDL_Color color); // Helper function to render text
};

#endif // STATEMANAGER_H
