#include "voxel_state.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define CGLM_ALL_UNALIGNED
#include <cglm/cglm.h>
#include <cglm/struct.h>

#include "../renderer/shader.h"
#include "../renderer/texture.h"
#include "../main.h"
#include "../camera.h"
#include "../skybox.h"

#define CHUNK_RENDER_RADIUS 16
#define CHUNK_DELETE_RADIUS 18
#define MAX_CHUNKS_GENERATED_PER_FRAME 32

const static int8_t FACES[6][3] = {
  { 1, 0, 0},
  {-1, 0, 0},
  { 0, 1, 0},
  { 0,-1, 0},
  { 0, 0, 1},
  { 0, 0,-1}
};

const static char* voxel_vertex_shader_source = ""
  #include "../shaders/voxel.vs"
;

const static char* voxel_fragment_shader_source = ""
  #include "../shaders/voxel.fs"
;

struct chunk *chunks[MAX_CHUNKS];
unsigned short chunk_count = 0;
unsigned char running = 1;

unsigned int shader, texture, projection_location, view_location, camera_position_location, light_direction_location, light_intensity_location;
Skybox skybox;
vec3 light_direction = {0.0f, 1.0f, 0.1f};
float light_angle = 0.0f;
mat4 projection = GLMS_MAT4_IDENTITY_INIT;

pthread_t chunk_update_thread_id;

unsigned char reloadPress = 0;

uint8_t current_block = 1;

float dti(float val){
  return fabsf(val - roundf(val));
}

void get_sight_vector(float rx, float ry, float *vx, float *vy, float *vz){
  float m = cosf(ry);
  *vx = cosf(rx - glm_rad(90.0f)) * m;
  *vy = sinf(ry);
  *vz = sinf(rx - glm_rad(90.0f)) * m;
}

int _hit_test(uint8_t *blocks, float max_distance, float x, float y, float z, float vx, float vy, float vz, int *hx, int *hy, int *hz){
  uint8_t m = 8;
  int px = 0;
  int py = 0;
  int pz = 0;

  for(unsigned short i = 0; i < max_distance * m; i++){
    int nx = round(x);
    int ny = round(y);
    int nz = round(z);

    if(nx != px || ny != py || nz != pz){
      if(blocks[block_index(nx % CHUNK_SIZE, ny, nz % CHUNK_SIZE)]){
        printf("hit block %d at %d %d\n", blocks[block_index(nx % CHUNK_SIZE, ny, nz % CHUNK_SIZE)], nx % CHUNK_SIZE, nz % CHUNK_SIZE);
        *hx = nx;
        *hy = ny;
        *hz = nz;

        return 1;
      }
    }

    x += vx / m;
    y += vy / m;
    z += vz / m;
  }

  return 0;
}

int hit_test(float x, float y, float z, float rx, float ry, int *bx, int *by, int *bz){
  int result = 0;
  float best = 0.0f;

  int cx = round(x) / CHUNK_SIZE;
  int cz = round(z) / CHUNK_SIZE;

  float vx, vy, vz;
  get_sight_vector(rx, ry, &vx, &vy, &vz);

  for(unsigned short i = 0; i < chunk_count; i++){
    struct chunk *chunk = chunks[i];
    int dx = chunk->x - cx;
    int dz = chunk->z - cz;
    if(abs(dx) > 1 || abs(dz) > 1){
      continue;
    }

    int hx, hy, hz;
    if(_hit_test(chunk->blocks, 4.0f, x, y, z, vx, vy, vz, &hx, &hy, &hz)){
      float d = sqrtf(powf(hx - x, 2.0f) + powf(hy - y, 2.0f) + powf(hz - z, 2.0f));
      if(best == 0 || d < best){
        best = d;
        *bx = hx;
        *by = hy;
        *bz = hz;
      }

      result = 1;
    }
  }

  return result;
}

void* chunk_update_thread(){
  while(running){
    unsigned short chunk_meshes_generated = 0;
    for(unsigned short i = 0; i < chunk_count; i++){
      if(chunk_meshes_generated >= MAX_CHUNKS_GENERATED_PER_FRAME){
        break;
      }

      chunk_meshes_generated += chunk_update(chunks[i]);
    }

#ifdef _WIN32
    Sleep(0);
#else
    sleep(0);
#endif
  }

  return NULL;
}

void ensure_chunks(int x, int z){
  for(unsigned short i = 0; i < chunk_count; i++){
    struct chunk *chunk = chunks[i];
    int dx = x - chunk->x;
    int dz = z - chunk->z;

    if(abs(dx) >= CHUNK_DELETE_RADIUS || abs(dz) >= CHUNK_DELETE_RADIUS){
      chunk_free(chunk);

      struct chunk *other = chunks[--chunk_count];
      memcpy(chunk, other, sizeof(struct chunk));
    }
  }

  for(char i = -CHUNK_CREATE_RADIUS; i <= CHUNK_CREATE_RADIUS; i++){
    for(char j = -CHUNK_CREATE_RADIUS; j <= CHUNK_CREATE_RADIUS; j++){
      int a = x + i;
      int b = z + j;
      unsigned char create = 1;

      for(unsigned short k = 0; k < chunk_count; k++){
        struct chunk *chunk = chunks[k];
        if(chunk->x == a && chunk->z == b){
          create = 0;
          break;
        }
      }

      if(create){
        if(chunk_count >= MAX_CHUNKS){
          printf("reached max number of chunks\n");
          return;
        }

        struct chunk *chunk = chunk_init(a, b);
        chunks[chunk_count++] = chunk;
      }
    }
  }
}

#if WALKING
uint8_t get_block(int x, int y, int z){
  vec2i chunk_position = {floor(x / CHUNK_SIZE), floor(z / CHUNK_SIZE)};
  vec2i block_local_position = {
    (CHUNK_SIZE + (x % CHUNK_SIZE)) % CHUNK_SIZE,
    (CHUNK_SIZE + (x % CHUNK_SIZE)) % CHUNK_SIZE
  };

  unsigned short access = block_index(block_local_position[0], y, block_local_position[1]);

  for(unsigned short i = 0; i < chunk_count; i++){
    struct chunk *other = chunks[i];
    if(other->x == chunk_position[0] && other->z == chunk_position[1]){
      return other->blocks[access];
    }
  }
}

uint8_t _collide(uint8_t height, float *_x, float *_y, float *_z){
  uint8_t result = 0;
  float pad = 0.25f;
  float x = *_x;
  float y = *_y;
  float z = *_z;
  int nx = round(x);
  int ny = round(y);
  int nz = round(z);
  float p[3] = {x, y, z};
  int np[3] = {nx, ny, nz};

  for(uint8_t face = 0; face < 6; face++){
    for(uint8_t i = 0; i < 3; i++){
      int8_t dir = FACES[face][i];
      if(!dir){
        continue;
      }

      float dist = (p[i] - np[i]) * dir;
      if(dist < pad){
        continue;
      }

      for(uint8_t dy = 0; dy < height; dy++){
        int op[3] = {nx, ny - dy, nz};
        op[i] += dir;

        if(!get_block(op[0], op[1], op[2])){
          continue;
        }

        p[i] -= (dist - pad) * dir;

        if(i == 1){
          result = 1;
        }

        break;
      }
    }
  }

  *_x = p[0];
  *_y = p[1];
  *_z = p[2];
  return result;
}

uint8_t collide(float *x, float *y, float *z){
  int px = round(*x) / CHUNK_SIZE;
  int pz = round(*z) / CHUNK_SIZE;

  for(unsigned short i = 0; i < chunk_count; i++){
    struct chunk *chunk = chunks[i];
    int dx = chunk->x - px;
    int dz = chunk->z - pz;

    if(abs(dx) > 1 || abs(dz) > 1){
      continue;
    }

    if(_collide(2, x, y, z)){
      return 1;
    }
  }

  return 0;
}
#endif

void voxel_state_init(){
  printf("voxel state init\n");

  shader = shader_create(voxel_vertex_shader_source, voxel_fragment_shader_source);
  shader_bind(shader);

  projection_location = shader_uniform_position(shader, "projection");
  view_location = shader_uniform_position(shader, "view");
  camera_position_location = shader_uniform_position(shader, "camera_position");
  light_direction_location = shader_uniform_position(shader, "light_direction");
  light_intensity_location = shader_uniform_position(shader, "light_intensity");
  shader_uniform1i(shader, "diffuse_texture", 0);

  texture = texture_create("tiles.png", GL_NEAREST);

  skybox_init();
  skybox_create(&skybox);

  if(pthread_create(&chunk_update_thread_id, NULL, &chunk_update_thread, NULL)){
    fprintf(stderr, "failed to create chunk mesh thread\n");
  }

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void voxel_state_destroy(){
  printf("voxel state destroy\n");

  for(unsigned short i = 0; i < chunk_count; i++){
    chunk_free(chunks[i]);
  }

  skybox_delete(&skybox);
  skybox_free();

  texture_delete(&texture);
  shader_delete(shader);

  running = 0;
  pthread_join(chunk_update_thread_id, NULL);

  glfwSetInputMode(cengine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void voxel_state_update(float deltaTime){
  if(glfwGetKey(cengine.window, GLFW_KEY_F11) == GLFW_PRESS){
    reloadPress = 1;
  }else if(reloadPress == 1 && glfwGetKey(cengine.window, GLFW_KEY_F11) == GLFW_RELEASE){
    reloadPress = 0;
    printf("%.6f %.6f\n", camera_position[0], camera_position[2]);
    // printf("reloading...\n");
    // voxel_state_destroy();
    // voxel_state_init();
    // printf("done\n");
  }

  light_angle += 10.0f * deltaTime;
  double light_angle_rad = glm_rad(light_angle);
  light_direction[0] = (float)sin(light_angle_rad);
  light_direction[1] = (float)cos(light_angle_rad);

  camera_update();
#if WALKING
  if(collide(&camera_position[0], &camera_position[1], &camera_position[2])){
    camera_dy = 0.0f;
  }
#endif

  unsigned char left_mouse = glfwGetMouseButton(cengine.window, GLFW_MOUSE_BUTTON_LEFT);
  unsigned char right_mouse = glfwGetMouseButton(cengine.window, GLFW_MOUSE_BUTTON_RIGHT);
  
  if(left_mouse || right_mouse){
    int hx, hy, hz;
    if(hit_test(camera_position[0], camera_position[1], camera_position[2], camera_pitch, camera_yaw, &hx, &hy, &hz)){
      int cx = hx / CHUNK_SIZE;
      int cz = hz / CHUNK_SIZE;

      for(unsigned short i = 0; i < chunk_count; i++){
        struct chunk *chunk = chunks[i];
        if(chunk->x == cx && chunk->z == cz){
          chunk_set(chunk, hx % CHUNK_SIZE, hy, hz % CHUNK_SIZE, left_mouse ? 0 : current_block);
          break;
        }
      }
    }
  }
}

void voxel_state_draw(){
  shader_bind(shader);
  texture_bind(texture, 0);

  float light_intensity = 1.0f;
  if(light_angle >= 90.0f){
    light_intensity = 0.0f;

    if(light_angle >= 360.0f){
      light_angle = -90.0f;
    }
  }else if(light_angle <= -80.0f || light_angle >= 80.0f){
    light_intensity = 1.0f - (float)(abs(light_angle) - 80.0f) / 10.0f;
  }

  glUniform3fv(light_direction_location, 1, light_direction);
  glUniform1f(light_intensity_location, light_intensity);

  glm_perspective(glm_rad(65.0f), (float)cengine.width/(float)cengine.height, 0.1f, SKYBOX_SIZE * 2.0f, projection);
  shader_uniform_matrix4fv_at(projection_location, projection[0]);

  glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
  glUniform3fv(camera_position_location, 1, camera_position);

  int x = floor(camera_position[0] / CHUNK_SIZE);
  int z = floor(camera_position[2] / CHUNK_SIZE);
  ensure_chunks(x, z);

  for(unsigned short i = 0; i < chunk_count; i++){
    struct chunk *chunk = chunks[i];

    if(!chunk->elements || abs(x - chunk->x) > CHUNK_RENDER_RADIUS || abs(z - chunk->z) > CHUNK_RENDER_RADIUS){
      continue;
    }

    mat4 model = GLMS_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){chunk->x * CHUNK_SIZE, 0, chunk->z * CHUNK_SIZE});
    shader_uniform_matrix4fv(shader, "model", model[0]);

    chunk_draw(chunk);
  }

  skybox_projection(projection[0]);
  skybox_draw(&skybox);
}

void voxel_state_on_key(GLFWwindow* window, int key, int scancode, int action, int mods){
  int control = mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER);
  if(control && key == GLFW_KEY_V){
    const char* buffer = glfwGetClipboardString(window);

    if(strstr(buffer, "/reload") == buffer){
      texture_delete(&texture);
      texture = texture_create("tiles.png", GL_NEAREST);
    }
  }

  if(key >= '1' && key <= '6'){
    current_block = key - '1' + 1;
  }

  if(key == GLFW_KEY_E){
    current_block = current_block % 6 + 1;
  }
}
