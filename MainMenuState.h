// MainMenuState.h
#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "GameState.h"
#include <iostream>

class MainMenuState : public GameState {
public:
    void Enter() override;
    void Update() override;
    void Render(SDL_Renderer* renderer) override;
    void Exit() override;
};

#endif // MAINMENUSTATE_H
