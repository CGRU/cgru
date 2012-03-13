#ifndef polysplitsel_h
#define polysplitsel_h

#include "../../../definitions.h"

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <maya/MSelectionList.h>

class tm_polySplitLoopSel: public MPxCommand
{
public:
					tm_polySplitLoopSel();
	virtual			~tm_polySplitLoopSel();
	static void*	creator();
	MStatus	doIt( const MArgList& );

	static MSyntax newSyntax();

	bool		isUndoable() const;
	MStatus		redoIt();
	MStatus		undoIt();

private:

	MSelectionList oldSelList;
	MSelectionList newSelList;
};

#endif
