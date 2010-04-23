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

#include <maya/MFnPlugin.h>
#include <maya/MDagPath.h>
#include <maya/MIOStream.h>
#include <maya/MFStream.h>
#include <maya/MItDag.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

#include "MD2Exporter.h"
#include "MD2Writer.h"

/*********************************************************
 * ~MD2Exporter() : destructor method, does nothing
 ********************************************************/
MD2Exporter::~MD2Exporter() {
	
}

/*********************************************************
 * MD2Exporter::creator() : allows maya to allocate
 * a new instance of this object
 ********************************************************/
void* MD2Exporter::creator() {
	return new MD2Exporter();
}

/*********************************************************
 * MD2Exporter::defaultExtension() : called when Maya 
 * needs to know the default extension of this file
 ********************************************************/
MString MD2Exporter::defaultExtension () const {
	return "md2";
}

/*********************************************************
 * MD2Exporter::initializePlugin() : registers the plugin
 * with Maya
 ********************************************************/
MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, "andyrifken", "0.5", "Any");

	//register the plugin
	status = plugin.registerFileTranslator("MD2",
											"",
											MD2Exporter::creator,
											"MD2ExportOptionsScript",
											"option1=1",
											true);
	if (!status) {
		MGlobal::displayInfo("error loading md2 plugin");
		status.perror("error - registerFileTranslator");
		return status;
	}

	MGlobal::displayInfo("plugin registered successfully");
	return status;
}


/*********************************************************
 * MD2Exporter::uninitializePlugin() : deregisters
 * plugin with Maya
 ********************************************************/
MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterFileTranslator("MD2");
	if (!status) {
		status.perror("error in deregisterFileTranslator");
		return status;
	}

	return status;
}


/*********************************************************
 * MD2Exporter::createPolyWriter : creates a polywriter for 
 * MD2 format
 ********************************************************/
MD2Writer* MD2Exporter::createPolyWriter(MDagPath dagPath, MStatus &status) {
	return new MD2Writer(dagPath, status);
}

/*********************************************************
 * MD2Exporter::createPolyWriter : creates a polywriter for 
 * MD2 format
 ********************************************************/
MD2Writer* MD2Exporter::createPolyWriter(MDagPath dagPath, MStatus &status, MString &optionsString) {
	return new MD2Writer(dagPath, status, optionsString);
}



/*********************************************************
 * MD2Exporter::writer : overriding parent method in order 
 * save options string
 ********************************************************/
MStatus MD2Exporter::writer(const MFileObject &file,
							 const MString &options,
							 MPxFileTranslator::FileAccessMode mode) {
	optionsString = options;
	
	// create a new output stream
#if defined (OSMac_)
	char nameBuffer[MAXPATHLEN];
	strcpy (nameBuffer, file.fullName().asChar());
	const MString fileName(nameBuffer);
#else
	const MString fileName = file.fullName();
#endif
	
	ofstream newFile(fileName.asChar(), ios::out | ios::binary);
	if (!newFile) {
		MGlobal::displayError(fileName + ": could not be opened for reading");
		return MS::kFailure;
	}
	
	//check which objects are to be exported, and invoke the corresponding
	//methods; only 'export all' and 'export selection' are allowed
	//
	if (MPxFileTranslator::kExportAccessMode == mode) {
		if (MStatus::kFailure == exportAll(newFile)) {
			return MStatus::kFailure;
		}
	} else if (MPxFileTranslator::kExportActiveAccessMode == mode) {
		if (MStatus::kFailure == exportSelection(newFile)) {
			return MStatus::kFailure;
		}
	} else {
		return MStatus::kFailure;
	}
	
	
	newFile.flush();
	newFile.close();
	
	// if successful, notify here
	MGlobal::displayInfo("Export to " + fileName + " successful!");
	
	
	return MS::kSuccess;
}

bool MD2Exporter::haveWriteMethod() const {
	return true;
}


bool MD2Exporter::haveReadMethod() const {
	return false;
}

bool MD2Exporter::canBeOpened() const {
	return true;
}

/*********************************************************
 * MD2Exporter::exportAll : exports all meshes in the scene
 ********************************************************/
MStatus MD2Exporter::exportAll(ostream& os) {
	MStatus status;
	
	//create an iterator for only the mesh components of the DAG
	MItDag itDag(MItDag::kDepthFirst, MFn::kMesh, &status);
	
	if (MStatus::kFailure == status) {
		MGlobal::displayError("MItDag::MItDag");
		return MStatus::kFailure;
	}
	
	for(;!itDag.isDone();itDag.next()) {
		//get the current DAG path
		MDagPath dagPath;
		if (MStatus::kFailure == itDag.getPath(dagPath)) {
			MGlobal::displayError("MDagPath::getPath");
			return MStatus::kFailure;
		}
		
		MFnDagNode visTester(dagPath);
		
		//if this node is visible, then process the poly mesh it represents
		if(isVisible(visTester, status) && MStatus::kSuccess == status) {
			if (MStatus::kFailure == processPolyMesh(dagPath, os)) {
				return MStatus::kFailure;
			}
		}
	}
	return MStatus::kSuccess;
}

/*********************************************************
 * MD2Exporter::exportSelected : exports selected
 * mesh only.
 ********************************************************/
MStatus MD2Exporter::exportSelection(ostream& os)  {
	MStatus status;
	
	//create an iterator for the selected mesh components of the DAG
	MSelectionList selectionList;
	if (MStatus::kFailure == MGlobal::getActiveSelectionList(selectionList)) {
		MGlobal::displayError("MGlobal::getActiveSelectionList");
		return MStatus::kFailure;
	}
	
	MItSelectionList itSelectionList(selectionList, MFn::kMesh, &status);	
	if (MStatus::kFailure == status) {
		return MStatus::kFailure;
	}
	
	for (itSelectionList.reset(); !itSelectionList.isDone(); itSelectionList.next()) {
		MDagPath dagPath;
		
		//get the current dag path and process the poly mesh on it
		if (MStatus::kFailure == itSelectionList.getDagPath(dagPath)) {
			MGlobal::displayError("MItSelectionList::getDagPath");
			return MStatus::kFailure;
		}
		
		if (MStatus::kFailure == processPolyMesh(dagPath, os)) {
			return MStatus::kFailure;
		}
	}
	return MStatus::kSuccess;
}


/*********************************************************
 * MD2Exporter::processPolyMesh : override to save optionsString to
 * pWriter
 ********************************************************/
MStatus MD2Exporter::processPolyMesh(const MDagPath dagPath, ostream& os) {
	MStatus status;
	
	MD2Writer* md2Writer = createPolyWriter(dagPath, status, this->optionsString);
	
	if (MStatus::kFailure == status) {
		delete md2Writer;
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == md2Writer->saveMD2(os)) {
		delete md2Writer;
		return MStatus::kFailure;
	}

	delete md2Writer;
	return MStatus::kSuccess;
}



bool MD2Exporter::isVisible(MFnDagNode & fnDag, MStatus& status) 
//Summary:	determines if the given DAG node is currently visible
//Args   :	fnDag - the DAG node to check
//Returns:	true if the node is visible;		
//			false otherwise
{
	if(fnDag.isIntermediateObject())
		return false;

	MPlug visPlug = fnDag.findPlug("visibility", &status);
	if (MStatus::kFailure == status) {
		MGlobal::displayError("MPlug::findPlug");
		return false;
	} else {
		bool visible;
		status = visPlug.getValue(visible);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MPlug::getValue");
		}
		return visible;
	}
}





