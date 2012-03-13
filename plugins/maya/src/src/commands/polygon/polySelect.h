#ifndef polySelect_h
#define polySelect_h

#include "../../definitions.h"

#include <maya/MPxCommand.h>

#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MSelectionList.h>


class tm_polySelect: public MPxCommand
{
public:
					tm_polySelect();
	virtual			~tm_polySelect();
	static void*	creator();

	MStatus	doIt( const MArgList& );

	bool isUndoable() const;
	MStatus redoIt();
	MStatus undoIt();

	static MSyntax newSyntax();
	static const char *loop_Flag, *loop_LongFlag;
	static const char *ring_Flag, *ring_LongFlag;

private:

	MSelectionList oldSelList;
	MSelectionList newSelList;

	bool doItSuccess;

};

#endif
