#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define NUM_SNOWFLAKES 300 // Number of snowflakes
#define TIMER_ID 1
#define FPS 30

typedef struct {
    float x;
    float y;
    float speedY;
    float speedX;
    int radius;
} Snowflake;

Snowflake snow[NUM_SNOWFLAKES];
int screenWidth, screenHeight;

// Initialize snowflake properties and positions
void InitSnow() {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_SNOWFLAKES; i++) {
        snow[i].x = (float)(rand() % screenWidth);
        snow[i].y = (float)(rand() % screenHeight);
        snow[i].speedY = 1.5f + (float)(rand() % 30) / 10.0f; // Falling speed
        snow[i].speedX = ((float)(rand() % 20) - 10.0f) / 15.0f; // Horizontal drift
        snow[i].radius = 2 + (rand() % 4); // Size of the snowflake
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TIMER: {
            // Update snowflake positions
            for (int i = 0; i < NUM_SNOWFLAKES; i++) {
                snow[i].y += snow[i].speedY;
                snow[i].x += snow[i].speedX;

                // Reset snowflake to top if it goes past the bottom
                if (snow[i].y > screenHeight) {
                    snow[i].y = -10.0f;
                    snow[i].x = (float)(rand() % screenWidth);
                }
                // Wrap around left and right screen edges
                if (snow[i].x > screenWidth) snow[i].x = 0;
                if (snow[i].x < 0) snow[i].x = (float)screenWidth;
            }
            // Request window redraw
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Double buffering to prevent screen flicker
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            // 1. Fill background with pure black (acts as transparency color key)
            HBRUSH hBlackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RECT rect = {0, 0, screenWidth, screenHeight};
            FillRect(hdcMem, &rect, hBlackBrush);

            // 2. Draw white snowflakes
            HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
            HPEN hWhitePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            SelectObject(hdcMem, hWhiteBrush);
            SelectObject(hdcMem, hWhitePen);

            for (int i = 0; i < NUM_SNOWFLAKES; i++) {
                Ellipse(hdcMem,
                        (int)(snow[i].x - snow[i].radius),
                        (int)(snow[i].y - snow[i].radius),
                        (int)(snow[i].x + snow[i].radius),
                        (int)(snow[i].y + snow[i].radius));
            }

            // 3. Transfer buffered image to the screen
            BitBlt(hdc, 0, 0, screenWidth, screenHeight, hdcMem, 0, 0, SRCCOPY);

            // Clean up drawing resources
            DeleteObject(hWhiteBrush);
            DeleteObject(hWhitePen);
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    InitSnow();

    // Register the window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TransparentSnowOverlay";
    RegisterClass(&wc);

    // Create a topmost, transparent, click-through borderless window
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "TransparentSnowOverlay",
        "Desktop Snow Overlay",
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, "Window creation failed.", "Error", MB_ICONERROR);
        return 1;
    }

    // Set black (RGB 0,0,0) as the transparent color key
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // Show window and start animation timer
    ShowWindow(hwnd, SW_SHOW);
    SetTimer(hwnd, TIMER_ID, 1000 / FPS, NULL);

    // Main message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
