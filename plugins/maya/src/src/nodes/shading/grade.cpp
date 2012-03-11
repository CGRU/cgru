#include "grade.h"

#include <math.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MDataHandle.h>

#ifdef _DEBUG
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#endif

MTypeId grade_tm::id( grade_tm__id );

MObject grade_tm::inColor;

MObject grade_tm::pointBlackObj;
MObject grade_tm::pointWhiteObj;
MObject grade_tm::satObj;
MObject grade_tm::gainObj;
MObject grade_tm::gammaObj;
MObject grade_tm::offsetObj;
MObject grade_tm::multObj;
MObject grade_tm::clampBlackObj;
MObject grade_tm::clampWhiteObj;
MObject grade_tm::invertObj;

MObject grade_tm::outColor;

grade_tm::grade_tm() {}
grade_tm::~grade_tm() {}

MStatus grade_tm::compute( const MPlug& plug, MDataBlock& data)
{

	MStatus status;
 
	if( plug == outColor)
	{
		MDataHandle dataHandle;
		float *p_c;
		
// getting input attributes:
//
		dataHandle = data.inputValue( inColor, &status);
		p_c = dataHandle.asFloat3();
		float color[3]; color[0] = p_c[0]; color[1] = p_c[1]; color[2] = p_c[2];

		dataHandle = data.inputValue( pointBlackObj, &status);
		p_c = dataHandle.asFloat3();
		float black[3]; black[0] = p_c[0]; black[1] = p_c[1]; black[2] = p_c[2];

		dataHandle = data.inputValue( pointWhiteObj, &status);
		p_c = dataHandle.asFloat3();
		float white[3]; white[0] = p_c[0]; white[1] = p_c[1]; white[2] = p_c[2];

		dataHandle = data.inputValue( satObj, &status);
		float saturation = dataHandle.asFloat();

		dataHandle = data.inputValue( gainObj, &status);
		float gain = dataHandle.asFloat();

		dataHandle = data.inputValue( gammaObj, &status);
		float gamma = dataHandle.asFloat();

		dataHandle = data.inputValue( multObj, &status);
		p_c = dataHandle.asFloat3();
		float mult[3]; mult[0] = p_c[0]; mult[1] = p_c[1]; mult[2] = p_c[2];

		dataHandle = data.inputValue( offsetObj, &status);
		p_c = dataHandle.asFloat3();
		float offset[3]; offset[0] = p_c[0]; offset[1] = p_c[1]; offset[2] = p_c[2];

		dataHandle = data.inputValue( clampBlackObj, &status);
		bool clampBlack = dataHandle.asBool();
		dataHandle = data.inputValue( clampWhiteObj, &status);
		bool clampWhite = dataHandle.asBool();

		dataHandle = data.inputValue( invertObj, &status);
		bool invert = dataHandle.asBool();

// calculating result:
//
		color[0] = (color[0] - black[0]) / (white[0] - black[0]);
		color[1] = (color[1] - black[1]) / (white[1] - black[1]);
		color[2] = (color[2] - black[2]) / (white[2] - black[2]);

		if( saturation != 1.0f)
		{
			float luminance = 0.2125f*color[0] + 0.7154f*color[1] + .0721f*color[2];
			color[0] = color[0] * saturation + (1 - saturation) * luminance;
			color[1] = color[1] * saturation + (1 - saturation) * luminance;
			color[2] = color[2] * saturation + (1 - saturation) * luminance;
		}

		if(gain != 1.0f ) {color[0] *= gain; color[1] *= gain; color[2] *= gain;}

		if( gamma != 1.0f)
		if( color[0] > 0) color[0] = pow( color[0], gamma);
		if( color[1] > 0) color[1] = pow( color[1], gamma);
		if( color[2] > 0) color[2] = pow( color[2], gamma);

		color[0] *= mult[0]; color[1] *= mult[1]; color[2] *= mult[2];

		color[0] += offset[0]; color[1] += offset[1]; color[2] += offset[2];

		if(invert) { color[0] = 1 - color[0]; color[1] = 1 - color[1]; color[2] = 1 - color[2];}

		if(clampBlack)
		{
			if(color[0] < 0) color[0] = 0;
			if(color[1] < 0) color[1] = 0;
			if(color[2] < 0) color[2] = 0;
		}
		if(clampWhite)
		{
			if(color[0] > 1) color[0] = 1.0f;
			if(color[1] > 1) color[1] = 1.0f;
			if(color[2] > 1) color[2] = 1.0f;
		}

// setting output attribute:
//

		MDataHandle outputHandle = data.outputValue( outColor, &status);
		outputHandle.set( color[0], color[1], color[2]);

		data.setClean(plug);
	}
	else
	{
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

MStatus grade_tm::initialize()
{
	MFnNumericAttribute nAttr;
	MStatus				status;

// creating attributes:
//
	inColor = nAttr.createColor( "inColor", "iC");
 	nAttr.setKeyable(true);
	nAttr.setDefault( 0.5f, 0.5f, 0.5f);

	pointBlackObj = nAttr.createColor( "pointBlack", "p_b");
 	nAttr.setKeyable(true);
	nAttr.setDefault( 0.0f, 0.0f, 0.0f);

	pointWhiteObj = nAttr.createColor( "pointWhite", "p_w");
 	nAttr.setKeyable(true);
	nAttr.setDefault( 1.0f, 1.0f, 1.0f);

	satObj = nAttr.create( "saturation", "sat", MFnNumericData::kFloat, 1.0f);
	nAttr.setMin( 0.0f);
 	nAttr.setKeyable(true);

	multObj = nAttr.createColor( "multiply", "mult");
 	nAttr.setKeyable(true);
	nAttr.setDefault( 1.0f, 1.0f, 1.0f);

	offsetObj = nAttr.createColor( "offset", "offset");
 	nAttr.setKeyable(true);
	nAttr.setDefault( 0.0f, 0.0f, 0.0f);

	gainObj = nAttr.create( "gain", "gain", MFnNumericData::kFloat, 1.0f);
 	nAttr.setKeyable(true);

	gammaObj = nAttr.create( "gamma", "gamma", MFnNumericData::kFloat, 1.0f);
 	nAttr.setKeyable(true);

	invertObj = nAttr.create( "invert", "inv", MFnNumericData::kBoolean, false);
 	nAttr.setKeyable(true);

	clampBlackObj = nAttr.create( "clampBlack", "cl_b", MFnNumericData::kBoolean, true);
 	nAttr.setKeyable(true);
	clampWhiteObj = nAttr.create( "clampWhite", "cl_w", MFnNumericData::kBoolean, false);
 	nAttr.setKeyable(true);

	outColor = nAttr.createColor( "outColor", "oC");


// adding attributes:
//
	status = addAttribute( inColor );
		if (!status) { status.perror("addAttribute inColor"); return status;}

	status = addAttribute( pointBlackObj );
		if (!status) { status.perror("addAttribute pointBlackObj"); return status;}
	status = addAttribute( pointWhiteObj );
		if (!status) { status.perror("addAttribute pointWhiteObj"); return status;}
	status = addAttribute( satObj );
		if (!status) { status.perror("addAttribute multObj"); return status;}
	status = addAttribute( gainObj );
		if (!status) { status.perror("addAttribute gainObj"); return status;}
	status = addAttribute( gammaObj );
		if (!status) { status.perror("addAttribute gammaObj"); return status;}
	status = addAttribute( multObj );
		if (!status) { status.perror("addAttribute multObj"); return status;}
	status = addAttribute( offsetObj );
		if (!status) { status.perror("addAttribute offsetObj"); return status;}
	status = addAttribute( invertObj );
		if (!status) { status.perror("addAttribute invertObj"); return status;}
	status = addAttribute( clampBlackObj );
		if (!status) { status.perror("addAttribute clampBlackObj"); return status;}
	status = addAttribute( clampWhiteObj );
		if (!status) { status.perror("addAttribute clampWhiteObj"); return status;}
		
	status = addAttribute( outColor );
		if (!status) { status.perror("addAttribute outColor"); return status;}


// setting attributes affects:
//
	status = attributeAffects( inColor, outColor );
		if (!status) { status.perror("attributeAffects inColor"); return status;}

	status = attributeAffects( pointBlackObj, outColor );
		if (!status) { status.perror("attributeAffects pointBlackObj"); return status;}
	status = attributeAffects( pointWhiteObj, outColor );
		if (!status) { status.perror("attributeAffects pointWhiteObj"); return status;}
	status = attributeAffects( satObj, outColor );
		if (!status) { status.perror("attributeAffects satObj"); return status;}
	status = attributeAffects( gainObj, outColor );
		if (!status) { status.perror("attributeAffects gainObj"); return status;}
	status = attributeAffects( gammaObj, outColor );
		if (!status) { status.perror("attributeAffects gammaObj"); return status;}
	status = attributeAffects( multObj, outColor );
		if (!status) { status.perror("attributeAffects multObj"); return status;}
	status = attributeAffects( offsetObj, outColor );
		if (!status) { status.perror("attributeAffects offsetObj"); return status;}
	status = attributeAffects( invertObj, outColor );
		if (!status) { status.perror("attributeAffects invertObj"); return status;}
	status = attributeAffects( clampBlackObj, outColor );
		if (!status) { status.perror("attributeAffects clampBlackObj"); return status;}
	status = attributeAffects( clampWhiteObj, outColor );
		if (!status) { status.perror("attributeAffects clampWhiteObj"); return status;}
/**/
	return MS::kSuccess;
}

void* grade_tm::creator()
{
	return new grade_tm();
}
