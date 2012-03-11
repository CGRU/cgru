#ifndef tm_polySlotNode_h
#define tm_polySlotNode_h

#include "../../../definitions.h"

#include "../polyModifierNode.h"
#include "polySlotFty.h"

// General Includes
//
#include <maya/MTypeId.h>
 
class tm_polySlotNode : public polyModifierNode
{
public:
						tm_polySlotNode();
	virtual				~tm_polySlotNode(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:

	static  MObject edgesList;

	static  MObject offset;
	static  MObject offsetDir;
	static  MObject offsetDirX;
	static  MObject offsetDirY;
	static  MObject offsetDirZ;

	static  MObject useMesh;

	static  MObject edgesPos;
	static  MObject edgesDir;

	static	MTypeId id;

	tm_polySlotFty fSlotFactory;

	bool recompute;
	bool firstTime;
};

#endif
