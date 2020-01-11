#include "texture.h"

#define GLEW_STATIC
#include <GL/glew.h>

unsigned int texture_create(texture_id texture_index){
  unsigned int texture;
  glGenTextures(1, &texture);
  texture_bind(texture);

  struct texture *texture_data = textures_get(texture_index);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, texture_data->width, texture_data->height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data->pixel_data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return texture;
}

void texture_bind(unsigned int texture){
  glBindTexture(GL_TEXTURE_2D, texture);
}

void texture_delete(unsigned int *texture){
  glDeleteTextures(1, texture);
  *texture = 0;
}
