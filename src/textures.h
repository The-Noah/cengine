#ifndef TEXTURES_H
#define TEXTURES_H

typedef enum{
  GRASS = 0,
  BOX,
  BOX_SPECULAR,
  SPAWNBEACON,
  SPAWNBEACON_NORMAL,
  BRICKWALL,
  BRICKWALL_SPECULAR,
  BRICKWALL_NORMAL
} texture_id;

struct texture{
  unsigned int width;
  unsigned int height;
  unsigned char	*pixel_data;
};

void textures_init();
struct texture *textures_get(texture_id id);
void textures_free();

#endif
