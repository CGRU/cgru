#ifndef killNode_h
#define killNode_h

#include "../definitions.h"

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>

#include <maya/MObject.h>
#include <maya/MFnDependencyNode.h>

#include <maya/MPxCommand.h>
#include <maya/MArgDatabase.h>
#include <maya/MSyntax.h>
#include <maya/MSelectionList.h>

#include <maya/MDagModifier.h>

class tm_killNode: public MPxCommand
{
public:
					tm_killNode() {};
	virtual			~tm_killNode();
	static void*	creator();
	MStatus	doIt( const MArgList& );

	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const;

	static MSyntax newSyntax();

private:

	MDagModifier dagModifier;
};

#endif
