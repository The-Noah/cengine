#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "renderer/utils.h"
#include "noise.h"
#include "main.h"
#include "blocks.h"

#define TEXTURE_SIZE 4
#define VOID_BLOCK 0 // block id if there is no neighbor for block
// #define PRINT_TIMING

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

// use magic numbers >.> to check if a block ID is transparent
bool is_transparent(uint8_t block){
  switch(block){
    case 0:
    case 6:
      return true;
    default:
      return false;
  }
}

// get the neighbors of this chunk to be referenced for faster chunk generation
// void chunk_get_neighbors(struct chunk *chunk){
//   // return;
//   // if we have all chunk neighbors no need to find any
//   if(chunk->px != NULL && chunk->nx != NULL && chunk->py != NULL && chunk->ny != NULL && chunk->pz != NULL && chunk->nz != NULL){
//     return;
//   }

//   // loop through all the chunks to check if it is a neighbouring chunk and set it as this chunks neighbour and this chunk as its neighbour
//   for(unsigned short i = 0; i < chunk_count; i++){
//     struct chunk *other = &chunks[i];

//     if(chunk->px == NULL && other->x == chunk->x + 1 && other->y == chunk->y && other->z == chunk->z){
//       chunk->px = other;
//       chunk->changed = 1;
//     }else if(chunk->nx == NULL && other->x == chunk->x - 1 && other->y == chunk->y && other->z == chunk->z){
//       chunk->nx = other;
//       chunk->changed = 1;
//     }else if(chunk->py == NULL && other->x == chunk->x && other->y == chunk->y + 1 && other->z == chunk->z){
//       chunk->py = other;
//       chunk->changed = 1;
//     }else if(chunk->ny == NULL && other->x == chunk->x && other->y == chunk->y - 1 && other->z == chunk->z){
//       chunk->ny = other;
//       chunk->changed = 1;
//     }else if(chunk->pz == NULL && other->x == chunk->x && other->y == chunk->y && other->z == chunk->z + 1){
//       chunk->pz = other;
//       chunk->changed = 1;
//     }else if(chunk->nz == NULL && other->x == chunk->x && other->y == chunk->y && other->z == chunk->z - 1){
//       chunk->nz = other;
//       chunk->changed = 1;
//     }
//   }
// }

// convert a local 3d position into a 1d block index
unsigned short block_index(uint8_t x, uint8_t y, uint8_t z){
  return x | (y << 5) | (z << 10);
}

void Chunk::init(int _x, int _y, int _z){
  #if defined DEBUG && defined PRINT_TIMING
    double start = glfwGetTime();
#endif

  // allocate the required space for the chunk
  blocks = (uint8_t*)malloc(CHUNK_SIZE_CUBED * sizeof(uint8_t));

  // set no elements and no mesh changes but remesh the chunk
  elements = 0;
  changed = true;
  mesh_changed = false;

  // set this chunks position
  x = _x;
  y = _y;
  z = _z;

  // allocate space for vertices, lighting, brightness, 
  vertex = (byte4*)malloc(CHUNK_SIZE_CUBED * 2 * sizeof(byte4));
  brightness = (char*)malloc(CHUNK_SIZE_CUBED * 2 * sizeof(char));
  normal = (byte3*)malloc(CHUNK_SIZE_CUBED * 2 * sizeof(byte3));
  texCoords = (float*)malloc(CHUNK_SIZE_CUBED * 4 * sizeof(float));

  // initialize all neighbours to null to make sure neighbors don't have a value
  px = NULL;
  nx = NULL;
  py = NULL;
  ny = NULL;
  pz = NULL;
  nz = NULL;

  glGenVertexArrays(1, &vao);

#if defined DEBUG && defined PRINT_TIMING
  unsigned short count = 0;
#endif

  for(uint8_t dx = 0; dx < CHUNK_SIZE; dx++){
    for(uint8_t dz = 0; dz < CHUNK_SIZE; dz++){
      int cx = x * CHUNK_SIZE + dx;
      int cz = z * CHUNK_SIZE + dz;

      float f = simplex2(cx * 0.003f, cz * 0.003f, 6, 0.6f, 1.5f);
      int h = pow((f + 1) / 2  + 1, 9);
      int rh = h - y * CHUNK_SIZE;

      for(uint8_t dy = 0; dy < CHUNK_SIZE; dy++){
        uint8_t thickness = rh - dy;
        uint8_t block = h < CHUNK_SIZE / 4 && thickness <= 3 ? 5 : thickness == 1 ? 1 : thickness <= 3 ? 3 : 2;

        blocks[block_index(dx, dy, dz)] = dy < rh ? h == 0 ? 4 : block : 0;
#if defined DEBUG && defined PRINT_TIMING
        if(dy < h){
          count++;
        }
#endif
      }
    }
  }

#if defined DEBUG && defined PRINT_TIMING
  printf("chunk gen: %.2fms with %d blocks\n", (glfwGetTime() - start) * 1000.0, count);
#endif
}

void Chunk::destroy(){
  // delete the vertex array
  glDeleteVertexArrays(1, &vao);

  // delete the stored data
  free(blocks);
  free(vertex);
  free(brightness);
  free(normal);
  free(texCoords);

  // remove this chunk from it's neighbors
  if(px != NULL){
    px->nx = NULL;
  }
  if(nx != NULL){
    nx->px = NULL;
  }
  if(py != NULL){
    py->ny = NULL;
  }
  if(ny != NULL){
    ny->py = NULL;
  }
  if(pz != NULL){
    pz->nz = NULL;
  }
  if(nz != NULL){
    nz->pz = NULL;
  }

  // remove this chunk's pointers to adjacent chunks
  px = NULL;
  nx = NULL;
  py = NULL;
  ny = NULL;
  pz = NULL;
  nz = NULL;
}

unsigned char Chunk::update(){
  // find chunk neighbors if needed
  // get_neighbors();

  // if the chunk does not need to remesh then stop
  if(!changed){
    return 0;
  }

#if defined DEBUG && defined PRINT_TIMING
  double start = glfwGetTime();
#endif

  // updating is taken care of - reset flag
  changed = false;

  unsigned int i = 0; // vertex index
  unsigned int j = 0; // lighting and normal index
  unsigned int texCoord = 0;

  // texure coordinates
  float du, dv;
  float a = 0.0f;
  float b = 1.0f;

  for(uint8_t _y = 0; _y < CHUNK_SIZE; _y++){
    for(uint8_t _x = 0; x < CHUNK_SIZE; _x++){
      for(uint8_t _z = 0; z < CHUNK_SIZE; _z++){
        uint8_t block = blocks[block_index(_x, _y, _z)];

        if(!block){
          continue;
        }

        // texture coords
        uint8_t w;
  printf("a\n");

        // add a face if -x is transparent
        if(is_transparent(get(_x - 1, _y, _z))){
          w = BLOCKS[block][0]; // get texture coordinates
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

          // set the vertex data for the face
          byte4_set(_x, _y, _z, block, vertex[i++]);
          byte4_set(_x, _y + 1, _z + 1, block, vertex[i++]);
          byte4_set(_x, _y + 1, _z, block, vertex[i++]);
          byte4_set(_x, _y, _z, block, vertex[i++]);
          byte4_set(_x, _y, _z + 1, block, vertex[i++]);
          byte4_set(_x, _y + 1, _z + 1, block, vertex[i++]);

          // set the brightness data for the face
          for(int k = 0; k < 6; k++){
            brightness[j] = 0;
            byte3_set(-1, 0, 0, normal[j++]);
          }

          // set the texture data for the face
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // add a face if +x is transparent
        if(is_transparent(get(_x + 1, _y, _z))){
          w = BLOCKS[block][1]; // get texture coordinates
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

          // set the vertex data for the face
          byte4_set(_x + 1, _y, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z + 1, block, vertex[i++]);
          byte4_set(_x + 1, _y, _z + 1, block, vertex[i++]);
          byte4_set(_x + 1, _y, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z + 1, block, vertex[i++]);

          // set the brightness data for the face
          for(int k = 0; k < 6; k++){
            brightness[j] = 0;
            byte3_set(1, 0, 0, normal[j++]);
          }

          // set the texture data for the face
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // add a face if -z is transparent
        if(is_transparent(get(_x, _y, _z - 1))){
          w = BLOCKS[block][4]; // get texture coordinates
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

          // set the vertex data for the face
          byte4_set(_x, _y, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y, _z, block, vertex[i++]);
          byte4_set(_x, _y, _z, block, vertex[i++]);
          byte4_set(_x, _y + 1, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z, block, vertex[i++]);

          // set the brightness data for the face
          for(int k = 0; k < 6; k++){
            brightness[j] = 1;
            byte3_set(0, 0, -1, normal[j++]);
          }

          // set the texture data for the face
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // add a face if +z is transparent
        if(is_transparent(get(_x, _y, _z + 1))){
          w = BLOCKS[block][5]; // get texture coordinates
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

          // set the vertex data for the face
          byte4_set(_x, _y, _z + 1, block, vertex[i++]);
          byte4_set(_x + 1, _y, _z + 1, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z + 1, block, vertex[i++]);
          byte4_set(_x, _y, _z + 1, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z + 1, block, vertex[i++]);
          byte4_set(_x, _y + 1, _z + 1, block, vertex[i++]);

          // set the brightness data for the face
          for(int k = 0; k < 6; k++){
            brightness[j] = 1;
            byte3_set(0, 0, 1, normal[j++]);
          }

          // set the texture data for the face
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // add a face if -y is transparent
        if(is_transparent(get(_x, _y - 1, _z))){
          w = BLOCKS[block][3]; // get texture coordinates
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

          // set the vertex data for the face
          byte4_set(_x, _y, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y, _z + 1, block, vertex[i++]);
          byte4_set(_x, _y, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y, _z + 1, block, vertex[i++]);
          byte4_set(_x, _y, _z + 1, block, vertex[i++]);

          // set the brightness data for the face
          for(int k = 0; k < 6; k++){
  printf("b\n");
            brightness[j] = 2;
  printf("c\n");
            byte3_set(0, -1, 0, normal[j++]);
          }

          // set the texture data for the face
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
        }

        // add a face if +y is transparent
        if(is_transparent(get(_x, _y + 1, _z))){
          w = BLOCKS[block][2]; // get texture coordinates
          // du = (w % TEXTURE_SIZE) * s; dv = (w / TEXTURE_SIZE) * s;
          du = w % TEXTURE_SIZE; dv = w / TEXTURE_SIZE;

          // set the vertex data for the face
          byte4_set(_x, _y + 1, _z, block, vertex[i++]);
          byte4_set(_x, _y + 1, _z + 1, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z + 1, block, vertex[i++]);
          byte4_set(_x, _y + 1, _z, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z + 1, block, vertex[i++]);
          byte4_set(_x + 1, _y + 1, _z, block, vertex[i++]);

          // set the brightness data for the face
          for(int k = 0; k < 6; k++){
            brightness[j] = 2;
            byte3_set(0, 1, 0, normal[j++]);
          }

          // set the texture data for the face
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(a + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(a + dv);
          texCoords[texCoord++] = half_pixel_correction(b + du); texCoords[texCoord++] = half_pixel_correction(b + dv);
        }
      }
    }
  }

  elements = i; // set number of vertices
  mesh_changed = true; // set mesh has changed flag

#if defined DEBUG && defined PRINT_TIMING
  printf("created chunk with %d vertices in %.2fms\n", i, (glfwGetTime() - start) * 1000.0);
#endif

  return 1;
}

void Chunk::buffer_mesh(){
  // if the mesh has not been modified then don't bother
  if(!mesh_changed){
    return;
  }

  glBindVertexArray(vao);

  unsigned int vertex_buffer = make_buffer(GL_ARRAY_BUFFER, elements * sizeof(*vertex), vertex);
  glVertexAttribPointer(0, 4, GL_BYTE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  unsigned int brightness_buffer = make_buffer(GL_ARRAY_BUFFER, elements * sizeof(*brightness), brightness);
  glVertexAttribIPointer(1, 1, GL_BYTE, 0, 0);
  glEnableVertexAttribArray(1);

  unsigned int normal_buffer = make_buffer(GL_ARRAY_BUFFER, elements * sizeof(*normal), normal);
  glVertexAttribPointer(2, 3, GL_BYTE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(2);

  unsigned int texure_buffer = make_buffer(GL_ARRAY_BUFFER, elements * 2 * sizeof(*texCoords), texCoords);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(3);

  glBindVertexArray(0);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &brightness_buffer);
  glDeleteBuffers(1, &normal_buffer);
  glDeleteBuffers(1, &texure_buffer);

  mesh_changed = false;
}

void Chunk::draw(){
  // don't draw if chunk has no mesh
  printf("a\n");
  if(!elements){
    return;
  }
  printf("b\n");

  buffer_mesh();

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, elements);
}

uint8_t Chunk::get(int _x, int _y, int _z){
  uint8_t block;

  if(x < 0){
    block = nx == NULL ? VOID_BLOCK : nx->blocks[block_index(CHUNK_SIZE + _x, _y, _z)];
  }else if(x >= CHUNK_SIZE){
    block = px == NULL ? VOID_BLOCK : px->blocks[block_index(_x % CHUNK_SIZE, _y, _z)];
  }else if(y < 0){
    block = ny == NULL ? VOID_BLOCK : ny->blocks[block_index(_x, CHUNK_SIZE + _y, _z)];
  }else if(y >= CHUNK_SIZE){
    block = py == NULL ? VOID_BLOCK : py->blocks[block_index(_x, _y % CHUNK_SIZE, _z)];
  }else if(z < 0){
    block = nz == NULL ? VOID_BLOCK : nz->blocks[block_index(_x, _y, CHUNK_SIZE + _z)];
  }else if(z >= CHUNK_SIZE){
    block = pz == NULL ? VOID_BLOCK : pz->blocks[block_index(_x, _y, _z % CHUNK_SIZE)];
  }else{
    block = blocks[block_index(_x, _y, _z)];
  }

  return block;
}

void Chunk::set(int _x, int _y, int _z, uint8_t block){
  printf("chunk set: %d %d\n", x, z);
  unsigned short access = block_index(_x, _y, _z);
  uint8_t _block = blocks[access];
  printf("trying to set block %d %d %d from %d to %d\n", _x, _y, _z, _block, block);
  if(_block == 4 || _block == block){
    return;
  }

  blocks[access] = block;
  changed = true;

  if(_x == 0 && nx != NULL){
    nx->changed = true;
  }else if(_x == CHUNK_SIZE - 1 && px != NULL){
    px->changed = true;
  }else if(_y == 0 && ny != NULL){
    ny->changed = true;
  }else if(_y == CHUNK_SIZE - 1 && py != NULL){
    py->changed = true;
  }else if(_z == 0 && nz != NULL){
    nz->changed = true;
  }else if(_z == CHUNK_SIZE - 1 && pz != NULL){
    pz->changed = 1;
  }
}
