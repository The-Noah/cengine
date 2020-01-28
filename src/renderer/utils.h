#ifndef UTILS_H
#define UTILS_H

#define GLEW_STATIC
#include "GL/glew.h"

unsigned int make_buffer(GLenum target, GLsizei size, const void* data);

#endif
