#ifndef grade_h
#define grade_h

#include "../../definitions.h"

#include <maya/MArrayDataBuilder.h>

#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MPxNode.h>
#include <maya/MDataBlock.h>

class grade_tm: public MPxNode
{
public:
		grade_tm();
	virtual ~grade_tm();

	virtual MStatus compute( const MPlug& plug, MDataBlock& data );

	static  void* creator();
	static  MStatus initialize();
	static	MTypeId id;

private:
	static MObject inColor;

	static MObject pointBlackObj;
	static MObject pointWhiteObj;
	static MObject satObj;
	static MObject gainObj;
	static MObject gammaObj;
	static MObject multObj;
	static MObject offsetObj;
	static MObject clampBlackObj;
	static MObject clampWhiteObj;
	static MObject invertObj;

	static MObject outColor;
};

#endif
