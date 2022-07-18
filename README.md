# fsjoy
Freespace Open Joystick Selector

#Requirements

This project requires SDL2 2.0.17 or higher.  
This requirement is inherited from [Dear ImGui](https://github.com/ocornut/imgui) which it uses to draw the GUI.

#Building

I have only tested this on linux (Pop!OS 22.04)  
Use [cmkr](https://cmkr.build/): 
> ```
> curl https://raw.githubusercontent.com/build-cpp/cmkr/main/cmake/cmkr.cmake -o cmkr.cmake
> cmake -P cmkr.cmake
> ```
> 
> After the bootstrapping process finishes, modify cmake.toml and open the project in your favorite IDE or build with CMake:
> ```
> cmake -B build
> cmake --build build
> ```

