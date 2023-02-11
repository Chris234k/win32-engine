#ifndef UNICODE
#define UNICODE
#endif

// os includes
#include <windows.h>
#include <wingdi.h>   // graphics
#include <cguid.h>
#include <dsound.h>   // sound


#include <cstdio>    // printf
#include "cstdint"   // uint32_t
#include "math.h"    // fmod

// write to the null pointer to halt the program
#define assert(expression) if(!(expression)) (*(int *) 0 = 0) 

// typedefs
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float f32; // TODO this is platform dependent? compiler dependent?
typedef double f64;

// game includes
// must come after typedefs
#include "game.h"
#include "game.cpp"

// game has a similar structure, but the game cannot have any Windows dependencies (i.e. BITMAPINFO)
struct Win32GraphicsBuffer {
    int width, height;
    int bytesPerPixel;
    BITMAPINFO bitmapInfo;
    u8* data;
};

struct Win32SoundBuffer {
    u32 sampleIndex;
    u32 latencySampleCount;
    
    u16 blockAlign;
    u32 samplesPerSecond;
    u8 bytesPerSample;
    
    u32 bufferSize;
    LPDIRECTSOUNDBUFFER secondary;
    f32* data;
};

// forward declarations
LRESULT CALLBACK Win32_WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Win32_CreateGraphicsBuffer(Win32GraphicsBuffer* buffer, int width, int height);
void Win32_DrawBufferToWindow(Win32GraphicsBuffer* buffer, HWND windowHandle, RECT clientRect);
bool Win32_CreateSoundBuffer(Win32SoundBuffer* buffer, HWND windowHandle);
void Win32_WriteSoundToDevice(DWORD startingByte, DWORD numBytesToWrite, Win32SoundBuffer* buffer, f32 note);

// consts
const int BYTES_PER_PIXEL = 4;

const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 512;

const int BUFFER_WIDTH = 16;
const int BUFFER_HEIGHT = 16;

const float PI = 3.14159265358;

// globals
bool IsGameRunning = true;
Win32GraphicsBuffer graphicsBuffer;
Win32SoundBuffer soundBuffer;
GameInput gameInput;

int
main() {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    WNDCLASS window = {};
    
    window.style = CS_HREDRAW | CS_VREDRAW; // redraw when resized
    window.lpfnWndProc = Win32_WindowProc;
    window.hInstance = hInstance;
    window.lpszClassName = CLASS_NAME;
    
    RegisterClass(&window);
    
    HWND windowHandle = CreateWindow(
        CLASS_NAME,
        L"Learn to Program Windows",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        
        CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT,
        
        NULL,
        NULL,
        hInstance,
        NULL
    );
    
    if(windowHandle == NULL) {
        return 0;
    }
    ShowWindow(windowHandle, SW_SHOWNORMAL);
    
    
    RECT rect;
    GetClientRect(windowHandle, &rect);
    // engine allocations
    Win32_CreateGraphicsBuffer(&graphicsBuffer, BUFFER_WIDTH, BUFFER_HEIGHT);
    if(!Win32_CreateSoundBuffer(&soundBuffer, windowHandle)) {
        return 0;
    }
    
    if(FAILED(soundBuffer.secondary->Play(0, 0, DSBPLAY_LOOPING))) {
        printf("Failed to play DirectSound secondary buffer\n");
        return false;
    }
    
    // game allocations
    int gameMemorySize = 1024 * 1024 * 1; 
    GameMemory* gameMemory = (GameMemory*)VirtualAlloc(NULL, gameMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    gameMemory->size = gameMemorySize;
    
    GraphicsBuffer gameGraphicsBuffer = {};
    gameInput = {};
    
    // pass along revelant data to the game
    gameGraphicsBuffer.width           = graphicsBuffer.width;
    gameGraphicsBuffer.height          = graphicsBuffer.height;
    gameGraphicsBuffer.bytesPerPixel   = graphicsBuffer.bytesPerPixel;
    gameGraphicsBuffer.data            = (u8*) graphicsBuffer.data; // pointer to the engine's graphics buffer data. Game writes to it, and engine knows how to display it
    
    SoundBuffer gameSoundBuffer = {};

    GameInit(gameMemory);
    
    MSG msg = {};
    
    // timing
    double time = 0.0;
    double max_dt = 1.0 / 60.0;
    LARGE_INTEGER currentTime;
    LARGE_INTEGER frequency; // ticks per second
    
    QueryPerformanceCounter(&currentTime);
    QueryPerformanceFrequency(&frequency); // fixed at system boot, need only query once
    
    while(IsGameRunning) {
        LARGE_INTEGER newTime;
        QueryPerformanceCounter(&newTime);
        
        LARGE_INTEGER elapsedMicroseconds;
        elapsedMicroseconds.QuadPart = newTime.QuadPart - currentTime.QuadPart;
        elapsedMicroseconds.QuadPart *= 1000000; // avoid loss of precision. convert to microseconds
        elapsedMicroseconds.QuadPart /= frequency.QuadPart; // elapsed = ticks / ticks per second
        
        double frameTime = (double)elapsedMicroseconds.QuadPart / 1000; // convert to milliseconds
        currentTime = newTime;
        
        // [input]
        while(PeekMessage(&msg, windowHandle, 0, 0, PM_REMOVE)) {
            WPARAM wParam = msg.wParam;
            switch(msg.message) {
                
                case WM_KEYDOWN:
                case WM_KEYUP:
                {
                    WORD keyFlags = HIWORD(msg.lParam);
                    
                    // https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#keystroke-message-flags
                    bool wasDown = (keyFlags & KF_REPEAT) == KF_REPEAT;
                    bool isDown = ((keyFlags & KF_UP) == KF_UP) == 0;
                    
                    if(wasDown != isDown) { // state has changed
                        switch(wParam) {
                            case '1':
                                gameInput.Alpha1.isDown = isDown;
                                break;
                                
                            case '2':
                                gameInput.Alpha2.isDown = isDown;
                                break;
                                
                            case '3':
                                gameInput.Alpha3.isDown = isDown;
                                break;
                                
                            case 'W':
                            case VK_UP:
                                gameInput.Up.isDown = isDown;
                                break;
                                
                            case 'S':
                            case VK_DOWN:
                                gameInput.Down.isDown = isDown;
                                break;
                                
                            case 'A':
                            case VK_LEFT:
                                gameInput.Left.isDown = isDown;
                                break;
                                
                            case 'D':
                            case VK_RIGHT:
                                gameInput.Right.isDown = isDown;
                                break;
                        }
                    }
                    break;
                }
                    
                // passthrough to windowproc
                default:
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    break;
            }
        }
        
        // [update]
        // https://new.gafferongames.com/post/fix_your_timestep/
        while(frameTime > 0.0) {
            float deltaTime = frameTime;
            
            if(deltaTime > max_dt) { // maximum step is max_dt
                deltaTime = max_dt;
            }
            
            GameUpdate(gameMemory, gameInput, &gameSoundBuffer, deltaTime);
            
            frameTime -= deltaTime;
            time += deltaTime;
        }
        
        
        DWORD playCursor;
        soundBuffer.secondary->GetCurrentPosition(&playCursor, 0);
        
        // cursor is greater unless circular buffer wraps around
        u32 startingByte = (soundBuffer.sampleIndex*soundBuffer.bytesPerSample) % soundBuffer.bufferSize;
        u32 targetCursorPosition = (playCursor + soundBuffer.latencySampleCount) % soundBuffer.bufferSize;
        u32 bytes;
        if(startingByte == targetCursorPosition) {
            bytes = soundBuffer.bufferSize;
        } else if(startingByte > targetCursorPosition) {
            // |--c----b--|
            bytes = (soundBuffer.bufferSize - startingByte) + targetCursorPosition;
        } else {
            // |--b----c--|
            bytes = (targetCursorPosition - startingByte);
        }
        
        Win32_WriteSoundToDevice(startingByte, bytes, &soundBuffer, gameSoundBuffer.note);
        
        // [render]
        // queue WM_PAINT, forces the entire window to redraw
        GameRender(gameMemory, &gameGraphicsBuffer);
        InvalidateRect(windowHandle, &rect, true);
    }
    
    VirtualFree(graphicsBuffer.data, 0, MEM_RELEASE);
    VirtualFree(soundBuffer.data, 0, MEM_RELEASE);
    
    return 0;
}

LRESULT CALLBACK
Win32_WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_KEYDOWN:
        case WM_KEYUP:
            printf("ERROR: input in windowproc"); // PeekMessage in main loop must handle input
            break;
            
        case WM_DESTROY:
            IsGameRunning = false;
            PostQuitMessage(0);
            break;
            
        case WM_PAINT:
            {
                RECT rect;
                GetClientRect(windowHandle, &rect);
                Win32_DrawBufferToWindow(&graphicsBuffer, windowHandle, rect);
            }
            return 0;
    }
    
    return DefWindowProc(windowHandle, uMsg, wParam, lParam);
}

void 
Win32_DrawBufferToWindow(Win32GraphicsBuffer* buffer, HWND windowHandle, RECT windowRect) {
    PAINTSTRUCT ps;
    HDC deviceContext = BeginPaint(windowHandle, &ps);
    
    int width = (windowRect.right - windowRect.left);
    int height = (windowRect.bottom - windowRect.top);
    
    // stretch the graphics buffer pixels onto the window
    StretchDIBits(deviceContext,
        windowRect.left, windowRect.top,
        width, height,
        
        0, 0, 
        buffer->width, buffer->height,
        
        buffer->data,
        &buffer->bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY
    );
    
    EndPaint(windowHandle, &ps);
}

void 
Win32_CreateGraphicsBuffer(Win32GraphicsBuffer* buffer, int width, int height) {
    BITMAPINFO bitmapInfo = {};
    memset(&bitmapInfo, 0, sizeof(bitmapInfo)); // TODO necessary?
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = height;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 8*BYTES_PER_PIXEL;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    buffer->width = width;
    buffer->height = height;
    buffer->bytesPerPixel = BYTES_PER_PIXEL;
    buffer->bitmapInfo = bitmapInfo;
    buffer->data = (u8*) VirtualAlloc(NULL, width*height*BYTES_PER_PIXEL, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); // texture size * 4 bytes per pixel (RGBA)
}

bool 
Win32_CreateSoundBuffer(Win32SoundBuffer* buffer, HWND windowHandle) {
        // DirectSound setup
    LPDIRECTSOUND directSound;
    // first parameter NULL = default sound device
    if(FAILED(DirectSoundCreate(NULL, &directSound, NULL))) {
        printf("Failed to create DirectSound\n");
        return false;
    } 
    
    if(FAILED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY))) {
        printf("Failed to set DirectSound cooperative level\n");
        return false;
    }
    
    // create sound buffer
    LPDIRECTSOUNDBUFFER secondaryBuffer;
    
    // setup format
    // 48 kHz 16-bit stereo
    u32 samplesPerSecond = 48000;
    u8 channels = 2;
    u32 bitsPerSample = sizeof(u16) * 8;
    u32 bytesPerSample = bitsPerSample / 8;
    
    const f32 LATENCY_MS = 10.0 / 1000.0;
    
    // https://learn.microsoft.com/en-us/windows/win32/multimedia/using-the-waveformatex-structure
    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag       = WAVE_FORMAT_PCM;
    waveFormat.nChannels        = channels;
    waveFormat.nSamplesPerSec   = samplesPerSecond;
    waveFormat.wBitsPerSample   = bitsPerSample;
    waveFormat.nBlockAlign      = bytesPerSample * channels;
    waveFormat.nAvgBytesPerSec  = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize           = 0;
    
    // setup actual buffer
    DSBUFFERDESC soundBufferDesc = {};
    soundBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    soundBufferDesc.dwBufferBytes = samplesPerSecond * bytesPerSample;
    soundBufferDesc.dwReserved = 0;
    soundBufferDesc.lpwfxFormat = &waveFormat;
    
    if(FAILED(directSound->CreateSoundBuffer(&soundBufferDesc, &secondaryBuffer, NULL))) {
        printf("Failed to create DirectSound buffer\n");
        return false;
    }
    
    buffer->blockAlign = waveFormat.nBlockAlign;
    buffer->samplesPerSecond = waveFormat.nSamplesPerSec;
    buffer->bytesPerSample = bytesPerSample;
    buffer->latencySampleCount = buffer->samplesPerSecond * LATENCY_MS;
    buffer->bufferSize = soundBufferDesc.dwBufferBytes;
    buffer->secondary = secondaryBuffer;
    buffer->data = (f32*)VirtualAlloc(NULL, soundBufferDesc.dwBufferBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    return true;
}

void
Win32_WriteSoundToDevice(DWORD startingByte, DWORD numBytesToWrite, Win32SoundBuffer* buffer, f32 note) {
    assert(numBytesToWrite > 0);
    
    u8 *byteBlock1, *byteBlock2;
    DWORD numBlock1Bytes, numBlock2Bytes;
    
    if(FAILED(buffer->secondary->Lock(startingByte, numBytesToWrite, (void**)&byteBlock1, &numBlock1Bytes, (void**)&byteBlock2, &numBlock2Bytes, 0))) {
        printf("Failed to lock DirectSound secondary buffer.\n");
        return;
    }
    
    const int VOLUME = 20;
    f32 noteSample = note / buffer->blockAlign / (f32)buffer->samplesPerSecond;

    DWORD block1SampleCount = numBlock1Bytes / buffer->bytesPerSample;
    DWORD block2SampleCount = numBlock2Bytes / buffer->bytesPerSample;

    // copy in data
    for(DWORD i = 0; i < block1SampleCount; i++) {
        f32 pos = noteSample * buffer->sampleIndex;
        buffer->sampleIndex++;
        
        // convert to radians
        f32 remainder = (pos - floor(pos)); // [0, 1]
        f32 radians = remainder * 2 * PI;   // [0, 2PI]
        f32 tone = sin(radians);            // [-1, 1]

        // amplitude of the wave == volume
        byteBlock1[i] = VOLUME * tone;
    }
    
    // sound buffer is circular, block 2 describes a wrap around
    // p = play cursor
    // w = write cursor
    // |------p---------w--|
    // |2-----p---------1--|
    if(block2SampleCount > 0) {
        for(DWORD i = 0; i < block2SampleCount; i++) {
            f32 pos = noteSample * buffer->sampleIndex;
            buffer->sampleIndex++;
            
            // convert to radians
            f32 remainder = (pos - floor(pos)); // [0, 1]
            f32 radians = remainder * 2 * PI;   // [0, 2PI]
            f32 tone = sin(radians);            // [-1, 1]

            // amplitude of the wave == volume
            byteBlock2[i] = VOLUME * tone;
        }
    }
    
    buffer->sampleIndex %= buffer->bufferSize;
    
    if(buffer->secondary->Unlock(byteBlock1, numBlock1Bytes, byteBlock2, numBlock2Bytes)) {
        printf("Failed to unlock DirectSound secondary buffer\n");
        return;
    }
}