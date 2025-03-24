#include "GameStateManager.h"
#include "SDL2/SDL.h"

GameStateManager::GameStateManager() {
    // You can optionally initialize the manager with a starting state.
}

GameStateManager::~GameStateManager() {
    while (!stateStack.empty()) {
        delete stateStack.top();  // Clean up the allocated states
        stateStack.pop();
    }
}

void GameStateManager::PushState(GameState* state) {
    if (!stateStack.empty()) {
        stateStack.top()->Exit();  // Exit the current state
    }
    stateStack.push(state);  // Push the new state onto the stack
    stateStack.top()->Enter();  // Call Enter on the new state
}

void GameStateManager::PopState() {
    if (!stateStack.empty()) {
        stateStack.top()->Exit();  // Exit the current state
        delete stateStack.top();   // Clean up the state
        stateStack.pop();          // Pop the state from the stack
    }

    if (!stateStack.empty()) {
        stateStack.top()->Enter();  // Re-enter the next state (if any)
    }
}

void GameStateManager::ChangeState(GameState* state) {
    if (!stateStack.empty()) {
        stateStack.top()->Exit();  // Exit the current state
        delete stateStack.top();   // Clean up the state
        stateStack.pop();          // Pop the current state
    }

    PushState(state);  // Push and enter the new state
}

void GameStateManager::HandleEvents() {
    if (!stateStack.empty()) {
        stateStack.top()->Update();  // Handle events or updates for the current state
    }
}

void GameStateManager::Update() {
    if (!stateStack.empty()) {
        // You can add updates here, such as updating animations, physics, etc.
    }
}

void GameStateManager::Render(SDL_Renderer* renderer) {
    if (!stateStack.empty()) {
        stateStack.top()->Render(renderer);  // Render the current state
    }
}
