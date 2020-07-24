#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#define GLEW_STATIC
#include "GL/glew.h"

class Chunk;

#include "states/voxel_state.h"

#define CHUNK_SIZE 32
#define CHUNK_SIZE_SQUARED 1024
#define CHUNK_SIZE_CUBED 32768

typedef GLbyte byte4[4];
typedef GLbyte byte3[3];
typedef int vec2i[2];

class Chunk{
public:
  void init(int _x, int _y, int _z);
  void destroy();
  unsigned char update();
  void draw();
  uint8_t get(int _x, int _y, int _z);
  void set(int _x, int _y, int _z, uint8_t block);

  uint8_t *blocks;
  unsigned int vao;
  int elements;
  bool changed;
  bool mesh_changed;
  int x;
  int y;
  int z;
  byte4 *vertex;
  char *brightness;
  byte3 *normal;
  float *texCoords;
  // neighboring chunks
  Chunk *px;
  Chunk *nx;
  Chunk *py;
  Chunk *ny;
  Chunk *pz;
  Chunk *nz;

private:
  void buffer_mesh();
};

// struct chunk{
//   uint8_t *blocks;
//   unsigned int vao;
//   int elements;
//   uint8_t changed;
//   uint8_t mesh_changed;
//   int x;
//   int y;
//   int z;
//   byte4 *vertex;
//   char *brightness;
//   byte3 *normal;
//   float *texCoords;
//   // neighboring chunks
//   struct chunk *px;
//   struct chunk *nx;
//   struct chunk *py;
//   struct chunk *ny;
//   struct chunk *pz;
//   struct chunk *nz;
// };

unsigned short block_index(uint8_t x, uint8_t y, uint8_t z);
// struct chunk chunk_init(int x, int y, int z);
// void chunk_free(struct chunk *chunk);
// unsigned char chunk_update(struct chunk *chunk);
// void chunk_draw(struct chunk *chunk);
// uint8_t chunk_get(struct chunk *chunk, int x, int y, int z);
// void chunk_set(struct chunk *chunk, int x, int y, int z, uint8_t block);

#endif
