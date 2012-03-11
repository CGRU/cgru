
#include "kxbci.h"

MTypeId     tm_kxbci::id( tm_kxbci__id );
MObject     tm_kxbci::input_x;        
MObject     tm_kxbci::clampInput;        
MObject     tm_kxbci::clampInput_min;        
MObject     tm_kxbci::clampInput_max;        
MObject     tm_kxbci::invertInput;        
MObject     tm_kxbci::in_k;        
MObject     tm_kxbci::in_b;        
MObject     tm_kxbci::invertResult;        
MObject     tm_kxbci::clampResult;        
MObject     tm_kxbci::clampResult_min;        
MObject     tm_kxbci::clampResult_max;        
MObject     tm_kxbci::output;       

tm_kxbci::tm_kxbci() {}
tm_kxbci::~tm_kxbci() {}

MStatus tm_kxbci::compute( const MPlug& plug, MDataBlock& data )
{
	
	MStatus returnStatus;
 
	if( plug == output )
	{
		MDataHandle inputData_x = data.inputValue( input_x, &returnStatus );
		MDataHandle inputData_clp_i = data.inputValue( clampInput, &returnStatus );
		MDataHandle inputData_clp_i_min = data.inputValue( clampInput_min, &returnStatus );
		MDataHandle inputData_clp_i_max = data.inputValue( clampInput_max, &returnStatus );
		MDataHandle inputData_inv_i = data.inputValue( invertInput, &returnStatus );
		MDataHandle inputData_k = data.inputValue( in_k, &returnStatus );
		MDataHandle inputData_b = data.inputValue( in_b, &returnStatus );
		MDataHandle inputData_inv_r = data.inputValue( invertResult, &returnStatus );
		MDataHandle inputData_clp_r = data.inputValue( clampResult, &returnStatus );
		MDataHandle inputData_clp_r_min = data.inputValue( clampResult_min, &returnStatus );
		MDataHandle inputData_clp_r_max = data.inputValue( clampResult_max, &returnStatus );

		if( returnStatus == MS::kSuccess )
		{
			float result = inputData_x.asFloat();
			float clamp;
			if( inputData_clp_i.asBool() == true ){
				clamp = inputData_clp_i_min.asFloat();
				if( result < clamp ) result = clamp;
				clamp = inputData_clp_i_max.asFloat();
				if( result > clamp ) result = clamp;
			}
			if( inputData_inv_i.asBool() == true ) result = -result;

			result = inputData_k.asFloat() * result + inputData_b.asFloat();

			if( inputData_inv_r.asBool() == true ) result = -result;
			if( inputData_clp_r.asBool() == true ){
				clamp = inputData_clp_r_min.asFloat();
				if( result < clamp ) result = clamp;
				clamp = inputData_clp_r_max.asFloat();
				if( result > clamp ) result = clamp;
			}

			MDataHandle outputHandle = data.outputValue( tm_kxbci::output );
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

void* tm_kxbci::creator()
{
	return new tm_kxbci();
}

MStatus tm_kxbci::initialize()
{
	MFnNumericAttribute nAttr;
	MStatus				stat;

	input_x = nAttr.create( "input_x", "in_x", MFnNumericData::kFloat, 0.0 );
 	nAttr.setKeyable(true);
	clampInput = nAttr.create( "clampInput", "clpi", MFnNumericData::kBoolean, 0 );
 	nAttr.setKeyable(true);
	clampInput_min = nAttr.create( "clampInput_min", "clpimin", MFnNumericData::kFloat, 0 );
 	nAttr.setKeyable(true);
	clampInput_max = nAttr.create( "clampInput_max", "clpimax", MFnNumericData::kFloat, 0 );
 	nAttr.setKeyable(true);
	invertInput = nAttr.create( "invertInput", "invi", MFnNumericData::kBoolean, 0 );
 	nAttr.setKeyable(true);
	in_k = nAttr.create( "in_k", "k", MFnNumericData::kFloat, 0.0 );
 	nAttr.setKeyable(true);
	in_b = nAttr.create( "in_b", "b", MFnNumericData::kFloat, 0.0 );
 	nAttr.setKeyable(true);
	invertResult = nAttr.create( "invertResult", "invr", MFnNumericData::kBoolean, 0 );
 	nAttr.setKeyable(true);
	clampResult = nAttr.create( "clampResult", "clpr", MFnNumericData::kBoolean, 0 );
 	nAttr.setKeyable(true);
	clampResult_min = nAttr.create( "clampResult_min", "clprmin", MFnNumericData::kFloat, 0 );
 	nAttr.setKeyable(true);
	clampResult_max = nAttr.create( "clampResult_max", "clprmax", MFnNumericData::kFloat, 0 );
 	nAttr.setKeyable(true);

	output = nAttr.create( "output", "out", MFnNumericData::kFloat, 0.0 );
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	stat = addAttribute( input_x );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( clampInput );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( clampInput_min );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( clampInput_max );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( invertInput );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( in_k );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( in_b );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( invertResult );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( clampResult );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( clampResult_min );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( clampResult_max );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( output );
		if (!stat) { stat.perror("addAttribute"); return stat;}

	stat = attributeAffects( input_x, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( clampInput, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( clampInput_min, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( clampInput_max, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( invertInput, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( in_k, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( in_b, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( invertResult, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( clampResult, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( clampResult_min, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( clampResult_max, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}

	return MS::kSuccess;
}
