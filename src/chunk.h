#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define CHUNK_SIZE 32
#define CHUNK_SIZE_SQUARED 1024
#define CHUNK_SIZE_CUBED 32768

typedef GLbyte byte4[4];

struct chunk{
  uint8_t *blocks;
  unsigned int vao;
  int elements;
  uint8_t changed;
  int x;
  int z;
};

struct chunk* chunk_init(int x, int z);
void chunk_free(struct chunk *chunk);
void chunk_draw(struct chunk *chunk);
uint8_t chunk_get(struct chunk *chunk, int x, int y, int z);
void set(struct chunk *chunk, int x, int y, int z, uint8_t block);

#endif
