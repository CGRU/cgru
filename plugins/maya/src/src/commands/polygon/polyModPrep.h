#ifndef polyModPrep_h
#define polyModPrep_h

#include "../../definitions.h"

#include <maya/MStatus.h>
#include <maya/MGlobal.h>

#include <maya/MPxCommand.h>
#include <maya/MArgDatabase.h>
#include <maya/MSyntax.h>
#include <maya/MSelectionList.h>

#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>

#include <maya/MIntArray.h>
#include <maya/MFloatVectorArray.h>

#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MObjectArray.h>

#include <maya/MFnMesh.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>

class tm_polyModPrep: public MPxCommand
{
public:
					tm_polyModPrep();
	virtual			~tm_polyModPrep();
	static void*	creator();
	MStatus	doIt( const MArgList& );

	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const;

	static MSyntax newSyntax();

private:

	bool hasTweaks;
	bool hasHistory;

	MObject meshObj;
	MFnMesh meshFn;
	MString meshName;
	MPlug mesh_pntsPlug;
	MPlug mesh_inMeshPlug;
	unsigned pntsCount;

	MIntArray pntsIndexesArray;
	MObject polyTweakObject;
	MFloatVectorArray tweakVectorsArray;

	MDGModifier dgModifier;
	MDagModifier dagModifier;
	MDagModifier tempDagModifier;		// for undo duplicate

};

#endif
