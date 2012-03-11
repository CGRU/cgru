#ifndef TM_TRANSFORMUV_H
#define TM_TRANSFORMUV_H

#include "../../definitions.h"

#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h>

#include <maya/MPxNode.h>
#include <maya/MFnMesh.h>

class tm_transformUV : public MPxNode
{
public:
   tm_transformUV();
   ~tm_transformUV();

   MStatus compute( const MPlug& plug, MDataBlock& data );

   static  void* creator();
   static  MStatus initialize();

public:

   static MTypeId id;

   static MObject inMesh_attrMObj;

   static MObject translateu_attrMObj;
   static MObject translatev_attrMObj;
   static MObject rotate_attrMObj;
   static MObject scaleu_attrMObj;
   static MObject scalev_attrMObj;

   static MObject pivotu_attrMObj;
   static MObject pivotv_attrMObj;

   static MObject uvsettype_attrMObj;
   static MObject uvsetname_attrMObj;

   static MObject outMesh_attrMObj;

private:

   void getTransformValues( MDataBlock& data, MStatus& status);
   void transformUVs( MFnMesh& meshFn, MString &uvSetName, MStatus& status);

   double translateu;
   double translatev;
   double rotate;
   double scaleu;
   double scalev;
   double pivotu;
   double pivotv;
   int uvsettype;
   MString uvsetname;

};

#endif
