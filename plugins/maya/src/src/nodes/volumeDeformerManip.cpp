#include "volumeDeformerManip.h"
#include "../definitions.h"

#include <maya/MObject.h>
#include <maya/MPlugArray.h>

#include <maya/MFnDagNode.h>
#include <maya/MFnTransform.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericData.h>

#include <maya/MFnDirectionManip.h>
#include <maya/MFnDistanceManip.h>
#include <maya/MFnFreePointTriadManip.h>
#include <maya/MFnRotateManip.h>
#include <maya/MFnScaleManip.h>

MTypeId tm_volumeDeformerManip::id( tm_volumeDeformer_Manip__id );

tm_volumeDeformerManip::tm_volumeDeformerManip(){}
tm_volumeDeformerManip::~tm_volumeDeformerManip(){}
void* tm_volumeDeformerManip::creator() { return new tm_volumeDeformerManip();}
MStatus tm_volumeDeformerManip::initialize()
{
   MStatus stat;
   stat = MPxManipContainer::initialize();
   return stat;
}

MStatus tm_volumeDeformerManip::createChildren()
{
   MStatus stat = MStatus::kSuccess;
   fDistanceManip = addDistanceManip( "raduisManip", "raduis");
   return stat;
}

MStatus tm_volumeDeformerManip::connectToDependNode(const MObject &node)
{
   MStatus stat;

   MFnDependencyNode nodeFn;
   nodeFn.setObject(node);
   MPlug plug;
   MObject numObject;
   MFnNumericData numDataFn;

   MPoint bboxmin, bboxmax, bboxcenter;
   double radius;
   plug = nodeFn.findPlug( "bboxmin", &stat);
   plug.getValue( numObject);
   numDataFn.setObject( numObject);
   numDataFn.getData( bboxmin[0], bboxmin[1], bboxmin[2]);
   plug = nodeFn.findPlug( "bboxmax", &stat);
   plug.getValue( numObject);
   numDataFn.setObject( numObject);
   numDataFn.getData( bboxmax[0], bboxmax[1], bboxmax[2]);
   bboxcenter = bboxmax + bboxmin;
   bboxcenter.x *= 0.5;
   bboxcenter.y *= 0.5;
   bboxcenter.z *= 0.5;

   plug = nodeFn.findPlug( "radius", &stat);
   plug.getValue( radius);
   radiusOffset.x = bboxcenter.x;
   radiusOffset.y = bboxcenter.y + radius;
   radiusOffset.z = bboxcenter.z;

   MFnDistanceManip distanceManipFn;
   distanceManipFn.setObject( fDistanceManip);
   MPlug radiusPlug = nodeFn.findPlug("radius", &stat);
   if( MStatus::kFailure != stat)
   {
      stat = distanceManipFn.connectToDistancePlug( radiusPlug);
   }
   else stat.perror("can't connectToDependNode DistanceManip");
   distanceManipFn.setStartPoint( bboxcenter);
   distanceManipFn.setDirection( MVector( 0.0, 0.5, 0.0));

   finishAddingManips();
   MPxManipContainer::connectToDependNode( node);
   return stat;
}

void tm_volumeDeformerManip::draw(M3dView & view,
                                    const MDagPath &path,
                                    M3dView::DisplayStyle style,
                                    M3dView::DisplayStatus status)
{
   MPxManipContainer::draw( view, path, style, status);
   MString distanceText( "raduis");

   view.beginGL();
      view.drawText( distanceText, radiusOffset, M3dView::kLeft);
   view.endGL();
}
