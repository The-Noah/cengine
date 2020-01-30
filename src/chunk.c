#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>

#define CGLM_ALL_UNALIGNED
#include <cglm/cglm.h>

#include "renderer/utils.h"
#include "noise.h"

#define MAX(a, b) ((a) >= (b)) ? (a) : (b)

void byte4_set(GLbyte x, GLbyte y, GLbyte z, GLbyte w, byte4 dest){
  dest[0] = x;
  dest[1] = y;
  dest[2] = z;
  dest[3] = w;
}

void byte3_set(GLbyte x, GLbyte y, GLbyte z, byte3 dest){
  dest[0] = x;
  dest[1] = y;
  dest[2] = z;
}

unsigned short block_index(uint8_t x, uint8_t y, uint8_t z){
  return x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED;
}

struct chunk* chunk_init(int x, int z){
  struct chunk* chunk = malloc(sizeof(struct chunk));
  chunk->blocks = malloc(CHUNK_SIZE_CUBED * sizeof(uint8_t));
  chunk->elements = 0;
  chunk->changed = 1;
  chunk->x = x;
  chunk->z = z;
  glGenVertexArrays(1, &chunk->vao);

  unsigned int count = 0;
  for(uint8_t dx = 0; dx < CHUNK_SIZE; dx++){
    for(uint8_t dz = 0; dz < CHUNK_SIZE; dz++){
      int cx = chunk->x * CHUNK_SIZE + dx;
      int cz = chunk->z * CHUNK_SIZE + dz;

      float f = simplex2(cx * 0.01f, cz * 0.01f, 4, 0.5f, 2.0f);
      int h = (f + 1) / 2 * (CHUNK_SIZE - 1) + 1;

      for(uint8_t dy = 0; dy < CHUNK_SIZE; dy++){
        uint8_t thickness = h - dy;
        uint8_t block = thickness == 1 ? 1 : thickness <= 3 ? 3 : 2;
        chunk->blocks[block_index(dx, dy, dz)] = dy < h ? dy == 0 ? 4 : block : 0;
        if(dy < h){
          count++;
        }
      }
    }
  }

  // printf("generated chunk at %d %d with %d blocks\n", x, z, count);

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
  byte3 *normal = malloc(CHUNK_SIZE_CUBED * 2 * sizeof(byte3));
  float *texCoords = malloc(CHUNK_SIZE_CUBED * 4 * sizeof(float));
  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int texCoord = 0;

  float s = 0.25f;
  float du, dv;
  float a = 0.0f;
  float b = s;

  for(uint8_t y = 0; y < CHUNK_SIZE; y++){
    for(uint8_t x = 0; x < CHUNK_SIZE; x++){
      for(uint8_t z = 0; z < CHUNK_SIZE; z++){
        uint8_t block = chunk->blocks[block_index(x, y, z)];

        if(!block){
          continue;
        }

        uint8_t w = block - 1;

        // -x
        if(chunk_get(chunk, x - 1, y, z) == 0){
          du = w * s; dv = s;

          byte4_set(x, y, z, block, vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x, y + 1, z, block, vertex[i++]);
          byte4_set(x, y, z, block, vertex[i++]);
          byte4_set(x, y, z + 1, block, vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, vertex[i++]);

          for(int k = 0; k < 6; k++){
            brightness[j] = 0;
            byte3_set(-1, 0, 0, normal[j++]);
          }

          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
        }

        // +x
        if(chunk_get(chunk, x + 1, y, z) == 0){
          du = w * s; dv = s;

          byte4_set(x + 1, y, z, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y, z, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);

          for(int k = 0; k < 6; k++){
            brightness[j] = 0;
            byte3_set(1, 0, 0, normal[j++]);
          }

          texCoords[texCoord++] = b + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = b + dv;
        }

        // -z
        if(chunk_get(chunk, x, y, z - 1) == 0){
          du = w * s ; dv = s;

          byte4_set(x, y, z, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, vertex[i++]);
          byte4_set(x + 1, y, z, block, vertex[i++]);
          byte4_set(x, y, z, block, vertex[i++]);
          byte4_set(x, y + 1, z, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, vertex[i++]);

          for(int k = 0; k < 6; k++){
            brightness[j] = 1;
            byte3_set(0, 0, -1, normal[j++]);
          }

          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
        }

        // +z
        if(chunk_get(chunk, x, y, z + 1) == 0){
          du = w * s ; dv = s;

          byte4_set(x, y, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x, y, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, vertex[i++]);

          for(int k = 0; k < 6; k++){
            brightness[j] = 1;
            byte3_set(0, 0, 1, normal[j++]);
          }

          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = b + dv;
        }

        // -y
        if(chunk_get(chunk, x, y - 1, z) == 0){
          du = w * s; dv = 0.0f;

          byte4_set(x, y, z, block, vertex[i++]);
          byte4_set(x + 1, y, z, block, vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, vertex[i++]);
          byte4_set(x, y, z, block, vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, vertex[i++]);
          byte4_set(x, y, z + 1, block, vertex[i++]);

          for(int k = 0; k < 6; k++){
            brightness[j] = 2;
            byte3_set(0, -1, 0, normal[j++]);
          }

          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = b + dv;
        }

        // +y
        if(chunk_get(chunk, x, y + 1, z) == 0){
          du = w * s; dv = s + s;

          byte4_set(x, y + 1, z, block, vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x, y + 1, z, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, vertex[i++]);

          for(int k = 0; k < 6; k++){
            brightness[j] = 2;
            byte3_set(0, 1, 0, normal[j++]);
          }

          texCoords[texCoord++] = a + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = a + du; texCoords[texCoord++] = b + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = a + dv;
          texCoords[texCoord++] = b + du; texCoords[texCoord++] = b + dv;
        }
      }
    }
  }

  chunk->elements = i;

  glBindVertexArray(chunk->vao);

  unsigned int vertex_buffer = make_buffer(GL_ARRAY_BUFFER, chunk->elements * sizeof(*vertex), vertex);
  glVertexAttribPointer(0, 4, GL_BYTE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  unsigned int brightness_buffer = make_buffer(GL_ARRAY_BUFFER, j * sizeof(*brightness), brightness);
  glVertexAttribIPointer(1, 1, GL_BYTE, 0, 0);
  glEnableVertexAttribArray(1);

  unsigned int normal_buffer = make_buffer(GL_ARRAY_BUFFER, j * sizeof(*normal), normal);
  glVertexAttribPointer(2, 3, GL_BYTE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(2);

  unsigned int texure_buffer = make_buffer(GL_ARRAY_BUFFER, texCoord * sizeof(*texCoords), texCoords);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(3);

  glBindVertexArray(0);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &brightness_buffer);
  glDeleteBuffers(1, &normal_buffer);
  glDeleteBuffers(1, &texure_buffer);

  free(vertex);
  free(brightness);
  free(normal);
  free(texCoords);
}

unsigned char chunk_draw(struct chunk *chunk){
  unsigned char updated = chunk->changed;
  if(chunk->changed){
    chunk_update(chunk);
  }

  if(!chunk->elements){
    return updated;
  }

  glBindVertexArray(chunk->vao);
  glDrawArrays(GL_TRIANGLES, 0, chunk->elements);

  return updated;
}

uint8_t chunk_get(struct chunk *chunk, int x, int y, int z){
  if(y < 0 || y >= CHUNK_SIZE){
    return 0;
  }

  if(x < 0 || x >= CHUNK_SIZE ||
     z < 0 || z >= CHUNK_SIZE){
    vec2i block_global_position = {floor(chunk->x * CHUNK_SIZE + x), floor(chunk->z * CHUNK_SIZE + z)};
    vec2i chunk_position = {floor(block_global_position[0] / CHUNK_SIZE), floor(block_global_position[1] / CHUNK_SIZE)};
    vec2i block_local_position = {
      (CHUNK_SIZE + (block_global_position[0] % CHUNK_SIZE)) % CHUNK_SIZE,
      (CHUNK_SIZE + (block_global_position[1] % CHUNK_SIZE)) % CHUNK_SIZE
    };

    unsigned short access = block_index(block_local_position[0], y, block_local_position[1]);

    for(unsigned short i = 0; i < chunk_count; i++){
      struct chunk *other = chunks[i];
      if(other->x == chunk_position[0] && other->z == chunk_position[1]){
        return other->blocks[access];
      }
    }

    return 0;
  }

  return chunk->blocks[block_index(x, y, z)];
}

void set(struct chunk *chunk, int x, int y, int z, uint8_t block){
  chunk->blocks[block_index(x, y, z)] = block;
  chunk->changed = 1;
}
