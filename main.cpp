#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <wingdi.h>
#include <cstdio>
#include "cstdint"   // uint32_t
#include "math.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// types
struct GraphicsBuffer {
    int width, height;
    BITMAPINFO bitmapInfo;
    HBITMAP bitmapHandle;
    void* data;
};

// this is the game level input, "actions"
// these map to real key presses at the engine level
struct GameAction {
    bool isDown;
};

struct GameInputState {
    GameAction red;
    GameAction green;
    GameAction blue;
};

// forward declarations
LRESULT CALLBACK WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam);
GraphicsBuffer CreateGraphicsBuffer(int width, int height);
void DrawBufferToWindow(GraphicsBuffer* buffer, HWND windowHandle, RECT clientRect);
void WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b);
void Update(float dt);


// TODO C++ consts
int BYTES_PER_PIXEL = 4;

int SCREEN_WIDTH = 512;
int SCREEN_HEIGHT = 512;

int BUFFER_WIDTH = 16;
int BUFFER_HEIGHT = 16;

// globals
bool IsGameRunning = true;
GraphicsBuffer graphicsBuffer;
GameInputState input;
u8 windowRed, windowGreen, windowBlue;

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
    
    graphicsBuffer = CreateGraphicsBuffer(BUFFER_WIDTH, BUFFER_HEIGHT);
    input = {};
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
                                input.red.isDown = isDown;
                                break;
                                
                            case '2':
                                input.green.isDown = isDown;
                                break;
                                
                            case '3':
                                input.blue.isDown = isDown;
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
            float deltaTime = std::min(frameTime, max_dt); // maximum step is max_dt
            Update(deltaTime);
            
            frameTime -= deltaTime;
            time += deltaTime;
        }
        
        // [render]
        // updates buffer, drawn at next WM_PAINT
        WriteColorToBuffer(&graphicsBuffer, windowRed, windowGreen, windowBlue);
        // queue WM_PAINT
        InvalidateRect(windowHandle, &rect, true); // TODO this forces the entire window to redraw. Necessary?
    }
    
    return 0;
}

float r, g, b;
void Update(float dt) {
    double growth = 0.1 * dt;
    
    if(input.red.isDown) {
        r += growth;
        r = fmod(r, 255);    
        
        windowRed = round(r);
    }
    
    if(input.green.isDown) {
        g += growth;
        g = fmod(g, 255);
        
        windowGreen = round(g);
    }
    
    if(input.blue.isDown) {
        b += growth;
        b = fmod(b, 255);
        
        windowBlue = round(b);
    }
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
DrawBufferToWindow(GraphicsBuffer* buffer, HWND windowHandle, RECT windowRect) {
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

GraphicsBuffer
CreateGraphicsBuffer(int width, int height) {    
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 8*BYTES_PER_PIXEL;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    GraphicsBuffer gb;
    gb.bitmapInfo = bmi;
    gb.width = width;
    gb.height = height;
    gb.data = new u32[width*height*BYTES_PER_PIXEL]; // texture size * 4 bytes per pixel (RGBA)
    
    // clear to black
    WriteColorToBuffer(&gb, 0, 0, 0);
    
    return gb;
}

void 
WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b) {
    u8* row = (u8 *)buffer->data; // current row
    int rowSize = buffer->width*BYTES_PER_PIXEL; // 2D array of pixels, mapped into a 1D array (column x is (width*x) in memory)
    
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