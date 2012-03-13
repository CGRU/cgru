
#include "noiseNode.h"

#define McheckErr(stat,msg)		\
	if ( MS::kSuccess != stat ) {	\
		stat.perror(msg);				\
		return MS::kFailure;		\
	}



MTypeId     tm_noisePerlin::id( tm_noisePerlin__id );

MObject     tm_noisePerlin::dummy;

MObject     tm_noisePerlin::amplitude;
MObject     tm_noisePerlin::frequency;
MObject     tm_noisePerlin::levels;
MObject     tm_noisePerlin::lev_Mamp;
MObject     tm_noisePerlin::lev_Mfreq;
MObject     tm_noisePerlin::scale;
MObject     tm_noisePerlin::scaleAmpX;
MObject     tm_noisePerlin::scaleAmpY;
MObject     tm_noisePerlin::scaleAmpZ;
MObject     tm_noisePerlin::scaleFreqX;
MObject     tm_noisePerlin::scaleFreqY;
MObject     tm_noisePerlin::scaleFreqZ;
MObject     tm_noisePerlin::variation;


tm_noisePerlin::tm_noisePerlin(){}
tm_noisePerlin::~tm_noisePerlin(){}
void* tm_noisePerlin::creator(){return new tm_noisePerlin();}

MStatus tm_noisePerlin::initialize()
{
	MStatus status = MS::kSuccess;
	MFnNumericAttribute nAttr;

	amplitude=nAttr.create( "amplitude", "amp", MFnNumericData::kDouble, 1.0, &status );
	McheckErr(status, "Error creating amplitude attribute\n");
//	nAttr.setDefault(1.0);
	nAttr.setKeyable(true);
	addAttribute( amplitude);

	frequency=nAttr.create( "frequency", "freq", MFnNumericData::kDouble, 1.0, &status );
	McheckErr(status, "Error creating frequency attribute\n");
//	nAttr.setDefault(1.0);
	nAttr.setKeyable(true);
	addAttribute( frequency);

	levels=nAttr.create( "levels", "lev", MFnNumericData::kShort, 2.0, &status );
	McheckErr(status, "Error creating levels attribute\n");
//	nAttr.setDefault(2);
	nAttr.setKeyable(true);
	addAttribute( levels);

	lev_Mamp=nAttr.create( "lev_Mamp", "levma", MFnNumericData::kDouble, 0.25, &status );
	McheckErr(status, "Error creating lev_Mamp attribute\n");
//	nAttr.setDefault(0.25);
	nAttr.setKeyable(true);
	addAttribute( lev_Mamp);

	lev_Mfreq=nAttr.create( "lev_Mfreq", "levmf", MFnNumericData::kDouble );
	nAttr.setDefault(4.0);
	nAttr.setKeyable(true);
	addAttribute( lev_Mfreq);

	scale=nAttr.create( "scale", "sc", MFnNumericData::kDouble );
	nAttr.setDefault(1.0);
	nAttr.setKeyable(true);
	addAttribute( scale);

	scaleAmpX=nAttr.create( "scaleAmpX", "sx", MFnNumericData::kDouble );
	nAttr.setDefault(1.0);
	nAttr.setKeyable(true);
	addAttribute( scaleAmpX);

	scaleAmpY=nAttr.create( "scaleAmpY", "sy", MFnNumericData::kDouble );
	nAttr.setDefault(1.0);
	nAttr.setKeyable(true);
	addAttribute( scaleAmpY);

	scaleAmpZ=nAttr.create( "scaleAmpZ", "sz", MFnNumericData::kDouble );
	nAttr.setDefault(1.0);
	nAttr.setKeyable(true);
	addAttribute( scaleAmpZ);

	scaleFreqX=nAttr.create( "scaleFreqX", "sfx", MFnNumericData::kDouble );
	nAttr.setDefault(1.0);
	nAttr.setKeyable(true);
	addAttribute( scaleFreqX);

	scaleFreqY=nAttr.create( "scaleFreqY", "sfy", MFnNumericData::kDouble );
	nAttr.setDefault(1.0);
	nAttr.setKeyable(true);
	addAttribute( scaleFreqY);

	scaleFreqZ=nAttr.create( "scaleFreqZ", "sfz", MFnNumericData::kDouble );
	nAttr.setDefault(1.0);
	nAttr.setKeyable(true);
	addAttribute( scaleFreqZ);

	variation=nAttr.create( "variation", "var", MFnNumericData::kDouble );
	nAttr.setDefault(0.0);
	nAttr.setKeyable(true);
	addAttribute( variation);

	attributeAffects( tm_noisePerlin::amplitude, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::frequency, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::levels, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::lev_Mamp, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::lev_Mfreq, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::scale, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::scaleAmpX, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::scaleAmpY, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::scaleAmpZ, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::scaleFreqX, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::scaleFreqY, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::scaleFreqZ, tm_noisePerlin::outputGeom );
	attributeAffects( tm_noisePerlin::variation, tm_noisePerlin::outputGeom );

	return MS::kSuccess;
}

MStatus
tm_noisePerlin::deform( MDataBlock& block,
				MItGeometry& iter,
				const MMatrix& /*m*/,
				unsigned int /*multiIndex*/)
{
	MStatus status = MS::kSuccess;

	// It's a fake data access try to workaround strange behavior on x86_64 linux systems...
	MDataHandle dummyData = block.inputValue(dummy,&status);

	MDataHandle lev_MampData = block.inputValue(lev_Mamp,&status);
	McheckErr(status, "Error getting lev_Mamp data handle\n");
	double _lev_Mamp = lev_MampData.asDouble();

	MDataHandle lev_MfreqData = block.inputValue(lev_Mfreq,&status);
	McheckErr(status, "Error getting lev_Mfreq data handle\n");
	double lev_Mfreq = lev_MfreqData.asDouble();

	MDataHandle levelsData = block.inputValue(levels,&status);
	McheckErr(status, "Error getting frequency data handle\n");
	short levels = levelsData.asShort();

	MDataHandle scaleData = block.inputValue(scale,&status);
	McheckErr(status, "Error getting scale data handle\n");
	double scale = scaleData.asDouble();

	MDataHandle scaleAmpXData = block.inputValue(scaleAmpX,&status);
	McheckErr(status, "Error getting scaleAmpX data handle\n");
	double scaleAmpX = scaleAmpXData.asDouble();

	MDataHandle scaleAmpYData = block.inputValue(scaleAmpY,&status);
	McheckErr(status, "Error getting scaleAmpY data handle\n");
	double scaleAmpY = scaleAmpYData.asDouble();

	MDataHandle scaleAmpZData = block.inputValue(scaleAmpZ,&status);
	McheckErr(status, "Error getting scaleAmpZ data handle\n");
	double scaleAmpZ = scaleAmpZData.asDouble();

	MDataHandle scaleFreqXData = block.inputValue(scaleFreqX,&status);
	McheckErr(status, "Error getting scaleFreqX data handle\n");
	double scaleFreqX = scaleFreqXData.asDouble();

	MDataHandle scaleFreqYData = block.inputValue(scaleFreqY,&status);
	McheckErr(status, "Error getting scaleFreqY data handle\n");
	double scaleFreqY = scaleFreqYData.asDouble();

	MDataHandle scaleFreqZData = block.inputValue(scaleFreqZ,&status);
	McheckErr(status, "Error getting scaleFreqZ data handle\n");
	double scaleFreqZ = scaleFreqZData.asDouble();

	MDataHandle variationData = block.inputValue(variation,&status);
	McheckErr(status, "Error getting variation data handle\n");
	double variation = variationData.asDouble();

	MDataHandle envData = block.inputValue(envelope,&status);
	McheckErr(status, "Error getting envelope data handle\n");
	double env = envData.asDouble();

	MDataHandle amplitudeData = block.inputValue(amplitude,&status);
	McheckErr(status, "Error getting amplitude data handle\n");
	double amplitude = amplitudeData.asDouble();

	MDataHandle frequencyData = block.inputValue(frequency,&status);
	McheckErr(status, "Error getting frequency data handle\n");
	double frequency = frequencyData.asDouble();

	amplitude = amplitude * scale;
	frequency = frequency * 0.01 / scale;


	for ( ; !iter.isDone(); iter.next()) {

		MPoint pt = iter.position();
		vector noisePnt; noisePnt.x = 0; noisePnt.y = 0; noisePnt.z = 0;

		double l_amp = amplitude;

		double x = scaleFreqX * pt.x * frequency;
		double y = scaleFreqY * pt.y * frequency;
		double z = scaleFreqZ * pt.z * frequency;

		for( int lev = 0; lev < levels; lev++)
		{
			x *= lev_Mfreq;
			y *= lev_Mfreq;
			z *= lev_Mfreq;
			vector lev_Pnt = INoise::noise4d_v(x, y, z, variation);
			noisePnt.x += lev_Pnt.x * l_amp;
			noisePnt.y += lev_Pnt.y * l_amp;
			noisePnt.z += lev_Pnt.z * l_amp;
			l_amp *= _lev_Mamp;
		}

		pt.x += noisePnt.x * scaleAmpX;
		pt.y += noisePnt.y * scaleAmpY;
		pt.z += noisePnt.z * scaleAmpZ;

		iter.setPosition(pt);
	}
	return status;
}
