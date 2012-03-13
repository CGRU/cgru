#include "polySelect.h"

#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>

#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>

#include <maya/MFnMesh.h>
#include <maya/MArgDatabase.h>

#include <maya/MItSelectionList.h>
#include <maya/MFnSingleIndexedComponent.h>

#include "../../utilities/polygon_selectring.h"
#include "../../utilities/polygon_selectloop.h"

tm_polySelect::tm_polySelect():
				doItSuccess(false)
{
}

tm_polySelect::~tm_polySelect(){}

void* tm_polySelect::creator()
{
	return new tm_polySelect;
}

const char *tm_polySelect::loop_Flag = "-l";
const char *tm_polySelect::loop_LongFlag = "-loop";

const char *tm_polySelect::ring_Flag = "-r";
const char *tm_polySelect::ring_LongFlag = "-ring";

MSyntax tm_polySelect::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( loop_Flag, loop_LongFlag, MSyntax::kLong, MSyntax::kDouble);
	syntax.addFlag( ring_Flag, ring_LongFlag, MSyntax::kLong, MSyntax::kDouble);

	syntax.setObjectType( MSyntax::kSelectionList);
	syntax.useSelectionAsDefault( true);

	return syntax;
}

MStatus tm_polySelect::doIt( const MArgList& args )
{
	MStatus status = MS::kSuccess;
	MArgDatabase argData( syntax(), args);
	MGlobal::getActiveSelectionList( oldSelList);

	clearResult();

	MDagPath dagPath;
	MObject component;
	MIntArray oldArray;
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
		compFn.getElements( oldArray );
		unsigned len = oldArray.length();
		selCompsCounts.append( len);
		for( unsigned i = 0; i < len; i++) selCompsArray.append( oldArray[i]);

		dagPath.extendToShape();
		dagPathArray.append( dagPath);
	}
	unsigned numComps = selCompsArray.length();
	unsigned numMeshes = dagPathArray.length();


	if( !numComps ){ return MStatus::kSuccess;}
	
	int mode = 3;
	double angle = 45;
	long maxCount = 25000;
	MIntArray newArray;
	MIntArray *pNewArrays = new MIntArray[ numComps];
	bool loop = false;
	if(argData.isFlagSet( loop_Flag))
	{
		argData.getFlagArgument( loop_Flag, 0, mode);
		if( mode < 1) mode = 1;
		if( mode > 3) mode = 3;
		argData.getFlagArgument( loop_Flag, 1, angle);
		if( angle < 0) angle = 1;
		if( angle > 90.0) angle = 90.0;
		loop = true;
	} else
	if(argData.isFlagSet( ring_Flag))
	{
		argData.getFlagArgument( ring_Flag, 0, mode);
		if( mode < 1) mode = 1;
		if( mode > 3) mode = 3;
		argData.getFlagArgument( ring_Flag, 1, angle);
		if( angle < 0) angle = 1;
		if( angle > 90.0) angle = 90.0;
	}
	else
	{
		setResult( "\n// Type \"help tm_polySelect\" to query flags.\n");
		doItSuccess = false;
		return MStatus::kFailure;
	}

	int i = 0;
	for( unsigned m = 0; m < numMeshes; m++)
	{
		for( int c = 0; c < selCompsCounts[m]; c++)
		{
			if( loop)
			{
				tm_polygon_selectloop loopTool;
				loopTool.setMesh( dagPathArray[m].node());
				doItSuccess = loopTool.calculate( newArray, selCompsArray[i], mode, angle, maxCount);
				pNewArrays[i] = newArray;
			}
			else
			{
				tm_polygon_selectring ringTool;
				ringTool.setMesh( dagPathArray[m].node());
				doItSuccess = ringTool.calculate( newArray, selCompsArray[i], mode, angle, maxCount);
				pNewArrays[i] = newArray;
			}
			i++;
		}
	}

	if(!doItSuccess)
	{
		MGlobal::displayError("tm_polySelect: can't find edges.");
		return MStatus::kFailure;
	}
	newSelList.clear();
	{
	int i = 0;
	for( unsigned m = 0; m < numMeshes; m++)
	{
		MFnSingleIndexedComponent compFn;
		compFn.create( MFn::kMeshEdgeComponent);
		for( int c = 0; c < selCompsCounts[m]; c++)
		{
			compFn.addElements( pNewArrays[i++]);
		}
		newSelList.add( dagPathArray[m], compFn.object());
	}
	}
	if(pNewArrays) delete [] pNewArrays;

	return redoIt();
}

MStatus tm_polySelect::redoIt()
{
	MStatus status;
	if(!doItSuccess) return status;
	status = MGlobal::setActiveSelectionList( newSelList);
	return status;
}

MStatus tm_polySelect::undoIt()
{
	MStatus status;
	if(!doItSuccess) return status;
	status = MGlobal::setActiveSelectionList( oldSelList);
	return status;
}

bool tm_polySelect::isUndoable() const
{
	return true;
}
