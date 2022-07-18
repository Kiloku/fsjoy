#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <vector>
#include <iostream>
#include <algorithm>

const char* ORGANIZATION_NAME = "HardLightProductions";
const char* APPLICATION_NAME = "FreeSpaceOpen";
const std::string FS_INI = "fs2_open.ini";

const int MAX_JOYSTICKS = 4;

bool done = false;
bool quit_modal = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
SDL_WindowFlags window_flags;
SDL_Window* window;
SDL_Renderer* renderer;

struct joystick_info{
	int id;
	std::string name;
	std::string guid;
};

const joystick_info NONE_JOYSTICK { -1, "None", ""};

inline std::string str_tolower(std::string &in_str) {
	std::string str = std::string(in_str);
	std::transform(str.begin(), str.end(), str.begin(), [](char c) { return std::tolower(c); });
	return str;
}

std::vector<joystick_info> system_joysticks;
joystick_info fso_joysticks[MAX_JOYSTICKS];
joystick_info selected_joysticks[MAX_JOYSTICKS];
std::string get_system_joystick_name(std::string guid);


bool dirty = false;

void find_system_joysticks();

void find_ini_joysticks();

void update_fso_settings();

void gui_init();

void loop();

void cleanup();

void joystick_combo(int index);