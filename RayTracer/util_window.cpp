#include "util_window.h"
#include "cmath"

const char* wnd_class_name = "Placeholder Classname";

void* pixels;
char* pixel_data;
BITMAPINFO info;
HBITMAP hbm;
const int BITCOUNT_PER_PIXEL = 24;
const int title_height = 39;

long long bytes_per_row;
bool screen_changed = false;
HDC hdc;
HDRAWDIB hdd;
HDC bitmap_dc;
HGDIOBJ old_obj;

HWND create_window(HINSTANCE &hInstance) {
	HWND hWnd;
	WNDCLASSEX wnd_class = { 0 };
	init_wnd_class(wnd_class, hInstance);

	RegisterClassEx(&wnd_class);
	create_hwnd(hWnd, hInstance);
	init(hWnd);

	return hWnd;
}

void init(HWND &hWnd) {
	memset(&info, 0, sizeof(info));
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = screen_width;
	info.bmiHeader.biHeight = screen_height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = BITCOUNT_PER_PIXEL;
	info.bmiHeader.biCompression = BI_RGB;

	hdc = GetDC(hWnd);
	pixels = NULL;
	hbm = CreateDIBSection(hdc, &info, DIB_RGB_COLORS, &pixels, NULL, 0);
	pixel_data = (char*)pixels;

	hdd = DrawDibOpen();

	bitmap_dc = CreateCompatibleDC(hdc);
	old_obj = SelectObject(bitmap_dc, hbm);
	bytes_per_row = ceil(screen_width * 0.03) * 100;
}

void create_hwnd(HWND &hwnd, HINSTANCE &hInstance)
{
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		wnd_class_name,
		TEXT("CAST THE RAY DAMMIT"),  // window caption
		WS_OVERLAPPEDWINDOW,      // window style
		30,            // initial x position
		30,            // initial y position
		screen_width + 16,            // Some weird horizontal dead pixels
		screen_height + title_height,
		NULL,                     // parent window handle
		NULL,                     // window menu handle
		hInstance,                // program instance handle
		NULL);                    // creation parameters
}

void init_wnd_class(WNDCLASSEX &wndClass, HINSTANCE &hInstance) {
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = 0;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = wnd_class_name;
	wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
}

void destroy_window() {
	DrawDibEnd(hdd);
	DrawDibClose(hdd);
	SelectObject(bitmap_dc, old_obj);
	DeleteDC(hdc);
	DeleteDC(bitmap_dc);
	DeleteObject(hbm);
	DeleteObject(old_obj);
}


void set_pixel(unsigned int x, unsigned int y, Vec3f color) {
	unsigned long pixel_index = y * bytes_per_row + x * 3;

	pixel_data[pixel_index + 0] = (char)(color.z * 255);
	pixel_data[pixel_index + 1] = (char)(color.y * 255);
	pixel_data[pixel_index + 2] = (char)(color.x * 255);

	if (!screen_changed)
		screen_changed = true;
}

void Update() {
	if (!screen_changed)
		return;
	DrawDibDraw(hdd, hdc, 0, 0, screen_width, screen_height, &info.bmiHeader, pixels, 0, 0, screen_width, screen_height, 0);
#pragma omp parallel for
	for (int i = 0; i < screen_width; i++)
		for (int j = 0; j < screen_height; j++)
			set_pixel(i, j, Vec3f(0,0,0));
	screen_changed = false;
}
