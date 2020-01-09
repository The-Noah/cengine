#ifndef TEXTURES_H
#define TEXTURES_H

typedef enum{
  GRASS = 0
} texture_id;

struct texture{
  unsigned int width;
  unsigned int height;
  unsigned char	pixel_data[128 * 128 * 4 + 1];
} textures[1];

#endif
