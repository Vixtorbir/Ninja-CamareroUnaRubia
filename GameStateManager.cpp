#include "GameStateManager.h"
#include <iostream>

StateManager::StateManager(SDL_Renderer* renderer)
    : renderer(renderer), currentState(GameState::MAIN_MENU) { // Initialize currentState to MAIN_MENU
    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
    }
    font = TTF_OpenFont("assets/font.ttf", 24);
    if (!font) {
        std::cerr << "TTF_OpenFont failed: " << TTF_GetError() << std::endl;
    }
}


StateManager::~StateManager() {
    TTF_CloseFont(font);
    TTF_Quit();
}

void StateManager::ChangeState(GameState newState) {
    std::cout << "Changing state to: " << static_cast<int>(newState) << std::endl;
    currentState = newState;
}

void StateManager::Update(float deltaTime) {
    switch (currentState) {
    case GameState::MAIN_MENU:
        // Update logic for the main menu
        break;
    case GameState::PLAY:
        // Update logic for the play state
        break;
    case GameState::PAUSE:
        // Update logic for the pause state
        break;
    case GameState::GAME_OVER:
        // Update logic for the game over state
        break;
    }
}

void StateManager::Render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Clear screen with black
    SDL_RenderClear(renderer);

    switch (currentState) {
    case GameState::MAIN_MENU:
        RenderText("Main Menu", 300, 250, { 255, 255, 255, 255 });
        RenderText("Press ENTER to Play", 250, 300, { 255, 255, 255, 255 });
        break;
    case GameState::PLAY:
        RenderText("Play State", 300, 250, { 255, 255, 255, 255 });
        RenderText("Press ESC to Pause", 250, 300, { 255, 255, 255, 255 });
        break;
    case GameState::PAUSE:
        RenderText("Paused", 300, 250, { 255, 255, 255, 255 });
        RenderText("Press ESC to Resume", 250, 300, { 255, 255, 255, 255 });
        break;
    case GameState::GAME_OVER:
        RenderText("Game Over", 300, 250, { 255, 255, 255, 255 });
        RenderText("Press ENTER to Restart", 250, 300, { 255, 255, 255, 255 });
        break;
    }

    SDL_RenderPresent(renderer);
}

void StateManager::HandleEvents(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (currentState) {
        case GameState::MAIN_MENU:
            if (event.key.keysym.sym == SDLK_RETURN) {
                ChangeState(GameState::PLAY);
            }
            break;
        case GameState::PLAY:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                ChangeState(GameState::PAUSE);
            }
            break;
        case GameState::PAUSE:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                ChangeState(GameState::PLAY);
            }
            break;
        case GameState::GAME_OVER:
            if (event.key.keysym.sym == SDLK_RETURN) {
                ChangeState(GameState::MAIN_MENU);
            }
            break;
        }
    }
}

void StateManager::RenderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}