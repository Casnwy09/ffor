#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "map.h"

int main(int argc, char **argv)
{
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF);
  SDL_Window *window = SDL_CreateWindow("Food Fight Online Remastered",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  
  int running = 1;
  float camerax = 0.0f, cameray = 0.0f, camera_scale = 1.0f;
  struct Map map;
  struct MapDrawer map_drawer;
 
  MapCreate(&map, "./maps/test.json");
  MapDrawerCreate(&map_drawer, renderer);

  while (running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
      switch (event.type)
      {
        case SDL_QUIT: running = 0; break;
      }
    }

    const unsigned char *keystates = SDL_GetKeyboardState(NULL);
    float camera_speed = 1.0f;
    camerax += (float)(keystates[SDL_SCANCODE_RIGHT] - keystates[SDL_SCANCODE_LEFT]) * camera_speed;
    cameray += (float)(keystates[SDL_SCANCODE_DOWN] - keystates[SDL_SCANCODE_UP]) * camera_speed;
    camera_scale += (float)(keystates[SDL_SCANCODE_RIGHTBRACKET] - keystates[SDL_SCANCODE_LEFTBRACKET]) * 0.001f;

    SDL_RenderClear(renderer);
    MapDrawerDraw(&map_drawer, &map, camerax, cameray,
      (int)(camera_scale * 1280.0f), (int)(camera_scale * 720.0f), 0, 0, 1280, 720);
    SDL_RenderPresent(renderer);
  }

  MapDrawerDestroy(&map_drawer);
  MapDestroy(&map);
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
  
  return 0;
}
