#ifndef UTIL_HEADER
#define UTIL_HEADER

#include <windows.h>
#include <Vfw.h>
#include <vector>
#include "geometry.h"
#pragma comment(lib, "Vfw32.lib")

extern const char* wnd_class_name;
extern void* pixels;
extern char* pixel_data;
extern BITMAPINFO info;
extern HBITMAP hbm;
extern const int BITCOUNT_PER_PIXEL;
extern const int title_height;
extern long long bytes_per_row;
extern bool screen_changed;
extern HDC hdc;
extern HDRAWDIB hdd;
extern HDC bitmap_dc;
extern HGDIOBJ old_obj;

extern const int screen_width;
extern const int screen_height;


void init(HWND &hWnd);
void destroy_window();
HWND create_window(HINSTANCE &hInstance);
void create_hwnd(HWND &hwnd, HINSTANCE &hInstance);
void init_wnd_class(WNDCLASSEX &wndClass, HINSTANCE &hInstance);
void set_pixel(unsigned int x, unsigned int y, Vec3f color);
void Update();

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void CALLBACK FixedUpdate(HWND hwnd, UINT message, UINT uInt, DWORD dWord);
#endif 
