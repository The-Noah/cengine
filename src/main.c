#include <stdio.h>
#include <math.h>

#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "cengine.h"
#include "camera.h"
#include "states/game_state.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;
char firstMouse = 1;
float lastX = 400.0f, lastY = 300.0f;

void processInput(GLFWwindow *window){
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, 1);
  }
  const float camera_speed = 5.0f * deltaTime;
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
    firstMouse = 0;
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

  struct CEngineOptions options = {0};
  CEngine cengine;
  int init_status = cengine_init(&cengine, &options);
  if(init_status < 0){
    return init_status;
  }

  glfwSetCursorPosCallback(cengine.window, mouse_callback);
  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  State game_state;
  game_state.init = game_state_init;
  game_state.destroy = game_state_destroy;
  game_state.update = game_state_update;
  game_state.draw = game_state_draw;

  state_manager_push(&cengine.state_manager, &game_state);

  camera_init();

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe;
  while(!cengine.quit){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    cengine_update(&cengine);

    processInput(cengine.window);

    state_manager_update(&cengine.state_manager, deltaTime);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    state_manager_draw(&cengine.state_manager);

    glfwSwapBuffers(cengine.window);
    glfwPollEvents();
  }

  cengine_free(&cengine);

  return 0;
}
