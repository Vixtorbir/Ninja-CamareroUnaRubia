// MainMenuState.cpp
#include "MainMenuState.h"

void MainMenuState::Enter() {
    std::cout << "Entering Main Menu State..." << std::endl;
}

void MainMenuState::Update() {
    std::cout << "Main Menu: Press Play to Start." << std::endl;
}

void MainMenuState::Render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void MainMenuState::Exit() {
    std::cout << "Exiting Main Menu State..." << std::endl;
}
