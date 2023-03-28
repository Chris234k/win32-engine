#include "game.h"

void DrawColorToBuffer(GraphicsBuffer* buffer, Color32 color);
void DrawPlayer(GraphicsBuffer* buffer, int32 xPos, int32 yPos, Color32 color);
void DrawBorder(GraphicsBuffer* buffer, Color32 color);

void WriteSound(f32 note, SoundBuffer* soundBuffer);


int32 clamp(int32 current, int32 min, int32 max) {
    if(current > max) {
        return max;
    } else if(current < min) {
        return min;
    }
    
    return current;
}

void 
GameInit(GameMemory* memory) {
    assert(sizeof(GameState) <= (memory->permanentSize));
    
    GameState* state = (GameState*) memory->permanent;
    
    state->backgroundColor.packed = 0xFF000000;
    
    state->playerColor.packed = 0xFF0000FF;
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
    
    // TODO utility function to request current window dimensions?
    const f32 BUFFER_SIZE = 512;
    const f32 SCREEN_SIZE = 1024;
    const int HALF_PLAYER_SIZE = 25;
    f32 ratio = BUFFER_SIZE / SCREEN_SIZE;

    state->playerX = input.mouseX * ratio; // mouse is in screen coordinates
    state->playerY = input.mouseY * ratio;
    
    state->playerX -= HALF_PLAYER_SIZE;
    state->playerY -= HALF_PLAYER_SIZE;
    
    WriteSound(state->note, soundBuffer);
}

void 
GameRender(GameMemory* memory, GraphicsBuffer* graphicsBuffer) {
    GameState* state = (GameState*) memory->permanent;
    // TODO I can see how... knowing the position and desired color of things you'd be able to translate that into screen space

    DrawColorToBuffer(graphicsBuffer, state->backgroundColor);
    DrawPlayer(graphicsBuffer, state->playerX, state->playerY, state->playerColor);
    DrawBorder(graphicsBuffer, state->playerColor);
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
        row += buffer->bytesPerRow;
    }
}

void
DrawPlayer(GraphicsBuffer* buffer, int32 xPos, int32 yPos, Color32 color) {
    const int PLAYER_SIZE = 50;
    
    int32 xMax = xPos+PLAYER_SIZE;
    int32 yMax = yPos+PLAYER_SIZE;
    
    // max pos bounds
    xMax = clamp(xMax, 0, buffer->width);
    yMax = clamp(yMax, 0, buffer->height);
    
    // min pos bounds
    xPos = max(0, xPos);
    yPos = max(0, yPos);
    
    
    // pixels to render
    int32 xPixels = xMax - xPos;
    int32 yPixels = yMax - yPos;
    
    u32 xOffset = (xPos*buffer->bytesPerPixel);
    u32 yOffset = (yPos*buffer->bytesPerPixel);
    
    u8* row = buffer->data;
    row += (buffer->bytesPerRow*yPos) + xOffset;
    
    for(int32 y = 0; y < yPixels; y++) {
        u32* pixel = (u32*)row;
        
        for(int32 x = 0; x < xPixels; x++) {
            *pixel = color.packed;
            pixel++;
        }
        
        row += buffer->bytesPerRow;
    }
}

void
DrawBorder(GraphicsBuffer* buffer, Color32 color) {
    // vertical
    u32* left = (u32*)buffer->data;
    u32* right = (u32*)(buffer->data + (buffer->bytesPerPixel*(buffer->width-1)));
    
    for(int32 y = 0; y < buffer->height; y++) {
        *left = color.packed;
        *right = color.packed;
        
        left += (buffer->bytesPerRow / buffer->bytesPerPixel);
        right += (buffer->bytesPerRow / buffer->bytesPerPixel);
    }
    
    // horizontal
    u32* bot = (u32*)buffer->data;
    u32* top = (u32*)(buffer->data + buffer->bytesPerRow * (buffer->height-1));
    
    for(int32 x = 0; x < buffer->width; x++) {
        *bot = color.packed;
        *top = color.packed;
        
        bot++;
        top++;
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