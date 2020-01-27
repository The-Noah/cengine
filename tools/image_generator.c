#include <stdlib.h>
#include <stdio.h>
#include <dir.h> 
#define STB_IMAGE_IMPLEMENTATION	
#include "../include/stb_image.h"

int main(int argc, char** argv){
  if(argc != 2){
    printf("you must provide a texture name\n");
    return -1;
  }

  mkdir("res/generated");

  char* source_file = (char*)malloc(sizeof(char) * 8 + sizeof(argv[1]));
  sprintf(source_file, "res/%s.png", argv[1]);

  char* dest_file = (char*)malloc(sizeof(char) * 24 + sizeof(argv[1]));
  sprintf(dest_file, "res/generated/%s_texture.c", argv[1]);

  printf("generating texture %s from %s\n", dest_file, source_file);

  FILE* fs = fopen(source_file, "rb");
  if(!fs){
    printf("failed to open source file %s\n", source_file);
    return -1;
  }

  FILE* fd = fopen(dest_file, "w");
  if(!fd){
    printf("failed to open dest file %s\n", dest_file);
    return -1;
  }

  fprintf(fd, "#include \"../../src/textures.h\"\n");
  fprintf(fd, "struct texture %s_texture = {", argv[1]);
  // fprintf(fd, "static const unsigned char %s_texture[] = {", argv[1]);

  int width, height, channels;
  // stbi_set_flip_vertically_on_load(1);
  unsigned char* data = stbi_load(source_file, &width, &height, &channels, STBI_rgb);

  fprintf(fd, "%d, %d, (unsigned char[]){", width, height);
  unsigned int size = width * height * 4;
  printf("%d\n", size);
  for(unsigned int i = 0; i < size; i++) {
    fprintf(fd, "0x%.2X,", (int)data[i]);
  }

  fprintf(fd, "}};");

  fclose(fs);
  fclose(fd);

  stbi_image_free(data);

  printf("finished!\n");

  return 0;
}
