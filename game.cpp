#include "game.h"

void WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b, int xPos, int yPos);
void WriteSound(f32 note, SoundBuffer* soundBuffer);

void 
GameInit(GameMemory* memory) {
    assert(sizeof(GameState) <= (memory->permanentSize));
    
    GameState* state = (GameState*) memory->permanent;
    
    state->r = 255;
    state->g = 255;
    state->b = 255;
    
    state->x = 0;
    state->y = 0;
    
    state->note = 261; // middle c to start
    
    FileContent content = FileReadAll("c:\\users\\chris\\github\\win32-engine\\input.txt");
    
    if(content.data) {
        FileWriteAll("c:\\users\\chris\\github\\win32-engine\\output.txt", content.data, content.byteCount);
        FileReleaseMemory(content.data);
    }
}

void 
GameUpdate(GameMemory* memory, GameInput input, SoundBuffer* soundBuffer, f32 dt) {
    // cast memory to state
    // the engine to provides a fixed memory region for the game to operate in
    GameState* state = (GameState*)memory->permanent;
    
    double growth = 100 * dt;
    double moveSpeed = 100 * dt;

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

        state->note += growth;
    } else if(input.Down.isDown) {
        state->y -= moveSpeed;

        state->note -= growth;
    }
    
    if(input.Left.isDown) {
        state->x -= moveSpeed;
    } else if(input.Right.isDown) {
        state->x += moveSpeed;
    }
    
    WriteSound(state->note, soundBuffer);
}

void 
GameRender(GameMemory* memory, GraphicsBuffer* graphicsBuffer) {
    GameState* state = (GameState*) memory->permanent;
    // TODO I can see how... knowing the position and desired color of things you'd be able to translate that into screen space
    WriteColorToBuffer(graphicsBuffer, state->r, state->g, state->b, round(state->x), round(state->y));
}

void 
WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b, int32 xPos, int32 yPos) {
    u8* row = buffer->data; // current row
    
    const int PLAYER_SIZE = 10;
    
    for(int32 y = 0; y < buffer->height; y++) {
        u8* pixel = (u8*)row;
        
        for(int32 x = 0; x < buffer->width; x++) {
            int32 drawRed = 0;
            int32 drawGreen = 0;
            int32 drawBlue = 0;
            
            bool xClose = abs(x-xPos) < PLAYER_SIZE;
            bool yClose = abs(y-yPos) < PLAYER_SIZE;
            
            if(xClose && yClose) {
                // draw player
                drawRed = r;
                drawGreen = g;
                drawBlue = b;
            } else if ((x == 0) || (y == 0) || (x == (buffer->width-1)) || (y == (buffer->height-1)) ) {
                // draw outline around the window
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
            
            // TODO empty (alpha?)
            *pixel = 0;
            pixel++;
        }
        
        // move to next row
        row += buffer->rowSize;
    }
}

void
WriteSound(f32 note, SoundBuffer* soundBuffer) {    
    static f32 sine;
    const f32 volume = 10000;
    
    // sine wave for the tone
    f32 period = soundBuffer->samplesPerSecond / note;
    
    int16* output = soundBuffer->samples;
    for(int i = 0; i < soundBuffer->numSamplesToWrite; i++) {
        int16 sample = (int16)(sinf(sine) * volume);
        
        // left and right channels have the same sample
        *(output++) = sample;
        *(output++) = sample;
        
        sine += 2.0f * PI * 1.0f / period;
    }
}