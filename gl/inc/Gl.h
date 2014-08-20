#ifndef INCLUDED_GL_H
#define INCLUDED_GL_H

#ifdef ANDROID
#ifndef INCLUDED_GLES_GL2_H
#define INCLUDED_GLES_GL2_H
#include <GLES2/gl2.h>
#endif
#else
#include <GLFW/glfw3.h>
#endif

#endif
