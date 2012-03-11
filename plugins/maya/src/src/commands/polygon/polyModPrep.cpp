#include "polyModPrep.h"

tm_polyModPrep::tm_polyModPrep():
	hasTweaks(false),
	hasHistory(false)
{}

tm_polyModPrep::~tm_polyModPrep(){}

void* tm_polyModPrep::creator()
{
	return new tm_polyModPrep;
}

MSyntax tm_polyModPrep::newSyntax()
{
	MSyntax syntax;
	syntax.addArg( MSyntax::kSelectionItem);
	return syntax;
}

MStatus tm_polyModPrep::doIt( const MArgList& args )
{
	MStatus status = MS::kSuccess;
	MArgDatabase argData( syntax(), args);
	MSelectionList selectionList;
	status = argData.getCommandArgument( 0, selectionList);
	if(!status){MGlobal::displayError("***### tm_polyModPrep: Invalid command argument.");return status;}
	status = selectionList.getDependNode( 0, meshObj);
	if(!status){MGlobal::displayError("***### tm_polyModPrep: Can't find object.");return status;}
	status = meshFn.setObject( meshObj);
	if(!status){MGlobal::displayError("***### tm_polyModPrep: Can't find mesh.");return status;}
	MPlug mesh_inMeshPlug = meshFn.findPlug( "inMesh");
	if( mesh_inMeshPlug.isConnected()) hasHistory = true;
	meshName = meshFn.name();

//####################################################### reading shape tweaks data
	mesh_pntsPlug = meshFn.findPlug( "pnts" );
	MObjectArray tweakDataObjArray;
	if( !mesh_pntsPlug.isNull() )
	{
		MPlug tweakPlug;
		MObject tweakDataObj;
		MFnNumericData numericDataFn;
		MFloatVector tweakVector;
		pntsCount = mesh_pntsPlug.numElements();
		pntsIndexesArray.setLength( pntsCount);
		tweakDataObjArray.setLength( pntsCount);
		tweakVectorsArray.setLength( pntsCount);
		for( unsigned i = 0; i < pntsCount; i++ )
		{
			tweakPlug = mesh_pntsPlug.elementByPhysicalIndex( i, &status );
			if( status == MS::kSuccess && !tweakPlug.isNull() )
			{
				tweakPlug.getValue( tweakDataObj);
				numericDataFn.setObject( tweakDataObj);
				numericDataFn.getData( tweakVector[0], tweakVector[1], tweakVector[2]);
				if( 0 != tweakVector.x ||
					0 != tweakVector.y ||
					0 != tweakVector.z )
				{
					hasTweaks = true;
					tweakVectorsArray[i] = tweakVector;
					unsigned logicalIndex = tweakPlug.logicalIndex();
					pntsIndexesArray[i] = logicalIndex;
					tweakDataObjArray[i] = tweakDataObj;
				}
			}
		}
	}
	if((!hasTweaks) && (hasHistory)) return status;
//################################# creating polyTweak node and set it's data
	MFnDependencyNode depNodeFn;
	MObject upstreamNodeTransform;
	MPlug upstreamNodeSrcPlug;
	MPlug tweakNode_inputPolymeshPlug;
	MPlug tweakNode_outputPlug;
	if( hasTweaks)
	{
		polyTweakObject = dgModifier.createNode("polyTweak");
		depNodeFn.setObject( polyTweakObject);
		tweakNode_inputPolymeshPlug = depNodeFn.findPlug( "inputPolymesh"); 
		tweakNode_outputPlug = depNodeFn.findPlug( "output"); 
		if( hasHistory)
		{
			MPlugArray tempPlugArray;
			mesh_inMeshPlug.connectedTo( tempPlugArray, true, false);
			upstreamNodeSrcPlug = tempPlugArray[0];
			dgModifier.disconnect( upstreamNodeSrcPlug, mesh_inMeshPlug);
			dgModifier.connect( upstreamNodeSrcPlug, tweakNode_inputPolymeshPlug);
			dgModifier.connect( tweakNode_outputPlug, mesh_inMeshPlug);
		}
	}
	if( !hasHistory)
	{
		MFnDagNode dagNodeFn;
		dagNodeFn.setObject( meshObj);
		MString meshName = dagNodeFn.name();
		MObject meshNodeTransform = dagNodeFn.parent(0);
		upstreamNodeTransform = dagNodeFn.duplicate( false, false );
		dagNodeFn.setObject( upstreamNodeTransform );
		MObject upstreamNodeShape = dagNodeFn.child(0);
		status = dagModifier.reparentNode( upstreamNodeShape, meshNodeTransform );
		if(!status){MGlobal::displayError("***### tm_polyModPrep: Can't reparent duplicated shape.");return status;}
		depNodeFn.setObject( upstreamNodeShape);
		depNodeFn.setName( meshName + "_Original");
		upstreamNodeSrcPlug = depNodeFn.findPlug( "outMesh");
		dagNodeFn.setObject( upstreamNodeShape );
		dagNodeFn.setIntermediateObject( true );
		dgModifier.deleteNode( upstreamNodeTransform);
		tempDagModifier.deleteNode( upstreamNodeTransform);		// for undo duplicate
		if( hasTweaks)
		{
			dgModifier.connect( upstreamNodeSrcPlug, tweakNode_inputPolymeshPlug);
			dgModifier.connect( tweakNode_outputPlug, mesh_inMeshPlug);
		}
		else
			dgModifier.connect( upstreamNodeSrcPlug, mesh_inMeshPlug);
	}
	return redoIt();
}

MStatus tm_polyModPrep::redoIt()
{
	MStatus status = MS::kSuccess;
	if((!hasTweaks) && (hasHistory)) return status;
	if(!hasHistory) tempDagModifier.undoIt();		// for undo duplicate
	dagModifier.doIt();
	dgModifier.doIt();
	if( hasTweaks)
	{
		MFnDependencyNode depNodeFn;
		MFnNumericData numericDataFn;
		MObject tweakVectorObj;
//############################################# set the polyTweak node data:
		depNodeFn.setObject( polyTweakObject);
		tweakVectorObj = numericDataFn.create( MFnNumericData::k3Float);;
		MObject tweakNodeTweakAttr = depNodeFn.attribute("tweak");
		MPlug polyTweakPlug( polyTweakObject, tweakNodeTweakAttr );
		MPlug tweak;
		for( unsigned i = 0; i < pntsCount; i++)
		{
			numericDataFn.setData( tweakVectorsArray[i][0], tweakVectorsArray[i][1], tweakVectorsArray[i][2]);
			tweak = polyTweakPlug.elementByLogicalIndex( pntsIndexesArray[i] );
			tweak.setValue( tweakVectorObj);
		}

//############################################## reset shape tweaks data to zero:
		tweakVectorObj = numericDataFn.create( MFnNumericData::k3Float);
		numericDataFn.setData( 0.0f, 0.0f, 0.0f);
		MPlug tweakPlug;
		for( unsigned i = 0; i < pntsCount; i++)
		{
			tweakPlug = mesh_pntsPlug.elementByPhysicalIndex( i, &status );
			tweakPlug.setValue( tweakVectorObj);
		}
	}

	return status;
}

MStatus tm_polyModPrep::undoIt()
{
	MStatus status = MS::kSuccess;
	if((!hasTweaks) && (hasHistory)) return status;
	if( hasTweaks)
	{
		MFnDependencyNode depNodeFn;
		depNodeFn.setObject( polyTweakObject);
		MObject tweakNodeTweakAttr = depNodeFn.findPlug( "tweak");
		MPlug polyTweakPlug( polyTweakObject, tweakNodeTweakAttr );
		MPlug tweak;
		MFnNumericData numericDataFn;
		MObject nullVectorObj = numericDataFn.create( MFnNumericData::k3Float);
		numericDataFn.setData( 0.0f, 0.0f, 0.0f);
		for( unsigned i = 0; i < pntsCount; i++)
		{
			tweak = polyTweakPlug.elementByLogicalIndex( pntsIndexesArray[i] );
			tweak.setValue( nullVectorObj );
		}
		MString cmd( "dgeval -src " );
		cmd += meshName;
		cmd += ".outMesh";
		status = MGlobal::executeCommand( cmd, false, false );
		if( !status) MGlobal::displayError("Can't eval dependency graph.");
	}
	dgModifier.undoIt();
	dagModifier.undoIt();
//############################################## set tweaks data back to shape
	if( hasTweaks)// && hasHistory)
	{
		MFnNumericData numericDataFn;
		MObject tweakVectorObj = numericDataFn.create( MFnNumericData::k3Float);;
		MPlug tweakPlug;
		for( unsigned i = 0; i < pntsCount; i++)
		{
			numericDataFn.setData( tweakVectorsArray[i][0], tweakVectorsArray[i][1], tweakVectorsArray[i][2]);
			tweakPlug = mesh_pntsPlug.elementByPhysicalIndex( i, &status );
			tweakPlug.setValue( tweakVectorObj);
		}
	}
	if(!hasHistory) tempDagModifier.doIt();		// for undo duplicate

	return status;
}

bool tm_polyModPrep::isUndoable() const
{
	return true;
}
