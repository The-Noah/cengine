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

const float cube_vertices[] = {
  // front
  1.0f, 1.0f, 1.0f, 1.2f,
  0.0f, 1.0f, 1.0f, 1.2f,
  0.0f, 0.0f, 1.0f, 1.2f,
  1.0f, 0.0f, 1.0f, 1.2f,
  // back
  0.0f, 1.0f, 0.0f, 1.2f,
  1.0f, 1.0f, 0.0f, 1.2f,
  1.0f, 0.0f, 0.0f, 1.2f,
  0.0f, 0.0f, 0.0f, 1.2f,
  // left
  0.0f, 1.0f, 1.0f, 1.1f,
  0.0f, 1.0f, 0.0f, 1.1f,
  0.0f, 0.0f, 0.0f, 1.1f,
  0.0f, 0.0f, 1.0f, 1.1f,
  // right
  1.0f, 1.0f, 0.0f, 1.1f,
  1.0f, 1.0f, 1.0f, 1.1f,
  1.0f, 0.0f, 1.0f, 1.1f,
  1.0f, 0.0f, 0.0f, 1.1f,
  // top
  1.0f, 1.0f, 0.0f, 1.3f,
  0.0f, 1.0f, 0.0f, 1.3f,
  0.0f, 1.0f, 1.0f, 1.3f,
  1.0f, 1.0f, 1.0f, 1.3f,
  // bottom
  0.0f, 0.0f, 0.0f, 1.3f,
  1.0f, 0.0f, 0.0f, 1.3f,
  1.0f, 0.0f, 1.0f, 1.3f,
  0.0f, 0.0f, 1.0f, 1.3f
};

const unsigned char cube_indices[] = {
  0, 1, 2, 2, 3, 0,
  4, 5, 6, 6, 7, 4,
  8, 9, 10, 10, 11, 8,
  12, 13, 14, 14, 15, 12,
  16, 17, 18, 18, 19, 16,
  20, 21, 22, 22, 23, 20
};

const char* cube_vertex_shader_source = ""
  #include "../shaders/cube.vs"
;

const char* cube_fragment_shader_source = ""
  #include "../shaders/cube.fs"
;

unsigned int VAO, VBO, EBO, shader, texture, modelLoc, projectionLoc, viewLoc;
Skybox skybox;

void game_state_init(){
  printf("game state init\n");

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // brightness
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

  shader = shader_create(cube_vertex_shader_source, cube_fragment_shader_source);
  shader_bind(shader);

  glActiveTexture(GL_TEXTURE0);
  texture = texture_create(GRASS);

  modelLoc = glGetUniformLocation(shader, "model");
  mat4 model = GLMS_MAT4_IDENTITY_INIT;
  glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);

  projectionLoc = glGetUniformLocation(shader, "projection");
  mat4 projection = GLMS_MAT4_IDENTITY_INIT;
  glm_perspective(glm_rad(65.0f), 800.0f/600.0f, 0.1f, 100.0f, projection);
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection[0]);

  viewLoc = glGetUniformLocation(shader, "view");

  mat4 skybox_proj = GLMS_MAT4_IDENTITY_INIT;
  glm_mat4_mul(skybox_proj, projection, skybox_proj);

  skybox_init(skybox_proj[0]);
  skybox_create(&skybox);
}

void game_state_destroy(){
  printf("game state destroy\n");

  texture_delete(&texture);
  shader_delete(shader);

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

  skybox_delete(&skybox);
  skybox_free();

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void game_state_update(float deltaTime){
}

void game_state_draw(){
  texture_bind(texture);
  shader_bind(shader);

  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view[0]);

  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);

  skybox_draw(&skybox);
}
