#pragma once
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "SDL2/SDL.h"

class GameState {
public:
    virtual ~GameState() {}
    virtual void Enter() = 0;
    virtual void Update() = 0;
    virtual void Render(SDL_Renderer* renderer) = 0;
    virtual void Exit() = 0;
};

#endif // GAMESTATE_H

