#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#define CGLM_ALL_UNALIGNED
#include <cglm\cglm.h>
#include <cglm\struct.h>

#include "renderer/shader.h"
#include "renderer/texture.h"
#include "camera.h"
#include "state_manager.h"
#include "states/game_state.h"

GLFWwindow* window;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 400.0f, lastY = 300.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window){
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, 1);
  }
  const float camera_speed = 3.0f * deltaTime;
  if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    camera_move_forward(camera_speed);
  }
  if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
    camera_move_forward(-camera_speed);
  }
  if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    camera_move_right(-camera_speed);
  }
  if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
    camera_move_right(camera_speed);
  }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
  if(firstMouse){
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; 
  lastX = xpos;
  lastY = ypos;

  float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  camera_yaw += xoffset;
  camera_pitch += yoffset;

  if(camera_pitch > 89.0f){
    camera_pitch = 89.0f;
  }else if(camera_pitch < -89.0f){
    camera_pitch = -89.0f;
  }

  camera_update_rotation();
}

int main(){
  printf("starting\n");

  if(glfwInit() != GLFW_TRUE){
    fprintf(stderr, "failed to init glfw\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #endif

  window = glfwCreateWindow(800, 600, "cengine", NULL, NULL);
  if(window == NULL){
    fprintf(stderr, "failed to create window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);

  glEnable(GL_DEPTH_TEST);

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if(err != GLEW_OK){
    fprintf(stderr, "glew failed to init: %s\n", glewGetErrorString(err));
    return -1;
  }

  StateManager state_manager;
  state_manager_init(&state_manager);

  State game_state = {0};
  game_state.init = game_state_init;
  game_state.destroy = game_state_destroy;
  game_state.update = game_state_update;
  game_state.draw = game_state_draw;

  state_manager_push(&state_manager, &game_state);

  camera_update_rotation();

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe;
  while(!glfwWindowShouldClose(window)){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    state_manager_update(&state_manager, deltaTime);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    state_manager_draw(&state_manager);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  state_manager_free(&state_manager);

  glfwTerminate();
  return 0;
}
