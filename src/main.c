#include "main.h"

#include <stdio.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "cengine.h"
#include "camera.h"
#include "states/game_state.h"
#include "states/menu_state.h"
#include "states/voxel_state.h"

double deltaTime = 0.0;
char firstMouse = 1;
float lastX = 400.0f, lastY = 300.0f;

CEngine cengine;
State voxel_state;

unsigned char escPress = 0;
unsigned char f1Press = 0;
unsigned char mousePress = 0;
void processInput(GLFWwindow *window){
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    escPress = 1;
  }else if(escPress == 1 && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE){
    char s = state_manager_pop(&cengine.state_manager);
    escPress = 0;
  }
  if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
    mousePress = 1;
  }else if(mousePress == 1 && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE){
    if(cengine.state_manager.top < 1){
      state_manager_push(&cengine.state_manager, &voxel_state);
    }
    mousePress = 0;
  }

  if(cengine.state_manager.top < 1){
    return;
  }

#if WALKING
  const float camera_speed = 10.0f * deltaTime;
#else
  float camera_speed = 25.0f * deltaTime;
  if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)){
    camera_speed *= 10.0f;
  }
#endif

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
  if(cengine.state_manager.top < 1){
    return;
  }

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

  // camera_update_rotation();
}

int main(){
  printf("starting\n");

  struct CEngineOptions options = {0};
  int init_status = cengine_init(&cengine, &options);
  if(init_status < 0){
    return init_status;
  }

  glfwSetCursorPosCallback(cengine.window, mouse_callback);

  State menu_state;
  menu_state.init = menu_state_init;
  menu_state.destroy = menu_state_destroy;
  menu_state.update = menu_state_update;
  menu_state.draw = menu_state_draw;
  state_manager_push(&cengine.state_manager, &menu_state);

  State game_state;
  game_state.init = game_state_init;
  game_state.destroy = game_state_destroy;
  game_state.update = game_state_update;
  game_state.draw = game_state_draw;

  voxel_state.init = voxel_state_init;
  voxel_state.destroy = voxel_state_destroy;
  voxel_state.update = voxel_state_update;
  voxel_state.draw = voxel_state_draw;

  camera_init();

  double lastPrintTime;
  double lastTime = lastPrintTime = glfwGetTime();
  unsigned short frames = 0;
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe;
  while(!cengine.quit){
    double currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    frames++;
    if(currentTime - lastPrintTime >= 1.0){
      printf("%.1fms (%dfps)\n", 1000.0/(double)frames, frames);
      frames = 0;
      lastPrintTime += 1.0;
    }

    cengine_update(&cengine);

    processInput(cengine.window);

    state_manager_update(&cengine.state_manager, deltaTime);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    state_manager_draw(&cengine.state_manager);

    if(cengine.state_manager.top == -1){
      glfwSetWindowShouldClose(cengine.window, 1);
    }

    glfwSwapBuffers(cengine.window);
    glfwPollEvents();
  }

  printf("end\n");
  cengine_free(&cengine);

  return 0;
}
