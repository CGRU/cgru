#include "volumeDeformer.h"

#include <iostream>

#include <maya/MItGeometry.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <math.h>

#include <maya/MPxManipContainer.h>

MTypeId tm_volumeDeformer::id( tm_volumeDeformer__id );

MObject tm_volumeDeformer::multiplier_attrMObj;

MObject tm_volumeDeformer::bboxXmin_attrMObj;
MObject tm_volumeDeformer::bboxXmax_attrMObj;
MObject tm_volumeDeformer::bboxYmin_attrMObj;
MObject tm_volumeDeformer::bboxYmax_attrMObj;
MObject tm_volumeDeformer::bboxZmin_attrMObj;
MObject tm_volumeDeformer::bboxZmax_attrMObj;
MObject tm_volumeDeformer::bboxmin_attrMObj;
MObject tm_volumeDeformer::bboxmax_attrMObj;
MObject tm_volumeDeformer::bboxMode_attrMObj;

MObject tm_volumeDeformer::translatex_attrMObj;
MObject tm_volumeDeformer::translatey_attrMObj;
MObject tm_volumeDeformer::translatez_attrMObj;
MObject tm_volumeDeformer::translate_attrMObj;
MObject tm_volumeDeformer::rotatex_attrMObj;
MObject tm_volumeDeformer::rotatey_attrMObj;
MObject tm_volumeDeformer::rotatez_attrMObj;
MObject tm_volumeDeformer::rotate_attrMObj;
MObject tm_volumeDeformer::scalex_attrMObj;
MObject tm_volumeDeformer::scaley_attrMObj;
MObject tm_volumeDeformer::scalez_attrMObj;
MObject tm_volumeDeformer::scale_attrMObj;
MObject tm_volumeDeformer::pivotx_attrMObj;
MObject tm_volumeDeformer::pivoty_attrMObj;
MObject tm_volumeDeformer::pivotz_attrMObj;
MObject tm_volumeDeformer::pivot_attrMObj;

MObject tm_volumeDeformer::radius_attrMObj;
MObject tm_volumeDeformer::rolloff_attrMObj;
MObject tm_volumeDeformer::polymesh_attrMObj;

tm_volumeDeformer::tm_volumeDeformer(){}
tm_volumeDeformer::~tm_volumeDeformer(){}

void tm_volumeDeformer::getAttributesValues( MDataBlock& block, MStatus& status)
{
/*
   MDataHandle envData = block.inputValue( envelope, &status);
   if( !status) {status.perror("ERROR getting envelope data"); return;}
   env = envData.asDouble();
*/

   MDataHandle bboxXminData = block.inputValue( bboxXmin_attrMObj, &status);
   if( !status) {status.perror("ERROR getting bboxXmin data"); return;}
   bboxXmin = bboxXminData.asDouble();
   MDataHandle bboxXmaxData = block.inputValue( bboxXmax_attrMObj, &status);
   if( !status) {status.perror("ERROR getting bboxXmax data"); return;}
   bboxXmax = bboxXmaxData.asDouble();
   MDataHandle bboxYminData = block.inputValue( bboxYmin_attrMObj, &status);
   if( !status) {status.perror("ERROR getting bboxYmin data"); return;}
   bboxYmin = bboxYminData.asDouble();
   MDataHandle bboxYmaxData = block.inputValue( bboxYmax_attrMObj, &status);
   if( !status) {status.perror("ERROR getting bboxYmax data"); return;}
   bboxYmax = bboxYmaxData.asDouble();
   MDataHandle bboxZminData = block.inputValue( bboxZmin_attrMObj, &status);
   if( !status) {status.perror("ERROR getting bboxZmin data"); return;}
   bboxZmin = bboxZminData.asDouble();
   MDataHandle bboxZmaxData = block.inputValue( bboxZmax_attrMObj, &status);
   if( !status) {status.perror("ERROR getting bboxZmax data"); return;}
   bboxZmax = bboxZmaxData.asDouble();

   MDataHandle bboxModeData = block.inputValue( bboxMode_attrMObj, &status);
   if( !status) {status.perror("ERROR getting bboxMode data"); return;}
   bboxMode = bboxModeData.asShort();

   MDataHandle translatexData = block.inputValue( translatex_attrMObj, &status);
   if( !status) {status.perror("ERROR getting translatex data"); return;}
   translatex = translatexData.asDouble();
   MDataHandle translateyData = block.inputValue( translatey_attrMObj, &status);
   if( !status) {status.perror("ERROR getting translatey data"); return;}
   translatey = translateyData.asDouble();
   MDataHandle translatezData = block.inputValue( translatez_attrMObj, &status);
   if( !status) {status.perror("ERROR getting translatez data"); return;}
   translatez = translatezData.asDouble();
   MDataHandle rotatexData = block.inputValue( rotatex_attrMObj, &status);
   if( !status) {status.perror("ERROR getting rotatex data"); return;}
   rotatex = rotatexData.asDouble();
   MDataHandle rotateyData = block.inputValue( rotatey_attrMObj, &status);
   if( !status) {status.perror("ERROR getting rotatey data"); return;}
   rotatey = rotateyData.asDouble();
   MDataHandle rotatezData = block.inputValue( rotatez_attrMObj, &status);
   if( !status) {status.perror("ERROR getting rotatez data"); return;}
   rotatez = rotatezData.asDouble();
   MDataHandle scalexData = block.inputValue( scalex_attrMObj, &status);
   if( !status) {status.perror("ERROR getting scalex data"); return;}
   scalex = scalexData.asDouble();
   MDataHandle scaleyData = block.inputValue( scaley_attrMObj, &status);
   if( !status) {status.perror("ERROR getting scaley data"); return;}
   scaley = scaleyData.asDouble();
   MDataHandle scalezData = block.inputValue( scalez_attrMObj, &status);
   if( !status) {status.perror("ERROR getting scalez data"); return;}
   scalez = scalezData.asDouble();
   MDataHandle pivotxData = block.inputValue( pivotx_attrMObj, &status);
   if( !status) {status.perror("ERROR getting pivotx data"); return;}
   pivotx = pivotxData.asDouble();
   MDataHandle pivotyData = block.inputValue( pivoty_attrMObj, &status);
   if( !status) {status.perror("ERROR getting pivoty data"); return;}
   pivoty = pivotyData.asDouble();
   MDataHandle pivotzData = block.inputValue( pivotz_attrMObj, &status);
   if( !status) {status.perror("ERROR getting pivotz data"); return;}
   pivotz = pivotzData.asDouble();

   MDataHandle multiplierData = block.inputValue( multiplier_attrMObj, &status);
   if( !status) {status.perror("ERROR getting multiplier data"); return;}
   multiplier = multiplierData.asDouble();

   MDataHandle radiusData = block.inputValue( radius_attrMObj, &status);
   if( !status) {status.perror("ERROR getting radius data"); return;}
   radius = radiusData.asDouble();
   MDataHandle rolloffData = block.inputValue( rolloff_attrMObj, &status);
   if( !status) {status.perror("ERROR getting rolloff data"); return;}
   rolloff = rolloffData.asShort();

   MDataHandle polymeshData = block.inputValue( polymesh_attrMObj, &status);
   if( !status) {status.perror("ERROR getting polymesh data"); return;}
   polymesh = polymeshData.asMesh();
}

void tm_volumeDeformer::transformPoint( MPoint& pt)
{
   double x0 = (pt.x - pivotx)*scalex;
   double y0 = (pt.y - pivoty)*scaley;
   double z0 = (pt.z - pivotz)*scalez;

   double x1 = x0;
   double y1 = y0*cos( rotatex) - z0*sin( rotatex);
   double z1 = y0*sin( rotatex) + z0*cos( rotatex);

   double x2 = z1*sin( rotatey) + x1*cos( rotatey);
   double y2 = y1;
   double z2 = z1*cos( rotatey) - x1*sin( rotatey);

   double x3 = x2*cos( rotatez) - y2*sin( rotatez);
   double y3 = x2*sin( rotatez) + y2*cos( rotatez);
   double z3 = z2;

   pt.x += multiplier*(x3 + translatex - pt.x + pivotx);
   pt.y += multiplier*(y3 + translatey - pt.y + pivoty);
   pt.z += multiplier*(z3 + translatez - pt.z + pivotz);
}

void tm_volumeDeformer::transformPoint_part( MPoint& pt, double& part)
{
/*
   double tx = part*(translatex - pivotx);
   double ty = part*(translatey - pivoty);
   double tz = part*(translatez - pivotz);
*/
   double tx = part*translatex;
   double ty = part*translatey;
   double tz = part*translatez;
   double rx = part*rotatex;
   double ry = part*rotatey;
   double rz = part*rotatez;
   double sx = 1 + (part*(scalex - 1));
   double sy = 1 + (part*(scaley - 1));
   double sz = 1 + (part*(scalez - 1));

   double x0 = (pt.x - pivotx)*sx;
   double y0 = (pt.y - pivoty)*sy;
   double z0 = (pt.z - pivotz)*sz;

   double x1 = x0;
   double y1 = y0*cos( rx) - z0*sin( rx);
   double z1 = y0*sin( rx) + z0*cos( rx);

   double x2 = z1*sin( ry) + x1*cos( ry);
   double y2 = y1;
   double z2 = z1*cos( ry) - x1*sin( ry);

   double x3 = x2*cos( rz) - y2*sin( rz);
   double y3 = x2*sin( rz) + y2*cos( rz);
   double z3 = z2;

   pt.x += multiplier*( x3 + tx - pt.x + pivotx);
   pt.y += multiplier*( y3 + ty - pt.y + pivoty);
   pt.z += multiplier*( z3 + tz - pt.z + pivotz);
}

bool tm_volumeDeformer::isInBBoxRadius( MPoint& pt)
{

   if( pt.x < bboxXmin-radius) return false;
   if( pt.x > bboxXmax+radius) return false;
   if( pt.y < bboxYmin-radius) return false;
   if( pt.y > bboxYmax+radius) return false;
   if( pt.z < bboxZmin-radius) return false;
   if( pt.z > bboxZmax+radius) return false;

   return true;
}

MStatus tm_volumeDeformer::deform( MDataBlock& block, MItGeometry& iter, const MMatrix& worldSpace, unsigned int /*multiIndex*/)
{
   MStatus status = MS::kSuccess;

   getAttributesValues( block, status);
   if( !status) return status;

   if( multiplier == 0 ) return status;

   MFnMesh mesh( polymesh, &status);
   if( !status) {status.perror("ERROR getting mesh."); return status;}

   if( bboxMode > 0)
   {
      double bboxX = 0.5*(bboxXmax + bboxXmin);
      double bboxY = 0.5*(bboxYmax + bboxYmin);
      double bboxZ = 0.5*(bboxZmax + bboxZmin);

      pivotx += bboxX;
      pivoty += bboxY;
      pivotz += bboxZ;

      if( bboxMode < 2)
      {
         translatex += bboxX;
         translatey += bboxY;
         translatez += bboxZ;
      }
   }

   MPoint mp; MVector mv;
   for ( ; !iter.isDone(); iter.next())
   {
      MPoint pt = iter.position();

      pt *= worldSpace;

      if( isInBBoxRadius( pt))
      {
         status = mesh.getClosestPointAndNormal( pt, mp, mv, MSpace::kWorld);
         if( mv*MVector(mp-pt) > 0)
            transformPoint( pt);
         else
         {
            if( radius == 0) continue;

            mp.x -= pt.x; mp.y -= pt.y; mp.z -= pt.z;
            double part = mp.x*mp.x + mp.y*mp.y + mp.z*mp.z;
            if( part < 0) part = 0;
            else part = sqrt( part);

            if( part >= radius) continue;

            part = part/radius*2;
            if( part < 1)
            {
               degree(part);
               part = 2-part;
            }
            else
            {
               part = part-2;
               degree(part);
            }
            part = 0.5*part;

            transformPoint_part( pt, part);
         }
      }

      pt *= worldSpace.inverse();

      iter.setPosition( pt);
   }
   return status;
}

void tm_volumeDeformer::degree( double& arg)
{
   switch( rolloff)
   {
      case 0:
      {
         if( arg<0) arg = -arg;
         return;
      }
      case 1:
      {
         arg=arg*arg;
         return;
      }
      case 2:
      {
         arg=arg*arg*arg;
         if( arg<0) arg = -arg;
         return;
      }
   }
}

void* tm_volumeDeformer::creator(){return new tm_volumeDeformer();}
MStatus tm_volumeDeformer::initialize()
{
   MFnNumericAttribute nAttr;
   MFnUnitAttribute    uAttr;
   MFnTypedAttribute   tAttr;
   MFnEnumAttribute    eAttr;

   translatex_attrMObj = nAttr.create( "translatex", "tx", MFnNumericData::kDouble, 0.0);
   translatey_attrMObj = nAttr.create( "translatey", "ty", MFnNumericData::kDouble, 0.0);
   translatez_attrMObj = nAttr.create( "translatez", "tz", MFnNumericData::kDouble, 0.0);
   translate_attrMObj = nAttr.create( "translate", "tr", translatex_attrMObj, translatey_attrMObj, translatez_attrMObj);
      nAttr.setKeyable( true);
      addAttribute( translate_attrMObj);
      attributeAffects( translate_attrMObj, outputGeom );
   rotatex_attrMObj = uAttr.create( "rotatex", "rx", MFnUnitAttribute::kAngle, 0.0);
   rotatey_attrMObj = uAttr.create( "rotatey", "ry", MFnUnitAttribute::kAngle, 0.0);
   rotatez_attrMObj = uAttr.create( "rotatez", "rz", MFnUnitAttribute::kAngle, 0.0);
   rotate_attrMObj = nAttr.create( "rotate", "rt", rotatex_attrMObj, rotatey_attrMObj, rotatez_attrMObj);
      nAttr.setKeyable( true);
      addAttribute( rotate_attrMObj);
      attributeAffects( rotate_attrMObj, outputGeom );
   scalex_attrMObj = nAttr.create( "scalex", "sx", MFnNumericData::kDouble, 1.0);
   scaley_attrMObj = nAttr.create( "scaley", "sy", MFnNumericData::kDouble, 1.0);
   scalez_attrMObj = nAttr.create( "scalez", "sz", MFnNumericData::kDouble, 1.0);
   scale_attrMObj = nAttr.create( "scale", "sc", scalex_attrMObj, scaley_attrMObj, scalez_attrMObj);
      nAttr.setKeyable( true);
      addAttribute( scale_attrMObj);
      attributeAffects( scale_attrMObj, outputGeom );
   pivotx_attrMObj = nAttr.create( "pivotx", "px", MFnNumericData::kDouble, 0.0);
   pivoty_attrMObj = nAttr.create( "pivoty", "py", MFnNumericData::kDouble, 0.0);
   pivotz_attrMObj = nAttr.create( "pivotz", "pz", MFnNumericData::kDouble, 0.0);
   pivot_attrMObj = nAttr.create( "pivot", "pt", pivotx_attrMObj, pivoty_attrMObj, pivotz_attrMObj);
      nAttr.setKeyable( true);
      addAttribute( pivot_attrMObj);
      attributeAffects( pivot_attrMObj, outputGeom );

   bboxXmin_attrMObj = nAttr.create( "bboxXmin", "mnx", MFnNumericData::kDouble,-1.0);
   bboxXmax_attrMObj = nAttr.create( "bboxXmax", "mxx", MFnNumericData::kDouble, 1.0);
   bboxYmin_attrMObj = nAttr.create( "bboxYmin", "mny", MFnNumericData::kDouble,-1.0);
   bboxYmax_attrMObj = nAttr.create( "bboxYmax", "mxy", MFnNumericData::kDouble, 1.0);
   bboxZmin_attrMObj = nAttr.create( "bboxZmin", "mnz", MFnNumericData::kDouble,-1.0);
   bboxZmax_attrMObj = nAttr.create( "bboxZmax", "mxz", MFnNumericData::kDouble, 1.0);
   bboxmin_attrMObj = nAttr.create( "bboxmin", "mn", bboxXmin_attrMObj, bboxYmin_attrMObj, bboxZmin_attrMObj);
      nAttr.setKeyable( true);
      addAttribute( bboxmin_attrMObj);
      attributeAffects( bboxmax_attrMObj, outputGeom );
   bboxmax_attrMObj = nAttr.create( "bboxmax", "mx", bboxXmax_attrMObj, bboxYmax_attrMObj, bboxZmax_attrMObj);
      nAttr.setKeyable( true);
      addAttribute( bboxmax_attrMObj);
      attributeAffects( bboxmax_attrMObj, outputGeom );
   bboxMode_attrMObj = eAttr.create("bboxMode", "bm", 3);
#ifndef MAYA_VERSION_6_0
      eAttr.setChannelBox( true);
#endif
      eAttr.addField( "off", 0);
      eAttr.addField( "transform", 1);
      eAttr.addField( "pivot", 2);
      eAttr.setDefault( 2);
      addAttribute( bboxMode_attrMObj);
      attributeAffects( bboxMode_attrMObj, outputGeom );

   radius_attrMObj = nAttr.create( "radius", "rd", MFnNumericData::kDouble, 1.0);
      nAttr.setMin( 0);
      nAttr.setKeyable( true);
      addAttribute( radius_attrMObj);
      attributeAffects( radius_attrMObj, outputGeom );
   rolloff_attrMObj = eAttr.create("rollfoff", "rf", 3);
#ifndef MAYA_VERSION_6_0
      eAttr.setChannelBox( true);
#endif
      eAttr.addField( "linear", 0);
      eAttr.addField( "quadratic", 1);
      eAttr.addField( "cubic", 2);
      eAttr.setDefault( 1);
      addAttribute( rolloff_attrMObj);
      attributeAffects( rolloff_attrMObj, outputGeom );

   multiplier_attrMObj = nAttr.create( "multiplier", "ml", MFnNumericData::kDouble, 0.0);
      nAttr.setKeyable(true);
      addAttribute( multiplier_attrMObj);
      attributeAffects( multiplier_attrMObj, outputGeom );

   polymesh_attrMObj = tAttr.create("polymesh", "pm", MFnMeshData::kMesh);
      tAttr.setStorable(true);
      addAttribute( polymesh_attrMObj );
      attributeAffects( polymesh_attrMObj, outputGeom );

   MPxManipContainer::addToManipConnectTable(id);

   return MS::kSuccess;
}
