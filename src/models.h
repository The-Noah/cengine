#ifndef MODELS_H
#define MODELS_H

typedef enum{
  CUBE = 0,
  PLANE,
  SPHERE,
  MONKEY,
  SPAWN_BEACON
} model_id;

struct model{
  float *vertices;
  unsigned short vertex_count;
} models[5];

#endif
