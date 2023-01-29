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
    BITMAPINFO* bitmapInfo;
    HBITMAP bitmapHandle;
    void* data;
};

// forward declarations
LRESULT CALLBACK WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam);
GraphicsBuffer CreateGraphicsBuffer(int width, int height);
void DrawWindow(HWND windowHandle, RECT clientRect);


// globals
int SCREEN_WIDTH = 1024;
int SCREEN_HEIGHT = 1024;

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
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}


LRESULT CALLBACK
WindowProc(HWND windowHandle, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        case WM_PAINT:
            {
                RECT rect;
                GetClientRect(windowHandle, &rect);
                DrawWindow(windowHandle, rect);
            }
            return 0;
    }
    
    return DefWindowProc(windowHandle, uMsg, wParam, lParam);
}

void 
DrawWindow(HWND windowHandle, RECT windowRect) {
    PAINTSTRUCT ps;
    HDC deviceContext = BeginPaint(windowHandle, &ps);
    
    // convert bitmap from graphics buffer into DIB format
    GetDIBits(deviceContext, graphicsBuffer.bitmapHandle, 
        0, graphicsBuffer.width,
        NULL,
        graphicsBuffer.bitmapInfo,
        DIB_RGB_COLORS
    );
    
    int width = (windowRect.right - windowRect.left);
    int height = (windowRect.bottom - windowRect.top);
    
    // stretch the graphics buffer pixels onto the window
    StretchDIBits(deviceContext,
        windowRect.left, windowRect.top,
        width, height,
        
        0, 0, 
        graphicsBuffer.width, graphicsBuffer.height,
        
        graphicsBuffer.data,
        graphicsBuffer.bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY
    );
    
    EndPaint(windowHandle, &ps);
}

GraphicsBuffer CreateGraphicsBuffer(int width, int height) {    
    int pixelBytes = 4;
    
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 8*pixelBytes;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    GraphicsBuffer gb;
    *gb.bitmapInfo = bmi;
    gb.width = width;
    gb.height = height;
    gb.data = new u32[width*height*pixelBytes]; // texture size * 4 bytes per pixel (RGBA)
    
    u8* row = (u8 *)gb.data; // current row
    int rowSize = width*pixelBytes; // 2D array of pixels, mapped into a 1D array (column x is (width*x) in memory)
    
    for(int y = 0; y < height; y++) {
        u8* pixel = (u8*)row;
        
        for(int x = 0; x < width; x++) {
            // blue
            *pixel = 0;
            pixel++;
            
            // green
            *pixel = 0;
            pixel++;
            
            // red
            *pixel = 255;
            pixel++;
            
            // alpha
            *pixel = 0;
            pixel++;
        }
        
        // move to next row
        row += rowSize;
    }
    
    return gb;
}