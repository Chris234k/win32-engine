#include "game.h"

void DrawColorToBuffer(GraphicsBuffer* buffer, Color32 color);
void DrawPlayer(GraphicsBuffer* buffer, int32 xPos, int32 yPos, Color32 color);
void DrawOutline(GraphicsBuffer* buffer, int32 thickness, Color32 color);

void WriteSound(f32 note, SoundBuffer* soundBuffer);

void 
GameInit(GameMemory* memory) {
    assert(sizeof(GameState) <= (memory->permanentSize));
    
    GameState* state = (GameState*) memory->permanent;
    
    state->backgroundColor.packed = 0xFF000000;
    state->outlineColor.packed = 0xFFFFFFFF;
    
    state->playerColor.packed = 0xFFFFFFFF;
    state->playerX = 0;
    state->playerY = 0;
    
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
    
    f64 growth = 100 * dt;
    int32 moveSpeed = 1;

    if(input.Alpha1.isDown) {
        state->playerColor.red += growth;
    }
    
    if(input.Alpha2.isDown) {
        state->playerColor.green += growth;
    }
    
    if(input.Alpha3.isDown) {
        state->playerColor.blue += growth;
    }
    
    
    if(input.Up.isDown) {
        state->playerY += moveSpeed;
        
        state->note += growth;
    } else if(input.Down.isDown) {
        state->playerY -= moveSpeed;
        
        state->note -= growth;
    }
    
    if(input.Left.isDown) {
        state->playerX -= moveSpeed;
    } else if(input.Right.isDown) {
        state->playerX += moveSpeed;
    }
    
    WriteSound(state->note, soundBuffer);
}

void 
GameRender(GameMemory* memory, GraphicsBuffer* graphicsBuffer) {
    GameState* state = (GameState*) memory->permanent;
    // TODO I can see how... knowing the position and desired color of things you'd be able to translate that into screen space
    
    DrawColorToBuffer(graphicsBuffer, state->backgroundColor);
    DrawPlayer(graphicsBuffer, state->playerX, state->playerY, state->playerColor);
    
    // TODO implement
    DrawOutline(graphicsBuffer, 1, state->outlineColor);
}

void 
DrawColorToBuffer(GraphicsBuffer* buffer, Color32 color) {
    u8* row = buffer->data; // current row
    
    for(int32 y = 0; y < buffer->height; y++) {
        u32* pixel = (u32*)row;
        
        for(int32 x = 0; x < buffer->width; x++) {
            *pixel = color.packed;
            pixel++;
        }
        
        // move to next row
        row += buffer->rowSize;
    }
}

void
DrawPlayer(GraphicsBuffer* buffer, int32 xPos, int32 yPos, Color32 color) {
    const int PLAYER_SIZE = 50;
    
    u8* row = buffer->data;
    
    // TODO bounds checks!
    u32 xOffset = (xPos*buffer->bytesPerPixel);
    u32 yOffset = (yPos*buffer->bytesPerPixel);
    
    row += (buffer->rowSize)*yOffset + xOffset;
    
    for(int32 y = 0; y < PLAYER_SIZE; y++) {
        u32* pixel = (u32*)row;
        
        for(int32 x = 0; x < PLAYER_SIZE; x++) {
            *pixel = color.packed;
            pixel++;
        }
        
        row += buffer->rowSize;
    }
}

void
DrawOutline(GraphicsBuffer* buffer, int32 thickness, Color32 color) {
    // u8* row = buffer->data;
    
    // if ((x == 0) || (y == 0) || (x == (buffer->width-1)) || (y == (buffer->height-1)) ) {
    //     // draw outline around the window
    //     drawRed = r;
    //     drawGreen = g;
    //     drawBlue = b;
    // }
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