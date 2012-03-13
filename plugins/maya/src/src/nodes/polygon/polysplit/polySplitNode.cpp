
#include "polySplitNode.h"
#include "../../../definitions.h"

// Function Sets
//
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>

// General Includes
//
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MIOStream.h>
#include <maya/MVector.h>
#include <maya/MQuaternion.h>

#include <maya/MPxManipContainer.h>

// Macros
//
#define MCheckStatus(status,message)	\
	if( MStatus::kSuccess != status ) {	\
		status.perror(message);		\
		return status;					\
	}


// Unique Node TypeId
//
MTypeId     tm_polySplitNode::id( tm_polySplitNode__id );

// Node attributes (in addition to inMesh and outMesh defined by polyModifierNode)
//

MObject tm_polySplitNode::inMesh;
MObject tm_polySplitNode::edgesList;

MObject tm_polySplitNode::useMesh;

MObject tm_polySplitNode::factor;
MObject tm_polySplitNode::edgeDirection;
MObject tm_polySplitNode::offsetX;
MObject tm_polySplitNode::offsetY;
MObject tm_polySplitNode::offsetZ;
MObject tm_polySplitNode::normalize;
MObject tm_polySplitNode::outMesh;
MObject tm_polySplitNode::uvMethod;
MObject tm_polySplitNode::offsetU;
MObject tm_polySplitNode::offsetV;
MObject tm_polySplitNode::offsetF;
MObject tm_polySplitNode::edgePos;

MObject tm_polySplitNode::splitAlgorithm;
MObject tm_polySplitNode::splitLoopMode;
MObject tm_polySplitNode::splitLoopAngle;
MObject tm_polySplitNode::splitLoopMaxCount;

tm_polySplitNode::tm_polySplitNode():
					recompute(true),
					firstTime(true)
{}

tm_polySplitNode::~tm_polySplitNode()
{}

MStatus tm_polySplitNode::compute( const MPlug& plug, MDataBlock& data )
//	Arguments:
//		plug - the plug to compute
//		data - object that provides access to the attributes for this node
{
#ifdef _DEBUG
cout << endl << "####################################################");
cout << "tm_polySplitNode::compute - DEBUG version info:");
#endif
	MStatus status = MS::kSuccess;
 
	MDataHandle stateData = data.outputValue( state, &status );
	MCheckStatus( status, "ERROR getting state" );

	// Check for the HasNoEffect/PassThrough flag on the node.
	//
	// (stateData is an enumeration standard in all depend nodes - stored as short)
	// 
	// (0 = Normal)
	// (1 = HasNoEffect/PassThrough)
	// (2 = Blocking)
	// ...
	//
	if( stateData.asShort() == 1 )
	{
		MDataHandle inputMeshData = data.inputValue( inMesh, &status );
		MCheckStatus(status,"ERROR getting inMesh");

		MDataHandle outputMeshData = data.outputValue( outMesh, &status );
		MCheckStatus(status,"ERROR getting outMesh");

		// Simply redirect the inMesh to the outMesh for the PassThrough effect
		//
		outputMeshData.set(inputMeshData.asMesh());
	}
	else
	{
		if (plug == outMesh)
		{
			MDataHandle outputMeshData = data.outputValue( outMesh, &status);
			if( !status) {status.perror("ERROR getting outMesh data"); return status;}

			MDataHandle factorData = data.inputValue( factor, &status );
			if( !status) {status.perror("ERROR factor data"); return status;}
			double edgeFactror = factorData.asDouble();
			
			MDataHandle offsetXData = data.inputValue( offsetX, &status );
			if( !status) {status.perror("ERROR offsetX data"); return status;}
			double ox = offsetXData.asDouble();
			
			MDataHandle offsetYData = data.inputValue( offsetY, &status );
			if( !status) {status.perror("ERROR offsetY data"); return status;}
			double oy = offsetYData.asDouble();
			
			MDataHandle offsetZData = data.inputValue( offsetZ, &status );
			if( !status) {status.perror("ERROR offsetZ data"); return status;}
			double oz = offsetZData.asDouble();
			
			MDataHandle offsetFData = data.inputValue( offsetF, &status );
			if( !status) {status.perror("ERROR offsetF data"); return status;}
			double of = offsetFData.asDouble();
			
			MDataHandle offsetUData = data.inputValue( offsetU, &status );
			if( !status) {status.perror("ERROR offsetU data"); return status;}
			double ou = offsetUData.asDouble();
			
			MDataHandle offsetVData = data.inputValue( offsetV, &status );
			if( !status) {status.perror("ERROR offsetV data"); return status;}
			double ov = offsetVData.asDouble();
			
			MDataHandle normalizeData = data.inputValue( normalize, &status );
			if( !status) {status.perror("ERROR normalize data"); return status;}
			bool norm = false;
			if(normalizeData.asShort() == 1) norm = true;

			MDataHandle useMeshData = data.inputValue( useMesh, &status );
			if( !status) {status.perror("ERROR useMesh data"); return status;}
			bool useInternalMesh = true;
			if(useMeshData.asShort() == 1) useInternalMesh = false;

			MDataHandle uvMethodData = data.inputValue( uvMethod, &status );
			if( !status) {status.perror("ERROR uvMethod data"); return status;}
			bool uvMethod_factor = true;
			if(uvMethodData.asShort() == 1) uvMethod_factor = false;

			MDataHandle edgeDirectionData = data.inputValue( edgeDirection, &status );
			if( !status) {status.perror("ERROR edgeDirection data"); return status;}
			bool reverseEdgeDirection = false;
			if(edgeDirectionData.asShort() == 1) reverseEdgeDirection = true;

			MObject meshObj;
			MFnMesh meshFn;

			if( recompute  || (useInternalMesh == false))
			{
				MDataHandle inputMeshData = data.inputValue( inMesh, &status );
				MCheckStatus(status,"ERROR getting inMesh");

				outputMeshData.set(inputMeshData.asMesh());
				meshObj = outputMeshData.asMesh();
				fSplitFactory.setMesh( meshObj );

				MDataHandle inputEdges = data.inputValue( edgesList, &status);
				MCheckStatus(status,"ERROR getting edgesList"); 
			
				// get split algorithm data
				MDataHandle splitAlgorithm_data;
				splitAlgorithm_data = data.inputValue( splitAlgorithm);
				if( splitAlgorithm_data.asShort())
				{
					fSplitFactory.fa_loop = false;
					fSplitFactory.fa_sel = true;
				}
				else
				{
					fSplitFactory.fa_loop = true;
					fSplitFactory.fa_sel = false;
				}
				splitAlgorithm_data = data.inputValue( splitLoopMode);
				if( splitAlgorithm_data.asShort() < 2)
					fSplitFactory.fa_loop_mode = 1;
				else if( splitAlgorithm_data.asShort() > 2)
					fSplitFactory.fa_loop_mode = 3;
				else fSplitFactory.fa_loop_mode = 2;
				splitAlgorithm_data = data.inputValue( splitLoopAngle);
				fSplitFactory.fa_loop_angle = splitAlgorithm_data.asDouble();
				splitAlgorithm_data = data.inputValue( splitLoopMaxCount);
				fSplitFactory.fa_maxcount = splitAlgorithm_data.asLong();

				if(firstTime)
				{
					MObject compList = inputEdges.data();
					MFnComponentListData compListFn( compList );
					edgesIds.clear();

					for( unsigned i = 0; i < compListFn.length(); i++ )
					{
						MObject comp = compListFn[i];
						if( comp.apiType() == MFn::kMeshEdgeComponent )
						{
							MFnSingleIndexedComponent edgeComp( comp );
							for( int j = 0; j < edgeComp.elementCount(); j++ )
							{
								int edgeId = edgeComp.element(j);
								edgesIds.append( edgeId );
							}
						}
					}
#ifdef _DEBUG
cout << endl << "##########################tm_polySplitNode::compute");
cout << endl << "edgesIds = ";for(unsigned i=0;i<edgesIds.length();i++) cout << edgesIds[i] << " ";cout << endl;
#endif
				}
				fSplitFactory.setEdgesIds( edgesIds );
				status = fSplitFactory.doIt();
				if(!status)
				{
					MGlobal::displayError( "tm_polySplitNode::compute - split action failed." );
					return MStatus::kFailure;
				}
				MDataHandle edgePosData = data.inputValue( edgePos, &status );
				edgePosData.set( fSplitFactory.averagePos[0], fSplitFactory.averagePos[1], fSplitFactory.averagePos[2]);

				recompute = false;
				firstTime = false;
				if( firstTime) fSplitFactory.firstTime = false;
			}
//######################################################## move concrete vertices:
			meshObj = outputMeshData.asMesh();
			meshFn.setObject( meshObj);
#ifdef _DEBUG
cout << endl << "#tm_polySplitNode - start to offset vertices...");
cout << endl << "fSplitFactory.newVtxCount = " << fSplitFactory.newVtxCount << endl;
cout << endl << "fSplitFactory.newUVsCount = " << fSplitFactory.newUVsCount << endl;
cout << endl << "fSplitFactory.oldVtxCount = " << fSplitFactory.oldVtxCount << endl;
cout << endl << "fSplitFactory.oldUVsCount = " << fSplitFactory.oldUVsCount << endl;
#endif
			MVector vector;
			MPoint point;
			//############################################## move vertices:
			{
			unsigned i = 0;
			for( int j = fSplitFactory.oldVtxCount; j < fSplitFactory.newVtxCount; j++)
			{
				vector.x = ox;
				vector.y = oy;
				vector.z = oz;
				if( reverseEdgeDirection)
				{
					if(norm)
						vector += fSplitFactory.splitedPoints_ndir_R[i] * edgeFactror;
					else
						vector += fSplitFactory.splitedPoints_dir_R[i] * edgeFactror;
					point = fSplitFactory.splitedPoints_start_R[i] + vector;
				}
				else
				{
					if(norm)
						vector += fSplitFactory.splitedPoints_ndir_N[i] * edgeFactror;
					else
						vector += fSplitFactory.splitedPoints_dir_N[i] * edgeFactror;
					point = fSplitFactory.splitedPoints_start_N[i] + vector;
				}
				status = meshFn.setPoint( j, point);
				if(!status) status.perror("can't move points");
				i++;
			}
			}
			//################################################## move UVs:
			{
			unsigned i = 0;
			if( uvMethod_factor)	edgeFactror += of;
			else					edgeFactror = of;
			for( int j = fSplitFactory.oldUVsCount; j < fSplitFactory.newUVsCount; j++)
			{
				double u = ou;
				double v = ov;
				if( reverseEdgeDirection)
				{
					if(norm)
					{
						u += fSplitFactory.splitedUVsU_start_R[i] + (fSplitFactory.splitedUVsU_ndir_R[i] * edgeFactror);
						v += fSplitFactory.splitedUVsV_start_R[i] + (fSplitFactory.splitedUVsV_ndir_R[i] * edgeFactror);
					}
					else
					{
						u += fSplitFactory.splitedUVsU_start_R[i] + (fSplitFactory.splitedUVsU_dir_R[i] * edgeFactror);
						v += fSplitFactory.splitedUVsV_start_R[i] + (fSplitFactory.splitedUVsV_dir_R[i] * edgeFactror);
					}
				}
				else
				{
					if(norm)
					{
						u += fSplitFactory.splitedUVsU_start_N[i] + (fSplitFactory.splitedUVsU_ndir_N[i] * edgeFactror);
						v += fSplitFactory.splitedUVsV_start_N[i] + (fSplitFactory.splitedUVsV_ndir_N[i] * edgeFactror);
					}
					else
					{
						u += fSplitFactory.splitedUVsU_start_N[i] + (fSplitFactory.splitedUVsU_dir_N[i] * edgeFactror);
						v += fSplitFactory.splitedUVsV_start_N[i] + (fSplitFactory.splitedUVsV_dir_N[i] * edgeFactror);
					}
				}
				meshFn.setUV( j, (float)u, (float)v);
				i++;
			}
			}
			outputMeshData.setClean();
		}
		else
		{
			status = MS::kUnknownParameter;
		}
	}

	return status;
}

void* tm_polySplitNode::creator()
{
	return new tm_polySplitNode();
}

MStatus tm_polySplitNode::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute typedAttr;
	MFnEnumAttribute enumAttr;
	MStatus stat;

	useMesh = enumAttr.create("useMesh", "um", 0, 0);
	enumAttr.addField( "cashed", 0);
	enumAttr.addField( "recompute", 1);
 	enumAttr.setKeyable(true);

	inMesh = typedAttr.create("inMesh", "im", MFnMeshData::kMesh);
	typedAttr.setStorable(true);

	edgesList = typedAttr.create("inputComponents", "ics", MFnComponentListData::kComponentList);

	factor = nAttr.create( "factor", "f", MFnNumericData::kDouble, 0.5 );
 	nAttr.setKeyable(true);

	offsetX = nAttr.create( "offsetX", "ox", MFnNumericData::kDouble, 0.0 );
 	nAttr.setKeyable(true);
	offsetY = nAttr.create( "offsetY", "oy", MFnNumericData::kDouble, 0.0 );
 	nAttr.setKeyable(true);
	offsetZ = nAttr.create( "offsetZ", "oz", MFnNumericData::kDouble, 0.0 );
 	nAttr.setKeyable(true);

	normalize = enumAttr.create("normalize", "n", 0, 0);
	enumAttr.addField( "off", 0);
	enumAttr.addField( "on", 1);
 	enumAttr.setKeyable(true);

	edgeDirection = enumAttr.create("edgeDirection", "ed", 0, 0);
	enumAttr.addField( "normal", 0);
	enumAttr.addField( "reverse", 1);
 	enumAttr.setKeyable(true);

	outMesh = typedAttr.create("outMesh", "om", MFnMeshData::kMesh);
	typedAttr.setStorable(true);
	typedAttr.setWritable(false);

	uvMethod = enumAttr.create("uvMethod", "uvm", 0, 0);
	enumAttr.addField( "factor", 0);
	enumAttr.addField( "concrete", 1);
 	enumAttr.setKeyable(true);

	offsetF = nAttr.create( "offsetF", "of", MFnNumericData::kDouble, 0.0 );
 	nAttr.setKeyable(true);
	offsetU = nAttr.create( "offsetU", "ou", MFnNumericData::kDouble, 0.0 );
 	nAttr.setKeyable(true);
	offsetV = nAttr.create( "offsetV", "ov", MFnNumericData::kDouble, 0.0 );
 	nAttr.setKeyable(true);

	edgePos = nAttr.create( "edgePos", "ep", MFnNumericData::k3Double);
 	nAttr.setHidden(true);
	nAttr.setStorable(true);

	splitAlgorithm = enumAttr.create( "splitAlgorithm", "alg", 0);
	enumAttr.addField( "loop", 0);
	enumAttr.addField( "sel", 1);
 	enumAttr.setKeyable(true);
	enumAttr.setStorable(true);
	splitLoopMode = enumAttr.create( "loopMode", "lm", 3);
	enumAttr.addField( "topology", 1);
	enumAttr.addField( "angle", 2);
	enumAttr.addField( "top_angle", 3);
 	enumAttr.setKeyable(true);
	enumAttr.setStorable(true);
	splitLoopAngle = nAttr.create( "loopAngle", "la", MFnNumericData::kDouble, 45.0);
	nAttr.setMin( 0.0);
	nAttr.setMax( 90.0);
 	nAttr.setKeyable(true);
	nAttr.setStorable(true);
	splitLoopMaxCount = nAttr.create( "loopMaxCount", "mc", MFnNumericData::kLong, 25000);
	nAttr.setMin( 10);
	nAttr.setMax( 100000);
 	nAttr.setHidden(true);
	nAttr.setStorable(true);


// Add the attributes we have created to the node
	stat = addAttribute( factor );
		if (!stat) { stat.perror("addAttribute factor"); return stat;}
	stat = addAttribute( edgeDirection);
		if (!stat) { stat.perror("addAttribute edgeDirection"); return stat;}
	stat = addAttribute( normalize);
		if (!stat) { stat.perror("addAttribute normalize"); return stat;}
	stat = addAttribute( offsetX);
		if (!stat) { stat.perror("addAttribute offsetX"); return stat;}
	stat = addAttribute( offsetY);
		if (!stat) { stat.perror("addAttribute offsetY"); return stat;}
	stat = addAttribute( offsetZ);
		if (!stat) { stat.perror("addAttribute offsetZ"); return stat;}

	stat = addAttribute( uvMethod);
		if (!stat) { stat.perror("addAttribute uvMethod"); return stat;}
	stat = addAttribute( offsetF);
		if (!stat) { stat.perror("addAttribute offsetF"); return stat;}
	stat = addAttribute( offsetU);
		if (!stat) { stat.perror("addAttribute offsetU"); return stat;}
	stat = addAttribute( offsetV);
		if (!stat) { stat.perror("addAttribute offsetV"); return stat;}

	stat = addAttribute( edgesList );
		if (!stat) { stat.perror("addAttribute edgesList"); return stat;}
	stat = addAttribute( useMesh);
		if (!stat) { stat.perror("addAttribute useMesh"); return stat;}
	stat = addAttribute( inMesh );
		if (!stat) { stat.perror("addAttribute inMesh"); return stat;}
	stat = addAttribute( outMesh);
		if (!stat) { stat.perror("addAttribute outMesh"); return stat;}

	stat = addAttribute( edgePos);
		if (!stat) { stat.perror("addAttribute edgePos"); return stat;}

	stat = addAttribute( splitAlgorithm );
		if (!stat) { stat.perror("addAttribute splitAlgorithm"); return stat;}
	stat = addAttribute( splitLoopMode);
		if (!stat) { stat.perror("addAttribute splitLoopMode"); return stat;}
	stat = addAttribute( splitLoopAngle);
		if (!stat) { stat.perror("addAttribute splitLoopAngle"); return stat;}
	stat = addAttribute( splitLoopMaxCount);
		if (!stat) { stat.perror("addAttribute splitLoopMaxCount"); return stat;}

// Set up a dependency between the input and the output.
	stat = attributeAffects( uvMethod, outMesh );
		if (!stat) { stat.perror("attributeAffects uvMethod"); return stat;}
	stat = attributeAffects( offsetF, outMesh );
		if (!stat) { stat.perror("attributeAffects offsetF"); return stat;}
	stat = attributeAffects( offsetU, outMesh );
		if (!stat) { stat.perror("attributeAffects offsetU"); return stat;}
	stat = attributeAffects( offsetV, outMesh );
		if (!stat) { stat.perror("attributeAffects offsetV"); return stat;}
	stat = attributeAffects( factor, outMesh );
		if (!stat) { stat.perror("attributeAffects factor"); return stat;}
	stat = attributeAffects( inMesh, outMesh );
		if (!stat) { stat.perror("attributeAffects inMesh"); return stat;}
	stat = attributeAffects( useMesh, outMesh );
		if (!stat) { stat.perror("attributeAffects useMesh"); return stat;}
	stat = attributeAffects( edgesList, outMesh );
		if (!stat) { stat.perror("attributeAffects edgesList"); return stat;}
	stat = attributeAffects( offsetX, outMesh );
		if (!stat) { stat.perror("attributeAffects offsetX"); return stat;}
	stat = attributeAffects( offsetY, outMesh );
		if (!stat) { stat.perror("attributeAffects offsetY"); return stat;}
	stat = attributeAffects( offsetZ, outMesh );
		if (!stat) { stat.perror("attributeAffects offsetZ"); return stat;}
	stat = attributeAffects( normalize, outMesh );
		if (!stat) { stat.perror("attributeAffects normalize"); return stat;}
	stat = attributeAffects( edgeDirection, outMesh );
		if (!stat) { stat.perror("attributeAffects edgeDirection"); return stat;}

	stat = attributeAffects( splitAlgorithm, outMesh );
		if (!stat) { stat.perror("attributeAffects splitAlgorithm"); return stat;}
	stat = attributeAffects( splitLoopMode, outMesh );
		if (!stat) { stat.perror("attributeAffects splitLoopMode"); return stat;}
	stat = attributeAffects( splitLoopAngle, outMesh );
		if (!stat) { stat.perror("attributeAffects splitLoopAngle"); return stat;}
	stat = attributeAffects( splitLoopMaxCount, outMesh );
		if (!stat) { stat.perror("attributeAffects splitLoopMaxCount"); return stat;}


	MPxManipContainer::addToManipConnectTable(id);

	return MS::kSuccess;

}
