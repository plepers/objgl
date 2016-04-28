


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


typedef struct bounds{
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
    bounds():
        minX(0),
        minY(0),
        minZ(0),
        maxX(0),
        maxY(0),
        maxZ(0)
    {}
} bounds;


inline void fillBounds( bounds &b, float x, float y, float z ){
    b.minX = fmin( x, b.minX );
    b.minY = fmin( y, b.minY );
    b.minZ = fmin( z, b.minZ );
    b.maxX = fmax( x, b.maxX );
    b.maxY = fmax( y, b.maxY );
    b.maxZ = fmax( z, b.maxZ );
}


#endif

