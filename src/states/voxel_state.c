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

char* voxel_vertex_shader_source = ""
  #include "../shaders/voxel.vs"
;

char* voxel_fragment_shader_source = ""
  #include "../shaders/voxel.fs"
;

unsigned int shader, projection_location, view_location, texture;
Skybox skybox;
struct chunk* chunk;

unsigned char reloadPress = 0;

void voxel_state_init(){
  printf("voxel state init\n");

  shader = shader_create(voxel_vertex_shader_source, voxel_fragment_shader_source);
  shader_bind(shader);

  projection_location = shader_uniform_position(shader, "projection");
  view_location = shader_uniform_position(shader, "view");
  shader_uniform1i(shader, "diffuse_texture", 0);

  texture = texture_create("grass.png", GL_NEAREST);

  chunk = chunk_init();

  skybox_init();
  skybox_create(&skybox);

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void voxel_state_destroy(){
  printf("voxel state destroy\n");

  chunk_free(chunk);

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
}

void voxel_state_draw(){
  shader_bind(shader);
  texture_bind(texture, 0);

  mat4 projection = GLMS_MAT4_IDENTITY_INIT;
  glm_perspective(glm_rad(65.0f), (float)cengine.width/(float)cengine.height, 0.1f, 100.0f, projection);
  shader_uniform_matrix4fv_at(projection_location, projection[0]);

  glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);

  mat4 model = GLMS_MAT4_IDENTITY_INIT;
  shader_uniform_matrix4fv(shader, "model", model[0]);

  chunk_draw(chunk);

  skybox_projection(projection[0]);
  skybox_draw(&skybox);
}
