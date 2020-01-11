#include "textures.h"

#include <stdlib.h>

#define TEXTURE_LIMIT 8

extern struct texture grass_texture;
extern struct texture box_texture;
extern struct texture box_specular_texture;
extern struct texture spawnbeacon_texture;
extern struct texture spawnbeacon_normal_texture;
extern struct texture boxwall_texture;
extern struct texture boxwall_specular_texture;
extern struct texture boxwall_normal_texture;

struct texture *textures;

void textures_init(){
  textures = malloc(sizeof(struct texture) * TEXTURE_LIMIT);

  textures[GRASS] = grass_texture;
  textures[BOX] = box_texture;
  textures[BOX_SPECULAR] = box_specular_texture;
  textures[SPAWNBEACON] = spawnbeacon_texture;
  textures[SPAWNBEACON_NORMAL] = spawnbeacon_normal_texture;
  textures[BRICKWALL] = boxwall_texture;
  textures[BRICKWALL_SPECULAR] = boxwall_specular_texture;
  textures[BRICKWALL_NORMAL] = boxwall_normal_texture;
}

struct texture *textures_get(texture_id id){
  return &textures[id];
}

void textures_free(){
  free(textures);
}
