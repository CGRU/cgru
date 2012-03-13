
#include "polySplitCmd.h"
#include "polySplitNode.h"

// Function Sets
//
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnComponentListData.h>

// Iterators
//
#include <maya/MItSelectionList.h>
#include <maya/MItMeshPolygon.h>

// General Includes
//
#include <maya/MPlugArray.h>
#include <maya/MArgDatabase.h>
#include <maya/MDagPath.h>

#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

// Status Checking Macro - MCheckStatus (Debugging tool)
//
#define MCheckStatus(status,message)	\
	if( MS::kSuccess != status ) {		\
		status.perror(message);		\
		return status;					\
	}


tm_polySplit::tm_polySplit():
					cmd_flag_loop(true),
					cmd_flag_loop_mode(3),
					cmd_flag_loop_angle(45.0),
					cmd_flag_loop_maxcount(25000),
					cmd_flag_sel(false)
{
}

tm_polySplit::~tm_polySplit()
{
}

void* tm_polySplit::creator()
{
	return new tm_polySplit();
}

const char *tm_polySplit::loop_Flag = "-l";
const char *tm_polySplit::loop_LongFlag = "-loop";
const char *tm_polySplit::sel_Flag = "-s";
const char *tm_polySplit::sel_LongFlag = "-selected";

MSyntax tm_polySplit::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( loop_Flag, loop_LongFlag, MSyntax::kLong, MSyntax::kDouble);//, MSyntax::kLong); // - max count;
	syntax.addFlag( sel_Flag, sel_LongFlag);
	syntax.setObjectType( MSyntax::kSelectionList);
	syntax.useSelectionAsDefault( true);
	return syntax;
}

MStatus tm_polySplit::doIt( const MArgList& args)
{
#ifdef _DEBUG
cout << endl << "####################################################" << endl;
cout << "tm_polySplit::doIt - DEBUG version info:" << endl;
#endif
	MStatus status;

	MGlobal::getActiveSelectionList( oldSelList );

	MArgDatabase argData( syntax(), args);

	//parse flags
	//
	if(argData.isFlagSet( loop_Flag))
	{
		cmd_flag_loop = true;
		argData.getFlagArgument( loop_Flag, 0, cmd_flag_loop_mode);
		argData.getFlagArgument( loop_Flag, 1, cmd_flag_loop_angle);
//		argData.getFlagArgument( loop_Flag, 2, cmd_flag_loop_maxcount); // - max count;
	}
	else if(argData.isFlagSet( sel_Flag))
	{
		cmd_flag_loop = false;
		cmd_flag_sel = true;
	}


	MSelectionList selectionList;
	argData.getObjects( selectionList);
	MItSelectionList selListIter( selectionList );
	selListIter.setFilter( MFn::kMesh );

	// The tm_polySplit node only accepts a component list input, so we build
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
	MObject meshObj;

	for( ; !selListIter.isDone(); selListIter.next() )
	{
		MDagPath dagPath;
		MObject component;
		selListIter.getDagPath( dagPath, component );
		meshObj = dagPath.node();

		// Check for selected Edges components
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
#ifdef _DEBUG
cout << endl << "########################## checking arguments:" << endl;
cout << endl << "fSelEdges = ";for(unsigned i=0;i<fSelEdges.length();i++) cout << fSelEdges[i] << " ";cout << endl;
#endif
	if( foundMultiple )
	{
		displayWarning("Found more than one object with selected Edges - Only operating on first found object.");
	}
	if( !found )
	{
		displayError( "tm_polySplit command failed: Unable to find selected edges" );
		return MStatus::kFailure;
	}
	// Initialize the polyModifierCmd node type - mesh node already set
	//
	setModifierNodeType( tm_polySplitNode::id );
//##################################################################
	alwaysWithConstructionHistory = true;
	status = doModifyPoly();
	if( !status){displayError( "tm_polySplit command failed!" );return status;}
//################################################################## get polymodifier node name and select it
	getModifierNodeName();
	newSelList.add( modifierNodeName);
	MGlobal::setActiveSelectionList( newSelList);//, MGlobal::kAddToList);
//##################################################################
	setResult( modifierNodeName);

	return MStatus::kSuccess;
/*
cout<<endl<<"**********"<<endl;return MStatus::kSuccess;
*/
}


bool tm_polySplit::isUndoable() const
{
	return true;
}

MStatus tm_polySplit::redoIt()
//
//	Description:
//		Implements redo for the MEL tm_polySplit command. 
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
	if( !status) displayError( "tm_polySplit command redo failed!" );
	MGlobal::setActiveSelectionList( newSelList);
	return status;
}

MStatus tm_polySplit::undoIt()
//
//	Description:
//		implements undo for the MEL tm_polySplit command.  
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
	if( !status) setResult( "tm_polySplit command undo failed!" );
	MGlobal::setActiveSelectionList( oldSelList);
	return status;
}


MStatus tm_polySplit::initModifierNode( MObject modifierNode )
{
	// We need to tell the tm_polySplit node which UVs to operate on. By overriding
	// the polyModifierCmd::initModifierNode() method, we can insert our own
	// modifierNode initialization code.
	//
	MFnDependencyNode depNodeFn( modifierNode );
	MObject attrObj;
	attrObj = depNodeFn.attribute( "inputComponents" );
	MPlug plug( modifierNode, attrObj );
	plug.setValue( fComponentList );
#ifdef _DEBUG
cout << endl << "tm_polySplit::initModifierNode : splitAlgorithm=";
cout<<cmd_flag_sel<<" loopMode="<<cmd_flag_loop_mode<<" loop_angle="<<cmd_flag_loop_angle<<" loop_maxcount="<<cmd_flag_loop_maxcount<<endl;
#endif
	if( cmd_flag_loop)
	{
		attrObj = depNodeFn.attribute( "splitAlgorithm");
		plug.setAttribute( attrObj);
		plug.setValue( 0);

		attrObj = depNodeFn.attribute( "loopMode");
		plug.setAttribute( attrObj);
		plug.setValue( cmd_flag_loop_mode);

		attrObj = depNodeFn.attribute( "loopAngle");
		plug.setAttribute( attrObj);
		plug.setValue( cmd_flag_loop_angle);

		attrObj = depNodeFn.attribute( "loopMaxCount");
		plug.setAttribute( attrObj);
		plug.setValue( cmd_flag_loop_maxcount);
	}
	else
	{
		attrObj = depNodeFn.attribute( "splitAlgorithm");
		plug.setAttribute( attrObj);
		plug.setValue( 1);
	}

	return MStatus::kSuccess;
}

MStatus tm_polySplit::directModifier( MObject mesh )
{
	MStatus status;

	fSplitFactory.setMesh( mesh );
	fSplitFactory.setEdgesIds( fSelEdges );

	status = fSplitFactory.doIt();

	return status;
}
/**/
// Private Methods
//
/**/


void tm_polySplit::getModifierNodeName()
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
