#define _USE_MATH_DEFINES 
#define NOMINMAX
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <algorithm>
#include "geometry.h"
#include "util_window.h"
#include "elements.h"


const int screen_width =800;
const int screen_height = 800;
const int fov = M_PI / 2.;
std::vector<Vec3f> framebuffer(screen_width*screen_height);


void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights);
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
	float diffuse_light_intensity = 0;
	for (size_t i = 0; i<lights.size(); i++) {
		Vec3f light_dir = (lights[i].position - point).normalize();
		diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir*N);
	}
	return material.diffuse_color* diffuse_light_intensity;
}

void putFrame() {
#pragma omp parallel for
	for (int i = 0; i < screen_width; i++)
		for (int j = 0; j < screen_height; j++)
			set_pixel(i, j, framebuffer[screen_width*j + i]);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND                hwnd;
	MSG                 Msg;

	hwnd = create_window(hInstance);
	ShowWindow(hwnd, nCmdShow);
	std::vector<Light>  lights;
	lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
	Material      ivory(Vec3f(0.4, 0.4, 0.3));
	Material red_rubber(Vec3f(0.3, 0.1, 0.1));
	std::vector<Sphere> spheres;
	spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
	spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red_rubber));
	spheres.push_back(Sphere(Vec3f(10, 10, -18), 3, red_rubber));
	spheres.push_back(Sphere(Vec3f(3, 3, -13), 4, ivory));
	render(spheres,lights);

		putFrame();
		Update();							
		spheres.pop_back();
		spheres.push_back(Sphere(Vec3f(3, 3, -19), 4, ivory));
		render(spheres,lights);
										
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
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

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {

	
	#pragma omp parallel for
	for (size_t j = 0; j<screen_height; j++) {
		for (size_t i = 0; i<screen_width; i++) {
			float x = (2 * (i + 0.5) / (float)screen_width - 1)*tan(fov / 2.)*screen_width / (float)screen_height;
			float y = -(2 * (j + 0.5) / (float)screen_height - 1)*tan(fov / 2.);
			Vec3f dir = Vec3f(x, y, -1).normalize();
			framebuffer[i + j * screen_width] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
		}
	}
	std::ofstream ofs; // save the framebuffer to file
	ofs.open("./out.ppm", std::ios::binary);
	ofs << "P6\n" << screen_width << " " << screen_height << "\n255\n";
	for (size_t i = 0; i < screen_height*screen_width; ++i) {
		Vec3f &c = framebuffer[i];
		float max = std::max(c[0], std::max(c[1], c[2]));
		if (max>1) c = c * (1. / max);
		for (size_t j = 0; j<3; j++) {
			ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
		}
	}
	ofs.close();
}
