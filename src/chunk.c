#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define CGLM_ALL_UNALIGNED
#include <cglm/cglm.h>

#include "renderer/utils.h"
#include "noise.h"
#include "main.h"
#include "blocks.h"

#define TEXTURE_SIZE 4

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

unsigned char is_transparent(uint8_t block){
  switch(block){
    case 0:
    case 6:
      return 1;
    default:
      return 0;
  }
}

void chunk_get_neighbors(struct chunk *chunk){
  if(chunk->px != NULL && chunk->nx != NULL && chunk->pz != NULL && chunk->nz != NULL){
    return;
  }

  for(unsigned short i = 0; i < chunk_count; i++){
    struct chunk *other = chunks[i];

    if(other->x == chunk->x + 1 && other->z == chunk->z){
      chunk->px = other;
    }else if(other->x == chunk->x - 1 && other->z == chunk->z){
      chunk->nx = other;
    }else if(other->x == chunk->x && other->z == chunk->z + 1){
      chunk->pz = other;
    }else if(other->x == chunk->x && other->z == chunk->z - 1){
      chunk->nz = other;
    }
  }
}

unsigned short block_index(uint8_t x, uint8_t y, uint8_t z){
  return x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED;
}

struct chunk* chunk_init(int x, int z){
  struct chunk* chunk = malloc(sizeof(struct chunk));
  chunk->blocks = malloc(CHUNK_SIZE_CUBED * sizeof(uint8_t));
  chunk->elements = 0;
  chunk->changed = 1;
  chunk->mesh_changed = 0;
  chunk->x = x;
  chunk->z = z;
  chunk->vertex = malloc(CHUNK_SIZE_CUBED * 2 * sizeof(byte4));
  chunk->brightness = malloc(CHUNK_SIZE_CUBED * 2 * sizeof(char));
  chunk->normal = malloc(CHUNK_SIZE_CUBED * 2 * sizeof(byte3));
  chunk->texCoords = malloc(CHUNK_SIZE_CUBED * 4 * sizeof(float));
  chunk->px = NULL;
  chunk->nx = NULL;
  chunk->pz = NULL;
  chunk->nz = NULL;

  glGenVertexArrays(1, &chunk->vao);

  unsigned short count = 0;
  for(uint8_t dx = 0; dx < CHUNK_SIZE; dx++){
    for(uint8_t dz = 0; dz < CHUNK_SIZE; dz++){
      int cx = chunk->x * CHUNK_SIZE + dx;
      int cz = chunk->z * CHUNK_SIZE + dz;

      float f = simplex2(cx * 0.01f, cz * 0.01f, 4, 0.5f, 2.0f);
      int h = (f + 1) / 2 * (CHUNK_SIZE - 1) + 1;

      for(uint8_t dy = 0; dy < CHUNK_SIZE; dy++){
        uint8_t thickness = h - dy;
        uint8_t block = dy < 9 && thickness <= 3 ? 5 : thickness == 1 ? 1 : thickness <= 3 ? 3 : 2;
        chunk->blocks[block_index(dx, dy, dz)] = dy < h ? dy == 0 ? 4 : dy >= CHUNK_SIZE - 6 ? 6 : block : 0;
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
  free(chunk->vertex);
  free(chunk->brightness);
  free(chunk->normal);
  free(chunk->texCoords);
  chunk->px = NULL;
  chunk->nx = NULL;
  chunk->pz = NULL;
  chunk->nz = NULL;
}

unsigned char chunk_update(struct chunk *chunk){
  chunk_get_neighbors(chunk);
  if(chunk->px == NULL || chunk->nx == NULL || chunk->pz == NULL || chunk->nz == NULL){
    return 0;
  }

  if(!chunk->changed){
    return 0;
  }

  double start = glfwGetTime();

  chunk->changed = 0;

  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int texCoord = 0;

  float s = 1.0f / TEXTURE_SIZE;
  float du, dv;
  float a = 0.0f + 1.0f / 1024.0f;
  float b = s - 1.0f / 1024.0f;

  for(uint8_t y = 0; y < CHUNK_SIZE; y++){
    for(uint8_t x = 0; x < CHUNK_SIZE; x++){
      for(uint8_t z = 0; z < CHUNK_SIZE; z++){
        uint8_t block = chunk->blocks[block_index(x, y, z)];

        if(!block){
          continue;
        }

        uint8_t w;

        // -x
        if(is_transparent(chunk_get(chunk, x - 1, y, z))){
          w = blocks[block][0];
          du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;

          byte4_set(x, y, z, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z, block, chunk->vertex[i++]);
          byte4_set(x, y, z, block, chunk->vertex[i++]);
          byte4_set(x, y, z + 1, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, chunk->vertex[i++]);

          for(int k = 0; k < 6; k++){
            chunk->brightness[j] = 0;
            byte3_set(-1, 0, 0, chunk->normal[j++]);
          }

          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
        }

        // +x
        if(is_transparent(chunk_get(chunk, x + 1, y, z))){
          w = blocks[block][1];
          du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;

          byte4_set(x + 1, y, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, chunk->vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, chunk->vertex[i++]);
          byte4_set(x + 1, y, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, chunk->vertex[i++]);

          for(int k = 0; k < 6; k++){
            chunk->brightness[j] = 0;
            byte3_set(1, 0, 0, chunk->normal[j++]);
          }

          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = b + dv;
        }

        // -z
        if(is_transparent(chunk_get(chunk, x, y, z - 1))){
          w = blocks[block][4];
          du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;

          byte4_set(x, y, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y, z, block, chunk->vertex[i++]);
          byte4_set(x, y, z, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, chunk->vertex[i++]);

          for(int k = 0; k < 6; k++){
            chunk->brightness[j] = 1;
            byte3_set(0, 0, -1, chunk->normal[j++]);
          }

          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
        }

        // +z
        if(is_transparent(chunk_get(chunk, x, y, z + 1))){
          w = blocks[block][5];
          du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;

          byte4_set(x, y, z + 1, block, chunk->vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, chunk->vertex[i++]);
          byte4_set(x, y, z + 1, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, chunk->vertex[i++]);

          for(int k = 0; k < 6; k++){
            chunk->brightness[j] = 1;
            byte3_set(0, 0, 1, chunk->normal[j++]);
          }

          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = b + dv;
        }

        // -y
        if(is_transparent(chunk_get(chunk, x, y - 1, z))){
          w = blocks[block][3];
          du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;

          byte4_set(x, y, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, chunk->vertex[i++]);
          byte4_set(x, y, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y, z + 1, block, chunk->vertex[i++]);
          byte4_set(x, y, z + 1, block, chunk->vertex[i++]);

          for(int k = 0; k < 6; k++){
            chunk->brightness[j] = 2;
            byte3_set(0, -1, 0, chunk->normal[j++]);
          }

          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = b + dv;
        }

        // +y
        if(is_transparent(chunk_get(chunk, x, y + 1, z))){
          w = blocks[block][2];
          du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;

          byte4_set(x, y + 1, z, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z + 1, block, chunk->vertex[i++]);
          byte4_set(x + 1, y + 1, z, block, chunk->vertex[i++]);

          for(int k = 0; k < 6; k++){
            chunk->brightness[j] = 2;
            byte3_set(0, 1, 0, chunk->normal[j++]);
          }

          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = a + du; chunk->texCoords[texCoord++] = b + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = a + dv;
          chunk->texCoords[texCoord++] = b + du; chunk->texCoords[texCoord++] = b + dv;
        }
      }
    }
  }

  chunk->elements = i;
  chunk->mesh_changed = 1;
  // printf("created chunk with %d vertices\n", i);

  // printf("cm: %.2fms\n", (glfwGetTime() - start) * 1000.0);

  return 1;
}

void chunk_buffer_mesh(struct chunk *chunk){
  if(!chunk->mesh_changed){
    return;
  }

  glBindVertexArray(chunk->vao);

  unsigned int vertex_buffer = make_buffer(GL_ARRAY_BUFFER, chunk->elements * sizeof(*chunk->vertex), chunk->vertex);
  glVertexAttribPointer(0, 4, GL_BYTE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  unsigned int brightness_buffer = make_buffer(GL_ARRAY_BUFFER, chunk->elements * sizeof(*chunk->brightness), chunk->brightness);
  glVertexAttribIPointer(1, 1, GL_BYTE, 0, 0);
  glEnableVertexAttribArray(1);

  unsigned int normal_buffer = make_buffer(GL_ARRAY_BUFFER, chunk->elements * sizeof(*chunk->normal), chunk->normal);
  glVertexAttribPointer(2, 3, GL_BYTE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(2);

  unsigned int texure_buffer = make_buffer(GL_ARRAY_BUFFER, chunk->elements * 2 * sizeof(*chunk->texCoords), chunk->texCoords);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(3);

  glBindVertexArray(0);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &brightness_buffer);
  glDeleteBuffers(1, &normal_buffer);
  glDeleteBuffers(1, &texure_buffer);

  chunk->mesh_changed = 0;
}

void chunk_draw(struct chunk *chunk){
  if(!chunk->elements){
    return;
  }

  chunk_buffer_mesh(chunk);

  glBindVertexArray(chunk->vao);
  glDrawArrays(GL_TRIANGLES, 0, chunk->elements);
}

uint8_t chunk_get(struct chunk *chunk, int x, int y, int z){
  if(y < 0 || y >= CHUNK_SIZE){
    return 0;
  }

  // if(x < 0 || x >= CHUNK_SIZE ||
  //    z < 0 || z >= CHUNK_SIZE){
  //   vec2i block_global_position = {floor(chunk->x * CHUNK_SIZE + x), floor(chunk->z * CHUNK_SIZE + z)};
  //   vec2i chunk_position = {floor(block_global_position[0] / CHUNK_SIZE), floor(block_global_position[1] / CHUNK_SIZE)};
  //   vec2i block_local_position = {
  //     (CHUNK_SIZE + (block_global_position[0] % CHUNK_SIZE)) % CHUNK_SIZE,
  //     (CHUNK_SIZE + (block_global_position[1] % CHUNK_SIZE)) % CHUNK_SIZE
  //   };

  //   unsigned short access = block_index(block_local_position[0], y, block_local_position[1]);

  //   for(unsigned short i = 0; i < chunk_count; i++){
  //     struct chunk *other = chunks[i];
  //     if(other->x == chunk_position[0] && other->z == chunk_position[1]){
  //       return other->blocks[access];
  //     }
  //   }

  //   return 0;
  // }
  if(x < 0){
    return chunk->nx->blocks[block_index(CHUNK_SIZE + x, y, z)];
  }else if(x >= CHUNK_SIZE){
    return chunk->px->blocks[block_index(x % CHUNK_SIZE, y, z)];
  }else if(z < 0){
    return chunk->nz->blocks[block_index(x, y, CHUNK_SIZE + z)];
  }else if(z >= CHUNK_SIZE){
    return chunk->pz->blocks[block_index(x, y, z % CHUNK_SIZE)];
  }

  return chunk->blocks[block_index(x, y, z)];
}

void chunk_set(struct chunk *chunk, int x, int y, int z, uint8_t block){
  unsigned short access = block_index(x, y, z);
  uint8_t _block = chunk->blocks[access];
  if(_block == 4 || _block == block){
    return;
  }

  chunk->blocks[access] = block;
  chunk->changed = 1;

  if(x == 0 && chunk->nx != NULL){
    chunk->nx->changed = 1;
  }else if(x == CHUNK_SIZE - 1 && chunk->px != NULL){
    chunk->px->changed = 1;
  }else if(z == 0 && chunk->nz != NULL){
    chunk->nz->changed = 1;
  }else if(z == CHUNK_SIZE - 1 && chunk->pz != NULL){
    chunk->pz->changed = 1;
  }
}
