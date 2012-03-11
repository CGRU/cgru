#ifndef tm_polySplitNode_h
#define tm_polySplitNode_h

#include "../../../definitions.h"

#include "../polyModifierNode.h"
#include "polySplitFty.h"

// General Includes
//
#include <maya/MTypeId.h>
 
class tm_polySplitNode : public polyModifierNode
{
public:
						tm_polySplitNode();
	virtual				~tm_polySplitNode(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:

	static  MObject factor;
	static  MObject edgeDirection;
	static  MObject normalize;
	static  MObject offsetX;
	static  MObject offsetY;
	static  MObject offsetZ;

	static  MObject uvMethod;
	static  MObject offsetF;
	static  MObject offsetU;
	static  MObject offsetV;

	static  MObject useMesh;
	static  MObject inMesh;
	static  MObject edgesList;
	static  MObject edgePos;
	static  MObject outMesh;

	static  MObject splitAlgorithm;
	static  MObject splitLoopMode;
	static  MObject splitLoopAngle;
	static  MObject splitLoopMaxCount;

	static	MTypeId id;

private:
	tm_polySplitFty fSplitFactory;

	bool recompute;
	bool firstTime;

	MIntArray edgesIds;
};

#endif
