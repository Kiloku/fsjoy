#include <SDL.h>
#include <SDL_joystick.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "main.h"
#include <fstream>
#include <array>

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

int main()
{
	if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK ) < 0)
	{
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	find_system_joysticks();

	for (auto &j : selected_joysticks) {
		j = NONE_JOYSTICK;
	}

	find_ini_joysticks();

	gui_init();

	while (!done)
	{
		loop();
	}

	cleanup();
}

void find_system_joysticks(){
	int num_joy, i;
	num_joy = SDL_NumJoysticks();
	printf("%d joysticks found\n", num_joy);
	system_joysticks.push_back(NONE_JOYSTICK);
	for(i = 0; i < num_joy; i++)
	{
		SDL_Joystick *joystick = SDL_JoystickOpen(i);
		joystick_info j;
		char* tempguid = new char[33];
		SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), tempguid, 33);
		j.guid = std::string(tempguid);
		j.id = SDL_JoystickInstanceID(joystick);
		j.name = SDL_JoystickName(joystick);
		system_joysticks.push_back(j);
	}
}

void find_ini_joysticks() {
	std::string ini = std::string(SDL_GetPrefPath(ORGANIZATION_NAME, APPLICATION_NAME)) + FS_INI;
	std::ifstream f(ini);

	if (f.good()){
		std::string line;
		while (std::getline(f, line)) {
			auto sep = line.find("=");

			if (sep > 0){
				std::string key = line.substr(0, sep);
				std::string val = "";
				if (key == "CurrentJoystick" || key == "Joy0ID") {
					val = line.substr(sep+1, std::string::npos);
					fso_joysticks[0].id = std::stoi(val);
				}
				if (key == "CurrentJoystickGUID" || key == "Joy0GUID")
				{
					val = line.substr(sep+1, std::string::npos);
					fso_joysticks[0].guid = val;
					fso_joysticks[0].name = get_system_joystick_name(fso_joysticks[0].guid);
				}
				for (int i = 1; i < MAX_JOYSTICKS; i++) {
					if (key == std::string("Joy") + std::to_string(i) + std::string("ID")) {
						val = line.substr(sep+1, std::string::npos);
						fso_joysticks[i].id = std::stoi(val);
					}
					if (key == std::string("Joy") + std::to_string(i) + std::string("GUID")) {
						val = line.substr(sep+1, std::string::npos);
						fso_joysticks[i].guid = val;
						fso_joysticks[i].name = get_system_joystick_name(fso_joysticks[i].guid);
					}
				}
			}
		}
	}
	f.close();

	for (int i = 0; i < MAX_JOYSTICKS; i++)
	{
		if (fso_joysticks[i].guid != ""){
			selected_joysticks[i] = fso_joysticks[i];
		}
	}
}

std::vector<std::string> selected_joysticks_to_lines(){
	std::vector<std::string> lines;
	lines.clear();
	for (int i = 0; i < MAX_JOYSTICKS; i++) {
		if (selected_joysticks[i].guid == "") continue;
		std::string key_id = std::string("Joy") + std::to_string(i) + std::string("ID");
		std::string key_guid = std::string("Joy") + std::to_string(i) + std::string("GUID");
		std::string val_id = std::to_string(selected_joysticks[i].id);
		std::string val_guid = selected_joysticks[i].guid;
		lines.push_back(key_id + "=" + val_id);
		lines.push_back(key_guid + "=" + str_tolower(val_guid));
	}
	return lines;
}

void update_fso_settings(){
	std::string ini = std::string(SDL_GetPrefPath(ORGANIZATION_NAME, APPLICATION_NAME)) + FS_INI;
	std::ifstream fi(ini);
	std::vector<std::string> settings;

	if (fi.good()){
		std::string line;
		while (std::getline(fi, line)) {
			if (line != "") {
				settings.push_back(line);
			}
		}
	} else {
		//Error: FSO Ini file not found.
		return;
	}
	fi.close();

	if (settings.empty()) {
		//Error: FSO Ini file is broken.
		return;
	}


	std::string section = "";
	int joyline = -1;
	int count = 0;
	for (auto &l : settings) {
		if (l[0] == '[' && l.back() == ']'){
			if (section == "[Default]" && joyline < 0)
				joyline = count;
			section = l;
		}
		auto sep = l.find("=");
		if (sep > 0) {
			if (l.substr(0, sep) == "Joy0ID" || l.substr(0, sep) == "Joy1ID" ||
				l.substr(0, sep) == "Joy2ID" || l.substr(0, sep) == "Joy3ID" ||
				l.substr(0, sep) == "Joy0GUID" || l.substr(0, sep) == "Joy1GUID" ||
				l.substr(0, sep) == "Joy2GUID" || l.substr(0, sep) == "Joy3GUID" ||
				l.substr(0, sep) == "CurrentJoystick" || l.substr(0, sep) == "CurrentJoystickGUID") {
				if (joyline < 0) joyline = count;
				settings[count] = "";
			}
		}
		count++;
	}
	
	auto it = settings.begin() + joyline;
	auto joys = selected_joysticks_to_lines();
	for (int i = 0; i < joys.size(); i++ ) {
		settings.emplace(it+i, joys[i] );
	}

	std::ofstream fo(ini);
	if (fo.good()) {
		for (auto &s : settings) {
			if (s != "")
				fo << s + "\n";
		}
	}
	fo.close();
	settings.clear();
	dirty = false;
}


std::string get_system_joystick_name(std::string guid) {
	for (auto &j : system_joysticks)
	{
		if (str_tolower(guid) == str_tolower(j.guid)) return j.name;
	}
	return "";
}

void gui_init() {
	window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	window = SDL_CreateWindow("FSO Joystick Selector", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 250, window_flags);
	// Setup SDL_Renderer instance
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	
	if (renderer == NULL)
	{
		SDL_Log("Error creating SDL_Renderer!");
		exit(1);
	}
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	
	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);
}

void loop(){
	// Poll and handle events (inputs, window resize, etc.)
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)))
		{
			if (dirty)
				quit_modal = true;
			else
				done = true;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);

		ImGui::Begin("FSO Joystick Selector", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);	
		if (quit_modal) ImGui::OpenPopup("Unsaved");
		for(int i = 0; i < MAX_JOYSTICKS; i++){
			joystick_combo(i);
		}

		if (ImGui::Button("Save")){
			update_fso_settings();
			dirty = false;
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		
		if (ImGui::BeginPopupModal("Unsaved"))
		{
			ImGui::Text("You have unsaved changes.\n\n");
			ImGui::Separator();
			
			if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); quit_modal = false; }
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Save and close")) { update_fso_settings(); done = true; }
			ImGui::SameLine();
			if (ImGui::Button("Close without saving")) { done = true; }
			ImGui::EndPopup();
		}
		
		ImGui::End();
	}


	// Rendering
	ImGui::Render();
	SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
	SDL_RenderClear(renderer);
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(renderer);
}

void cleanup() {
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void joystick_combo(int index){
	std::string combo_id = std::string("joy") + std::to_string(index);

	if (ImGui::BeginCombo(combo_id.c_str(), selected_joysticks[index].name.c_str(), 0))
	{
		for (int n = 0; n < system_joysticks.size(); n++)
		{
			if (ImGui::Selectable(system_joysticks[n].name.c_str()))
			{
				selected_joysticks[index] = system_joysticks[n];
				dirty = true;
			}
		}
		ImGui::EndCombo();
	}
}