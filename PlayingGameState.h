// PlayingState.h
#ifndef PLAYINGSTATE_H
#define PLAYINGSTATE_H

#include "GameState.h"
#include <iostream>

class PlayingState : public GameState {
public:
    void Enter() override;
    void Update() override;
    void Render(SDL_Renderer* renderer) override;
    void Exit() override;
};

#endif // PLAYINGSTATE_H