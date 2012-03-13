#pragma once

#include "../definitions.h"

#include <math.h>

#include <maya/MArrayDataBuilder.h>

#include <maya/MIOStream.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MItGeometry.h>
#include <maya/MTypeId.h> 
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPoint.h>
#include <maya/MMatrix.h>

class tm_randDef : public MPxDeformerNode
{
public:
						tm_randDef();
	virtual				~tm_randDef();

	static  void*		creator();
	static  MStatus		initialize();

//    virtual MStatus   	deform(MDataBlock& 		block,
    MStatus     deform(MDataBlock&      block,
							   MItGeometry& 	iter,
							   const MMatrix& 	mat,
							   unsigned int 	multiIndex);
public:
	static  MObject     multiplier;
	static  MObject     ampXmin;
	static  MObject     ampXmax;
	static  MObject     ampYmin;
	static  MObject     ampYmax;
	static  MObject     ampZmin;
	static  MObject     ampZmax;
	static  MObject     seed;
	
	static  MTypeId		id;
private:
};
