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
bool unsaved_modal = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
SDL_WindowFlags window_flags;
SDL_Window* window;
SDL_Renderer* renderer;

struct joystick_info {
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

/**
 * Uses SDL2 to get and parse a list of all plugged in joysticks. 
 * Populates the system_joysticks vector.
 * */
void find_system_joysticks();

/**
 * Loads the fs2_open.ini file and reads the currently selected joysticks into the `fso_joysticks` array.
 * It then compares those to the system_joysticks to figure out their names based on their guids.
 * Finally, also fills the selected_joysticks array.
 * */
void find_ini_joysticks();

/**
 * Loops through the selected_joysticks array and writes it in the ini's format, in the order they are indexed in the array.
 * */
std::vector<std::string> selected_joysticks_to_lines();

/**
 * Saves the joystick data from selected_joysticks into the fs2_open.ini file.
 * */
void update_fso_settings();

/**
 * Gets the name of a joystick, given its GUID.
 * */
std::string get_system_joystick_name(std::string guid);

/**
 * Do a few things ImGui and SDL need before starting rendering
 * */
void gui_init();

/**
 * Similar to a game's loop, mostly for the GUI. Avoid doing other logic here, just call functions from the relevant controls.
 * */
void loop();

/**
 * Do a few things ImGui and SDL need before closing the app.
 * */
void cleanup();

/**
 * Generate a combo control for the given joystick slot.
 * */
void joystick_combo(int index);