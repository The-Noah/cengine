#include "cengine.h"

#include <stdio.h>

GLFWwindow* window;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
  glViewport(0, 0, width, height);
}

int cengine_init(CEngine *cengine, struct CEngineOptions *options){
  if(glfwInit() != GLFW_TRUE){
    fprintf(stderr, "failed to init glfw\n");
    return -1;
  }

  if(!options->title){
    options->title = "cengine";
  }
  if(!options->width){
    options->width = 800;
  }
  if(!options->height){
    options->height = 600;
  }
  if(!options->antialiasing){
    options->antialiasing = 4;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, options->antialiasing);

  cengine->window = glfwCreateWindow(options->width, options->height, options->title, NULL, NULL);
  if(cengine->window == NULL){
    fprintf(stderr, "failed to create window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(cengine->window);
  glfwSetFramebufferSizeCallback(cengine->window, framebuffer_size_callback);

  glViewport(0, 0, 800, 600);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_MULTISAMPLE);
  glClearColor(0.3f, 0.6f, 0.8f, 1.0f);

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if(err != GLEW_OK){
    fprintf(stderr, "glew failed to init: %s\n", glewGetErrorString(err));
    return -1;
  }

  state_manager_init(&cengine->state_manager);

  cengine->quit = 0;
  return 0;
}

void cengine_update(CEngine *cengine){
  cengine->quit = glfwWindowShouldClose(cengine->window);
}

void cengine_free(CEngine *cengine){
  state_manager_free(&cengine->state_manager);
  glfwTerminate();
}