


#ifndef OBJGL_MAIN_H
#define OBJGL_MAIN_H

#include "obj.h"
#include "tclap/CmdLine.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef WIN
#include <Windows.h>
#endif


#ifdef __APPLE__
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#endif


typedef enum {
    CHAR = 1,
    SHORT= 2,
    INT  = 4
} IndexFormat;

typedef struct mat_map{
    int matId;
    unsigned int begintri;
    unsigned int numtris;
} mat_map;
#endif

