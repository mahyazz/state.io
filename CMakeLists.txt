cmake_minimum_required(VERSION 3.16)
project("state project")
include_directories("/opt/homebrew/include/")
link_directories("/opt/homebrew/lib/")
add_executable(state main.c soldiers.c)
target_link_libraries(state m SDL2 SDL2_gfx SDL2_ttf)
#target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
