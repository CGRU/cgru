#ifndef reparent_h
#define reparent_h

#include "../definitions.h"

#include <maya/MStatus.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>

#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

#include <maya/MObject.h>
#include <maya/MObjectArray.h>
#include <maya/MDagModifier.h>

class tm_reparent: public MPxCommand
{
public:
					tm_reparent() {};
	virtual			~tm_reparent();
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
