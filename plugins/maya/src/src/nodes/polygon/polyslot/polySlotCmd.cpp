
#include "polySlotCmd.h"
#include "polySlotNode.h"

// Function Sets
//
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>

// Iterators
//
#include <maya/MItSelectionList.h>
#include <maya/MItMeshPolygon.h>

// General Includes
//
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

#include <maya/MPlugArray.h>

// Status Checking Macro - MCheckStatus (Debugging tool)
//
#define MCheckStatus(status,message)	\
	if( MS::kSuccess != status ) {		\
		status.perror(message);		\
		return status;					\
	}


tm_polySlot::tm_polySlot():
					p_uArray( NULL),
					p_vArray( NULL),
					p_uvCounts( NULL),
					p_uvIds( NULL)
{}

tm_polySlot::~tm_polySlot()
{
	if(p_uArray) delete [] p_uArray;
	if(p_vArray) delete [] p_vArray;
	if(p_uvCounts) delete [] p_uvCounts;
	if(p_uvIds) delete [] p_uvIds;
}

void* tm_polySlot::creator()
{
	return new tm_polySlot();
}

bool tm_polySlot::isUndoable() const
{
	return true;
}

MStatus tm_polySlot::doIt( const MArgList& )
//
//	Description:
//		implements the MEL tm_polySlot command.
//
//	Arguments:
//		args - the argument list that was passes to the command from MEL
//
//	Return Value:
//		MS::kSuccess - command succeeded
//		MS::kFailure - command failed (returning this value will cause the 
//                     MEL script that is being run to terminate unless the
//                     error is caught using a "catch" statement.
//
{
#ifdef _DEBUG
cout << endl << "####################################################" << endl;
cout << "tm_polySlot::doIt - DEBUG version info:" << endl;
#endif
	MStatus status;

	// Parse the selection list for objects with selected UV components.
	// To simplify things, we only take the first object that we find with
	// selected UVs and operate on that object alone.
	//
	// All other objects are ignored and return warning messages indicating
	// this limitation.
	//
	MGlobal::getActiveSelectionList( oldSelList );
	MItSelectionList selListIter( oldSelList );
	selListIter.setFilter( MFn::kMesh );

	// The tm_polySlot node only accepts a component list input, so we build
	// a component list using MFnComponentListData.
	//
	// MIntArrays could also be passed into the node to represent the edgesIds,
	// but are less storage efficient than component lists, since consecutive 
	// components are bundled into a single entry in component lists.
	//
	MFnComponentListData compListFn;
	compListFn.create();
	bool found = false;
	bool foundMultiple = false;

	for( ; !selListIter.isDone(); selListIter.next() )
	{
		MDagPath dagPath;
		MObject component;
		selListIter.getDagPath( dagPath, component );

		// Check for selected UV components
		//
		if( component.apiType() == MFn::kMeshEdgeComponent )
		{
			if( !found )
			{
				// The variable 'component' holds all selected components on the selected
				// object, thus only a single call to MFnComponentListData::add() is needed
				// to store the selected components for a given object.
				//
				compListFn.add( component );

				// Copy the component list created by MFnComponentListData into our local
				// component list MObject member.
				//
				fComponentList = compListFn.object();

				// Locally store the actual edgesIds of the selected Edges so that this command
				// can directly modify the mesh in the case when there is no history and
				// history is turned off.
				//
				MFnSingleIndexedComponent compFn( component );
				compFn.getElements( fSelEdges );

				// Ensure that this DAG path will point to the shape of our object.
				// Set the DAG path for the polyModifierCmd.
				//
				dagPath.extendToShape();
				setMeshNode( dagPath );
				found = true;
			}
			else
			{
				// Break once we have found a multiple object holding selected UVs, since
				// we are not interested in how many multiple objects there are, only
				// the fact that there are multiple objects.
				//
				foundMultiple = true;
				break;
			}
		}
	}
	if( foundMultiple )
	{
		displayWarning("Found more than one object with selected Edges - Only operating on first found object.");
	}
	if( !found )
	{
		displayError( "tm_polySlot command failed: Unable to find selected edges" );
		return MStatus::kFailure;
	}
	// Initialize the polyModifierCmd node type - mesh node already set
	//
	setModifierNodeType( tm_polySlotNode::id );
//##################################################################
	alwaysWithConstructionHistory = true;
	status = doModifyPoly();
	if( !status){displayError( "tm_polySlot command failed!" );return status;}
//################################################################## get polymodifier node name and select it
	getModifierNodeName();
	newSelList.add( modifierNodeName);
	MGlobal::setActiveSelectionList( newSelList);//, MGlobal::kAddToList);
//##################################################################
	successResult();

	return MStatus::kSuccess;
}

MStatus tm_polySlot::redoIt()
//
//	Description:
//		Implements redo for the MEL tm_polySlot command. 
//
//		This method is called when the user has undone a command of this type
//		and then redoes it.  No arguments are passed in as all of the necessary
//		information is cached by the doIt method.
//
//	Return Value:
//		MS::kSuccess - command succeeded
//		MS::kFailure - redoIt failed.  this is a serious problem that will
//                     likely cause the undo queue to be purged
//
{
	MStatus status;
	status = redoModifyPoly();
	if( status == MS::kSuccess ) successResult();
	else displayError( "tm_polySlot command redo failed!" );
	MGlobal::setActiveSelectionList( newSelList);
	return status;
}

MStatus tm_polySlot::undoIt()
//
//	Description:
//		implements undo for the MEL tm_polySlot command.  
//
//		This method is called to undo a previous command of this type.  The 
//		system should be returned to the exact state that it was it previous 
//		to this command being executed.  That includes the selection state.
//
//	Return Value:
//		MS::kSuccess - command succeeded
//		MS::kFailure - redoIt failed.  this is a serious problem that will
//                     likely cause the undo queue to be purged
//
{
	MStatus status;
	status = undoModifyPoly();
	if( !status) setResult( "tm_polySlot undo failed!" );
	MGlobal::setActiveSelectionList( oldSelList);
 	return status;
}

MStatus tm_polySlot::initModifierNode( MObject modifierNode )
{
	MStatus status;

	// We need to tell the tm_polySlot node which UVs to operate on. By overriding
	// the polyModifierCmd::initModifierNode() method, we can insert our own
	// modifierNode initialization code.
	//
	MFnDependencyNode depNodeFn( modifierNode );
	MObject edgesListAttr;
	edgesListAttr = depNodeFn.attribute( "inputComponents" );

	// Pass the component list down to the tm_polySlot node
	//
	MPlug edgesListPlug( modifierNode, edgesListAttr );
	status = edgesListPlug.setValue( fComponentList );

	return status;
}

MStatus tm_polySlot::directModifier( MObject mesh )
{
	MStatus status;

	fSlotFactory.setMesh( mesh );
	fSlotFactory.setEdgesIds( fSelEdges );

	status = fSlotFactory.doIt();

	return status;
}

// Private Methods
//

void tm_polySlot::successResult()
{
		MString resultStr = "tm_polySlot command succeeded: ";
		resultStr += int(fSelEdges.length());
		resultStr += " edges slotted.";
		resultStr += " edges slotted. #";
		resultStr += modifierNodeName;
		setResult( resultStr);
}
void tm_polySlot::getModifierNodeName()
{
	MDagPath dagPath = getMeshNode();
	MObject meshObject = dagPath.node();
	MFnDependencyNode nodeFn( meshObject);
	MPlug inMeshPlug = nodeFn.findPlug( "inMesh");
	MPlugArray connectedPlugsArray;
	if(	inMeshPlug.connectedTo( connectedPlugsArray, 1, 0))
	{
		MObject modifierNodeObj = connectedPlugsArray[0].node();
		MFnDependencyNode modifierNode( modifierNodeObj);
		modifierNodeName = modifierNode.name();
	}
}

void tm_polySlot::getMeshUVs()
{
	MDagPath dagPath = getMeshNode();
	MObject meshObject = dagPath.node();
	MFnMesh meshFn( meshObject);

	MStringArray uvSetNames;
	MString currentUVSetName;
	MFloatArray *p_uArray = NULL;
	MFloatArray *p_vArray = NULL;
	MIntArray *p_uvCounts = NULL;
	MIntArray *p_uvIds = NULL;
	unsigned uvSetsCount = meshFn.numUVSets();
	if(uvSetsCount > 0)
	{
		meshFn.getUVSetNames( uvSetNames);
		p_uArray = new MFloatArray[uvSetsCount];
		p_vArray = new MFloatArray[uvSetsCount];
		p_uvCounts = new MIntArray[uvSetsCount];
		p_uvIds = new MIntArray[uvSetsCount];
		meshFn.getCurrentUVSetName( currentUVSetName);
		for( unsigned i = 0; i < uvSetsCount; i++)
		{
			meshFn.getUVs( p_uArray[i], p_vArray[i], &uvSetNames[i]);
			meshFn.getAssignedUVs( p_uvCounts[i], p_uvIds[i], &uvSetNames[i]); 
		}
	}
#ifdef _DEBUG
MString msg;
msg = "Old uvSetsCount = " + uvSetsCount;
msg += "UvSetNames = ";
for(unsigned i=0;i<uvSetNames.length();i++) msg += uvSetNames[i] + ", ";
MGlobal::displayInfo( msg);
#endif
}
void tm_polySlot::setMeshUVs()
{
	MStatus status( MStatus::kSuccess);
	MDagPath dagPath = getMeshNode();
	MObject meshObject = dagPath.node();
	MFnMesh meshFn( meshObject);
#ifdef _DEBUG
MString msg;
uvSetsCount = meshFn.numUVSets();
if(uvSetsCount > 0) meshFn.getUVSetNames( uvSetNames);
msg = "Old uvSetsCount = " + uvSetsCount;
msg += "UvSetNames = ";
for(unsigned i=0;i<uvSetNames.length();i++) msg += uvSetNames[i] + ", ";
MGlobal::displayInfo( msg);
#endif
	
/*	for( unsigned i = 0; i < uvSetNames.length(); i++)
	{
		status = meshFn.deleteUVSet( uvSetNames[i], &dgModifier);
		if(!status) MGlobal::displayError("Can't delete uvSet " + uvSetNames[i]);
	}
*/
//	i = 0;
	for( unsigned i = 0; i < uvSetsCount; i++)
	{
//		status = meshFn.setCurrentUVSetName( uvSetNames[i]);
//			if(!status) MGlobal::displayError("Can't set current " + uvSetNames[i] + " uvSet.");
//		status = meshFn.deleteUVSet( uvSetNames[i]);
//		if(!status) MGlobal::displayError("Can't delete uvSet " + uvSetNames[i]);
//		meshFn.clearUVs( &uvSetNames[i]);
//		status = meshFn.createUVSet( uvSetNames[i]);
//		if(!status) MGlobal::displayError("Can't create uvSet " + uvSetNames[i]);
		status = meshFn.setUVs( p_uArray[i], p_vArray[i], &uvSetNames[i]);
		if(!status) MGlobal::displayError("Can't set uvSet " + uvSetNames[i]);
		status = meshFn.assignUVs( p_uvCounts[i], p_uvIds[i], &uvSetNames[i]);
		if(!status) MGlobal::displayError("Can't assign uvSet " + uvSetNames[i]);
	}
}
