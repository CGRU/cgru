#pragma once

#include "../definitions.h"

#include <maya/MIOStream.h>
#include <math.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MPxNode.h>

class tm_kxbci : public MPxNode
{
public:
						tm_kxbci();
	virtual				~tm_kxbci(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:
	static  MObject		input_x;
	static  MObject		clampInput;
	static  MObject		clampInput_min;
	static  MObject		clampInput_max;
	static  MObject		invertInput;
	static  MObject		in_k;
	static  MObject		in_b;
	static  MObject		invertResult;
	static  MObject		clampResult;
	static  MObject		clampResult_min;
	static  MObject		clampResult_max;
	static  MObject		output;
	static	MTypeId		id;
};
