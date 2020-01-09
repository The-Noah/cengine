#include "camera.h"

#include <math.h>
#include <stdio.h>

vec3 camera_position = {0.0f, 0.0f, 3.0f};
mat4 view = GLMS_MAT4_IDENTITY_INIT;
float camera_yaw = -90.0f;
float camera_pitch = 0.0f;

vec3 camera_front = {0.0f, 0.0f, -1.0f};

void camera_init(){
  camera_update_rotation();
}

void camera_update_position(){
  vec3 center = GLMS_VEC3_ZERO_INIT;
  glm_vec3_add(camera_position, camera_front, center);
  glm_lookat(camera_position, center, (vec3){0.0f, 1.0f, 0.0f}, view);
}

void camera_update_rotation(){
  vec3 front = GLMS_VEC3_ZERO_INIT;
  front[0] = cos(glm_rad(camera_yaw)) * cos(glm_rad(camera_pitch));
  front[1] = sin(glm_rad(camera_pitch));
  front[2] = sin(glm_rad(camera_yaw)) * cos(glm_rad(camera_pitch));
  glm_normalize(front);
  glm_vec3_copy(front, camera_front);

  camera_update_position();
}

void camera_move_forward(float amount){
  glm_vec3_add(camera_position, (vec3){camera_front[0] * amount, camera_front[1] * amount, camera_front[2] * amount}, camera_position);

  camera_update_position();
}

void camera_move_right(float amount){
  vec3 a = GLMS_VEC3_ZERO_INIT;
  glm_cross(camera_front, (vec3){0.0f, 1.0f, 0.0f}, a);
  glm_normalize(a);
  glm_vec3_add(camera_position, (vec3){a[0] * amount, a[1] * amount, a[2] * amount}, camera_position);

  camera_update_position();
}
