#ifndef SKYBOX_H
#define SKYBOX_H

typedef struct{
  unsigned int vao;
} Skybox;

void skybox_init(float* projection_matrix);
void skybox_free();
void skybox_create(Skybox *skybox);
void skybox_delete(Skybox *skybox);
void skybox_draw(Skybox *skybox);

#endif
