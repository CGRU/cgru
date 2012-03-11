#include "reparent.h"

tm_reparent::~tm_reparent(){}

void* tm_reparent::creator()
{
   return new tm_reparent;
}

MSyntax tm_reparent::newSyntax()
{
   MSyntax syntax;
   syntax.setObjectType( MSyntax::kSelectionList);
   syntax.useSelectionAsDefault( true);
   return syntax;
}

MStatus tm_reparent::doIt( const MArgList& args )
{
   MStatus status = MS::kSuccess;

   MArgDatabase argData( syntax(), args);
   MSelectionList selectionList;
   argData.getObjects( selectionList);

   MObjectArray nodes;

   MItSelectionList selListIter( selectionList );
   bool found = false;
   bool foundMultiple = false;
   int count = 0;
   for( ; !selListIter.isDone(); selListIter.next() )
   {
      MObject obj;
      selListIter.getDependNode( obj);
      if(!obj.hasFn(MFn::kDagNode))
      {
         MGlobal::displayError("***###   Invalid dagNode - argument #" + count);
         return status;
      }
      nodes.append( obj);
      count++;
      /*
      MFnDependencyNode depNode( obj);
      MGlobal::displayInfo( depNode.name());
      */
   }
   if( count < 2 ){displayWarning("Specify at least two dag objects.");return MStatus::kFailure;}

   count--;
   for( int i = 0; i < count; i++)
   {
      dagModifier.reparentNode( nodes[i], nodes[count]);
   }

   return redoIt();
}

MStatus tm_reparent::redoIt()
{
   return dagModifier.doIt();
}

MStatus tm_reparent::undoIt()
{
   return dagModifier.undoIt();
}

bool tm_reparent::isUndoable() const
{
   return true;
}
