#ifndef VOLUMEDEFORMER_H
#define VOLUMEDEFORMER_H

#include "../definitions.h"

#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h>

#include <maya/MPxDeformerNode.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>

class tm_volumeDeformer : public MPxDeformerNode
{
public:
   tm_volumeDeformer();
   ~tm_volumeDeformer();

   static  void*     creator();
   static  MStatus   initialize();

   MStatus deform( MDataBlock& block, MItGeometry& iter, const MMatrix& worldSpace, unsigned int multiIndex);

public:

   static MObject multiplier_attrMObj;

   static MObject bboxXmin_attrMObj;
   static MObject bboxXmax_attrMObj;
   static MObject bboxYmin_attrMObj;
   static MObject bboxYmax_attrMObj;
   static MObject bboxZmin_attrMObj;
   static MObject bboxZmax_attrMObj;
   static MObject bboxmin_attrMObj;
   static MObject bboxmax_attrMObj;

   static MObject bboxMode_attrMObj;

   static MObject translatex_attrMObj;
   static MObject translatey_attrMObj;
   static MObject translatez_attrMObj;
   static MObject translate_attrMObj;
   static MObject rotatex_attrMObj;
   static MObject rotatey_attrMObj;
   static MObject rotatez_attrMObj;
   static MObject rotate_attrMObj;
   static MObject scalex_attrMObj;
   static MObject scaley_attrMObj;
   static MObject scalez_attrMObj;
   static MObject scale_attrMObj;

   static MObject pivotx_attrMObj;
   static MObject pivoty_attrMObj;
   static MObject pivotz_attrMObj;
   static MObject pivot_attrMObj;

   static MObject radius_attrMObj;
   static MObject rolloff_attrMObj;
   static MObject polymesh_attrMObj;

   static MTypeId id;

private:

   void getAttributesValues( MDataBlock& block, MStatus& status);
   void transformPoint( MPoint& pt);
   void transformPoint_part( MPoint& pt, double& part);
   bool isInBBoxRadius( MPoint& pt);
   void degree( double& arg);

   double env;
   double multiplier;

   double bboxXmin;
   double bboxXmax;
   double bboxYmin;
   double bboxYmax;
   double bboxZmin;
   double bboxZmax;
   int    bboxMode;

   double translatex;
   double translatey;
   double translatez;
   double rotatex;
   double rotatey;
   double rotatez;
   double scalex;
   double scaley;
   double scalez;

   double pivotx;
   double pivoty;
   double pivotz;

   double radius;
   int    rolloff;

   MObject polymesh;

};

#endif
