#pragma once
// GameOverState.h
#ifndef GAMEOVERSTATE_H
#define GAMEOVERSTATE_H

#include "GameState.h"
#include <iostream>

class GameOverState : public GameState {
public:
    void Enter() override;
    void Update() override;
    void Render(SDL_Renderer* renderer) override;
    void Exit() override;
};

#endif // GAMEOVERSTATE_H
