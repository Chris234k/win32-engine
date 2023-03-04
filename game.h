#ifndef GAME_H
#define GAME_H

typedef union {
    u32 packed; // packed bgra color union
    
    struct {
        u8 blue;
        u8 green;
        u8 red;
        u8 alpha;
    };
    
} Color32;



struct GameMemory {
    int64 permanentSize;
    void* permanent;
    
    int64 transientSize;
    void* transient;
};

struct GraphicsBuffer {
    int32 width, height;
    int32 bytesPerPixel;
    int32 bytesPerRow;
    u8* data;
};

struct SoundBuffer {
    int samplesPerSecond;
    int numSamplesToWrite; // engine requests the number of samples for the game to write
    int16* samples;
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
    Color32 backgroundColor;
    
    Color32 playerColor;
    int32 playerX, playerY;
    
    f32 note;
};

void GameInit(GameMemory* memory);
void GameUpdate(GameMemory* memory, GameInput input, SoundBuffer* soundBuffer, f32 dt);
void GameRender(GameMemory* memory, GraphicsBuffer* graphicBuffer);

#endif