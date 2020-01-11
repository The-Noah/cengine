#ifndef TEXTURES_H
#define TEXTURES_H

typedef enum{
  GRASS = 0,
  BOX,
  BOX_SPECULAR
} texture_id;

struct texture{
  unsigned int width;
  unsigned int height;
  unsigned char	*pixel_data;
} textures[3];

#endif
