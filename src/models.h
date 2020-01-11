#ifndef MODELS_H
#define MODELS_H

#define CGLM_ALL_UNALIGNED
#include <cglm\cglm.h>
#include <cglm\struct.h>

struct model{
  float *vertices;
  unsigned int vertex_count;
} models[3];

#endif
