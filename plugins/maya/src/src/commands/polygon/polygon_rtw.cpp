#include "polygon.h"

int tm_polygon::removeTweaks_Func( MSelectionList &selectionList)
{
   MStatus status;
   MObject object;
   status = selectionList.getDependNode( 0, object);
   if(!status)
   {
      MGlobal::displayError("***### tm_polygon: Can't find object.");
      return 0;
   }
   MFnMesh mesh( object, &status);
   if(!status)
   {
      MGlobal::displayError("***### tm_polygon: Can't find mesh.");
      return 0;
   }

   int pntsCount = 0;
   MPlug pntsPlug = mesh.findPlug( "pnts" );
   if( !pntsPlug.isNull() )
   {
      MPlug tweakPlug;
      MObject nullVector_object;
      MFnNumericData numDataFn( nullVector_object );
//      numDataFn.setData3Double( 0.0, 0.0, 0.0 );
      numDataFn.setData( 0.0, 0.0, 0.0 );
      pntsCount = pntsPlug.numElements();
      for( int i = 0; i < pntsCount; i++ )
      {
         tweakPlug = pntsPlug.elementByPhysicalIndex( (unsigned int)i, &status );
         if( status == MS::kSuccess && !tweakPlug.isNull() )
            tweakPlug.setValue( nullVector_object );
      }
   }
   return pntsCount;
}
