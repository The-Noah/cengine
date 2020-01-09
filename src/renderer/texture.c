#include "texture.h"

#define GLEW_STATIC
#include <GL\glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const char* res_path = "res/";

unsigned int texture_create(const char* name){
  unsigned int texture;
  glGenTextures(1, &texture);
  texture_bind(texture);

  // set the texture wrapping parameters
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // set texture filtering parameters
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  char name_buffer[32];
  strncpy(name_buffer, res_path, sizeof(res_path));
  strncat(name_buffer, name, sizeof(name) + 1);

  // load image, create texture and generate mipmaps
  int width, height, channels;
  stbi_set_flip_vertically_on_load(1);
  unsigned char *data = stbi_load(name_buffer, &width, &height, &channels, STBI_rgb_alpha);

  if(data){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }else{
    fprintf(stderr, "failed to load texture: %s\n", name_buffer);
    return -1;
  }

  stbi_image_free(data);

  return texture;
}

void texture_bind(unsigned int texture){
  glBindTexture(GL_TEXTURE_2D, texture);
}

void texture_delete(unsigned int *texture){
  glDeleteTextures(1, texture);
  *texture = 0;
}
