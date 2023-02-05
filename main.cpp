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

// game includes
// must come after typedefs
#include "game.h"
#include "game.cpp"

// game has a similar structure, but the game cannot have any Windows dependencies (i.e. BITMAPINFO)
struct Win32GraphicsBuffer {
    int width, height;
    int bytesPerPixel;
    BITMAPINFO bitmapInfo;
    void* data;
};

// forward declarations
LRESULT CALLBACK WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateGraphicsBuffer(Win32GraphicsBuffer* buffer, int width, int height);
void DrawBufferToWindow(Win32GraphicsBuffer* buffer, HWND windowHandle, RECT clientRect);

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
GameInput gameInput;

int
main() {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    WNDCLASS window = {};
    
    window.style = CS_HREDRAW | CS_VREDRAW; // redraw when resized
    window.lpfnWndProc = WindowProc;
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
    CreateGraphicsBuffer(&graphicsBuffer, BUFFER_WIDTH, BUFFER_HEIGHT);
    
    // DirectSound setup
    LPDIRECTSOUND directSound;
    // first parameter NULL = default sound device
    if(FAILED(DirectSoundCreate(NULL, &directSound, NULL))) {
        printf("Failed to create DirectSound");
        return 0;
    } 
    
    if(FAILED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY))) {
        printf("Failed to set DirectSound cooperative level");
        return 0;
    }
    
    // create sound buffer
    LPDIRECTSOUNDBUFFER secondaryBuffer;
    
    // setup format
    // 44.1 kHz 16-bit stereo
    u32 sampleRate = 44100L;
    u8 bitsPerSample = 16;
    u8 channels = 2;
    // https://learn.microsoft.com/en-us/windows/win32/multimedia/using-the-waveformatex-structure
    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag       = WAVE_FORMAT_PCM;
    waveFormat.nChannels        = channels;
    waveFormat.nSamplesPerSec   = sampleRate;
    waveFormat.wBitsPerSample   = bitsPerSample;
    waveFormat.nBlockAlign      = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
    waveFormat.nAvgBytesPerSec  = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize           = 0;
    
    // setup actual buffer
    DSBUFFERDESC soundBufferDesc = {};
    soundBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    soundBufferDesc.dwBufferBytes = sampleRate * bitsPerSample / 8 * channels * 1;
    soundBufferDesc.dwReserved = 0;
    soundBufferDesc.lpwfxFormat = &waveFormat;
    
    if(FAILED(directSound->CreateSoundBuffer(&soundBufferDesc, &secondaryBuffer, NULL))) {
        printf("Failed to create DirectSound buffer");
        return 0;
    }
    
    u8 *byteBlock1, *byteBlock2;
    DWORD block1BytesToWrite, block2BytesToWrite;
    
    if(FAILED(secondaryBuffer->Lock(0, 0, (void**)&byteBlock1, &block1BytesToWrite, (void**)&byteBlock2, &block2BytesToWrite, DSBLOCK_ENTIREBUFFER))) {
        printf("Failed to lock DirectSound secondary buffer");
    }
    
    assert(block2BytesToWrite == 0);
    
    int volume = 30;
    int middleC = 261 / waveFormat.nBlockAlign;
    // copy in data
    for(DWORD i = 0; i < block1BytesToWrite; i++) {
        float pos = middleC / (float)sampleRate * (float)i;
        
        // convert to radians
        float remainder = (pos - floor(pos));
        float radians = remainder * 2 * PI;
        float tone = sin(radians);

        // change multiplier to change amplitude of wave (aka volume)
        byteBlock1[i] = volume+(tone*volume);
    }
    
    if(secondaryBuffer->Unlock(byteBlock1, block1BytesToWrite, byteBlock2, block2BytesToWrite)) {
        printf("Failed to unlock DirectSound secondary buffer");
    }
    
    if(FAILED(secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING))) {
        printf("Failed to play DirectSound secondary buffer");
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
    gameGraphicsBuffer.data            = graphicsBuffer.data; // pointer to the engine's graphics buffer data. Game writes to it, and engine knows how to display it
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
            
            GameUpdate(gameMemory, gameInput, deltaTime);
            
            frameTime -= deltaTime;
            time += deltaTime;
        }
        
        // [render]
        // queue WM_PAINT, forces the entire window to redraw
        GameRender(gameMemory, &gameGraphicsBuffer);
        InvalidateRect(windowHandle, &rect, true);
    }
    
    VirtualFree(graphicsBuffer.data, 0, MEM_RELEASE);
    
    return 0;
}

LRESULT CALLBACK
WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
                DrawBufferToWindow(&graphicsBuffer, windowHandle, rect);
            }
            return 0;
    }
    
    return DefWindowProc(windowHandle, uMsg, wParam, lParam);
}

void 
DrawBufferToWindow(Win32GraphicsBuffer* buffer, HWND windowHandle, RECT windowRect) {
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
CreateGraphicsBuffer(Win32GraphicsBuffer* buffer, int width, int height) {
    BITMAPINFO bitmapInfo = {};
    memset(&bitmapInfo, 0, sizeof(bitmapInfo));
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
    buffer->data = VirtualAlloc(NULL, width*height*BYTES_PER_PIXEL, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); // texture size * 4 bytes per pixel (RGBA)
}