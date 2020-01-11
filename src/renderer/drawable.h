#ifndef DRAWABLE_H
#define DRAWABLE_H

#define CGLM_ALL_UNALIGNED
#include <cglm/cglm.h>

#include "../models.h"

typedef struct{
  unsigned int vao;
  unsigned short vertex_count;
} drawable;

drawable *drawable_create(struct model *model);
void drawable_free(drawable *drawable);
void drawable_draw(drawable *drawable);

#endif
