//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_GEOMETRY_H
#define INCLUDED_GEOMETRY_H

#ifndef INCLUDED_GL_H
#include "Gl.h"
#endif

#ifndef INCLUDED_GLM_HPP
#include "glm.hpp"
#define INCLUDED_GLM_HPP
#endif

//------------------------------------------------------------------------------

namespace gl {

//------------------------------------------------------------------------------

static const float quad[] = {-1.0f, -1.0f, 0.0f, 1.0f,  
                              1.0f, -1.0f, 1.0f, 1.0f,
                              1.0f,  1.0f, 1.0f, 0.0f, 
                             -1.0f,  1.0f, 0.0f, 0.0f};

static const float oquad[] = { 0.0f,  0.0f, 0.0f, 1.0f,  
                               1.0f,  0.0f, 1.0f, 1.0f,
                               1.0f,  1.0f, 1.0f, 0.0f, 
                               0.0f,  1.0f, 0.0f, 0.0f};


//------------------------------------------------------------------------------

} // namespace

//------------------------------------------------------------------------------

#endif
