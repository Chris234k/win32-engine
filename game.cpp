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
GameUpdate(GameMemory* memory, GameInput input, float dt) {
    // cast memory to state
    // the engine to provides a fixed memory region for the game to operate in
    GameState* state = (GameState*)memory;
    
    double growth = 0.1 * dt;
    double moveSpeed = 0.01 * dt;
    
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
    } else if(input.Down.isDown) {
        state->y -= moveSpeed;
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
WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b, int xPos, int yPos) {
    u8* row = buffer->data; // current row
    int rowSize = buffer->width*buffer->bytesPerPixel; // 2D array of pixels, mapped into a 1D array (column x is (width*x) in memory)
    
    for(int y = 0; y < buffer->height; y++) {
        u8* pixel = (u8*)row;
        
        for(int x = 0; x < buffer->width; x++) {
            int drawRed = 0;
            int drawGreen = 0;
            int drawBlue = 0;
            
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