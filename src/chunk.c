#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>

#include "noise.h"

void byte4_set(GLbyte x, GLbyte y, GLbyte z, GLbyte w, byte4 dest){
  dest[0] = x;
  dest[1] = y;
  dest[2] = z;
  dest[3] = w;
}

struct chunk* chunk_init(){
  printf("creating chunk\n");
  struct chunk* chunk = malloc(sizeof(struct chunk));

  chunk->blocks = malloc(CHUNK_SIZE_CUBED * sizeof(uint8_t));
  chunk->elements = 0;
  chunk->changed = 1;
  glGenBuffers(1, &chunk->vbo);

  unsigned int count = 0;
  for(uint8_t x = 0; x < CHUNK_SIZE; x++){
    for(uint8_t z = 0; z < CHUNK_SIZE; z++){
      float f = simplex2(x * 0.05f, z * 0.05f, 5, 0.5f, 2.0f);
      int h = (f + 1) / 2 * (CHUNK_SIZE - 1) + 1;

      for(uint8_t y = 0; y < CHUNK_SIZE; y++){
        chunk->blocks[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED] = y < h ? 1 : 0;
        if(y < h){
          count++;
        }
      }
    }
  }
  printf("generated chunk with %d blocks\n", count);

  return chunk;
}

void chunk_free(struct chunk *chunk){
  glDeleteBuffers(1, &chunk->vbo);
  free(chunk->blocks);
}

void chunk_update(struct chunk *chunk){
  chunk->changed = 0;

  byte4 *vertex = malloc(CHUNK_SIZE_CUBED * 36 * sizeof(byte4));
  unsigned int i = 0;

  for(uint8_t y = 0; y < CHUNK_SIZE; y++){
    for(uint8_t x = 0; x < CHUNK_SIZE; x++){
      for(uint8_t z = 0; z < CHUNK_SIZE; z++){
        uint8_t block = chunk->blocks[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED];

        if(!block){
          continue;
        }

        // -x
        byte4_set(x, y, z, block, vertex[i++]);
        byte4_set(x, y, z + 1, block, vertex[i++]);
        byte4_set(x, y + 1, z, block, vertex[i++]);
        byte4_set(x, y + 1, z, block, vertex[i++]);
        byte4_set(x, y, z + 1, block, vertex[i++]);
        byte4_set(x, y + 1, z + 1, block, vertex[i++]);

        // +x
        byte4_set(x + 1, y + 1, z, block, vertex[i++]);
        byte4_set(x + 1, y, z + 1, block, vertex[i++]);
        byte4_set(x + 1, y, z, block, vertex[i++]);
        byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
        byte4_set(x + 1, y, z + 1, block, vertex[i++]);
        byte4_set(x + 1, y + 1, z, block, vertex[i++]);

        // -z
        byte4_set(x, y + 1, z, block, vertex[i++]);
        byte4_set(x + 1, y, z, block, vertex[i++]);
        byte4_set(x, y, z, block, vertex[i++]);
        byte4_set(x + 1, y + 1, z, block, vertex[i++]);
        byte4_set(x + 1, y, z, block, vertex[i++]);
        byte4_set(x, y + 1, z, block, vertex[i++]);

        // +z
        byte4_set(x, y, z + 1, block, vertex[i++]);
        byte4_set(x + 1, y, z + 1, block, vertex[i++]);
        byte4_set(x, y + 1, z + 1, block, vertex[i++]);
        byte4_set(x, y + 1, z + 1, block, vertex[i++]);
        byte4_set(x + 1, y, z + 1, block, vertex[i++]);
        byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);

        // -y
        byte4_set(x + 1, y, z, block, vertex[i++]);
        byte4_set(x, y, z + 1, block, vertex[i++]);
        byte4_set(x, y, z, block, vertex[i++]);
        byte4_set(x + 1, y, z + 1, block, vertex[i++]);
        byte4_set(x, y, z + 1, block, vertex[i++]);
        byte4_set(x + 1, y, z, block, vertex[i++]);

        // +y
        byte4_set(x, y + 1, z, block, vertex[i++]);
        byte4_set(x, y + 1, z + 1, block, vertex[i++]);
        byte4_set(x + 1, y + 1, z, block, vertex[i++]);
        byte4_set(x + 1, y + 1, z, block, vertex[i++]);
        byte4_set(x, y + 1, z + 1, block, vertex[i++]);
        byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
      }
    }
  }

  printf("%d blocks in chunk\n", i / 36);
  chunk->elements = i;

  glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
  glBufferData(GL_ARRAY_BUFFER, chunk->elements * sizeof(*vertex), vertex, GL_STATIC_DRAW);

  free(vertex);
}

void chunk_draw(struct chunk *chunk){
  if(chunk->changed){
    chunk_update(chunk);
  }

  if(!chunk->elements){
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
  glVertexAttribPointer(0, 4, GL_BYTE, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLES, 0, chunk->elements);
}
