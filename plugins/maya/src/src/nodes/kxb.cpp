#include "kxb.h"

MTypeId     tm_kxb::id( tm_kxb__id );
MObject     tm_kxb::input_k;        
MObject     tm_kxb::input_x;        
MObject     tm_kxb::input_b;        
MObject     tm_kxb::output;       

tm_kxb::tm_kxb() {}
tm_kxb::~tm_kxb() {}

MStatus tm_kxb::compute( const MPlug& plug, MDataBlock& data )
{
	
	MStatus returnStatus;
 
	if( plug == output )
	{
		MDataHandle inputData_k = data.inputValue( input_k, &returnStatus );
		MDataHandle inputData_x = data.inputValue( input_x, &returnStatus );
		MDataHandle inputData_b = data.inputValue( input_b, &returnStatus );

		if( returnStatus == MS::kSuccess )
		{
			float result = inputData_k.asFloat() * inputData_x.asFloat() + inputData_b.asFloat();
			MDataHandle outputHandle = data.outputValue( tm_kxb::output );
			outputHandle.set( result );
			data.setClean(plug);
		}
	}
	else
	{
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

void* tm_kxb::creator()
{
	return new tm_kxb();
}

MStatus tm_kxb::initialize()
{
	MFnNumericAttribute nAttr;
	MStatus				stat;

	input_k = nAttr.create( "input_k", "in_k", MFnNumericData::kFloat, 0.0 );
 	nAttr.setKeyable(true);

	input_x = nAttr.create( "input_x", "in_x", MFnNumericData::kFloat, 0.0 );
 	nAttr.setKeyable(true);

	input_b = nAttr.create( "input_b", "in_b", MFnNumericData::kFloat, 0.0 );
 	nAttr.setKeyable(true);

	output = nAttr.create( "output", "out", MFnNumericData::kFloat, 0.0 );
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	stat = addAttribute( input_k );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( input_x );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( input_b );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( output );
		if (!stat) { stat.perror("addAttribute"); return stat;}

	stat = attributeAffects( input_k, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( input_x, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( input_b, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}

	return MS::kSuccess;
}
