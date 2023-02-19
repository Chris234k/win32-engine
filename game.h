#ifndef GAME_H
#define GAME_H

struct GameMemory {
    int64 permanentSize;
    void* permanent;
    
    int64 transientSize;
    void* transient;
};

struct GraphicsBuffer {
    int32 width, height;
    int32 rowSize;
    int32 bytesPerPixel;
    u8* data;
};

struct SoundBuffer {
    f32 note;
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
    
    float note;
};


void GameInit(GameMemory* memory);
void GameUpdate(GameMemory* memory, GameInput input, SoundBuffer* soundBuffer, f32 dt);
void GameRender(GameMemory* memory, GraphicsBuffer* graphicBuffer);

#endif