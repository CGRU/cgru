#pragma once
#include "../definitions.h"

#include <maya/MArrayDataBuilder.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPxNode.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MIOStream.h>

class tm_kxb : public MPxNode
{
public:
						tm_kxb();
	virtual				~tm_kxb(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:
	static  MObject		input_k;
	static  MObject		input_x;
	static  MObject		input_b;
	static  MObject		output;
	static	MTypeId		id;
};
