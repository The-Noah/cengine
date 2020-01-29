#include "voxel_state.h"

#include <stdio.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define CGLM_ALL_UNALIGNED
#include <cglm/cglm.h>
#include <cglm/struct.h>

#include "../renderer/shader.h"
#include "../renderer/texture.h"
#include "../main.h"
#include "../camera.h"
#include "../skybox.h"
#include "../chunk.h"

#define MAX_CHUNKS 4096
#define CHUNK_CREATE_RADIUS 31
#define CHUNK_RENDER_RADIUS 31
#define CHUNK_DELETE_RADIUS 32
#define CHUNK_MAX_GENERATED_PER_FRAME 32

char* voxel_vertex_shader_source = ""
  #include "../shaders/voxel.vs"
;

char* voxel_fragment_shader_source = ""
  #include "../shaders/voxel.fs"
;

unsigned int shader, texture, projection_location, view_location, camera_position_location, light_direction_location, light_intensity_location;
Skybox skybox;
struct chunk *chunks[MAX_CHUNKS];
unsigned short chunk_count = 0;
vec3 light_direction = {0.0f, 1.0f, 0.0f};
float light_angle = 0.0f;

unsigned char reloadPress = 0;

void ensure_chunks(int x, int z){
  for(unsigned short i = 0; i < chunk_count; i++){
    struct chunk *chunk = chunks[i];
    int dx = x - chunk->x;
    int dz = z - chunk->z;

    if(abs(dx) >= CHUNK_DELETE_RADIUS || abs(dz) >= CHUNK_DELETE_RADIUS){
      chunk_free(chunk);

      struct chunk *other = chunks[chunk_count - 1];
      chunk->blocks = other->blocks;
      chunk->vao = other->vao;
      chunk->elements = other->elements;
      chunk->changed = other->changed;
      chunk->x = other->x;
      chunk->z = other->z;
      chunk_count--;
    }
  }

  unsigned short chunks_created = 0;
  for(char i = -CHUNK_CREATE_RADIUS; i <= CHUNK_CREATE_RADIUS; i++){
    if(chunks_created >= CHUNK_MAX_GENERATED_PER_FRAME){
      break;
    }

    for(char j = -CHUNK_CREATE_RADIUS; j <= CHUNK_CREATE_RADIUS; j++){
      if(chunks_created >= CHUNK_MAX_GENERATED_PER_FRAME){
        break;
      }

      int a = x + i;
      int b = z + j;
      unsigned char create = 1;

      for(unsigned short k = 0; k < chunk_count; k++){
        struct chunk *chunk = chunks[k];
        if(chunk->x == a && chunk->z == b){
          create = 0;
          break;
        }
      }

      if(create){
        struct chunk *chunk = chunk_init(a, b);
        chunks[chunk_count++] = chunk;
        chunks_created++;
      }
    }
  }
}

void voxel_state_init(){
  printf("voxel state init\n");

  shader = shader_create(voxel_vertex_shader_source, voxel_fragment_shader_source);
  shader_bind(shader);

  projection_location = shader_uniform_position(shader, "projection");
  view_location = shader_uniform_position(shader, "view");
  camera_position_location = shader_uniform_position(shader, "camera_position");
  light_direction_location = shader_uniform_position(shader, "light_direction");
  light_intensity_location = shader_uniform_position(shader, "light_intensity");
  shader_uniform1i(shader, "diffuse_texture", 0);

  texture = texture_create("tiles.png", GL_NEAREST);

  skybox_init();
  skybox_create(&skybox);

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void voxel_state_destroy(){
  printf("voxel state destroy\n");

  for(unsigned short i = 0; i < chunk_count; i++){
    chunk_free(chunks[i]);
  }

  skybox_delete(&skybox);
  skybox_free();

  texture_delete(&texture);
  shader_delete(shader);

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void voxel_state_update(float deltaTime){
  if(glfwGetKey(cengine.window, GLFW_KEY_F11) == GLFW_PRESS){
    reloadPress = 1;
  }else if(reloadPress == 1 && glfwGetKey(cengine.window, GLFW_KEY_F11) == GLFW_RELEASE){
    reloadPress = 0;
    printf("reloading...\n");
    voxel_state_destroy();
    voxel_state_init();
    printf("done\n");
  }

  light_angle += 10.0f * deltaTime;
  double light_angle_rad = glm_rad(light_angle);
  light_direction[0] = (float)sin(light_angle_rad);
  light_direction[1] = (float)cos(light_angle_rad);
}

void voxel_state_draw(){
  shader_bind(shader);
  texture_bind(texture, 0);

  float light_intensity = 1.0f;
  if(light_angle >= 90.0f){
    light_intensity = 0.0f;

    if(light_angle >= 360.0f){
      light_angle = -90.0f;
    }
  }else if(light_angle <= -80.0f || light_angle >= 80.0f){
    light_intensity = 1.0f - (float)(abs(light_angle) - 80.0f) / 10.0f;
  }

  glUniform3fv(light_direction_location, 1, light_direction);
  glUniform1f(light_intensity_location, light_intensity);

  mat4 projection = GLMS_MAT4_IDENTITY_INIT;
  glm_perspective(glm_rad(65.0f), (float)cengine.width/(float)cengine.height, 0.1f, SKYBOX_SIZE * 2.0f, projection);
  shader_uniform_matrix4fv_at(projection_location, projection[0]);

  glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
  glUniform3fv(camera_position_location, 1, camera_position);

  int x = floor(camera_position[0] / CHUNK_SIZE);
  int z = floor(camera_position[2] / CHUNK_SIZE);
  ensure_chunks(x, z);

  for(unsigned short i = 0; i < chunk_count; i++){
    struct chunk *chunk = chunks[i];
    int dx = x - chunk->x;
    int dz = z - chunk->z;

    if(abs(dx) > CHUNK_RENDER_RADIUS || abs(dz) > CHUNK_RENDER_RADIUS){
      continue;
    }

    mat4 model = GLMS_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){chunks[i]->x * CHUNK_SIZE, 0, chunks[i]->z * CHUNK_SIZE});
    shader_uniform_matrix4fv(shader, "model", model[0]);

    chunk_draw(chunks[i]);
  }

  skybox_projection(projection[0]);
  skybox_draw(&skybox);
}
