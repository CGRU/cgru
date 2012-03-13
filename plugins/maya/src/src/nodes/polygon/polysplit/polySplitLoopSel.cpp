#include "polySplitLoopSel.h"

#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MStatus.h>
#include <maya/MArgDatabase.h>

#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>
#include <maya/MPlug.h>

tm_polySplitLoopSel::~tm_polySplitLoopSel(){}

void* tm_polySplitLoopSel::creator(){return new tm_polySplitLoopSel;}

bool tm_polySplitLoopSel::isUndoable() const { return true;}

tm_polySplitLoopSel::tm_polySplitLoopSel(){}

MSyntax tm_polySplitLoopSel::newSyntax()
{
	MSyntax syntax;
	syntax.addArg( MSyntax::kLong);
	syntax.addArg( MSyntax::kDouble);
	syntax.setObjectType( MSyntax::kSelectionList);
	syntax.useSelectionAsDefault( true);
	return syntax;
}

MStatus tm_polySplitLoopSel::doIt( const MArgList& args )
{
#ifdef _DEBUG
	cout << endl << "######################################### tm_polySplitLoopSel::doIt "<< endl;
#endif
	MGlobal::getActiveSelectionList( oldSelList);
	MArgDatabase argData( syntax(), args);
	MDagPath dagPath;
	MObject component;
	MIntArray selArray;
	MIntArray selCompsArray; selCompsArray.setSizeIncrement(64);
	MIntArray selCompsCounts; selCompsCounts.setSizeIncrement(64);
	MDagPathArray dagPathArray;
	MSelectionList selectionList;

	argData.getObjects( selectionList);
	MItSelectionList selListIter( selectionList, MFn::kMeshEdgeComponent);
	for( ; !selListIter.isDone(); selListIter.next() )
	{
		selListIter.getDagPath( dagPath, component );
		MFnSingleIndexedComponent compFn( component );
		compFn.getElements( selArray );
		unsigned len = selArray.length();
		selCompsCounts.append( len);
		for( unsigned i = 0; i < len; i++) selCompsArray.append( selArray[i]);

		dagPath.extendToShape();
		dagPathArray.append( dagPath);
	}
	unsigned numComps = selCompsArray.length();
	unsigned numMeshes = dagPathArray.length();

#ifdef _DEBUG
cout<< endl << "numComps = " << numComps << ", numMeshes = " << numMeshes << endl;
#endif

	if( !numComps ){ return MStatus::kSuccess;}

	int mode; double angle;
	MStatus  status;
	status = argData.getCommandArgument( 0, mode);
	if(!status) mode = 3;
	argData.getCommandArgument( 1, angle);
	if(!status) angle = 45.0;

	MString command;
	MString result;
	{
	command = "tm_polySplit -l ";
	command = command + mode + " ";
	command = command + angle + " ";
	int i = 0;
	for( unsigned m = 0; m < numMeshes; m++)
	{
		MFnDependencyNode meshNode(dagPathArray[m].node());
		MString meshName = meshNode.name();
		for( int c = 0; c < selCompsCounts[m]; c++)
		{
			MString cmd = command + " " + meshName + ".e[" + selCompsArray[i] + "];";
			MGlobal::displayInfo( cmd);
			MGlobal::executeCommand( cmd, result, true, true);
			newSelList.add(result);
			if( i > 0)
			{
				MObject nodeObj;
				newSelList.getDependNode( i, nodeObj);
				MFnDependencyNode node( nodeObj);
				MObject attrObj(node.attribute( "useMesh"));
				MPlug plug( nodeObj, attrObj);
				plug.setValue( 1);
			}
			i++;
		}
	}
	}

	return MGlobal::setActiveSelectionList( newSelList);
}

MStatus tm_polySplitLoopSel::redoIt()
{
	return MGlobal::setActiveSelectionList( newSelList);
}

MStatus tm_polySplitLoopSel::undoIt()
{
	return MGlobal::setActiveSelectionList( oldSelList);
}
