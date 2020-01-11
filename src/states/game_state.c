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

unsigned int shader, projection_location, viewLoc, cameraPosition_location;
unsigned int texture, texture_normal, texture_brickwall, texture_brickwall_specular, texture_brickwall_normal;
mat4 light_model = GLMS_MAT4_IDENTITY_INIT;
Skybox skybox;
drawable *plane;
drawable *object;
drawable *light;

void game_state_init(){
  printf("game state init\n");

  textures_init();

  plane = drawable_create(&models[PLANE]);
  object = drawable_create(&models[SPAWN_BEACON]);
  light = drawable_create(&models[SPHERE]);

  shader = shader_create(cube_vertex_shader_source, cube_fragment_shader_source);
  shader_bind(shader);

  texture = texture_create(SPAWNBEACON);
  texture_normal = texture_create(SPAWNBEACON_NORMAL);
  texture_brickwall = texture_create(BRICKWALL);
  texture_brickwall_specular = texture_create(BRICKWALL_SPECULAR);
  texture_brickwall_normal = texture_create(BRICKWALL_NORMAL);

  projection_location = shader_uniform_position(shader, "projection");

  viewLoc = shader_uniform_position(shader, "view");
  cameraPosition_location = shader_uniform_position(shader, "cameraPosition");
  shader_uniform1i(shader, "material.diffuse_texture", 0);
  shader_uniform1i(shader, "material.specular_texture", 1);
  shader_uniform1i(shader, "material.normal_texture", 2);
  shader_uniform1f(shader, "material.ambient", 0.1f);
  shader_uniform1f(shader, "material.diffuse", 1.0f);
  shader_uniform1f(shader, "material.specular", 1.2f);
  shader_uniform1f(shader, "material.shininess", 64.0f);

  glm_translate(light_model, (vec3){1.0f, 10.0f, -0.5f});
  glm_scale(light_model, (vec3){0.1f, 0.1f, 0.1f});

  shader_uniform3fv(shader, "dirLight.position", (vec3){1.0f, -10.0f, -0.5f});
  shader_uniform3fv(shader, "pointLights[0].position", (vec3){-4.0f, -2.0f, -4.0f});
  shader_uniform3fv(shader, "pointLights[1].position", (vec3){ 4.0f, -2.0f, -4.0f});
  shader_uniform3fv(shader, "pointLights[2].position", (vec3){-4.0f, -2.0f,  4.0f});
  shader_uniform3fv(shader, "pointLights[3].position", (vec3){ 4.0f, -2.0f,  4.0f});
  shader_uniform3fv(shader, "pointLights[0].color", (vec3){1.0f, 0.0f, 0.0f});
  shader_uniform3fv(shader, "pointLights[1].color", (vec3){0.0f, 1.0f, 0.0f});
  shader_uniform3fv(shader, "pointLights[2].color", (vec3){0.0f, 0.0f, 1.0f});
  shader_uniform3fv(shader, "pointLights[3].color", (vec3){1.0f, 1.0f, 0.0f});

  skybox_init();
  skybox_create(&skybox);

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void game_state_destroy(){
  printf("game state destroy\n");

  drawable_free(light);
  drawable_free(object);
  drawable_free(plane);

  texture_delete(&texture);
  texture_delete(&texture_normal);
  texture_delete(&texture_brickwall);
  texture_delete(&texture_brickwall_specular);
  texture_delete(&texture_brickwall_normal);
  shader_delete(shader);

  skybox_delete(&skybox);
  skybox_free();

  textures_free();

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void game_state_update(float deltaTime){
}

void game_state_draw(){
  texture_bind(texture, 0);
  texture_bind(texture_brickwall_specular, 1);
  if(use_normal_map == 1){
    texture_bind(texture_normal, 2);
  }else{
    texture_bind(0, 2);
  }
  shader_bind(shader);

  mat4 projection = GLMS_MAT4_IDENTITY_INIT;
  glm_perspective(glm_rad(65.0f), (float)cengine.width/(float)cengine.height, 0.1f, 100.0f, projection);
  shader_uniform_matrix4fv_at(projection_location, projection[0]);

  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view[0]);
  glUniform3fv(cameraPosition_location, 1, camera_position);

  mat4 model = GLMS_MAT4_IDENTITY_INIT;
  shader_uniform_matrix4fv(shader, "model", model[0]);
  drawable_draw(object);

  texture_bind(texture_brickwall, 0);
  if(use_normal_map == 1){
    texture_bind(texture_brickwall_normal, 2);
  }else{
    texture_bind(0, 2);
  }

  glm_scale(model, (vec3){10.0f, 0.0f, 10.0f});
  shader_uniform_matrix4fv(shader, "model", model[0]);
  drawable_draw(plane);

  texture_bind(0, 0);
  texture_bind(0, 1);
  texture_bind(0, 2);

  shader_uniform_matrix4fv(shader, "model", light_model[0]);
  // drawable_draw(light);

  skybox_projection(projection[0]);
  skybox_draw(&skybox);
}
