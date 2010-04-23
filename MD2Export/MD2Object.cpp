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

#include "MD2Object.h"
#include <maya/MGlobal.h>
#include <sstream>
// constructors/destructors
MD2Object::MD2Object() {
	//initialize header
	header.ident = 0;
	header.version = 0;
	header.skinwidth = 0;
	header.skinheight = 0;
	header.framesize = 0;
	header.num_skins = 0;
	header.num_vertices = 0;
	header.num_st = 0;
	header.num_tris = 0;
	header.num_glcmds = 0;
	header.num_frames = 0;
	header.offset_skins = 0;
	header.offset_st = 0;
	header.offset_tris = 0;
	header.offset_frames = 0;
	header.offset_glcmds = 0;
	header.offset_end = 0;
}

MD2Object::~MD2Object() {}


/*********************************************************
 * save(ostream &os) : save the data to an output stream 
 ********************************************************/
MStatus MD2Object::save(ostream &os) {
	std::cout << "\n============ SAVING MD2 FILE ===============\n";
	this->dump();
	std::cout << "-------------------------------------\n";
	std::cout << "\n\n";
	
	unsigned int i;

	os.write ((char*)&this->header.ident, sizeof(int));
	os.write ((char*)&this->header.version, sizeof(int));
	os.write ((char*)&this->header.skinwidth, sizeof(int));
	os.write ((char*)&this->header.skinheight, sizeof(int));
	os.write ((char*)&this->header.framesize, sizeof(int));

	os.write ((char*)&this->header.num_skins, sizeof(int));
	os.write ((char*)&this->header.num_vertices, sizeof(int));
	os.write ((char*)&this->header.num_st, sizeof(int));
	os.write ((char*)&this->header.num_tris, sizeof(int));
	os.write ((char*)&this->header.num_glcmds, sizeof(int));
	os.write ((char*)&this->header.num_frames, sizeof(int));

	os.write ((char*)&this->header.offset_skins, sizeof(int));
	os.write ((char*)&this->header.offset_st, sizeof(int));
	os.write ((char*)&this->header.offset_tris, sizeof(int));
	os.write ((char*)&this->header.offset_frames, sizeof(int));
	os.write ((char*)&this->header.offset_glcmds, sizeof(int));
	os.write ((char*)&this->header.offset_end, sizeof(int));

	std::cout << "starting skins, buffer position: " << os.tellp() << "\n";
	char strBuffer[100];

	// write the skins
	for (i=0; i < skins.size(); i++) {
		md2_skin_t currSkin = skins.at(i);

		int len = sprintf(strBuffer,"skin %i: %s",i,currSkin.name);
		MString msg(strBuffer);
		MGlobal::displayInfo(strBuffer);

		os.write((char*) &currSkin, sizeof(md2_skin_t));
	}

	std::cout << "starting texcoords, buffer position: " << os.tellp() << "\n";
	// write the texture coords
	for (i=0; i < texCoords.size(); i++) {
		md2_texCoord_t currTexCoord = texCoords.at(i);

		/*
		int len = sprintf(strBuffer,"texcoord %i: %i, %i",i,currTexCoord.s,currTexCoord.t);
		MString msg(strBuffer);
		MGlobal::displayInfo(strBuffer);
		*/

		os.write((char*) &currTexCoord, sizeof(md2_texCoord_t));
	}

	std::cout << "starting triangles, buffer position: " << os.tellp() << "\n";
	// write the faces
	for (i=0; i < triangles.size(); i++) {
		md2_triangle_t currTri = triangles.at(i);
		os.write((char*) &currTri, sizeof(md2_triangle_t));
	}

	std::cout << "starting frames, buffer position: " << os.tellp() << "\n";
	// write the frames
	for (i=0; i < frames.size(); i++) {
		md2_frame_t currFrame = frames.at(i);
		os.write((char*)currFrame.scale, sizeof(vec3_t));
		os.write((char*)currFrame.translate, sizeof(vec3_t));
		os.write((char*)currFrame.name, 16);
		for (int j=0; j<currFrame.verts.size(); j++) {
			os.write((char*) &currFrame.verts.at(j), sizeof(md2_vertex_t));
		}
	}

	std::cout << "starting glcmds, buffer position: " << os.tellp() << "\n";
	// write the gl commands
	for (i=0; i < glCommands.size(); i++) {
		md2_glcmd_t currCmd = glCommands.at(i);
		os.write((char*) &currCmd, sizeof(md2_glcmd_t));
	}
	
	std::cout << "end buffer position: " << os.tellp() << "\n";
	
	return MStatus::kSuccess;
}

/*********************************************************
 * dump : print data members to output window 
 ********************************************************/
MStatus MD2Object::dump() {
	std::cout << "------------- MD2 Header Information --------------\n"
		<< "ident: " << header.ident << "\n"
		<< "version: " << header.version << "\n"
		<< "skin width: " << header.skinwidth << "\n"
		<< "skin height: " << header.skinheight << "\n"
		<< "frame size: " << header.framesize << "\n"
		<< "number of skins: " << header.num_skins << "\n"
		<< "number of vertices: " << header.num_vertices << "\n"
		<< "number of tex coordinates: " << header.num_st << "\n"
		<< "number of faces: " << header.num_tris << "\n"
		<< "number of gl commands: " << header.num_glcmds << "\n"
		<< "number of frames: " << header.num_frames << "\n"
		<< "offset skins: " << header.offset_skins << "\n"
		<< "offset tex coords: " << header.offset_st << "\n"
		<< "offset faces: " << header.offset_tris << "\n"
		<< "offset frames: " << header.offset_frames << "\n"
		<< "offset gl commands: " << header.offset_glcmds << "\n"
		<< "offset end: " << header.offset_end << "\n";

	return MStatus::kSuccess;
}



/*********************************************************
 * the following methods add empty data sets to the MD2
 ********************************************************/
md2_texCoord_t* MD2Object::addTexCoord() {
	md2_texCoord_t newTexCoord;
	this->texCoords.push_back(newTexCoord);
	return &this->texCoords.back();
}

md2_triangle_t* MD2Object::addTriangle() {
	md2_triangle_t newTriangle;
	this->triangles.push_back(newTriangle);
	return &this->triangles.back();
}

md2_frame_t* MD2Object::addFrame() {
	md2_frame_t newFrame;
	this->frames.push_back(newFrame);
	return &this->frames.back();
}

md2_skin_t* MD2Object::addSkin() {
	md2_skin_t newSkin;
	this->skins.push_back(newSkin);
	return &this->skins.back();
}

md2_glcmd_t* MD2Object::addGlCommand() {
	md2_glcmd_t newGlCommand;
	this->glCommands.push_back(newGlCommand);
	return &this->glCommands.back();
}

/* ==================== Accessors/Mutators ================================= */

md2_frame_t* MD2Object::getFrame(unsigned int index) {
	return &this->frames[index];
}

