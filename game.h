#ifndef GAME_H
#define GAME_H

struct GameMemory {};

struct GraphicsBuffer {
    int width, height;
    int bytesPerPixel;
    void* data;
};

// this is the game level input
// these map to real key presses at the engine level
struct GameKey {
    bool isDown;
};

struct GameInput {
    GameKey Alpha1;
    GameKey Alpha2;
    GameKey Alpha3;
};

struct GameState {
    float r, g, b;
};

void Update(GameMemory* gameMemory, GraphicsBuffer* grahpicsBuffer, GameInput input, float dt);

#endif