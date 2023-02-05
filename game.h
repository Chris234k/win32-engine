#ifndef GAME_H
#define GAME_H

struct GameMemory {
    int64 size;
};

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
    GameKey Alpha1, Alpha2, Alpha3;
    GameKey Up, Down, Left, Right;
};

struct GameState {
    float r, g, b;
    float x, y;
};


void GameInit(GameMemory* memory);
void GameUpdate(GameMemory* memory, GameInput input, float dt);
void GameRender(GameMemory* memory, GraphicsBuffer* graphicBuffer);

#endif