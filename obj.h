

#ifndef OBJGL_OBJ_H
#define OBJGL_OBJ_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vectors */
typedef float vec3_t[3];
typedef float vec4_t[4];

/* Vertex */
struct obj_vertex_t
{
  vec4_t xyzw;
};

/* Texture coordinates */
struct obj_texCoord_t
{
  vec3_t uvw;
};

/* Normal vector */
struct obj_normal_t
{
  vec3_t ijk;
};

/* Polygon */
struct obj_face_t
{
  int num_elems;      /* number of vertices */

  int *vert_indices;  /* vertex indices */
  int *uvw_indices;   /* texture coordinate indices */
  int *norm_indices;  /* normal vector indices */
};

/* OBJ model structure */
typedef struct obj_model_t
{
  int num_verts;                     /* number of vertices */
  int num_texCoords;                 /* number of texture coords. */
  int num_normals;                   /* number of normal vectors */
  int num_faces;                     /* number of polygons */

  int has_texCoords;                 /* has texture coordinates? */
  int has_normals;                   /* has normal vectors? */

  struct obj_vertex_t *vertices;     /* vertex list */
  struct obj_texCoord_t *texCoords;  /* tex. coord. list */
  struct obj_normal_t *normals;      /* normal vector list */
  struct obj_face_t *faces;          /* model's polygons */
} ObjModel;


void FreeModel ( ObjModel *mdl);


int MallocModel (ObjModel *mdl);
int FirstPass (FILE *fp, ObjModel *mdl);
int SecondPass (FILE *fp, ObjModel *mdl);
int ReadOBJModel (const char *filename, ObjModel *mdl);

#endif