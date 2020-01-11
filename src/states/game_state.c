#include "game_state.h"

#include <stdio.h>

#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#define CGLM_ALL_UNALIGNED
#include <cglm\cglm.h>
#include <cglm\struct.h>

#include "../renderer/shader.h"
#include "../renderer/texture.h"
#include "../camera.h"
#include "../main.h"
#include "../textures.h"
#include "../skybox.h"
#include "../cengine.h"
#include "../models.h"

const char* cube_vertex_shader_source = ""
  #include "../shaders/standard.vs"
;

const char* cube_fragment_shader_source = ""
  #include "../shaders/standard.fs"
;

unsigned int VAO, VBO, shader, texture, projection_location, viewLoc, cameraPosition_location;
Skybox skybox;

void game_state_init(){
  printf("game state init\n");

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * models[0].vertex_count, models[0].vertices, GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // tex coord
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // normal
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  glDeleteBuffers(1, &VBO);

  shader = shader_create(cube_vertex_shader_source, cube_fragment_shader_source);
  shader_bind(shader);

  glActiveTexture(GL_TEXTURE0);
  texture = texture_create(GRASS);

  mat4 model = GLMS_MAT4_IDENTITY_INIT;
  glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
  shader_uniform_matrix4fv(shader, "model", model[0]);

  projection_location = shader_uniform_position(shader, "projection");

  viewLoc = shader_uniform_position(shader, "view");
  cameraPosition_location = shader_uniform_position(shader, "cameraPosition");
  shader_uniform1f(shader, "material.ambient", 0.1f);
  shader_uniform1f(shader, "material.diffuse", 1.0f);
  shader_uniform1f(shader, "material.specular", 0.5f);
  shader_uniform1f(shader, "material.shininess", 16.0f);

  skybox_init();
  skybox_create(&skybox);

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void game_state_destroy(){
  printf("game state destroy\n");

  texture_delete(&texture);
  shader_delete(shader);

  glDeleteVertexArrays(1, &VAO);

  skybox_delete(&skybox);
  skybox_free();

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void game_state_update(float deltaTime){
}

void game_state_draw(){
  texture_bind(texture);
  shader_bind(shader);

  mat4 projection = GLMS_MAT4_IDENTITY_INIT;
  glm_perspective(glm_rad(65.0f), (float)cengine.width/(float)cengine.height, 0.1f, 100.0f, projection);
  shader_uniform_matrix4fv_at(projection_location, projection[0]);

  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view[0]);
  glUniform3fv(cameraPosition_location, 1, camera_position);

  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, models[0].vertex_count);

  skybox_projection(projection[0]);
  skybox_draw(&skybox);
}
