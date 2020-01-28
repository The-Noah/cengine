#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define CHUNK_SIZE 32
#define CHUNK_SIZE_SQUARED 1024
#define CHUNK_SIZE_CUBED 32770

typedef GLbyte byte4[4];

void byte4_set(GLbyte x, GLbyte y, GLbyte z, GLbyte w, byte4 dest);

struct chunk{
  uint8_t *blocks;
  unsigned int vbo;
  int elements;
  uint8_t changed;
};

struct chunk* chunk_init();
void chunk_free(struct chunk *chunk);
void chunk_draw(struct chunk *chunk);
inline uint8_t chunk_get(struct chunk *chunk, int x, int y, int z){
  return chunk->blocks[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED];
}
inline void set(struct chunk *chunk, int x, int y, int z, uint8_t block){
  chunk->blocks[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED] = block;
  chunk->changed = 1;
}

#endif
