#include "game_state.h"

#include <stdio.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define CGLM_ALL_UNALIGNED
#include <cglm/cglm.h>
#include <cglm/struct.h>

#include "../renderer/shader.h"
#include "../renderer/texture.h"
#include "../renderer/drawable.h"
#include "../camera.h"
#include "../main.h"
#include "../textures.h"
#include "../skybox.h"
#include "../cengine.h"
#include "../models.h"

#define MODEL_NUMBER PLANE

const char* cube_vertex_shader_source = ""
  #include "../shaders/standard.vs"
;

const char* cube_fragment_shader_source = ""
  #include "../shaders/standard.fs"
;

unsigned int shader, texture, texture_specular, texture_normal, projection_location, viewLoc, cameraPosition_location;
Skybox skybox;
drawable *plane;

void game_state_init(){
  printf("game state init\n");

  textures_init();

  plane = drawable_create(&models[PLANE]);

  shader = shader_create(cube_vertex_shader_source, cube_fragment_shader_source);
  shader_bind(shader);

  texture = texture_create(BRICKWALL);
  texture_specular = texture_create(BRICKWALL_SPECULAR);
  texture_normal = texture_create(BRICKWALL_NORMAL);

  mat4 model = GLMS_MAT4_IDENTITY_INIT;
  glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
  shader_uniform_matrix4fv(shader, "model", model[0]);

  projection_location = shader_uniform_position(shader, "projection");

  viewLoc = shader_uniform_position(shader, "view");
  cameraPosition_location = shader_uniform_position(shader, "cameraPosition");
  shader_uniform1i(shader, "material.diffuse_texture", 0);
  shader_uniform1i(shader, "material.specular_texture", 1);
  shader_uniform1i(shader, "material.normal_texture", 2);
  shader_uniform1f(shader, "material.ambient", 0.1f);
  shader_uniform1f(shader, "material.diffuse", 1.1f);
  shader_uniform1f(shader, "material.specular", 2.0f);
  shader_uniform1f(shader, "material.shininess", 128.0f);

  skybox_init();
  skybox_create(&skybox);

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void game_state_destroy(){
  printf("game state destroy\n");

  drawable_free(plane);

  texture_delete(&texture);
  texture_delete(&texture_specular);
  texture_delete(&texture_normal);
  shader_delete(shader);

  skybox_delete(&skybox);
  skybox_free();

  textures_free();

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void game_state_update(float deltaTime){
}

void game_state_draw(){
  glActiveTexture(GL_TEXTURE0);
  texture_bind(texture);
  glActiveTexture(GL_TEXTURE1);
  texture_bind(texture_specular);
  glActiveTexture(GL_TEXTURE2);
  texture_bind(texture_normal);
  shader_bind(shader);

  mat4 projection = GLMS_MAT4_IDENTITY_INIT;
  glm_perspective(glm_rad(65.0f), (float)cengine.width/(float)cengine.height, 0.1f, 100.0f, projection);
  shader_uniform_matrix4fv_at(projection_location, projection[0]);

  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view[0]);
  glUniform3fv(cameraPosition_location, 1, camera_position);

  drawable_draw(plane);

  skybox_projection(projection[0]);
  skybox_draw(&skybox);
}
