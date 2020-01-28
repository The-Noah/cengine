#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>

#include "noise.h"
#include "renderer/utils.h"

void byte4_set(GLbyte x, GLbyte y, GLbyte z, GLbyte w, byte4 dest){
  dest[0] = x;
  dest[1] = y;
  dest[2] = z;
  dest[3] = w;
}

unsigned short block_index(uint8_t x, uint8_t y, uint8_t z){
  return x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED;
}

struct chunk* chunk_init(){
  printf("creating chunk\n");
  struct chunk* chunk = malloc(sizeof(struct chunk));

  chunk->blocks = malloc(CHUNK_SIZE_CUBED * sizeof(uint8_t));
  chunk->elements = 0;
  chunk->changed = 1;
  glGenVertexArrays(1, &chunk->vao);

  unsigned int count = 0;
  for(uint8_t x = 0; x < CHUNK_SIZE; x++){
    for(uint8_t z = 0; z < CHUNK_SIZE; z++){
      float f = simplex2(x * 0.05f, z * 0.05f, 5, 0.5f, 2.0f);
      int h = (f + 1) / 2 * (CHUNK_SIZE - 1) + 1;

      for(uint8_t y = 0; y < CHUNK_SIZE; y++){
        chunk->blocks[block_index(x, y, z)] = y < h ? 1 : 0;
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
  glDeleteVertexArrays(1, &chunk->vao);
  free(chunk->blocks);
}

void chunk_update(struct chunk *chunk){
  chunk->changed = 0;

  byte4 *vertex = malloc(CHUNK_SIZE_CUBED * 2 * sizeof(byte4));
  char *brightness = malloc(CHUNK_SIZE_CUBED * 2 * sizeof(char));
  unsigned int i = 0;
  unsigned int j = 0;

  for(uint8_t y = 0; y < CHUNK_SIZE; y++){
    for(uint8_t x = 0; x < CHUNK_SIZE; x++){
      for(uint8_t z = 0; z < CHUNK_SIZE; z++){
        uint8_t block = chunk->blocks[block_index(x, y, z)];

        if(!block){
          continue;
        }

        // -x
        if(chunk_get(chunk, x - 1, y, z) == 0){
          byte4_set(x, y, z, block, vertex[i++]);
          byte4_set(x, y, z + 1, block, vertex[i++]);
          byte4_set(x, y + 1, z, block, vertex[i++]);
          byte4_set(x, y + 1, z, block, vertex[i++]);
          byte4_set(x, y, z + 1, block, vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, vertex[i++]);
          for(int k = 0; k < 6; k++){
            brightness[j++] = 0;
          }
        }

        // +x
        if(chunk_get(chunk, x + 1, y, z) == 0){
          byte4_set(x + 1, y + 1, z, block, vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y, z, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, vertex[i++]);
          for(int k = 0; k < 6; k++){
            brightness[j++] = 0;
          }
        }

        // -z
        if(chunk_get(chunk, x, y, z - 1) == 0){
          byte4_set(x, y + 1, z, block, vertex[i++]);
          byte4_set(x + 1, y, z, block, vertex[i++]);
          byte4_set(x, y, z, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, vertex[i++]);
          byte4_set(x + 1, y, z, block, vertex[i++]);
          byte4_set(x, y + 1, z, block, vertex[i++]);
          for(int k = 0; k < 6; k++){
            brightness[j++] = 1;
          }
        }

        // +z
        if(chunk_get(chunk, x, y, z + 1) == 0){
          byte4_set(x, y, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
          for(int k = 0; k < 6; k++){
            brightness[j++] = 1;
          }
        }

        // -y
        if(chunk_get(chunk, x, y - 1, z) == 0){
          byte4_set(x + 1, y, z, block, vertex[i++]);
          byte4_set(x, y, z + 1, block, vertex[i++]);
          byte4_set(x, y, z, block, vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, vertex[i++]);
          byte4_set(x, y, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y, z, block, vertex[i++]);
          for(int k = 0; k < 6; k++){
            brightness[j++] = 2;
          }
        }

        // +y
        if(chunk_get(chunk, x, y + 1, z) == 0){
          byte4_set(x, y + 1, z, block, vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
          for(int k = 0; k < 6; k++){
            brightness[j++] = 2;
          }
        }
      }
    }
  }

  printf("%d blocks in chunk\n", i / 36);
  chunk->elements = i;

  glBindVertexArray(chunk->vao);

  unsigned int vertex_buffer = make_buffer(GL_ARRAY_BUFFER, chunk->elements * sizeof(*vertex), vertex);
  glVertexAttribPointer(0, 4, GL_BYTE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  unsigned int brightness_buffer = make_buffer(GL_ARRAY_BUFFER, j * sizeof(*brightness), brightness);
  glVertexAttribIPointer(1, 1, GL_BYTE, 0, 0);
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &brightness_buffer);

  free(brightness);
  free(vertex);
}

void chunk_draw(struct chunk *chunk){
  if(chunk->changed){
    chunk_update(chunk);
  }

  if(!chunk->elements){
    return;
  }

  glBindVertexArray(chunk->vao);
  glDrawArrays(GL_TRIANGLES, 0, chunk->elements);
}

uint8_t chunk_get(struct chunk *chunk, int x, int y, int z){
  if(x < 0 || x >= CHUNK_SIZE ||
     y < 0 || y >= CHUNK_SIZE ||
     z < 0 || z >= CHUNK_SIZE){
    return 0;
  }

  return chunk->blocks[block_index(x, y, z)];
}

void set(struct chunk *chunk, int x, int y, int z, uint8_t block){
  chunk->blocks[block_index(x, y, z)] = block;
  chunk->changed = 1;
}
