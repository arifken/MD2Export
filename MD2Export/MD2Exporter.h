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

#ifndef MD2EXPORTER_H
#define MD2EXPORTER_H

#include <maya/MPxFileTranslator.h>

class MD2Writer;
class MDagPath;
class MFnDagNode;

class MD2Exporter: public MPxFileTranslator {

public:
	MD2Exporter() : MPxFileTranslator() {};
	virtual ~MD2Exporter(); // inheriting destructor from parent class

	static void* creator();
	MString defaultExtension() const;
	MStatus initializePlugin(MObject obj);
	MStatus uninitializePlugin(MObject obj);

	virtual MStatus writer (const MFileObject& file,
									const MString& optionsString,
									MPxFileTranslator::FileAccessMode mode);
	bool haveWriteMethod () const;
	bool haveReadMethod () const;
	bool canBeOpened () const;

private:
	MD2Writer* MD2Exporter::createPolyWriter(MDagPath dagPath, MStatus &status);
	MD2Writer* createPolyWriter(const MDagPath dagPath, MStatus &status, MString &optionsString);
	MString optionsString;

protected:	
	bool			isVisible(MFnDagNode& fnDag, MStatus& status);
	MStatus			exportAll(ostream& os);
	MStatus			exportSelection(ostream& os);
	MStatus			processPolyMesh(const MDagPath dagPath, ostream& os);
};




#endif