#define _USE_MATH_DEFINES 
#define NOMINMAX
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <math.h>
#include <algorithm>
#include "geometry.h"
#include "util_window.h"
#include "elements.h"

const int TARGET_FRAMERATE = 60;

const int screen_width =400;
const int screen_height = 400;
const int fov = M_PI / 2.;
std::vector<Vec3f> framebuffer(screen_width*screen_height);
typedef std::chrono::high_resolution_clock Clock;


void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights);
Vec3f reflect(const Vec3f &I, const Vec3f &N) {
	return I - N * 2.f*(I*N);
}
bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material) {
	float spheres_dist = std::numeric_limits<float>::max();
	for (size_t i = 0; i < spheres.size(); i++) {
		float dist_i;
		if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
			spheres_dist = dist_i;
			hit = orig + dir * dist_i;
			N = (hit - spheres[i].center).normalize();
			material = spheres[i].material;
		}
	}
	return spheres_dist<1000;
}

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
	Vec3f point, N;
	Material material;

	if (!scene_intersect(orig, dir, spheres, point, N, material)) {
		return Vec3f(0.2, 0.7, 0.8); // background color
	}
	float diffuse_light_intensity = 0, specular_light_intensity = 0;
	for (size_t i = 0; i<lights.size(); i++) {
		Vec3f light_dir = (lights[i].position - point).normalize();
		diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir*N);
		specular_light_intensity += powf(std::max(0.f, reflect(light_dir, N)*dir), material.specular_exponent)*lights[i].intensity;
	}
	return material.diffuse_color* diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1];;
}

void putFrame() {
#pragma omp parallel for
	for (int i = 0; i < screen_width; i++)
		for (int j = 0; j <screen_height; j++)
			set_pixel(i, screen_height-j, framebuffer[screen_width*j + i]);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND                hwnd;
	MSG                 Msg;

	hwnd = create_window(hInstance);
	ShowWindow(hwnd, nCmdShow);
	SetTimer(hwnd, NULL, 1000 / TARGET_FRAMERATE, (TIMERPROC)FixedUpdate);

	std::vector<Light>  lights;
	lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
	lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
	lights.push_back(Light(Vec3f(30, 20, 30), 1.7));
	Material      ivory(Vec2f(0.6, 0.3), Vec3f(0.4, 0.4, 0.3), 50.);
	Material red_rubber(Vec2f(0.9, 0.1), Vec3f(0.3, 0.1, 0.1), 10.);
	std::vector<Sphere> spheres;
	float test= -13;
	float test2 = 3;
	spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
	spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red_rubber));
	spheres.push_back(Sphere(Vec3f(10, 10, -18), 3, red_rubber));
	spheres.push_back(Sphere(Vec3f(3, 3, -13), 4, ivory));
	
	render(spheres, lights);
	putFrame();
	Update();
	while (0) {
		auto first_time = Clock::now();
		render(spheres, lights);
		putFrame();
		Update();
		while(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - first_time).count() < 16);
		test-=0.2;
		test2 -= 0.1;
		spheres.back().center = Vec3f(test2, 3, test);
		
	}
	
	
										
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

void CALLBACK FixedUpdate(HWND hwnd, UINT message, UINT uInt, DWORD dWord) {

}

// Event-Handling Function
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
		putFrame();
		Update();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}
void normalizeVector(Vec3f &vector) {
	float max = std::max(vector[0], std::max(vector[1], vector[2]));
	if (max>1) vector = vector * (1. / max);
}
void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {

	
	#pragma omp parallel for
	for (size_t j = 0; j<screen_height; j++) {
		for (size_t i = 0; i<screen_width; i++) {
			float x = (2 * (i + 0.5) / (float)screen_width - 1)*tan(fov / 2.)*screen_width / (float)screen_height;
			float y = -(2 * (j + 0.5) / (float)screen_height - 1)*tan(fov / 2.);
			Vec3f dir = Vec3f(x, y, -1).normalize();
			framebuffer[i + j * screen_width] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
			normalizeVector(framebuffer[i + j * screen_width]);
		}
	}
	//std::ofstream ofs; // save the framebuffer to file
	//ofs.open("./out.ppm", std::ios::binary);
	//ofs << "P6\n" << screen_width << " " << screen_height << "\n255\n";
	//for (size_t i = 0; i < screen_height*screen_width; ++i) {

	//	for (size_t j = 0; j<3; j++) {
	//		ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
	//	}
	//}
	//ofs.close();
}
