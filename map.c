#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "SDL2/SDL_image.h"
#include "cJSON.h"
#include "blocks.h"
#include "map.h"

void MapCreate(struct Map *map, const char *mapfile)
{
  // First defualt everything in the struct
  memset(map, 0, sizeof(struct Map));
  
  // First try to load in the json file
  char *mapfile_raw_string = NULL;
  cJSON *mapfile_parsed_json = NULL;
  {
    // First try to see if the mapfile is going to be NULL
    if (mapfile == NULL)
    {
      printf("MapCreate: the mapfile specified is NULL!\n");
      return;
    }
    
    FILE *mapfile_fileptr = fopen(mapfile, "r");
    
    // Get the size of the mapfile
    fseek(mapfile_fileptr, 0, SEEK_END);
    int mapfile_raw_string_length = (int)ftell(mapfile_fileptr);
    fseek(mapfile_fileptr, 0, SEEK_SET);
    
    mapfile_raw_string = malloc(mapfile_raw_string_length);
    fread(mapfile_raw_string, mapfile_raw_string_length, 1, mapfile_fileptr);
    
    // Now parse the JSON file
    mapfile_parsed_json = cJSON_Parse(mapfile_raw_string);

    // Close the mapfile
    fclose(mapfile_fileptr);
  }

  // Parse the JSON file, filling the properties
  if (mapfile_parsed_json != NULL)
  {
    cJSON *mapfile_width = cJSON_GetObjectItemCaseSensitive(mapfile_parsed_json, "width"),
      *mapfile_height = cJSON_GetObjectItemCaseSensitive(mapfile_parsed_json, "height"),
      *mapfile_layers = cJSON_GetObjectItemCaseSensitive(mapfile_parsed_json, "layers");

    // Get the width and height
    map->Size[0] = cJSON_IsNumber(mapfile_width) ? (int)mapfile_width->valuedouble : 0;
    map->Size[1] = cJSON_IsNumber(mapfile_height) ? (int)mapfile_height->valuedouble : 0;
    
    // Allocate a map for the size of the width and height
    map->Blocks = malloc(map->Size[0] * map->Size[1] * sizeof(*map->Blocks));
    memset(map->Blocks, 0, map->Size[0] * map->Size[1] * sizeof(*map->Blocks));

    // Get the layers, and then get the default layer "Map"
    for (cJSON *mapfile_layer = (mapfile_layers != NULL ? mapfile_layers->child : NULL);
      mapfile_layer != NULL;
      mapfile_layer = mapfile_layer->next)
    {
      cJSON *mapfile_layer_name = cJSON_GetObjectItemCaseSensitive(mapfile_layer, "name");
      if (cJSON_IsString(mapfile_layer_name) && !strcmp(mapfile_layer_name->valuestring, "Map"))
      {
        // Loop through the "data" array and populate the spawns arrays as well
        cJSON *mapfile_layer_data = cJSON_GetObjectItemCaseSensitive(mapfile_layer, "data");
        int block = 0;
        for (cJSON *mapfile_block = (mapfile_layer_data != NULL ? mapfile_layer_data->child : NULL);
          mapfile_block != NULL;
          mapfile_block = mapfile_block->next)
        {
          int blockid = (int)mapfile_block->valuedouble, blockx = block % map->Size[0], blocky = block / map->Size[0];          

          // Check if the blockid is bigger than or equal to 200 for mobs, and 256 for player spawns
          if (blockid == 256)
          {
            map->PlayerSpawn[0] = blockx;
            map->PlayerSpawn[1] = blocky;
          }
          else if (blockid > 200)
          {
            int mobid = blockid - 201;
            map->SpawnCounts[mobid]++;
            map->Spawns[mobid] = realloc(map->Spawns[mobid], map->SpawnCounts[mobid] * 2 * sizeof(**map->Spawns));
            map->Spawns[mobid][map->SpawnCounts[mobid] * 2 - 2] = blockx;
            map->Spawns[mobid][map->SpawnCounts[mobid] * 2 - 1] = blocky;
          } 
          else
          {
            map->Blocks[block] = blockid;
          }          

          block++;
        }
      }
    }
  }

  // Clean up the JSON parser and the raw string used for it
  cJSON_Delete(mapfile_parsed_json);
  free(mapfile_raw_string);
}

void MapDestroy(struct Map *map)
{
  for (int i = 0; i < MobTypesMax; i++)
    free(map->Spawns[i]);
  free(map->Blocks);
}

void MapDrawerCreate(struct MapDrawer *map_drawer, SDL_Renderer *renderer)
{
  memset(map_drawer, 0, sizeof(struct MapDrawer));

  // Set the renderer and the render texture.
  map_drawer->Renderer = renderer;
  map_drawer->RenderTexture = SDL_CreateTexture(map_drawer->Renderer, SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_TARGET, MAP_MAX_RENDER_SIZE, MAP_MAX_RENDER_SIZE);

  // Load the file loaded textures
  {
    SDL_Surface *surface_loader;
    for (int i = 1; i < 256; i++)
    {
      char block_path[512] = {'\0'};
      strcpy(block_path, "./textures/");
      strcat(block_path, block_names[i]);
      strcat(block_path, ".png");
      surface_loader = IMG_Load(block_path);
      map_drawer->BlockTextures[i] = SDL_CreateTextureFromSurface(map_drawer->Renderer, surface_loader);
      SDL_FreeSurface(surface_loader);
    }
  }
}

void MapDrawerDestroy(struct MapDrawer *map_drawer)
{
  for (int i = 0; i < 256; i++)
    SDL_DestroyTexture(map_drawer->BlockTextures[i]);
  SDL_DestroyTexture(map_drawer->RenderTexture);
}

void MapDrawerDraw(struct MapDrawer *map_drawer, struct Map *map, int camerax, int cameray, int cameraw, int camerah,
  int screenx, int screeny, int screenw, int screenh)
{
  int bound_left = (camerax - cameraw / 2) / MAP_TILE_SIZE, bound_right = (camerax + cameraw / 2) / MAP_TILE_SIZE + 1;
  int bound_top = (cameray - camerah / 2) / MAP_TILE_SIZE, bound_bottom = (cameray + camerah / 2) / MAP_TILE_SIZE + 1;
  
  SDL_SetRenderTarget(map_drawer->Renderer, map_drawer->RenderTexture);
  cameraw = (screenw > MAP_MAX_RENDER_SIZE) ? MAP_MAX_RENDER_SIZE : screenw;
  camerah = (screenh > MAP_MAX_RENDER_SIZE) ? MAP_MAX_RENDER_SIZE : screenh;
  
  SDL_SetRenderDrawColor(map_drawer->Renderer, 95, 205, 245, 255);
  SDL_RenderClear(map_drawer->Renderer);

  for (int x = bound_left; x < bound_right; x++)
  {
    for (int y = bound_top; y < bound_bottom; y++)
    {
      if (x < 0 || x >= map->Size[0] || y < 0 || y >= map->Size[1])
        continue;
      
      // Get the simple id of the block and the translated position to draw to
      int blockid = map->Blocks[y * map->Size[0] + x];
      if (!blockid)
        continue;
      
      int translated_x = (x * MAP_TILE_SIZE - camerax) + cameraw / 2;
      int translated_y = (y * MAP_TILE_SIZE - cameray) + camerah / 2;
      
      // Figure out what frame of the block to use
      unsigned char air = (map->Blocks[(y - 1) * map->Size[0] + x] != blockid) |
        (map->Blocks[y * map->Size[0] + (x + 1)] != blockid) << 1 |
        (map->Blocks[(y + 1) * map->Size[0] + x] != blockid) << 2 |
        (map->Blocks[y * map->Size[0] + (x - 1)] != blockid) << 3;

      // Create the source and destination clip, and then copy it to the target
      SDL_Rect source, destination;
      source.x = air % 4 * MAP_TILE_SIZE;
      source.y = air / 4 * MAP_TILE_SIZE;
      source.w = MAP_TILE_SIZE;
      source.h = MAP_TILE_SIZE;
      destination.x = translated_x;
      destination.y = translated_y;
      destination.w = MAP_TILE_SIZE;
      destination.h = MAP_TILE_SIZE;
      
      SDL_RenderCopy(map_drawer->Renderer, map_drawer->BlockTextures[blockid], &source, &destination); 
    }
  }

  SDL_Rect render_source, render_destination;
  render_source.x = 0;
  render_source.y = 0;
  render_source.w = cameraw;
  render_source.h = camerah;
  render_destination.x = screenx;
  render_destination.y = screeny;
  render_destination.w = screenw;
  render_destination.h = screenh;

  SDL_SetRenderTarget(map_drawer->Renderer, NULL);
  SDL_RenderCopy(map_drawer->Renderer, map_drawer->RenderTexture, &render_source, &render_destination);
}
