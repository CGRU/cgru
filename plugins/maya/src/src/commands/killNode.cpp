#include "killNode.h"

tm_killNode::~tm_killNode(){}

void* tm_killNode::creator()
{
	return new tm_killNode;
}

MSyntax tm_killNode::newSyntax()
{
	MSyntax syntax;
	syntax.addArg( MSyntax::kSelectionItem);
	return syntax;
}

MStatus tm_killNode::doIt( const MArgList& args )
{
	MStatus status = MS::kSuccess;

	MSelectionList selectionList;
	MObject object;
	MArgDatabase argData( syntax(), args);
	status = argData.getCommandArgument( 0, selectionList);
	if(!status){MGlobal::displayError("***###   Invalid argument.");return status;}
	status = selectionList.getDependNode( 0, object);
	if(!status){MGlobal::displayError("***###   Invalid maya object.");return status;}
	MFnDependencyNode depNode( object, &status);
	if(status)
	{
		if(depNode.isFromReferencedFile())
		{
			MGlobal::displayError( "***###   This node is from referenced file!");
			return MStatus::kFailure;
		}
		if(depNode.isDefaultNode()) MGlobal::displayWarning( "***###   This node was default!");
		if(depNode.isShared()) MGlobal::displayWarning( "***###   This node was shared!");
		if(depNode.isLocked())
		{
			MGlobal::displayWarning( "***###   This node was locked!");
			depNode.setLocked( false);
		}
		MTypeId nodeId = depNode.typeId();
		MString info = "***###   ";
		info = info + depNode.typeName();
		info = info + "::id = ";
		info = info + nodeId.id();
		info = info + "   -  bye-bye !";
		MGlobal::displayInfo(info);
	}
	else
		MGlobal::displayWarning( "***###   It is not a dependency node!");

	dagModifier.deleteNode( object);

	return redoIt();
}

MStatus tm_killNode::redoIt()
{
	return dagModifier.doIt();
}

MStatus tm_killNode::undoIt()
{
	return dagModifier.undoIt();
}

bool tm_killNode::isUndoable() const
{
	return true;
}
