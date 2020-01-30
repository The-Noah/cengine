#ifndef VOXEL_STATE_H
#define VOXEL_STATE_H

#include "../chunk.h"

#define MAX_CHUNKS 4096
#define CHUNK_CREATE_RADIUS 17

struct chunk *chunks[MAX_CHUNKS];
unsigned short chunk_count;

void voxel_state_init();
void voxel_state_destroy();
void voxel_state_update(float deltaTime);
void voxel_state_draw();

#endif
