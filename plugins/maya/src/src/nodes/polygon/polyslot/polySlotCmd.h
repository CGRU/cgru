#ifndef tm_polySlotCmd_h
#define tm_polySlotCmd_h

#include "../../../definitions.h"

#include "../polyModifierCmd.h"
#include "polySlotFty.h"


#include <maya/MFnComponentListData.h>
#include <maya/MSelectionList.h>
#include <maya/MStringArray.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>

// Forward Class Declarations
//
class MArgList;

class tm_polySlot : public polyModifierCmd
{

public:
	////////////////////
	// Public Methods //
	////////////////////

				tm_polySlot();
	virtual		~tm_polySlot();

	static		void* creator();

	bool		isUndoable() const;

	MStatus		doIt( const MArgList& );
	MStatus		redoIt();
	MStatus		undoIt();

	/////////////////////////////
	// polyModifierCmd Methods //
	/////////////////////////////

	MStatus		initModifierNode( MObject modifierNode );
	MStatus		directModifier( MObject mesh );

private:

	MObject fComponentList;
	MIntArray fSelEdges;
//	MDGModifier dgModifier;
	MString modifierNodeName;
	void getModifierNodeName();

	tm_polySlotFty fSlotFactory;

	MSelectionList newSelList;
	MSelectionList oldSelList;

	void successResult();

	MStringArray uvSetNames;
	MString currentUVSetName;
	MFloatArray *p_uArray;
	MFloatArray *p_vArray;
	MIntArray *p_uvCounts;
	MIntArray *p_uvIds;
	unsigned uvSetsCount;

	void getMeshUVs();
	void setMeshUVs();
};

#endif
