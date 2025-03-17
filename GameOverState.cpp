// GameOverState.cpp
#include "GameOverState.h"

void GameOverState::Enter() {
    std::cout << "Entering Game Over State..." << std::endl;
}

void GameOverState::Update() {
    std::cout << "Game Over: Play Again?" << std::endl;
}

void GameOverState::Render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void GameOverState::Exit() {
    std::cout << "Exiting Game Over State..." << std::endl;
}
