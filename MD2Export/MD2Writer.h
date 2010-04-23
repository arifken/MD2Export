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

#ifndef MD2WRITER_H
#define MD2WRITER_H

#include <maya/MFnMesh.h>
#include "MD2Object.h"

class MD2Writer {
	
public:
	MD2Writer(MDagPath &dagPath, MStatus &status);
	MD2Writer(MDagPath &dagPath, MStatus &status, MString optionsStr);
	virtual ~MD2Writer();

	MStatus saveMD2(ostream &os);

	
private:
	// Methods
	MStatus validation();
	MStatus extractGeometry();
	MStatus fillMD2(MD2Object &md2_object);
	
	MIntArray getLocalIndex( MIntArray & getVertices, MIntArray & getTriangle );
	MStatus MD2Writer::getPointsAtTime(const MDagPath& dagPath,
									   const MTime& mayaTime,
									   MPointArray& points );

	void getFrameList();
//	int findStripLength(int tri_counter);
//	void stripifyTriList(/* edge key */);
//	int buildGlCommands(MD2Object &md2_object);

	void fillCharArrayWithRange(const char* substr, unsigned int substrLength, char* charArray, unsigned int charArrayLength);


	MStatus outputSingleSet(ostream &os,
							MString setName,
							MIntArray faces,
							MString textureName);

	void setOptionsString(MString optionsString);

	// Data Members
	MString optionsString;
	MString frameListPath;
	MString texturePath;
	std::vector<md2_framelist_entry> _framelist;

	//the current UV set's name
	MString				fCurrentUVSetName;

	//for storing DAG objects
	MFnMesh*			fMesh;
	MDagPath*			fDagPath;
	MObjectArray		fPolygonSets;
	MObjectArray		fPolygonComponents;
};
#endif