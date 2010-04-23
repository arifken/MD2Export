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


#ifndef MD2OBJECT_H
#define MD2OBJECT_H

#include <maya/MIOStream.h>
#include <maya/MGlobal.h>

#include "MD2DataTypes.h"

class MD2Object {	
public:
	MD2Object();
	~MD2Object();

	MStatus save(ostream &os);
	MStatus dump();
	
	
	
	md2_header_t header;
	
	md2_texCoord_t* addTexCoord();
	md2_triangle_t* addTriangle();
	md2_frame_t* addFrame();
	md2_skin_t* addSkin();
	md2_glcmd_t* addGlCommand();

	md2_frame_t* getFrame(unsigned int index);
	
private:
	
	std::vector<md2_texCoord_t> texCoords;
	std::vector<md2_triangle_t> triangles;
	std::vector<md2_frame_t> frames;
	std::vector<md2_skin_t> skins;
	std::vector<md2_glcmd_t> glCommands;
	

};

#endif
