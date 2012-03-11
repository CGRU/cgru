
#include "polySlotNode.h"
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
MTypeId     tm_polySlotNode::id( tm_polySlotNode__id );

// Node attributes (in addition to inMesh and outMesh defined by polyModifierNode)
//
MObject tm_polySlotNode::edgesList;

MObject tm_polySlotNode::offset;
MObject tm_polySlotNode::offsetDir;
MObject tm_polySlotNode::offsetDirX;
MObject tm_polySlotNode::offsetDirY;
MObject tm_polySlotNode::offsetDirZ;

MObject tm_polySlotNode::edgesPos;
MObject tm_polySlotNode::edgesDir;

MObject tm_polySlotNode::useMesh;

tm_polySlotNode::tm_polySlotNode():
					recompute(true),
					firstTime(true)
{}

tm_polySlotNode::~tm_polySlotNode()
{}

MStatus tm_polySlotNode::compute( const MPlug& plug, MDataBlock& data )
//	Arguments:
//		plug - the plug to compute
//		data - object that provides access to the attributes for this node
{
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
			MDataHandle offsetData = data.inputValue( offset, &status );
			if( !status) {status.perror("ERROR offset data"); return status;}
			double offsetAmp = offsetData.asDouble();
			
			MDataHandle offsetDirData = data.inputValue( offsetDir, &status );
			if( !status) {status.perror("ERROR offsetDir data"); return status;}
			MVector offsetDirVector = offsetDirData.asVector();

			MDataHandle outputMeshData = data.outputValue( outMesh, &status );
			MCheckStatus(status,"ERROR getting outMesh"); 

			MDataHandle inputEdges = data.inputValue( edgesList, &status);
			MCheckStatus(status,"ERROR getting edgesList"); 
			
			MDataHandle useMeshData = data.inputValue( useMesh, &status );
			if( !status) {status.perror("ERROR useMesh data"); return status;}
			bool useInternalMesh = true;
			if(useMeshData.asShort() == 1) useInternalMesh = false;

			MObject meshObj;
			MFnMesh meshFn;

			if( recompute  || (useInternalMesh == false))
			{
				MDataHandle inputMeshData = data.inputValue( inMesh, &status );
				MCheckStatus(status,"ERROR getting inMesh");

				outputMeshData.set(inputMeshData.asMesh());
				meshObj = outputMeshData.asMesh();
				fSlotFactory.setMesh( meshObj );

				if(firstTime)
				{
					MObject compList = inputEdges.data();
					MFnComponentListData compListFn( compList );
					unsigned i;
					int j;
					MIntArray edgesIds;

					for( i = 0; i < compListFn.length(); i++ )
					{
						MObject comp = compListFn[i];
						if( comp.apiType() == MFn::kMeshEdgeComponent )
						{
							MFnSingleIndexedComponent edgeComp( comp );
							for( j = 0; j < edgeComp.elementCount(); j++ )
							{
								int edgeId = edgeComp.element(j);
								edgesIds.append( edgeId );
							}
						}
					}
#ifdef _DEBUG
cout << endl << "##########################tm_polySlotNode::compute" << endl;
cout << endl << "edgesIds = ";for(i=0;i<edgesIds.length();i++) cout << edgesIds[i] << " ";cout << endl;
#endif
					fSlotFactory.setEdgesIds( edgesIds );
				}
				status = fSlotFactory.doIt();
				if(!status)
				{
					MGlobal::displayError( "tm_polySlotNode::compute - slot action failed." );
					return MStatus::kFailure;
				}
				MDataHandle edgesPosData = data.inputValue( edgesPos, &status );
				edgesPosData.set( fSlotFactory.averagePos[0], fSlotFactory.averagePos[1], fSlotFactory.averagePos[2]);
				MDataHandle edgesDirData = data.inputValue( edgesDir, &status );
				edgesDirData.set( fSlotFactory.averageDir[0], fSlotFactory.averageDir[1], fSlotFactory.averageDir[2]);

				recompute = false;
				firstTime = false;
				if( firstTime) fSlotFactory.firstTime = false;
			}
//######################################################## move concrete vertices:
			meshObj = outputMeshData.asMesh();
			meshFn.setObject( meshObj);
#ifdef _DEBUG
cout << endl << "#tm_polySlotNode - start to offset vertices..." << endl;
cout << "fSlotFactory.offsetVerticesCont_a = "<<fSlotFactory.offsetVerticesCont_a;
#endif
			MPoint point;
			MVector vector( 0.0, 1.0, 0.0);
			MQuaternion quaternion = vector.rotateTo( offsetDirVector);
			for( unsigned sva = 0; sva < fSlotFactory.offsetVerticesCont_a; sva++)
			{
				vector = fSlotFactory.offsetVerticesDir_a[sva].rotateBy( quaternion) * offsetAmp;
				point = fSlotFactory.offsetVerticesStart_a[sva] + vector;
				meshFn.setPoint( fSlotFactory.offsetVerticesIds_a[sva], point);
			}
			for( unsigned svb = 0; svb < fSlotFactory.offsetVerticesCont_b; svb++)
			{
				vector = fSlotFactory.offsetVerticesDir_b[svb].rotateBy( quaternion) * offsetAmp;
				point = fSlotFactory.offsetVerticesStart_b[svb] + vector;
				meshFn.setPoint( fSlotFactory.offsetVerticesIds_b[svb], point);
			}
#ifdef _DEBUG
cout << endl << "#tm_polySlotNode - end to offset vertices..." << endl;
#endif
			// Mark the output mesh as clean
			//
			outputMeshData.setClean();
		}
		else
		{
			status = MS::kUnknownParameter;
		}
	}

	return status;
}

void* tm_polySlotNode::creator()
{
	return new tm_polySlotNode();
}

MStatus tm_polySlotNode::initialize()
{
	MStatus				status;

	MFnTypedAttribute attrFn;
	MFnNumericAttribute nAttr;
	MFnEnumAttribute enumAttr;

	useMesh = enumAttr.create("useMesh", "um", 0, 0);
	enumAttr.addField( "cashed", 0);
	enumAttr.addField( "recompute", 1);
 	enumAttr.setKeyable(true);

	edgesList = attrFn.create("inputComponents", "ics", MFnComponentListData::kComponentList);
	attrFn.setStorable(true);

	inMesh = attrFn.create("inMesh", "im", MFnMeshData::kMesh);
	attrFn.setStorable(true);

	offset = nAttr.create( "offset", "o", MFnNumericData::kDouble, 0.001 );
 	nAttr.setKeyable(true);
	attrFn.setStorable(true);
	offsetDirX = nAttr.create( "offsetDirX", "odx", MFnNumericData::kDouble);
	offsetDirY = nAttr.create( "offsetDirY", "ody", MFnNumericData::kDouble);
	offsetDirZ = nAttr.create( "offsetDirZ", "odz", MFnNumericData::kDouble);
	offsetDir = nAttr.create( "offsetDir", "od", offsetDirX, offsetDirY, offsetDirZ, &status);
	nAttr.setKeyable(true);
	attrFn.setStorable(true);
	nAttr.setDefault( 0.0, 1.0, 0.0);

	outMesh = attrFn.create("outMesh", "om", MFnMeshData::kMesh);
	attrFn.setStorable(true);
	attrFn.setWritable(false);

	edgesPos = nAttr.create( "edgesPos", "ep", MFnNumericData::k3Double);
 	nAttr.setHidden(true);
	nAttr.setStorable(true);
	edgesDir = nAttr.create( "edgesDir", "ed", MFnNumericData::k3Double);
 	nAttr.setHidden(true);
	nAttr.setStorable(true);

	// Add the attributes we have created to the node
	//
	status = addAttribute( offset );
		if (!status) { status.perror("addAttribute offset"); return status;}
	status = addAttribute( offsetDir);
		if (!status) { status.perror("addAttribute offsetDir"); return status;}
	status = addAttribute( edgesList );
		if (!status){ status.perror("addAttribute"); return status;}
	status = addAttribute( inMesh );
		if (!status){ status.perror("addAttribute"); return status;}
	status = addAttribute( outMesh);
		if (!status){ status.perror("addAttribute"); return status;}
	status = addAttribute( useMesh);
		if (!status) { status.perror("addAttribute useMesh"); return status;}
	status = addAttribute( edgesPos);
		if (!status) { status.perror("addAttribute edgesPos"); return status;}
	status = addAttribute( edgesDir);
		if (!status) { status.perror("addAttribute edgesDir"); return status;}

	// Set up a dependency between the input and the output.  This will cause
	// the output to be marked dirty when the input changes.  The output will
	// then be recomputed the next time the value of the output is requested.
	//
	status = attributeAffects( offset, outMesh );
		if (!status) { status.perror("attributeAffects offset"); return status;}
	status = attributeAffects( offsetDir, outMesh );
		if (!status) { status.perror("attributeAffects offsetDir"); return status;}
	status = attributeAffects( inMesh, outMesh );
		if (!status){ status.perror("attributeAffects"); return status;}
	status = attributeAffects( edgesList, outMesh );
		if (!status){ status.perror("attributeAffects"); return status;}
	status = attributeAffects( useMesh, outMesh );
		if (!status) { status.perror("addAttribute attributeAffects"); return status;}

	MPxManipContainer::addToManipConnectTable(id);

	return MS::kSuccess;

}
