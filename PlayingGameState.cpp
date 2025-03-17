// PlayingState.cpp
#include "PlayingGameState.h"

void PlayingState::Enter() {
    std::cout << "Entering Playing State..." << std::endl;
}

void PlayingState::Update() {
    std::cout << "Game is running..." << std::endl;
}

void PlayingState::Render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void PlayingState::Exit() {
    std::cout << "Exiting Playing State..." << std::endl;
}