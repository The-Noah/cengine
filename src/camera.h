#ifndef CAMERA_H
#define CAMERA_H

#define CGLM_ALL_UNALIGNED
#include <cglm/cglm.h>
#include <cglm/struct.h>

extern vec3 camera_position;
extern mat4 view;
extern float camera_yaw;
extern float camera_pitch;
extern float camera_dy;

void camera_init();
void camera_update();
void camera_update_position();
void camera_update_rotation();
void camera_move_forward(float amount);
void camera_move_right(float amount);

#endif
