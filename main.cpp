#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <wingdi.h>
#include "cstdint"   // uint32_t

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

// forward declarations
LRESULT CALLBACK WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam);
GraphicsBuffer CreateGraphicsBuffer(int width, int height);
void DrawBufferToWindow(GraphicsBuffer* buffer, HWND windowHandle, RECT clientRect);
void WriteColorToBuffer(GraphicsBuffer* buffer, u8 r, u8 g, u8 b);


int BYTES_PER_PIXEL = 4;
bool IsGameRunning = true;

// globals
int SCREEN_WIDTH = 512;
int SCREEN_HEIGHT = 512;

int BUFFER_WIDTH = 16;
int BUFFER_HEIGHT = 16;

GraphicsBuffer graphicsBuffer;


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
    
    HWND windowHandle = CreateWindowEx(
        0, 
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
    
    graphicsBuffer = CreateGraphicsBuffer(BUFFER_WIDTH, BUFFER_HEIGHT);
    
    MSG msg = {};
    while(IsGameRunning) {
        if(GetMessage(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            break;
        }
    }
    
    return 0;
}


LRESULT CALLBACK
WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY:
            IsGameRunning = false;
            PostQuitMessage(0);
            return 0;
            
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
    
    // convert bitmap from graphics buffer into DIB format
    GetDIBits(deviceContext, buffer->bitmapHandle, 
        0, buffer->width,
        NULL,
        &buffer->bitmapInfo,
        DIB_RGB_COLORS
    );
    
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
    
    WriteColorToBuffer(&gb, 255, 0, 0);
    
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