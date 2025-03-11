#pragma once
#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include <stack>
#include "GameState.h"

class GameStateManager {
public:
    GameStateManager();
    ~GameStateManager();

    void PushState(GameState* state);
    void PopState();
    void ChangeState(GameState* state);

    void HandleEvents();
    void Update();
    void Render(SDL_Renderer* renderer);

private:
    std::stack<GameState*> stateStack;
};

#endif // GAMESTATEMANAGER_H

