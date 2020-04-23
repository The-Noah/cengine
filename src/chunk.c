#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "renderer/utils.h"
#include "noise.h"
#include "main.h"
#include "blocks.h"

#define DEBUG

#define TEXTURE_SIZE 4

float half_pixel_correction(float coord){
  coord *= (1.0f / TEXTURE_SIZE); // convert texture pos to uv coord
  return coord;
  // return (coord + 0.5f) / TEXTURE_SIZE;
}

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

// Use magic numbers >.> to check if a block ID is transparent
unsigned char is_transparent(uint8_t block){
  switch(block){
    case 0:
    case 6:
      return 1;
    default:
      return 0;
  }
}

// Get the neighbors of this chunk to be referenced for faster chunk generation
void chunk_get_neighbors(struct chunk *chunk){
  if(chunk->px != NULL && chunk->nx != NULL && chunk->py != NULL && chunk->ny != NULL && chunk->pz != NULL && chunk->nz != NULL){
    return;
  }
  // Loop through all the chunks to check if it is a neighbouring chunk and set it as this chunks neighbour and this chunk as its neighbour
  for(unsigned short i = 0; i < chunk_count; i++){
    struct chunk *other = &chunks[i];

    if(chunk->px == NULL && other->x == chunk->x + 1 && other->y == chunk->y && other->z == chunk->z){
      chunk->px = other;
      chunk->changed = 1;
    }else if(chunk->nx == NULL && other->x == chunk->x - 1 && other->y == chunk->y && other->z == chunk->z){
      chunk->nx = other;
      chunk->changed = 1;
    }else if(chunk->py == NULL && other->x == chunk->x && other->y == chunk->y + 1 && other->z == chunk->z){
      chunk->py = other;
      chunk->changed = 1;
    }else if(chunk->ny == NULL && other->x == chunk->x && other->y == chunk->y - 1 && other->z == chunk->z){
      chunk->ny = other;
      chunk->changed = 1;
    }else if(chunk->pz == NULL && other->x == chunk->x && other->y == chunk->y && other->z == chunk->z + 1){
      chunk->pz = other;
      chunk->changed = 1;
    }else if(chunk->nz == NULL && other->x == chunk->x && other->y == chunk->y && other->z == chunk->z - 1){
      chunk->nz = other;
      chunk->changed = 1;
    }
  }
}

// Convert a local 3d position into a 1d block index
unsigned short block_index(uint8_t x, uint8_t y, uint8_t z){
  return x | (y << 5) | (z << 10);
}

// Initialize the chunk
struct chunk chunk_init(int x, int y, int z){
#ifdef DEBUG
    double start = glfwGetTime();
#endif // DEBUG

  
  // Allocate the required space for the chunk
  struct chunk* chunk = malloc(sizeof(struct chunk));
  chunk->blocks = malloc(CHUNK_SIZE_CUBED * sizeof(uint8_t));

  // Set no elements and no mesh changes but remesh the chunk
  chunk->elements = 0;
  chunk->changed = 1;
  chunk->mesh_changed = 0;

  // set this chunks position
  chunk->x = x;
  chunk->y = y;
  chunk->z = z;

  // Allocate space for vertices, lighting, brightness, 
  chunk->vertex = malloc(CHUNK_SIZE_CUBED * 2 * sizeof(byte4));
  chunk->brightness = malloc(CHUNK_SIZE_CUBED * 2 * sizeof(char));
  chunk->normal = malloc(CHUNK_SIZE_CUBED * 2 * sizeof(byte3));
  chunk->texCoords = malloc(CHUNK_SIZE_CUBED * 4 * sizeof(float));
  
  // Initialize all neighbours to null to make sure neighbors don't have a value
  chunk->px = NULL;
  chunk->nx = NULL;
  chunk->py = NULL;
  chunk->ny = NULL;
  chunk->pz = NULL;
  chunk->nz = NULL;

  glGenVertexArrays(1, &chunk->vao);

#ifdef DEBUG
  unsigned short count = 0;
#endif // DEBUG

  
  for(uint8_t dx = 0; dx < CHUNK_SIZE; dx++){
    for(uint8_t dz = 0; dz < CHUNK_SIZE; dz++){
      int cx = chunk->x * CHUNK_SIZE + dx;
      int cz = chunk->z * CHUNK_SIZE + dz;

      float f = simplex2(cx * 0.003f, cz * 0.003f, 6, 0.6f, 1.5f);
      int h = (f + 1) / 2 * (CHUNK_SIZE * 6 - 1) + 1;
      int rh = h - chunk->y * CHUNK_SIZE;

      for(uint8_t dy = 0; dy < CHUNK_SIZE; dy++){

        uint8_t thickness = rh - dy;
        uint8_t block = h < CHUNK_SIZE * 1.5 && thickness <= 3 ? 5 : thickness == 1 ? 1 : thickness <= 3 ? 3 : 2;

        chunk->blocks[block_index(dx, dy, dz)] = dy < rh ? h == 0 ? 4 : block : 0;
#ifdef DEBUG
        if (dy < h) {
            count++;
#endif // DEBUG

        

        }
      }
    }
  }

#ifdef DEBUG
  printf("chunk gen: %.2fms with %d blocks\n", (glfwGetTime() - start) * 1000.0, count);
#endif

  return *chunk;
}

// delete the chunk
void chunk_free(struct chunk *chunk){
    // Don't delete a null chunk
  if(chunk == NULL){
    printf("NOOOO!\n");
  }
  //Delete the vertex array
  glDeleteVertexArrays(1, &chunk->vao);

  // Delete the stored data
  free(chunk->blocks);
  free(chunk->vertex);
  free(chunk->brightness);
  free(chunk->normal);
  free(chunk->texCoords);

  // remove this chunk from it's neighbors
  if(chunk->px != NULL){
    chunk->px->nx = NULL;
  }
  if(chunk->nx != NULL){
    chunk->nx->px = NULL;
  }
  if(chunk->py != NULL){
    chunk->py->ny = NULL;
  }
  if(chunk->ny != NULL){
    chunk->ny->py = NULL;
  }
  if(chunk->pz != NULL){
    chunk->pz->nz = NULL;
  }
  if(chunk->nz != NULL){
    chunk->nz->pz = NULL;
  }

  // remove this chunk's pointers to adjacent chunks
  chunk->px = NULL;
  chunk->nx = NULL;
  chunk->py = NULL;
  chunk->ny = NULL;
  chunk->pz = NULL;
  chunk->nz = NULL;
}

// Update the chunk
unsigned char chunk_update(struct chunk *chunk){
    // Don't update a null chunk
  if(chunk == NULL){
    printf("NOOOO!\n");
  }
  //Make sure all the neighbours are valid
  chunk_get_neighbors(chunk);

  // if the chunk does not need to remesh then stop
  if(!chunk->changed){
    return 0;
  }
#ifdef DEBUG
  double start = glfwGetTime();
#endif // DEBUG

  // Remesh the chunk

  // The chunk has been remeshed
  chunk->changed = 0;

  // i is index for vertices and j is index for lighting and normals
  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int texCoord = 0;

  // Texure coordinates
  float du, dv;
  float a = 0.0f;
  float b = 1.0f;

  for(uint8_t y = 0; y < CHUNK_SIZE; y++){
    for(uint8_t x = 0; x < CHUNK_SIZE; x++){
      for(uint8_t z = 0; z < CHUNK_SIZE; z++){
        uint8_t block = chunk->blocks[block_index(x, y, z)];

        if(!block){
          continue;
        }

        // texture coords
        uint8_t w;

        
        //Add a face if -x is transparent
        if(is_transparent(chunk_get(chunk, x - 1, y, z))){
          // Get texture coordinates
          w = blocks[block][0];
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

          // set the vertex data for the face
          byte4_set(x, y, z, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z, block, chunk->vertex[i++]);
          byte4_set(x, y, z, block, chunk->vertex[i++]);
          byte4_set(x, y, z + 1, block, chunk->vertex[i++]);
          byte4_set(x, y + 1, z + 1, block, chunk->vertex[i++]);

          // Set the brightness data for the face
          for(int k = 0; k < 6; k++){
            chunk->brightness[j] = 0;
            byte3_set(-1, 0, 0, chunk->normal[j++]);
          }

          // set the texture data for the face
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // just repeat my comments for the first face to all 6 faces lol pls

        // +x
        if(is_transparent(chunk_get(chunk, x + 1, y, z))){
          w = blocks[block][1];
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

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

          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // -z
        if(is_transparent(chunk_get(chunk, x, y, z - 1))){
          w = blocks[block][4];
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

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

          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // +z
        if(is_transparent(chunk_get(chunk, x, y, z + 1))){
          w = blocks[block][5];
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

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

          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // -y
        if(is_transparent(chunk_get(chunk, x, y - 1, z))){
          w = blocks[block][3];
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

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

          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // +y
        if(is_transparent(chunk_get(chunk, x, y + 1, z))){
          w = blocks[block][2];
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

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

          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(a + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(a + dv);
          chunk->texCoords[texCoord++] = half_pixel_correction(b + du); chunk->texCoords[texCoord++] = half_pixel_correction(b + dv);
        }
      }
    }
  }

  // the mesh has been changed and the number of verticies is i
  chunk->elements = i;
  chunk->mesh_changed = 1;

#ifdef DEBUG
  printf("created chunk with %d vertices\n", i);

  printf("cm: %.2fms\n", (glfwGetTime() - start) * 1000.0);
#endif // DEBUG

  return 1;
}

// if the chunk's mesh has been modified then send the new data to opengl (TODO: don't create a new buffer, just reuse the old one)
void chunk_buffer_mesh(struct chunk *chunk){
    // if the mesh has not been modified then don't bother
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

  uint8_t block;

  // printf("%d %d %d\n", x, y, z);
  if(x < 0){
    // printf("nx\n");
    // for(unsigned short i = 0; i < chunk_count; i++){
    //   struct chunk *other = &chunks[i];

    //   if(other->x == chunk->x - 1 && other->y == chunk->y && other->z == chunk->z){
    //     return other->blocks[block_index(CHUNK_SIZE + x, y, z)];
    //   }
    // }
    // return 4;

    block = chunk->nx == NULL ? 4 : chunk->nx->blocks[block_index(CHUNK_SIZE + x, y, z)];
  }else if(x >= CHUNK_SIZE){
    // printf("px\n");
    // for(unsigned short i = 0; i < chunk_count; i++){
    //   struct chunk *other = &chunks[i];

    //   if(other->x == chunk->x + 1 && other->y == chunk->y && other->z == chunk->z){
    //     return other->blocks[block_index(x % CHUNK_SIZE, y, z)];
    //   }
    // }
    // return 4;

    block = chunk->px == NULL ? 4 : chunk->px->blocks[block_index(x % CHUNK_SIZE, y, z)];
  }else if(y < 0){
    // printf("ny\n");
    // for(unsigned short i = 0; i < chunk_count; i++){
    //   struct chunk *other = &chunks[i];

    //   if(other->x == chunk->x && other->y == chunk->y - 1 && other->z == chunk->z){
    //     return other->blocks[block_index(x, CHUNK_SIZE + y, z)];
    //   }
    // }
    // return 4;

    block = chunk->ny == NULL ? 4 : chunk->ny->blocks[block_index(x, CHUNK_SIZE + y, z)];
  }else if(y >= CHUNK_SIZE){
    // printf("py\n");
    // for(unsigned short i = 0; i < chunk_count; i++){
    //   struct chunk *other = &chunks[i];

    //   if(other->x == chunk->x && other->y == chunk->y + 1 && other->z == chunk->z){
    //     return other->blocks[block_index(x, y % CHUNK_SIZE, z)];
    //   }
    // }
    // return 4;

    block = chunk->py == NULL ? 4 : chunk->py->blocks[block_index(x, y % CHUNK_SIZE, z)];
  }else if(z < 0){
    // printf("nz\n");
    // for(unsigned short i = 0; i < chunk_count; i++){
    //   struct chunk *other = &chunks[i];

    //   if(other->x == chunk->x && other->y == chunk->y && other->z == chunk->z - 1){
    //     return other->blocks[block_index(x, y, CHUNK_SIZE + z)];
    //   }
    // }
    // return 4;

    block = chunk->nz == NULL ? 4 : chunk->nz->blocks[block_index(x, y, CHUNK_SIZE + z)];
  }else if(z >= CHUNK_SIZE){
    // printf("pz\n");
    // for(unsigned short i = 0; i < chunk_count; i++){
    //   struct chunk *other = &chunks[i];

    //   if(other->x == chunk->x && other->y == chunk->y && other->z == chunk->z + 1){
    //     return other->blocks[block_index(x, y, z % CHUNK_SIZE)];
    //   }
    // }
    // return 4;

    block = chunk->pz == NULL ? 4 : chunk->pz->blocks[block_index(x, y, z % CHUNK_SIZE)];
  }else{
    // printf("c\n");
    block = chunk->blocks[block_index(x, y, z)];
  }

  // printf("c %d\n", block);
  return block;
}

void chunk_set(struct chunk *chunk, int x, int y, int z, uint8_t block){
  printf("chunk set: %d %d\n", chunk->x, chunk->z);
  unsigned short access = block_index(x, y, z);
  uint8_t _block = chunk->blocks[access];
  printf("trying to set block %d %d %d from %d to %d\n", x, y, z, _block, block);
  if(_block == 4 || _block == block){
    return;
  }

  chunk->blocks[access] = block;
  chunk->changed = 1;

  if(x == 0 && chunk->nx != NULL){
    chunk->nx->changed = 1;
  }else if(x == CHUNK_SIZE - 1 && chunk->px != NULL){
    chunk->px->changed = 1;
  }else if(y == 0 && chunk->ny != NULL){
    chunk->ny->changed = 1;
  }else if(y == CHUNK_SIZE - 1 && chunk->py != NULL){
    chunk->py->changed = 1;
  }else if(z == 0 && chunk->nz != NULL){
    chunk->nz->changed = 1;
  }else if(z == CHUNK_SIZE - 1 && chunk->pz != NULL){
    chunk->pz->changed = 1;
  }
}
