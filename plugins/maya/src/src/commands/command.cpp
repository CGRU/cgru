#include "command.h"

#include "../utilities/utilities.h"

tm_command::tm_command(){}

tm_command::~tm_command(){}

void* tm_command::creator(){return new tm_command;}

MSyntax tm_command::newSyntax()
{
	MSyntax syntax;
	syntax.setObjectType( MSyntax::kSelectionList);
	syntax.useSelectionAsDefault( true);
	return syntax;
}

MStatus tm_command::doIt( const MArgList& args )
{
    MGlobal::displayInfo("######################################### command::doIt");
	MStatus status = MS::kSuccess;
	
	MArgDatabase argData( syntax(), args);
	MSelectionList selectionList;
	argData.getObjects( selectionList);

	MDagPath dagPath;
	MIntArray selEdges;
	
	MItSelectionList selListIter( selectionList );
//	selListIter.setFilter( MFn::kMesh );
	bool found = false;
	bool foundMultiple = false;
	for( ; !selListIter.isDone(); selListIter.next() )
	{
		MObject component;
		selListIter.getDagPath( dagPath, component );
//		if( component.apiType() == MFn::kMeshEdgeComponent )
		{
			if( !found )
			{
//				MFnSingleIndexedComponent compFn( component );
//				compFn.getElements( selEdges );
				dagPath.extendToShape();
				found = true;
			}
			else
			{
				foundMultiple = true;
				break;
			}
		}
	}
	if( foundMultiple ){displayWarning("Found more than one object.");}
	if( !found ){displayError( "command failed: Unable to find object" ); return MStatus::kFailure;}

    MGlobal::displayInfo("object = ");

	return status;
}

