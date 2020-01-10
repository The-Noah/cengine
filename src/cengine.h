#ifndef CENEGINE_H
#define CENEGINE_H

#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "state_manager.h"

struct CEngineOptions{
  char *title;
  int width;
  int height;
  unsigned char antialiasing;
};

typedef struct{
  char quit;
  GLFWwindow* window;
  StateManager state_manager;
} CEngine;

int cengine_init(CEngine *cengine, struct CEngineOptions *options);
void cengine_update(CEngine *cengine);
void cengine_free(CEngine *cengine);

#endif
