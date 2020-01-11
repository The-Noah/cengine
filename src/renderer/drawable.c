#include "drawable.h"

#include <stdlib.h>

#define GLEW_STATIC
#include "GL/glew.h"

#include "shader.h"

drawable *drawable_create(struct model *model){
  drawable *drawable = malloc(sizeof(drawable));
  drawable->vertex_count = model->vertex_count;

  glGenVertexArrays(1, &drawable->vao);

  unsigned int vbo;
  glGenBuffers(1, &vbo);

  glBindVertexArray(drawable->vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * model->vertex_count, model->vertices, GL_STATIC_DRAW);

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
  glDeleteBuffers(1, &vbo);

  return drawable;
}

void drawable_free(drawable *drawable){
  glDeleteVertexArrays(1, &drawable->vao);
  drawable->vao = 0;
  drawable->vertex_count = 0;
}

void drawable_draw(drawable *drawable){
  glBindVertexArray(drawable->vao);
  glDrawArrays(GL_TRIANGLES, 0, drawable->vertex_count);
}
