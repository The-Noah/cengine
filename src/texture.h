#ifndef TEXTURE_H
#define TEXTURE_H

unsigned int texture_create(const char* name);
void texture_bind(unsigned int texture);
void texture_delete(unsigned int *texture);

#endif
