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

#include "MD2Writer.h"

#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MFnSet.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MPlug.h>
#include <maya/MIOStream.h>
#include <maya/MImage.h>
#include <maya/MStatus.h>
#include <maya/MPointArray.h>
#include <maya/MTime.h>
#include <maya/MAnimControl.h>

#include <maya/MItMeshPolygon.h>


#include "MD2Object.h"

#include <time.h>
#include <set>

#include <sstream>

// Macros:
#define MD2_MAX_TRIANGLES 4096
#define MD2_MAX_VERTICES 2048
#define MD2_MAX_TEXCOORDS 2048
#define MD2_MAX_FRAMES 512
#define MD2_MAX_SKINS 32
#define MD2_MAX_FRAMESIZE (MD2_MAX_VERTICES * 4 + 128)

const char PATH_SEPARATOR = '/';

const double MD2_NORMALS[][3] = {{-0.525731, 0.000000, 0.850651}, 
	{-0.442863, 0.238856, 0.864188}, 
	{-0.295242, 0.000000, 0.955423}, 
	{-0.309017, 0.500000, 0.809017}, 
	{-0.162460, 0.262866, 0.951056}, 
	{0.000000, 0.000000, 1.000000}, 
	{0.000000, 0.850651, 0.525731}, 
	{-0.147621, 0.716567, 0.681718}, 
	{0.147621, 0.716567, 0.681718}, 
	{0.000000, 0.525731, 0.850651}, 
	{0.309017, 0.500000, 0.809017}, 
	{0.525731, 0.000000, 0.850651}, 
	{0.295242, 0.000000, 0.955423}, 
	{0.442863, 0.238856, 0.864188}, 
	{0.162460, 0.262866, 0.951056}, 
	{-0.681718, 0.147621, 0.716567}, 
	{-0.809017, 0.309017, 0.500000}, 
	{-0.587785, 0.425325, 0.688191}, 
	{-0.850651, 0.525731, 0.000000}, 
	{-0.864188, 0.442863, 0.238856}, 
	{-0.716567, 0.681718, 0.147621}, 
	{-0.688191, 0.587785, 0.425325}, 
	{-0.500000, 0.809017, 0.309017}, 
	{-0.238856, 0.864188, 0.442863}, 
	{-0.425325, 0.688191, 0.587785}, 
	{-0.716567, 0.681718, -0.147621}, 
	{-0.500000, 0.809017, -0.309017}, 
	{-0.525731, 0.850651, 0.000000}, 
	{0.000000, 0.850651, -0.525731}, 
	{-0.238856, 0.864188, -0.442863}, 
	{0.000000, 0.955423, -0.295242}, 
	{-0.262866, 0.951056, -0.162460}, 
	{0.000000, 1.000000, 0.000000}, 
	{0.000000, 0.955423, 0.295242}, 
	{-0.262866, 0.951056, 0.162460}, 
	{0.238856, 0.864188, 0.442863}, 
	{0.262866, 0.951056, 0.162460}, 
	{0.500000, 0.809017, 0.309017}, 
	{0.238856, 0.864188, -0.442863}, 
	{0.262866, 0.951056, -0.162460}, 
	{0.500000, 0.809017, -0.309017}, 
	{0.850651, 0.525731, 0.000000}, 
	{0.716567, 0.681718, 0.147621}, 
	{0.716567, 0.681718, -0.147621}, 
	{0.525731, 0.850651, 0.000000}, 
	{0.425325, 0.688191, 0.587785}, 
	{0.864188, 0.442863, 0.238856}, 
	{0.688191, 0.587785, 0.425325}, 
	{0.809017, 0.309017, 0.500000}, 
	{0.681718, 0.147621, 0.716567}, 
	{0.587785, 0.425325, 0.688191}, 
	{0.955423, 0.295242, 0.000000}, 
	{1.000000, 0.000000, 0.000000}, 
	{0.951056, 0.162460, 0.262866}, 
	{0.850651, -0.525731, 0.000000}, 
	{0.955423, -0.295242, 0.000000}, 
	{0.864188, -0.442863, 0.238856}, 
	{0.951056, -0.162460, 0.262866}, 
	{0.809017, -0.309017, 0.500000}, 
	{0.681718, -0.147621, 0.716567}, 
	{0.850651, 0.000000, 0.525731}, 
	{0.864188, 0.442863, -0.238856}, 
	{0.809017, 0.309017, -0.500000}, 
	{0.951056, 0.162460, -0.262866}, 
	{0.525731, 0.000000, -0.850651}, 
	{0.681718, 0.147621, -0.716567}, 
	{0.681718, -0.147621, -0.716567}, 
	{0.850651, 0.000000, -0.525731}, 
	{0.809017, -0.309017, -0.500000}, 
	{0.864188, -0.442863, -0.238856}, 
	{0.951056, -0.162460, -0.262866}, 
	{0.147621, 0.716567, -0.681718}, 
	{0.309017, 0.500000, -0.809017}, 
	{0.425325, 0.688191, -0.587785}, 
	{0.442863, 0.238856, -0.864188}, 
	{0.587785, 0.425325, -0.688191}, 
	{0.688191, 0.587785, -0.425325}, 
	{-0.147621, 0.716567, -0.681718}, 
	{-0.309017, 0.500000, -0.809017}, 
	{0.000000, 0.525731, -0.850651}, 
	{-0.525731, 0.000000, -0.850651}, 
	{-0.442863, 0.238856, -0.864188}, 
	{-0.295242, 0.000000, -0.955423}, 
	{-0.162460, 0.262866, -0.951056}, 
	{0.000000, 0.000000, -1.000000}, 
	{0.295242, 0.000000, -0.955423}, 
	{0.162460, 0.262866, -0.951056}, 
	{-0.442863, -0.238856, -0.864188}, 
	{-0.309017, -0.500000, -0.809017}, 
	{-0.162460, -0.262866, -0.951056}, 
	{0.000000, -0.850651, -0.525731}, 
	{-0.147621, -0.716567, -0.681718}, 
	{0.147621, -0.716567, -0.681718}, 
	{0.000000, -0.525731, -0.850651}, 
	{0.309017, -0.500000, -0.809017}, 
	{0.442863, -0.238856, -0.864188}, 
	{0.162460, -0.262866, -0.951056}, 
	{0.238856, -0.864188, -0.442863}, 
	{0.500000, -0.809017, -0.309017}, 
	{0.425325, -0.688191, -0.587785}, 
	{0.716567, -0.681718, -0.147621}, 
	{0.688191, -0.587785, -0.425325}, 
	{0.587785, -0.425325, -0.688191}, 
	{0.000000, -0.955423, -0.295242}, 
	{0.000000, -1.000000, 0.000000}, 
	{0.262866, -0.951056, -0.162460}, 
	{0.000000, -0.850651, 0.525731}, 
	{0.000000, -0.955423, 0.295242}, 
	{0.238856, -0.864188, 0.442863}, 
	{0.262866, -0.951056, 0.162460}, 
	{0.500000, -0.809017, 0.309017}, 
	{0.716567, -0.681718, 0.147621}, 
	{0.525731, -0.850651, 0.000000}, 
	{-0.238856, -0.864188, -0.442863}, 
	{-0.500000, -0.809017, -0.309017}, 
	{-0.262866, -0.951056, -0.162460}, 
	{-0.850651, -0.525731, 0.000000}, 
	{-0.716567, -0.681718, -0.147621}, 
	{-0.716567, -0.681718, 0.147621}, 
	{-0.525731, -0.850651, 0.000000}, 
	{-0.500000, -0.809017, 0.309017}, 
	{-0.238856, -0.864188, 0.442863}, 
	{-0.262866, -0.951056, 0.162460}, 
	{-0.864188, -0.442863, 0.238856}, 
	{-0.809017, -0.309017, 0.500000}, 
	{-0.688191, -0.587785, 0.425325}, 
	{-0.681718, -0.147621, 0.716567}, 
	{-0.442863, -0.238856, 0.864188}, 
	{-0.587785, -0.425325, 0.688191}, 
	{-0.309017, -0.500000, 0.809017}, 
	{-0.147621, -0.716567, 0.681718}, 
	{-0.425325, -0.688191, 0.587785}, 
	{-0.162460, -0.262866, 0.951056}, 
	{0.442863, -0.238856, 0.864188}, 
	{0.162460, -0.262866, 0.951056}, 
	{0.309017, -0.500000, 0.809017}, 
	{0.147621, -0.716567, 0.681718}, 
	{0.000000, -0.525731, 0.850651}, 
	{0.425325, -0.688191, 0.587785}, 
	{0.587785, -0.425325, 0.688191}, 
	{0.688191, -0.587785, 0.425325}, 
	{-0.955423, 0.295242, 0.000000}, 
	{-0.951056, 0.162460, 0.262866}, 
	{-1.000000, 0.000000, 0.000000}, 
	{-0.850651, 0.000000, 0.525731}, 
	{-0.955423, -0.295242, 0.000000}, 
	{-0.951056, -0.162460, 0.262866}, 
	{-0.864188, 0.442863, -0.238856}, 
	{-0.951056, 0.162460, -0.262866}, 
	{-0.809017, 0.309017, -0.500000}, 
	{-0.864188, -0.442863, -0.238856}, 
	{-0.951056, -0.162460, -0.262866}, 
	{-0.809017, -0.309017, -0.500000}, 
	{-0.681718, 0.147621, -0.716567}, 
	{-0.681718, -0.147621, -0.716567}, 
	{-0.850651, 0.000000, -0.525731}, 
	{-0.688191, 0.587785, -0.425325}, 
	{-0.587785, 0.425325, -0.688191}, 
	{-0.425325, 0.688191, -0.587785}, 
	{-0.425325, -0.688191, -0.587785}, 
	{-0.587785, -0.425325, -0.688191}, 
	{-0.688191, -0.587785, -0.425325}};

/*********************************************************
 * MD2Writer::MD2Writer : Constructors / destructors
 ********************************************************/
MD2Writer::MD2Writer(MDagPath &dagPath, MStatus &status) {
	fDagPath = new MDagPath(dagPath);
	fMesh = new MFnMesh(*fDagPath, &status);
}

MD2Writer::MD2Writer(MDagPath &dagPath, MStatus &status, MString optionsStr) {
	fDagPath = new MDagPath(dagPath);
	fMesh = new MFnMesh(*fDagPath, &status);
	setOptionsString(optionsStr);
}


MD2Writer::~MD2Writer() {
	if (NULL != fDagPath) delete fDagPath;
	if (NULL != fMesh) delete fMesh;
}




//------------------------------- accessors/mutators
 
void MD2Writer::setOptionsString(MString optStr) {
	this->optionsString = optStr;
	//cout << "setting options string: " << optStr;
	
	// parse options string
	const MString flagFrameListPath("-frameListPath");
	const MString flagTexturePath("-texturePath");
	
	MStringArray optionsArray;
	
	optStr.split(' ', optionsArray);
	unsigned int numOptions = optionsArray.length();
	
	for (unsigned int i = 0; i<numOptions; i++) {
		if (optionsArray[i] == flagFrameListPath && numOptions >= i+1) {
			this->frameListPath = optionsArray[++i];
			//cout << "found frame list path: " << frameListPath;
		} else if (optionsArray[i] == flagTexturePath && numOptions >= i+1) {
			this->texturePath = optionsArray[++i];
			//cout << "found texture path: " << texturePath;
		}
	}
}

//-----------------------------------------------

/*********************************************************
 * MD2Writer::saveMD2 : Outputs mesh to file - equivalent
 * to "save_md2" function in blender script
 ********************************************************/
MStatus MD2Writer::saveMD2(ostream &os) {
	//MGlobal::displayInfo("Exporting " + fMesh->partialPathName());
	
	// create a blank md2 object
	MD2Object md2_object;

	// set the UVSet
	if (MStatus::kFailure == fMesh->getCurrentUVSetName(this->fCurrentUVSetName)) {
		MGlobal::displayError("MFnMesh::getCurrentUVSetName"); 
		return MStatus::kFailure;
	}
	
	// validate the mesh
	if (MStatus::kFailure == MD2Writer::validation()) {
		MGlobal::displayError("MD2Writer::writeToFile - error validating mesh");
		return MStatus::kFailure;
	}
	
	
	// fill the md2 object
	if (MStatus::kFailure == MD2Writer::fillMD2(md2_object)) {
		MGlobal::displayError("MD2Writer::writeToFile - error populating MD2 Object");
		return MStatus::kFailure;
	}

	
	
	// print the md2 header
	md2_object.dump();
	
	
	// write the file to disk
	if (MStatus::kFailure == md2_object.save(os)) {
		MGlobal::displayError("MD2Writer::writeToFile - error running save method on MD2Object");
		return MStatus::kFailure;
	}
	
	return MStatus::kSuccess;
}


/*********************************************************
 * MD2Writer::validation : validate that the mesh object can
 * be written
 ********************************************************/
MStatus MD2Writer::validation() {
	MStatus status;
	const MDagPath kFDagPath(*fDagPath);
	char strBuffer[50];

	MGlobal::displayInfo( "=========== beginning validation... =============");

	MGlobal::displayInfo("=> validating triangularity:");
	// validate that it's only triangles
	unsigned int faceCount = fMesh->numPolygons();
	unsigned int i;
	for (i=0; i<faceCount; i++) {
		unsigned int indexCount = fMesh->polygonVertexCount(i, &status);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("error in validation method - MFnMesh::polygonVertexCount");
			return MStatus::kFailure;
		}	
		if (indexCount != 3) {
			// TODO - triangulate if error is raised here
			MGlobal::displayError("error in validation method - mesh must be triangles only!");
			return MStatus::kFailure;
		}
	}
	MGlobal::displayInfo("   OK");

	// check for UV coords



	MGlobal::displayInfo("=> validating UVs:");
	MItMeshPolygon itPolygon(kFDagPath, MObject::kNullObj, &status);
	for (  ; !itPolygon.isDone(); itPolygon.next()) {
		MFloatArray currUs;
		MFloatArray currVs;
		status = itPolygon.getUVs(currUs,currVs,&fCurrentUVSetName);
		if (currUs.length() < 2 || currVs.length() < 2) {
			MGlobal::displayError("not all vertices have UVs");
			return MStatus::kFailure;
		}
	}
	MGlobal::displayInfo("   OK");
	


	// check for associated texmap
	MGlobal::displayInfo("=> validating textures...");
	MObjectArray textures;
	status = fMesh->getAssociatedUVSetTextures(fCurrentUVSetName, textures);
	if (MStatus::kFailure == status) {
		MGlobal::displayError("error getting list of textures from UV set (in MD2Writer::validation)");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("   got associated textures...... YES");

	if ((int)textures.length() < 1) {
		MGlobal::displayError("There must be at least one texture assigned to this UV Set");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("   at least one texture? ...... YES");


	
	// validate size of texmap
	MGlobal::displayInfo("=> validating texmap is 256x256...");

	MImage texImageFile;
	status = texImageFile.readFromTextureNode(textures[0]);
	if (MStatus::kFailure == status) {
		MGlobal::displayError("Error getting image file from texture node - MD2Writer::validation");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("   Found image from texture node? ...... YES");


	unsigned int width, height;
	status = texImageFile.getSize(width, height);
	if (MStatus::kFailure == status) {
		MGlobal::displayError("Error getting image dimensions from tex file - MD2Writer::validation");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("   Got the texture image size? ...... YES");

	if (width != 256 || height != 256) {
		MGlobal::displayInfo("WARNING: texture file size is not 256 x 256");
	}
	MGlobal::displayInfo("   Texture size is 256 x 256? ...... YES");

	// TODO: verify frame list data (currently just using default MD2 frame list)
	
	
	MGlobal::displayInfo("=> Validating mesh against MD2 requirements/limitations...");
	// verify that tri/vert/frame counts are within MD2 standard
	faceCount = fMesh->numPolygons(&status);
	if (MStatus::kFailure == status) {
		MGlobal::displayError("error getting face count");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("   got face count? .......... YES");
	
	unsigned int vertCount = fMesh->numVertices(&status);
	if (MStatus::kFailure == status) {
		MGlobal::displayError("error getting vertex count");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("   got vert count? .......... YES");
	int strLen = sprintf(strBuffer,"   faces: %u \n   verts: %u",faceCount,vertCount);
	const MString msg(strBuffer);
	MGlobal::displayInfo(msg);


	
	if (faceCount > MD2_MAX_TRIANGLES) {
		MGlobal::displayError("ERROR: too many triangles for md2 format");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("   face count < max polygons? .......... YES ");
	

	if (vertCount > MD2_MAX_VERTICES) {
		MGlobal::displayError("ERROR: too many vertices for md2 format");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("   vertex count < max vertices? ........... YES");
	


	// TODO - validate frame count length from frame list
	
	MGlobal::displayInfo("=> Validation successful");
	return MStatus::kSuccess;
	
}





/*********************************************************
 * MD2Writer::fillMD2 - fills the MD2 data structure
 ********************************************************/
MStatus MD2Writer::fillMD2(MD2Object &md2_object) {
	MStatus status;

	const MDagPath kFDagPath(*fDagPath);
	const MString kFCurrentUVSetName(fCurrentUVSetName);

	this->getFrameList();
	
	if (this->frameListPath == NULL)
		this->frameListPath = "";
	
	if (this->texturePath == NULL)
		this->texturePath = "";
	
	// set some header info
	md2_object.header.ident = 844121161;
	md2_object.header.version = 8;
	md2_object.header.num_vertices = fMesh->numVertices();
	md2_object.header.num_tris = fMesh->numPolygons();
	
	
	// get skin information -- use first face's image for texture
	MStringArray uvSetNames;
	MObjectArray uvSetTextures;
	
	if (MStatus::kFailure == fMesh->getUVSetNames(uvSetNames)) {
		MGlobal::displayError("error fillMD2- MFnMesh::getUVSetNames");
		return MStatus::kFailure;
	}
	
	if (uvSetNames.length() < 1) {
		MGlobal::displayError("error, no UV sets available");
		return MStatus::kFailure;
	}
	
	if (MStatus::kFailure == fMesh->getAssociatedUVSetTextures(uvSetNames[0],uvSetTextures)) {
		MGlobal::displayError("error getting uv set texture");
		return MStatus::kFailure;
	}


	MImage texImageFile;
	if (MStatus::kFailure == texImageFile.readFromTextureNode(uvSetTextures[0])) {
		MGlobal::displayError("Error getting image file from texture node - MD2Writer::fillMD2");
		return MStatus::kFailure;
	}
	
	unsigned int width, height;
	if (MStatus::kFailure == texImageFile.getSize(width, height)) {
		MGlobal::displayError("Error getting image size - MD2Writer::fillMD2");
		return MStatus::kFailure;
	}
		
	md2_object.header.skinwidth = width;
	md2_object.header.skinheight = height;
	md2_object.header.num_skins = 1;
	

	// add skin node to data structure
	md2_skin_t* aSkin = md2_object.addSkin();
	

	// get texture file path
	MString thisTexturePath;
	if ((uvSetTextures[0]).hasFn(MFn::kFileTexture)) {
		MFnDependencyNode fnFile(uvSetTextures[0]);
		MPlug ftnPlug = fnFile.findPlug("fileTextureName", &status);
		if (status == MS::kSuccess) {
			ftnPlug.getValue(thisTexturePath);
		} else {
			cout << "unsupported texture node";
		}
	}


	MString skinName(thisTexturePath); // full path to texture
	MStringArray pathComps;
	if (MS::kFailure == skinName.split(PATH_SEPARATOR,pathComps)) {
		MGlobal::displayError("MD2Writer::fillMD2 - error splitting texture path components");
		return MS::kFailure;
	}
	MString textureBasename(pathComps[pathComps.length()-1]);

	MGlobal::displayInfo("=> Validating skin name");

	char skinNameBuffer[100];
	std::string skinNameCStr(textureBasename.asUTF8());
	int strLen = sprintf(skinNameBuffer,"   skin name: [%s]",skinNameCStr.c_str());
	const MString msg(skinNameBuffer);
	MGlobal::displayInfo(msg);

	if (textureBasename.length() > 64) {
		MGlobal::displayError("texture file name cannot be more than 64 characters!");
		return MStatus::kFailure;
	}
	
	strLen = sprintf(skinNameBuffer,"%s",skinNameCStr.c_str());
	
	for (unsigned int i=0; i < skinNameCStr.length(); i++) {
		aSkin->name[i] = skinNameCStr.at(i);
	}
	

	MGlobal::displayInfo(  "valid skin name....... YES");

	
	// put in texture info -- build uv coord dictionary (prevents double entries-saves space)

	std::set<md2_texCoord_t> allUVs;
	
	unsigned int texCount = 0;
	MFloatArray   allUs, allVs;
	if (MS::kFailure == fMesh->getUVs( allUs, allVs, &kFCurrentUVSetName )) {
		MGlobal::displayError("MD2Writer::fillMD2 - error exporting uvs");
		return MS::kFailure;
	}
	
	MGlobal::displayInfo("processing uv...");
	for (unsigned int i = 0; i < allUs.length(); i++) {

		md2_texCoord_t* currTexCoord = md2_object.addTexCoord();
		currTexCoord->s = allUs[i] * md2_object.header.skinwidth;
		currTexCoord->t = ((allVs[i] * md2_object.header.skinheight)*-1);

		texCount++;
	}

	MItMeshPolygon itPolygon(kFDagPath, MObject::kNullObj);

	md2_object.header.num_st = texCount;
	std::set<md2_texCoord_t>::iterator it;

	MGlobal::displayInfo("=> processing triangles...");
	// put faces in md2 structure
	itPolygon.reset();
	for (  ; !itPolygon.isDone(); itPolygon.next()) {
		md2_triangle_t* currFace = md2_object.addTriangle();
		//MGlobal::displayInfo("triangle:");
		for (int i = 0; i < 3; i++) {
			// vert index
			currFace->vertex[i] = itPolygon.vertexIndex(i,&status);
			if (status == MS::kFailure) {
				MGlobal::displayError("error getting vertex index");
				return MStatus::kFailure;
			}

			// UV texture index
			int texIndex;
			if (MS::kFailure == itPolygon.getUVIndex(i,texIndex,&kFCurrentUVSetName)) {
				MGlobal::displayError("error getting texture index");
				return MStatus::kFailure;
			}
			currFace->st[i] = texIndex;

			//sprintf(strBuffer,"   vertex index: %i uv index: %i",currFace->vertex[i] ,currFace->st[i]);
			//MString msg2(strBuffer);
			//MGlobal::displayInfo(msg2);
		}
	}


	// TODO - compute GL commands
	
	// get frame data, calculate one frame size (in bytes)
	md2_object.header.framesize = 40 + (md2_object.header.num_vertices*4);
	
	
	
	// FRAMES/ANIMATION
	MAnimControl animctrl;
	MPointArray points;
	MTime currentTime = MAnimControl::minTime();
	MTime maxTime = MAnimControl::maxTime();

	md2_object.header.num_frames = maxTime.value()-currentTime.value()+1;
	
	



	while (currentTime <= maxTime) {
		//std::cout << "EXPORTING FRAME: " << currentTime.value() <<"\n";




		md2_frame_t *currFrame = md2_object.addFrame();
		const MDagPath dagPath(*fDagPath);
		status = MD2Writer::getPointsAtTime(*fDagPath, currentTime, points);
		if (status == MS::kFailure) {
			MGlobal::displayError("MD2Writer::fillMD2 - error getting frame points");
			return MStatus::kFailure;
		}
		
		double frame_min_x= 100000.0;
		double frame_max_x= -100000.0;
		double frame_min_y= 100000.0;
		double frame_max_y= -100000.0;
		double frame_min_z= 100000.0;
		double frame_max_z= -100000.0;
		
		
		itPolygon.reset();
		for (  ; !itPolygon.isDone(); itPolygon.next()) { 
			int numTriangles;
			itPolygon.numTriangles(numTriangles);
			
			while (numTriangles--) {
				MPointArray nonTweaked; // object-relative verst
				MIntArray triangleVertices; // face-relatives verts
				status = itPolygon.getTriangle(numTriangles,nonTweaked,triangleVertices, MSpace::kObject);
				if (status == MS::kFailure) {
					MGlobal::displayError("error getting triangles");
					return MStatus::kFailure;
				}
				


				for (unsigned int triVerts = 0; triVerts < triangleVertices.length(); triVerts++) {

					double pX = (points[triangleVertices[triVerts]])[0];
					double pY = (points[triangleVertices[triVerts]])[1];
					double pZ = (points[triangleVertices[triVerts]])[2];
				
					if (frame_min_x > pX) frame_min_x = pX;
					if (frame_max_x < pX) frame_max_x = pX;
					if (frame_min_y > pY) frame_min_y = pY;
					if (frame_max_y < pY) frame_max_y = pY;				
					if (frame_min_z > pZ) frame_min_z = pZ;
					if (frame_max_z < pZ) frame_max_z = pZ;
					
				}
			}
		}
		
		//the scale is the difference between the min and max (on that axis) / 255
		double frame_scale_x=(frame_max_x-frame_min_x)/255;
		double frame_scale_y=(frame_max_y-frame_min_y)/255;
		double frame_scale_z=(frame_max_z-frame_min_z)/255;

		//translate value of the mesh to center it on the origin
		double frame_trans_x=frame_min_x;
		double frame_trans_y=frame_min_y;
		double frame_trans_z=frame_min_z;
		
		// fill in the data
		currFrame->scale[0] = -frame_scale_x;
		currFrame->scale[1] = frame_scale_y;
		currFrame->scale[2] = frame_scale_z;
		currFrame->translate[0] = -frame_trans_x;
		currFrame->translate[1] = frame_trans_y;
		currFrame->translate[2] = frame_trans_z;
		


		// now for the frame verts
		for (unsigned int j=0; j<points.length(); j++) {
			
			md2_vertex_t currVert;
			currVert.v[0] = (char)((points[j][0]-frame_trans_x)/frame_scale_x);
			currVert.v[1] = (char)((points[j][1]-frame_trans_y)/frame_scale_y);
			currVert.v[2] = (char)((points[j][2]-frame_trans_z)/frame_scale_z);


			// add lookup table
			double maxdot = -999999.0;
			int maxdotindex = -1;
			
			for (int k = 0; k<162; k++) {
				//swap y and x for difference in axis orientation 
				double x1 = -points[j][0]; // TODO: does this value need to be multiplied by -1?
				double y1 = points[j][1];
				double z1 = points[j][2];
				double dot = (x1 * MD2_NORMALS[k][0] +
							  y1 * MD2_NORMALS[k][1] + 
							  z1 * MD2_NORMALS[k][2]);
				if (dot > maxdot) {
					maxdot = dot;
					maxdotindex = k;
				}
			}
			currVert.normalIndex = maxdotindex + 2;
			currFrame->verts.push_back(currVert);
		}

		
		currentTime++;
	}
	
	
	// iterate through user's frame list
	for (unsigned int i = 0; i < _framelist.size(); i++) {
		//std::cout << "adding framelist names for framelist entry " << i << "\n";
		// iterate over frame range for that frame entry
		for (unsigned int j = (_framelist[i]).start - 1; j < (_framelist[i]).stop; j++) {
			
			// add the name
			//std::ostringstream oss;
			//oss << _framelist[i].name << "_" << j;

			char frameNameBuffer[100];
			int frameNameLen = sprintf(frameNameBuffer,"%s_%i",_framelist[i].name,j-(_framelist[i].start)+1);
			
			char frameNameStr[64];

			for (int k = 0; k < 64 || k < frameNameLen; k++) {
				frameNameStr[k] = frameNameBuffer[k];
			}
			
			//string clippedStr(str.substr(0,endVal));
			
			if (j < md2_object.header.num_frames) {
				md2_object.getFrame(j)->setName(frameNameStr,frameNameLen);
				//std::cout << "    added framelist name: " << md2_object.getFrame(j)->name << "\n";
			}
		}
	}
	
	
	// finish computing header info
	int header_size = 17*4; //17 integers, and each integer is 4 bytes
	int skin_size = 64 * md2_object.header.num_skins;  //64 char per skin * number of skins
	int tex_coord_size = 4 * md2_object.header.num_st; // 2 short * number of texture coords
	int face_size = 12 * md2_object.header.num_tris; // 3 shorts for vertex index, 3 shorts for tex index
	int frames_size = (((12+12+16)+(4*md2_object.header.num_vertices)) * md2_object.header.num_frames); //frame info+verts per frame*num frames
	int gl_command_size = md2_object.header.num_glcmds*4; // each is an int or float, so 4 bytes per

	// fill in offset info
	md2_object.header.offset_skins = 0 + header_size;
	md2_object.header.offset_st = md2_object.header.offset_skins + skin_size;
	md2_object.header.offset_tris = md2_object.header.offset_st + tex_coord_size;
	md2_object.header.offset_frames = md2_object.header.offset_tris + face_size;
	md2_object.header.offset_glcmds = md2_object.header.offset_frames + frames_size;
	md2_object.header.offset_end = md2_object.header.offset_glcmds + gl_command_size;


	return MS::kSuccess;
	
}


/*********************************************************
 * MD2Writer::getFrameList() : reads in the frame list that
 * the user provided and assigns the content to an instance
 * variable
 ********************************************************/
void MD2Writer::getFrameList() {
	// get option from optionsString
	// read file into provided array

	/*
	while(!ifstream.eof()){
	ifstream.getline(...);
	numLines1++;
	}
	*/
	std::vector<md2_framelist_entry> framelist;
	
//	// TODO: get user frame list here

//	if (frameListPath != NULL && frameListPath.length() > 0) {
//	
//		return framelist;
//	} 
	
	// fallback on default frame list
	md2_framelist_entry stand; MD2Writer::fillCharArrayWithRange("stand",sizeof("stand"),stand.name,sizeof(stand.name)); stand.start = 1; stand.stop = 40;
	md2_framelist_entry run; MD2Writer::fillCharArrayWithRange("run",sizeof("run"),run.name,sizeof(run.name)); run.start = 41; run.stop = 46;
	md2_framelist_entry attack; MD2Writer::fillCharArrayWithRange("attack",sizeof("attack"),attack.name,sizeof(attack.name)); attack.start = 47; attack.stop = 54;
	md2_framelist_entry pain1; MD2Writer::fillCharArrayWithRange("pain1",sizeof("pain1"),pain1.name,sizeof(pain1.name)); pain1.start = 55; pain1.stop = 58;
	md2_framelist_entry pain2; MD2Writer::fillCharArrayWithRange("pain2",sizeof("pain2"),pain2.name,sizeof(pain2.name)); pain2.start = 59; pain2.stop = 62;
	md2_framelist_entry pain3; MD2Writer::fillCharArrayWithRange("pain3",sizeof("pain3"),pain3.name,sizeof(pain3.name)); pain3.start = 63; pain3.stop = 66;
	md2_framelist_entry jump; MD2Writer::fillCharArrayWithRange("jump",sizeof("jump"),jump.name,sizeof(jump.name)); jump.start = 67; jump.stop = 72;
	md2_framelist_entry flip; MD2Writer::fillCharArrayWithRange("flip",sizeof("flip"),flip.name,sizeof(flip.name)); flip.start = 73; flip.stop = 84;
	md2_framelist_entry salute; MD2Writer::fillCharArrayWithRange("salute",sizeof("salute"),salute.name,sizeof(salute.name)); salute.start = 85; salute.stop = 95;
	md2_framelist_entry taunt; MD2Writer::fillCharArrayWithRange("taunt",sizeof("taunt"),taunt.name,sizeof(taunt.name)); taunt.start = 96; taunt.stop = 112;
	md2_framelist_entry wave; MD2Writer::fillCharArrayWithRange("wave",sizeof("wave"),wave.name,sizeof(wave.name)); wave.start = 113; wave.stop = 123;
	md2_framelist_entry point; MD2Writer::fillCharArrayWithRange("point",sizeof("point"),point.name,sizeof(point.name)); point.start = 124; point.stop = 135;
	md2_framelist_entry crstnd; MD2Writer::fillCharArrayWithRange("crstnd",sizeof("crstnd"),crstnd.name,sizeof(crstnd.name)); crstnd.start = 136; crstnd.stop = 154;
	md2_framelist_entry crwalk; MD2Writer::fillCharArrayWithRange("crwalk",sizeof("crwalk"),crwalk.name,sizeof(crwalk.name)); crwalk.start = 155; crwalk.stop = 160;
	md2_framelist_entry crattack; MD2Writer::fillCharArrayWithRange("crattack",sizeof("crattack"),crattack.name,sizeof(crattack.name)); crattack.start = 161; crattack.stop = 169;
	md2_framelist_entry crpain; MD2Writer::fillCharArrayWithRange("crpain",sizeof("crpain"),crpain.name,sizeof(crpain.name)); crpain.start = 170; crpain.stop = 173;
	md2_framelist_entry crdeath; MD2Writer::fillCharArrayWithRange("crdeath",sizeof("crdeath"),crdeath.name,sizeof(crdeath.name));crdeath.start = 174; crdeath.stop = 178;
	md2_framelist_entry death1; MD2Writer::fillCharArrayWithRange("death1",sizeof("death1"),death1.name,sizeof(death1.name)); death1.start = 179; death1.stop = 184;
	md2_framelist_entry death2; MD2Writer::fillCharArrayWithRange("death2",sizeof("death2"),death2.name,sizeof(death2.name)); death2.start = 185; death2.stop = 190;
	md2_framelist_entry death3; MD2Writer::fillCharArrayWithRange("death3",sizeof("death3"),death3.name,sizeof(death3.name)); death3.start = 191; death3.stop = 198;		
	
	framelist.push_back(stand);
	framelist.push_back(run);
	framelist.push_back(attack);
	framelist.push_back(pain1);
	framelist.push_back(pain2);
	framelist.push_back(pain3);
	framelist.push_back(jump);
	framelist.push_back(flip);
	framelist.push_back(salute);
	framelist.push_back(taunt);
	framelist.push_back(wave);
	framelist.push_back(point);
	framelist.push_back(crstnd);
	framelist.push_back(crwalk);
	framelist.push_back(crpain);
	framelist.push_back(crdeath);
	framelist.push_back(death1);
	framelist.push_back(death2);
	framelist.push_back(death3);
	
	
	this->_framelist = framelist;
}

void MD2Writer::fillCharArrayWithRange(const char* substr, unsigned int substrLength, char* charArray, unsigned int charArrayLength) {
	for (unsigned int i = 0; i < substrLength && i < charArrayLength; i++) {
		charArray[i] = substr[i];
	}
}


// TODO: implement export of GL Commands:
/*********************************************************
 * MD2Writer::buildGlCommands : Build OpenGL command list
 ********************************************************/
/*
int buildGlCommands(MD2Object &md2_object) {
	int used_tris;
	// hashmap edge_dict
	// array strip_verts
	// array strip_tris
	// array strip_st
	int num_commands = 0;
	MD2GlCommandList *cmdList;

	used_tris = fMesh->numPolygons(); 

	// generate edge dictionary
	// create a dict - key: edge key, value: empty array
	// for each face..
		// for each key in face's edge_keys
			// add face.index for dict entry: edge_dict[key]

	// for each face in the mesh
		// test if used_tris object at index tri_counter is non-zero,
			// if so, skip it (must be used triangle)
		// else	
			// initialize strip_tris, strip_verts, strip_st, strip_first_run, odd
			// find strip length
			// mark tris as used 
			// "stripify" stripify_tri_list
			// print strip length
			cmdList->setNum(strip_tris.length()+2);
			num_commands++;

			// add s,t,vert for this command list
			// for command_counter in strip_tris.length()+2
			MD2GlCommand cmd;
				cmd->setS(strip_st[command_counter][0];
				cmd->setT(1.0-(strip_st[command_counter][1]));
				cmd->setIndex = strip_verts[command_counter]
				num_commands+=3;
				cmdList->addCommand(&cmd);

			cout << "Cmd List length: " << cmdList->numCommands() << "\n"
					<< "Cmd list num: " << cmdList.getNum() << "\n"
					<< "Cmd List " << cmdList->dump() << "\n";
			md2_object.addGlCommandList(cmdList);

	// add null command at the end
	MD2GlCommandList *tmpCmdList;
	tmpCmdList->setNum(0);
	md2_object->addGlCommandList(tmpCmdList);
	num_commands++;

	return num_commands;
}



/*********************************************************
 * MD2Writer::findStripLength - gl strip length
 ********************************************************/
/*
int findStripLength(int tri_counter,) {
}
*/

/*********************************************************
 * MD2Writer::stripifyTriList : 
 ********************************************************/
/*
void stripifyTriList() {
}
*/


/*********************************************************
 * MD2Writer::getLocalIndex : Gets the local vertex indices for a 
 * given triangle
 ********************************************************/
MIntArray MD2Writer::getLocalIndex( MIntArray & getVertices, MIntArray & getTriangle ) {
  MIntArray   localIndex;
  unsigned    gv, gt;

  if (getTriangle.length() != 3) {
	  MGlobal::displayError("MD2Writer::getLocalIndex - triangle must have 3 verts");
  }

  for ( gt = 0; gt < getTriangle.length(); gt++ )
  {
    for ( gv = 0; gv < getVertices.length(); gv++ )
    {
      if ( getTriangle[gt] == getVertices[gv] )
      {
        localIndex.append( gv );
        break;
      }
    }

    // if nothing was added, add default "no match"
    if ( localIndex.length() == gt )
      localIndex.append( -1 );
  }

  return localIndex;
}


/*********************************************************
 * MD2Writer::getPointsAtTime : updates the mesh points data
 * with the updated vertices at a given time 
 (as the timeline is advanced, this captures the changes in 
 the mesh as it is animated)
 ********************************************************/
MStatus MD2Writer::getPointsAtTime(
    const MDagPath& dagPath,
    const MTime& mayaTime,
    MPointArray& points )
{
  MStatus status = MS::kSuccess;
  points.clear();
  MFnMesh fnMesh;

  // Move Maya to current frame
  MGlobal::viewFrame( mayaTime );

  // You MUST reinitialize the function set after changing time!

  this->fMesh->setObject( *fDagPath );

  // Get vertices at this time
  status = fMesh->getPoints( points, MSpace::kObject);

  //cout << "verts at time: %i " << mayaTime.value() << "\n";
  for (unsigned int i = 0; i< points.length(); i++) {
	//cout << "vert[" << i << "] (" << points[i][0] << ", " << points[i][1] << ", " << points[i][2] << ")\n";
  }

  return status;
}