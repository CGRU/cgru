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

#include "../utilities/INoise.h"

class tm_noisePerlin : public MPxDeformerNode
{
public:
						tm_noisePerlin();
	virtual				~tm_noisePerlin();

	static  void*		creator();
	static  MStatus		initialize();

	// deformation function
	//
    virtual MStatus   	deform(MDataBlock& 		block,
							   MItGeometry& 	iter,
							   const MMatrix& 	mat,
							   unsigned int 	multiIndex);

public:
	// tm_noisePerlin attributes
	//
	static  MObject     dummy;
	static  MObject     amplitude;
	static  MObject     frequency;
	static  MObject     levels;
	static  MObject     lev_Mamp;
	static  MObject     lev_Mfreq;
	static  MObject     scale;
	static  MObject     scaleAmpX;
	static  MObject     scaleAmpY;
	static  MObject     scaleAmpZ;
	static  MObject     scaleFreqX;
	static  MObject     scaleFreqY;
	static  MObject     scaleFreqZ;
	static  MObject     variation;

	static  MTypeId		id;

private:

};
