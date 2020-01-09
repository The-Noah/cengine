#include "texture.h"

#define GLEW_STATIC
#include <GL\glew.h>

unsigned int texture_create(texture_id texture_index){
  unsigned int texture;
  glGenTextures(1, &texture);
  texture_bind(texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures[texture_index].width, textures[texture_index].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textures[texture_index].pixel_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  return texture;
}

void texture_bind(unsigned int texture){
  glBindTexture(GL_TEXTURE_2D, texture);
}

void texture_delete(unsigned int *texture){
  glDeleteTextures(1, texture);
  *texture = 0;
}
