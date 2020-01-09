#include "menu_state.h"

#include <stdio.h>

#define GLEW_STATIC
#include <GL\glew.h>
#define CGLM_ALL_UNALIGNED
#include <cglm\cglm.h>
#include <cglm\struct.h>

#include "../renderer/shader.h"
#include "../renderer/texture.h"
#include "../camera.h"


const float quad_vertices[] = {
  // front
   0.5f,  0.5f,
  -0.5f,  0.5f,
  -0.5f, -0.5f,
   0.5f, -0.5f
};

const unsigned int quad_indices[] = {
  0, 1, 2, 2, 3, 0
};

const char* vertex_shader_source = "#version 330 core\n"
  "layout (location = 0) in vec2 position;\n"
  "uniform mat4 projection;\n"
  "uniform mat4 view;\n"
  "uniform mat4 model;\n"
  "void main(){\n"
  "  gl_Position = projection * view * model * vec4(position, 0.0, 1.0);\n"
  "}";

const char* fragment_shader_source = "#version 330 core\n"
  "out vec4 FragColor;\n"
  "void main(){\n"
  "  FragColor = vec4(1.0);\n"
  "}";

unsigned int menu_VAO, menu_VBO, menu_EBO, menu_shader, menu_modelLoc, menu_projectionLoc, menu_viewLoc;

void menu_state_init(){
  printf("menu state init\n");

  glGenVertexArrays(1, &menu_VAO);
  glGenBuffers(1, &menu_VBO);
  glGenBuffers(1, &menu_EBO);

  glBindVertexArray(menu_VAO);

  glBindBuffer(GL_ARRAY_BUFFER, menu_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, menu_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  menu_shader = shader_create(vertex_shader_source, fragment_shader_source);
  shader_bind(menu_shader);

  menu_modelLoc = glGetUniformLocation(menu_shader, "model");
  mat4 model = GLMS_MAT4_IDENTITY_INIT;
  glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
  glUniformMatrix4fv(menu_modelLoc, 1, GL_FALSE, (float*)model);

  menu_projectionLoc = glGetUniformLocation(menu_shader, "projection");
  mat4 projection = GLMS_MAT4_IDENTITY_INIT;
  glm_perspective(glm_rad(65.0f), 800.0f/600.0f, 0.1f, 100.0f, projection);
  glUniformMatrix4fv(menu_projectionLoc, 1, GL_FALSE, projection[0]);

  menu_viewLoc = glGetUniformLocation(menu_shader, "view");
}

void menu_state_destroy(){
  printf("menu state destroy\n");

  shader_delete(menu_shader);

  glDeleteVertexArrays(1, &menu_VAO);
  glDeleteBuffers(1, &menu_VBO);
  glDeleteBuffers(1, &menu_EBO);
}

void menu_state_update(float deltaTime){
}

void menu_state_draw(){
  shader_bind(menu_shader);

  glUniformMatrix4fv(menu_viewLoc, 1, GL_FALSE, view[0]);

  glBindVertexArray(menu_VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
