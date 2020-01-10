#include "skybox.h"

#include <stdlib.h>

#define GLEW_STATIC
#include <GL\glew.h>

#include "renderer/shader.h"
#include "camera.h"

unsigned int skybox_shader;

#define SKYBOX_SIZE 50

const char skybox_vertices[] = {
   SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,
  -SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,
  -SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,
   SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,
  -SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,
   SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,
   SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
  -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
  -SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,
  -SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,
  -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
  -SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,
   SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,
   SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,
   SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,
   SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
   SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,
  -SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,
  -SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,
   SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,
  -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
   SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
   SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,
  -SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE
};

const unsigned char skybox_indices[] = {
  2, 1, 0, 0, 3, 2,
  6, 5, 4, 4, 7, 6,
  10, 9, 8, 8, 11, 10,
  14, 13, 12, 12, 15, 14,
  18, 17, 16, 16, 19, 18,
  22, 21, 20, 20, 23, 22
};

const char* skybox_vertex_shader_source = ""
  #include "shaders/skybox.vs"
;

const char* skybox_fragment_shader_source = ""
  #include "shaders/skybox.fs"
;

void skybox_init(float* projection_matrix){
  skybox_shader = shader_create(skybox_vertex_shader_source, skybox_fragment_shader_source);
  shader_bind(skybox_shader);
  glUniformMatrix4fv(glGetUniformLocation(skybox_shader, "projection"), 1, GL_FALSE, projection_matrix);
}

void skybox_free(){
  shader_delete(skybox_shader);
}

void skybox_create(Skybox *skybox){
  unsigned int vbo, ebo;
  glGenVertexArrays(1, &skybox->vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(skybox->vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skybox_indices), skybox_indices, GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, 3 * sizeof(char), (void*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
}

void skybox_delete(Skybox *skybox){
  glDeleteVertexArrays(1, &skybox->vao);
  free(skybox);
}

void skybox_draw(Skybox *skybox){
  shader_bind(skybox_shader);
  glUniformMatrix4fv(glGetUniformLocation(skybox_shader, "view"), 1, GL_FALSE, view[0]);

  glBindVertexArray(skybox->vao);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
}
