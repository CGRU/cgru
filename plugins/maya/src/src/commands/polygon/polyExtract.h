#ifndef polyExtract_h
#define polyExtract_h

#include "../../definitions.h"

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MDagPath.h>
#include <maya/MIntArray.h>
#include <maya/MObject.h>
//#include <maya/MObjectArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
//#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItSelectionList.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MIOStream.h>
#include <maya/MSyntax.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MSelectionList.h>
#include <maya/MArgDatabase.h>
#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>
#include <maya/MFnDagNode.h>
#include <maya/MIntArray.h>
//#include <maya/MFloatPointArray.h>
#include <maya/MFnSet.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnComponent.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>

//#include <math.h>

class tm_polyExtract: public MPxCommand
{
public:
					tm_polyExtract() {};
	virtual			~tm_polyExtract();
	static void*	creator();
	MStatus	doIt( const MArgList& );
//	MStatus redoIt();
//	MStatus undoIt();
//	bool isUndoable() const;

	static MSyntax newSyntax();
//	static const char *extractFaces_Flag, *extractFaces_LongFlag;

private:
	bool extractFaces_Func( MSelectionList &selectionList, MStringArray &node_names);
};

#endif
