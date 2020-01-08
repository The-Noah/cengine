#ifndef SHADER_H
#define SHADER_H

unsigned int shader_create(const char* vertex_source, const char* fragment_source);
void shader_bind(unsigned int shader);
void shader_delete(unsigned int shader);

#endif
