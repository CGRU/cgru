#include "randomDeformer.h"

MTypeId     tm_randDef::id( tm_randDef__id );

#define McheckErr(stat,msg)		\
	if ( MS::kSuccess != stat ) {	\
		stat.perror(msg);				\
		return MS::kFailure;		\
	}

MObject     tm_randDef::multiplier;
MObject     tm_randDef::ampXmin;
MObject     tm_randDef::ampXmax;
MObject     tm_randDef::ampYmin;
MObject     tm_randDef::ampYmax;
MObject     tm_randDef::ampZmin;
MObject     tm_randDef::ampZmax;
MObject     tm_randDef::seed;

tm_randDef::tm_randDef(){}
tm_randDef::~tm_randDef(){}
void* tm_randDef::creator(){return new tm_randDef();}

MStatus tm_randDef::initialize()
{
	MFnNumericAttribute nAttr;

	multiplier=nAttr.create( "multiplier", "mult", MFnNumericData::kDouble );
	    nAttr.setDefault(1.0);
	    nAttr.setKeyable(true);
	addAttribute( multiplier); 

	ampXmin=nAttr.create( "ampXmin", "axmn", MFnNumericData::kDouble );
	    nAttr.setDefault(0.0);
	    nAttr.setKeyable(true);
	addAttribute( ampXmin); 
	ampXmax=nAttr.create( "ampXmax", "axmx", MFnNumericData::kDouble );
	    nAttr.setDefault(1.0);
	    nAttr.setKeyable(true);
	addAttribute( ampXmax); 

	ampYmin=nAttr.create( "ampYmin", "aymn", MFnNumericData::kDouble );
	    nAttr.setDefault(0.0);
	    nAttr.setKeyable(true);
	addAttribute( ampYmin); 
	ampYmax=nAttr.create( "ampYmax", "aymx", MFnNumericData::kDouble );
	    nAttr.setDefault(1.0);
	    nAttr.setKeyable(true);
	addAttribute( ampYmax); 

	ampZmin=nAttr.create( "ampZmin", "azmn", MFnNumericData::kDouble );
	    nAttr.setDefault(0.0);
	    nAttr.setKeyable(true);
	addAttribute( ampZmin); 
	ampZmax=nAttr.create( "ampZmax", "azmx", MFnNumericData::kDouble );
	    nAttr.setDefault(1.0);
	    nAttr.setKeyable(true);
	addAttribute( ampZmax); 

	seed=nAttr.create( "seed", "s", MFnNumericData::kLong );
	    nAttr.setDefault(1);
	    nAttr.setKeyable(true);
	addAttribute( seed);

    attributeAffects( tm_randDef::multiplier, tm_randDef::outputGeom );
    attributeAffects( tm_randDef::ampXmin, tm_randDef::outputGeom );
    attributeAffects( tm_randDef::ampXmax, tm_randDef::outputGeom );
    attributeAffects( tm_randDef::ampYmin, tm_randDef::outputGeom );
    attributeAffects( tm_randDef::ampYmax, tm_randDef::outputGeom );
    attributeAffects( tm_randDef::ampZmin, tm_randDef::outputGeom );
    attributeAffects( tm_randDef::ampZmax, tm_randDef::outputGeom );
    attributeAffects( tm_randDef::seed, tm_randDef::outputGeom );

	return MS::kSuccess;
}

MStatus
tm_randDef::deform( MDataBlock& block,
				MItGeometry& iter,
				const MMatrix& /*m*/,
				unsigned int /*multiIndex*/)
{
	MStatus status = MS::kSuccess;

	MDataHandle multiplierData = block.inputValue(multiplier,&status);
	McheckErr(status, "Error getting multiplier data handle\n");
	double mult = multiplierData.asDouble();

	MDataHandle axmnData = block.inputValue(ampXmin,&status);
	McheckErr(status, "Error getting ampXmin data handle\n");
	double axmn = axmnData.asDouble();
	MDataHandle axmxData = block.inputValue(ampXmax,&status);
	McheckErr(status, "Error getting ampXmax data handle\n");
	double axmx = axmxData.asDouble();

	MDataHandle aymnData = block.inputValue(ampYmin,&status);
	McheckErr(status, "Error getting ampYmin data handle\n");
	double aymn = aymnData.asDouble();
	MDataHandle aymxData = block.inputValue(ampYmax,&status);
	McheckErr(status, "Error getting ampYmax data handle\n");
	double aymx = aymxData.asDouble();

	MDataHandle azmnData = block.inputValue(ampZmin,&status);
	McheckErr(status, "Error getting ampZmin data handle\n");
	double azmn = azmnData.asDouble();
	MDataHandle azmxData = block.inputValue(ampZmax,&status);
	McheckErr(status, "Error getting ampZmax data handle\n");
	double azmx = azmxData.asDouble();

	MDataHandle sdData = block.inputValue(seed,&status);
	McheckErr(status, "Error getting seed data handle\n");
	int sd = sdData.asLong();

//	MDataHandle envData = block.inputValue(envelope,&status);
//	McheckErr(status, "Error getting envelope data handle\n");	
//	double env = envData.asDouble();	

	const double halfRandMax = (double)RAND_MAX * 0.5;

	srand(sd);
	for ( ; !iter.isDone(); iter.next()) {
		
		MPoint pt = iter.position();
		double rx = (halfRandMax - (double)rand());
		double ry = (halfRandMax - (double)rand());
		double rz = (halfRandMax - (double)rand());

		double rl = sqrt(rx*rx + ry*ry + rz*rz);

		double dx, dy, dz;
		if(rl == 0)
		{
			dx = 0;
			dy = 0;
			dz = 0;
		}
		else
		{
			dx = rx * (fabs(axmx - axmn)) / rl;
			dy = ry * (fabs(aymx - aymn)) / rl;
			dz = rz * (fabs(azmx - azmn)) / rl;
		}

		if(rx<0) dx-= axmn; else dx+= axmn;
		if(ry<0) dy-= aymn; else dy+= aymn;
		if(rz<0) dz-= azmn; else dz+= azmn;

		pt.x += dx * mult;// * env;
		pt.y += dy * mult;// * env;
		pt.z += dz * mult;// * env;

		iter.setPosition(pt);
	}
	return status;
}
