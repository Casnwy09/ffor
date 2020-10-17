#ifndef _map_h
#define _map_h
#include "SDL2/SDL.h"
#include "mobs.h"

#define MAP_TILE_SIZE 16
#define MAP_MAX_RENDER_SIZE 4096

struct Map
{
  int SpawnCounts[MobTypesMax], *Spawns[MobTypesMax], PlayerSpawn[2];
  int Size[2];
  int *Blocks;
};

struct MapDrawer
{
  SDL_Renderer *Renderer;
  SDL_Texture *BlockTextures[256];
  SDL_Texture *RenderTexture;
};

// Mapfile: the file to the TilED json map file.
void MapCreate(struct Map *map, const char *mapfile);
void MapDestroy(struct Map *map);

void MapDrawerCreate(struct MapDrawer *map_drawer, SDL_Renderer *renderer);
void MapDrawerDestroy(struct MapDrawer *map_drawer);
void MapDrawerDraw(struct MapDrawer *map_drawer, struct Map *map, int camerax, int cameray, int cameraw, int camerah,
  int screenx, int screeny, int screenw, int screenh);

#endif
