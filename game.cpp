#include "game.h"

void WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b);

void 
Update(GameMemory* memory, GraphicsBuffer* graphicsBuffer, GameInput input, float dt) {
    // cast memory to state
    // the engine to provides a fixed memory region for the game to operate in
    GameState* state = (GameState*)memory;
    
    double growth = 0.1 * dt;
    
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
    
    WriteColorToBuffer(graphicsBuffer, state->r, state->g, state->b);
}


void 
WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b) {
    u8* row = (u8 *)buffer->data; // current row
    int rowSize = buffer->width*buffer->bytesPerPixel; // 2D array of pixels, mapped into a 1D array (column x is (width*x) in memory)
    
    for(int y = 0; y < buffer->height; y++) {
        u8* pixel = (u8*)row;
        
        for(int x = 0; x < buffer->width; x++) {
            // blue
            *pixel = b;
            pixel++;
            
            // green
            *pixel = g;
            pixel++;
            
            // red
            *pixel = r;
            pixel++;
            
            // alpha
            *pixel = 0;
            pixel++;
        }
        
        // move to next row
        row += rowSize;
    }
}