#include "game.h"

void WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b, int xPos, int yPos);

void 
GameInit(GameMemory* memory) {
    assert(sizeof(GameState) <= (memory->size));
    
    GameState* state = (GameState*) memory;
    
    state->r = 255;
    state->g = 255;
    state->b = 255;
    state->x = 0;
    state->y = 0;
}

void 
GameUpdate(GameMemory* memory, GameInput input, SoundBuffer* soundBuffer, f32 dt) {
    // cast memory to state
    // the engine to provides a fixed memory region for the game to operate in
    GameState* state = (GameState*)memory;
    
    double growth = 0.1 * dt;
    double moveSpeed = 0.01 * dt;

    soundBuffer->note = 261;

    if(input.Alpha1.isDown) {
        state->r += growth;
        state->r = fmod(state->r, 255);
    }
    
    if(input.Alpha2.isDown) {
        state->g += growth;
        state->g = fmod(state->g, 255);
    }
    
    if(input.Alpha3.isDown) {
        state->b += growth;
        state->b = fmod(state->b, 255);
    }
    
    
    if(input.Up.isDown) {
        state->y += moveSpeed;

        soundBuffer->note = 293;
    } else if(input.Down.isDown) {
        state->y -= moveSpeed;

        soundBuffer->note = 246;
    }
    
    if(input.Left.isDown) {
        state->x -= moveSpeed;
    } else if(input.Right.isDown) {
        state->x += moveSpeed;
    }
}

void 
GameRender(GameMemory* memory, GraphicsBuffer* graphicsBuffer) {
    GameState* state = (GameState*) memory;
    // TODO I can see how... knowing the position and desired color of things you'd be able to translate that into screen space
    WriteColorToBuffer(graphicsBuffer, state->r, state->g, state->b, round(state->x), round(state->y));
}

void 
WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b, int32 xPos, int32 yPos) {
    u8* row = buffer->data; // current row
    int32 rowSize = buffer->width*buffer->bytesPerPixel; // 2D array of pixels, mapped into a 1D array (column x is (width*x) in memory)
    
    for(int32 y = 0; y < buffer->height; y++) {
        u8* pixel = (u8*)row;
        
        for(int32 x = 0; x < buffer->width; x++) {
            int32 drawRed = 0;
            int32 drawGreen = 0;
            int32 drawBlue = 0;
            
            if(x == xPos && y == yPos) {
                drawRed = r;
                drawGreen = g;
                drawBlue = b;
            }
            
            // blue
            *pixel = drawBlue;
            pixel++;
            
            // green
            *pixel = drawGreen;
            pixel++;
            
            // red
            *pixel = drawRed;
            pixel++;
            
            // alpha
            *pixel = 0;
            pixel++;
        }
        
        // move to next row
        row += rowSize;
    }
}

// void WriteSoundToBuffer(SoundBuffer* buffer, float note) {
//     int volume = 20;
//     int noteConverted = note / buffer->blockAlign;
//     // copy in data
//     for(DWORD i = 0; i < block1BytesToWrite; i++) {
//         f32 pos = noteConverted / (f32)buffer->sampleRate * (f32)i;
        
//         // convert to radians
//         f32 decimal = (pos - floor(pos)); // TODO don't understand why only using the decimal
//         f32 radians = decimal * 2 * PI;
//         f32 tone = sin(radians);

//         // amplitude of the wave == volume
//         byteBlock1[i] = volume * tone;
//     }
// }