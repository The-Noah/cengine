#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#define CGLM_ALL_UNALIGNED
#include <cglm\cglm.h>
#include <cglm\struct.h>

#include "shader.h"
#include "texture.h"
#include "camera.h"

GLFWwindow* window;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 400.0f, lastY = 300.0f;

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

const unsigned int cube_indices[] = {
  0, 1, 2, 2, 3, 0,
  4, 5, 6, 6, 7, 4,
  8, 9, 10, 10, 11, 8,
  12, 13, 14, 14, 15, 12,
  16, 17, 18, 18, 19, 16,
  20, 21, 22, 22, 23, 20
};

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

  const char* vertex_shader_source = "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in float brightness;\n"
    "out float vBrightness;\n"
    "out vec2 vTexCoord;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 view;\n"
    "uniform mat4 model;\n"
    "void main(){\n"
    "  gl_Position = projection * view * model * vec4(position, 1.0);\n"
    "  vBrightness = brightness;\n"
    "  if(brightness < 1.12){\n"
    "    vTexCoord = position.zy;\n"
    "  }else if(brightness < 1.22){\n"
    "    vTexCoord = position.xy;\n"
    "  }else{\n"
    "    vTexCoord = position.xz;\n"
    "  }\n"
    "}";

  const char* fragment_shader_source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in float vBrightness;\n"
    "in vec2 vTexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main(){\n"
    "  FragColor = texture(texture1, vTexCoord) * vec4(vec3(vBrightness), 1.0);\n"
    "}";

  unsigned int VAO, VBO, EBO;
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
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  unsigned int shader = shader_create(vertex_shader_source, fragment_shader_source);
  shader_bind(shader);

  glActiveTexture(GL_TEXTURE0);
  unsigned int texture = texture_create("grass.png");

  unsigned int modelLoc = glGetUniformLocation(shader, "model");
  mat4 model = GLMS_MAT4_IDENTITY_INIT;
  glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);

  unsigned int projectionLoc = glGetUniformLocation(shader, "projection");
  mat4 projection = GLMS_MAT4_IDENTITY_INIT;
  glm_perspective(glm_rad(65.0f), 800.0f/600.0f, 0.1f, 100.0f, projection);
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection[0]);

  unsigned int viewLoc = glGetUniformLocation(shader, "view");

  camera_update_rotation();

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe;
  while(!glfwWindowShouldClose(window)){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    camera_update();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture_bind(texture);
    shader_bind(shader);

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view[0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  texture_delete(&texture);
  shader_delete(shader);

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

  glfwTerminate();
  return 0;
}
