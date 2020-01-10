#include "shader.h"

#include <stdio.h>

#define GLEW_STATIC
#include <GL\glew.h>

unsigned int shader_create(const char* vertex_source, const char* fragment_source){
  unsigned int vertex_shader;
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_source, NULL);
  glCompileShader(vertex_shader);

  unsigned int fragment_shader;
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_source, NULL);
  glCompileShader(fragment_shader);

  unsigned int shader_program;
  shader_program = glCreateProgram();

  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return shader_program;
}

void shader_bind(unsigned int shader){
  glUseProgram(shader);
}

void shader_delete(unsigned int shader){
  glDeleteProgram(shader);
  shader = 0;
}

// uniform setters

void shader_uniform1f(unsigned int shader, const char* name, float value){
  glUniform1f(glGetUniformLocation(shader, name), value);
}

void shader_uniform_matrix4fv(unsigned int shader, const char* name, float* value){
  glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE, value);
}
