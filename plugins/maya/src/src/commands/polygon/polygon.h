#pragma once

#include "../../definitions.h"

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MDagPath.h>

#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>

#include <maya/MObject.h>
#include <maya/MObjectArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>

#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItSelectionList.h>

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MPlug.h>

//#include <maya/MIOStream.h>
#include <maya/MSyntax.h>
#include <maya/MString.h>
#include <maya/MSelectionList.h>

#include <maya/MArgDatabase.h>
#include <maya/MFnNumericData.h>

#include <maya/MDGModifier.h>

#include <math.h>
#include <list>

class tm_polygon: public MPxCommand
{
public:
					tm_polygon() {};
	virtual			~tm_polygon();
	static void*	creator();
	MStatus	doIt( const MArgList& );

	static MSyntax newSyntax();
	static const char *isMesh_Flag, *isMesh_LongFlag;
	static const char *isEdgeSoft_Flag, *isEdgeSoft_LongFlag;
	static const char *getUVcoords_Flag, *getUVcoords_LongFlag;
	static const char *findPath_Flag, *findPath_LongFlag;
	static const char *selectLoop_Flag, *selectLoop_LongFlag;
	static const char *selectRing_Flag, *selectRing_LongFlag;
	static const char *removeTweaks_Flag, *removeTweaks_LongFlag;
	static const char *edgesToRing_Flag, *edgesToRing_LongFlag;
	static const char *edgesToLoop_Flag, *edgesToLoop_LongFlag;

private:
	MObject *pObject;
	MFnMesh *pMesh;
	MPointArray *pPointArray;
	static bool isMesh_Func( MString mString);
	static bool isEdgeSoft_Func( MSelectionList &selectionList, int index);
	static bool getUVcoords_Func( MSelectionList &selectionList, int index, float &u, float &v);
	bool findPath_Func( MSelectionList &selectionList, int vtxIndex_a, int vtxIndex_b, MIntArray &returnArray, double koeff_a);
		int *pVtxFlags;
		int findPath_findBestVtx( int startId, MIntArray &vtxList, MPoint &aimPoint, double koeff_a);
	bool selectLoop_Func( MIntArray &selArray, MSelectionList &selectionList, int selIndex, int mode, double angle, int maxCount);
	bool selectRing_Func( MIntArray &selArray, MSelectionList &selectionList, int selIndex, int mode, double angle, int maxCount);
		MItMeshVertex *pVtxIt;
		MItMeshEdge *pEdgeIt;
		MItMeshPolygon *pFaceIt;
	int  removeTweaks_Func( MSelectionList &selectionList);
		bool getFloat3PlugValue( MPlug & plug, MFloatVector & value );
		bool setFloat3PlugValue( MPlug & plug, MFloatVector & value );
	bool edgesToRing_Func( MIntArray &edgesArray, MSelectionList &selectionList);
	bool edgesToLoop_Func( MIntArray &edgesArray, MSelectionList &selectionList);
};
