#ifndef TEXTURE_H
#define TEXTURE_H

#include "../textures.h"

unsigned int texture_create(texture_id texture_index);
void texture_bind(unsigned int texture);
void texture_delete(unsigned int *texture);

#endif
