#pragma once

#include "../definitions.h"

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>

#include <maya/MObject.h>
#include <maya/MFnMesh.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MSyntax.h>
#include <maya/MIOStream.h>

#include <maya/MSelectionList.h>
#include <maya/MItDag.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshVertex.h>

#include <maya/MPoint.h>
#include <maya/MPointArray.h>

#include <math.h>

class tm_randPoint: public MPxCommand
{
public:
					tm_randPoint() {};
	virtual			~tm_randPoint();
	static void*	creator();
	MStatus	doIt( const MArgList& );
	static MSyntax newSyntax();
	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const;

	static const char *amp_Flag, *amp_LongFlag;
	static const char *ampX_Flag, *ampX_LongFlag;
	static const char *ampY_Flag, *ampY_LongFlag;
	static const char *ampZ_Flag, *ampZ_LongFlag;
	static const char *seed_Flag, *seed_LongFlag;
	static const char *obj_Flag, *obj_LongFlag;
	static const char *help_Flag, *help_LongFlag;

private:
	bool helpMode;
	MDagPathArray res_MDagPathArray;
	MPointArray *oldPointsArrays;
	MPointArray *newPointsArrays;
	bool selVtxMode;
};
