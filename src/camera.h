#ifndef CAMERA_H
#define CAMERA_H

#define CGLM_ALL_UNALIGNED
#include <cglm\cglm.h>
#include <cglm\struct.h>

vec3 camera_position;
mat4 view;
float camera_yaw;
float camera_pitch;

void camera_init();
void camera_update_position();
void camera_update_rotation();
void camera_move_forward(float amount);
void camera_move_right(float amount);

#endif
