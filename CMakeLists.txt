cmake_minimum_required(VERSION 3.1)
project(ffor C)
add_executable(ffor "main.c" "map.c" "cJSON.c" "blocks.c")
target_link_libraries(ffor -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net)
