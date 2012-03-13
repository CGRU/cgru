#ifndef tm_polySplitCmd_h
#define tm_polySplitCmd_h

#include "../../../definitions.h"

#include "../polyModifierCmd.h"
#include "polySplitFty.h"


#include <maya/MStatus.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>

#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MSelectionList.h>
#include <maya/MIntArray.h>

//#include <maya/MPxCommand.h>
class tm_polySplit : public polyModifierCmd
{

public:
	////////////////////
	// Public Methods //
	////////////////////

				tm_polySplit();
	virtual		~tm_polySplit();

	static		void* creator();

	bool		isUndoable() const;

	MStatus		doIt( const MArgList&);
	static MSyntax newSyntax();

	MStatus		redoIt();
	MStatus		undoIt();

	static const char *loop_Flag;
	static const char *loop_LongFlag;
	static const char *sel_Flag;
	static const char *sel_LongFlag;
	bool	cmd_flag_loop;
	int		cmd_flag_loop_mode;
	double	cmd_flag_loop_angle;
	int	cmd_flag_loop_maxcount;
	bool	cmd_flag_sel;

	/////////////////////////////
	// polyModifierCmd Methods //
	/////////////////////////////

	MStatus		initModifierNode( MObject modifierNode );
	MStatus		directModifier( MObject mesh );

private:

	MObject fComponentList;
	MIntArray fSelEdges;

	MString modifierNodeName;
	void getModifierNodeName();

	tm_polySplitFty fSplitFactory;

	MSelectionList newSelList;
	MSelectionList oldSelList;

};

#endif
