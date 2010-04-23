/*
Copyright (c) 2010 Andy Rifken (www.andyrifken.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// defines data types for encoding MD2 files
#ifndef MD2_DATATYPES_H
#define MD2_DATATYPES_H

#include <vector>

// Macros
#define MD2_MAX_TRIANGLES 4096
#define MD2_MAX_VERTICES 2048
#define MD2_MAX_TEXCOORDS 2048
#define MD2_MAX_FRAMES 512
#define MD2_MAX_SKINS 32
#define MD2_MAX_FRAMESIZE (MD2_MAX_VERTICES * 4 + 128)

using namespace std;

struct md2_header_t {
  int ident;                  /* magic number: "IDP2" */
  int version;                /* version: must be 8 */

  int skinwidth;              /* texture width */
  int skinheight;             /* texture height */

  int framesize;              /* size in bytes of a frame */

  int num_skins;              /* number of skins */
  int num_vertices;           /* number of vertices per frame */
  int num_st;                 /* number of texture coordinates */
  int num_tris;               /* number of triangles */
  int num_glcmds;             /* number of opengl commands */
  int num_frames;             /* number of frames */

  int offset_skins;           /* offset skin data */
  int offset_st;              /* offset texture coordinate data */
  int offset_tris;            /* offset triangle data */
  int offset_frames;          /* offset frame data */
  int offset_glcmds;          /* offset OpenGL command data */
  int offset_end;             /* offset end of file */
};

// Vector 
typedef float vec3_t[3];

// texture info
typedef struct md2_skin_t {
  char name[64];              /* texture file name */
} md2_skin_t;

// texture coordinates
typedef struct md2_texCoord_t {
  short s;
  short t;
  bool operator<(const md2_texCoord_t &texCoord) const {
	  return s < texCoord.s;
  }

  bool operator==(const md2_texCoord_t &texCoord) const {
	  return (texCoord.s == s && texCoord.t == t);
  }
} md2_texCoord_t;

// faces (triangles)
typedef struct md2_triangle_t {
  unsigned short vertex[3];   /* vertex indices of the triangle */
  unsigned short st[3];       /* tex. coord. indices */
} md2_triangle_t;

// vertices
typedef struct md2_vertex_t {
  unsigned char v[3];         /* position */
  unsigned char normalIndex;  /* normal vector index */
} md2_vertex_t;

// frames
typedef struct md2_frame_t {
  vec3_t scale;               /* scale factor */
  vec3_t translate;           /* translation vector */
  char name[16];              /* frame name */
  vector<md2_vertex_t> verts;

  void setName(const char* newName, unsigned int len) {
	  if (len > 16) return;
	  for (unsigned int k = 0; k < len; k++) {
		this->name[k] = newName[k];
	  }
  }
} md2_frame_t;

// gl commands
typedef struct md2_glcmd_t {
  float s;                    /* s texture coord. */
  float t;                    /* t texture coord. */
  int index;                  /* vertex index */
} md2_glcmd_t;

// gl command list
typedef struct md2_glcmdlist_t {
	int num;
	md2_glcmd_t *cmd_list;
} md2_glcmdlist_t;

// frame list item
typedef struct md2_framelist_entry {
	char name[16];
	unsigned int start;
	unsigned int stop;
} md2_framelist_entry;

#endif