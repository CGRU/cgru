#include "transformUV.h"
#include "../../definitions.h"

#include <math.h>

// Function Sets
//
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMeshData.h>

// General Includes
//
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MIOStream.h>
#include <maya/MFloatArray.h>

// Macros
//
#define MCheckStatus(status,message)	\
	if( MStatus::kSuccess != status ) {	\
		status.perror(message);		\
		return status;					\
	}


// Unique Node TypeId
//
MTypeId tm_transformUV::id( tm_transformUV__id );

MObject tm_transformUV::inMesh_attrMObj;

MObject tm_transformUV::pivotu_attrMObj;
MObject tm_transformUV::pivotv_attrMObj;

MObject tm_transformUV::translateu_attrMObj;
MObject tm_transformUV::translatev_attrMObj;
MObject tm_transformUV::rotate_attrMObj;
MObject tm_transformUV::scaleu_attrMObj;
MObject tm_transformUV::scalev_attrMObj;

MObject tm_transformUV::uvsettype_attrMObj;
MObject tm_transformUV::uvsetname_attrMObj;

MObject tm_transformUV::outMesh_attrMObj;

tm_transformUV::tm_transformUV() {}

tm_transformUV::~tm_transformUV() {}

void tm_transformUV::getTransformValues( MDataBlock& data , MStatus& status)
{
   MDataHandle translateuData = data.inputValue( translateu_attrMObj, &status );
   if( !status) {status.perror("ERROR getting translateu data"); return;}
   translateu = translateuData.asDouble();

   MDataHandle translatevData = data.inputValue( translatev_attrMObj, &status );
   if( !status) {status.perror("ERROR getting translatev data"); return;}
   translatev = translatevData.asDouble();

   MDataHandle rotatexData = data.inputValue( rotate_attrMObj, &status );
   if( !status) {status.perror("ERROR getting rotate data"); return;}
   rotate = rotatexData.asDouble()/180 * M_PI;

   MDataHandle scaleuData = data.inputValue( scaleu_attrMObj, &status );
   if( !status) {status.perror("ERROR getting scaleu data"); return;}
   scaleu = scaleuData.asDouble();

   MDataHandle scalevData = data.inputValue( scalev_attrMObj, &status );
   if( !status) {status.perror("ERROR getting scalev data"); return;}
   scalev = scalevData.asDouble();

   MDataHandle pivotuData = data.inputValue( pivotu_attrMObj, &status );
   if( !status) {status.perror("ERROR getting pivotu data"); return;}
   pivotu = pivotuData.asDouble();

   MDataHandle pivotvData = data.inputValue( pivotv_attrMObj, &status );
   if( !status) {status.perror("ERROR getting pivotv data"); return;}
   pivotv = pivotvData.asDouble();

   MDataHandle uvsettypeData = data.inputValue( uvsettype_attrMObj, &status);
   if( !status) {status.perror("ERROR getting uvsettype data"); return;}
   uvsettype = uvsettypeData.asShort();

   MDataHandle uvsetnameData = data.inputValue( uvsetname_attrMObj, &status);
   if( !status) {status.perror("ERROR getting uvsetname data"); return;}
   uvsetname = uvsetnameData.asString();
}

void tm_transformUV::transformUVs( MFnMesh& meshFn, MString &uvSetName, MStatus& status)
{
   MFloatArray uArray, vArray;

   status = meshFn.getUVs( uArray, vArray, &uvSetName);
   if( !status)
   {
      MGlobal::displayError("Can't get uvs from set " + uvSetName);
      return;
   }

   int length = uArray.length();

   for( int i = 0; i < length; i++)
   {
      float u1 = (float)((uArray[i] - pivotu) * scaleu);
      float v1 = (float)((vArray[i] - pivotv) * scalev);

      float u2 = (float)(u1*cos( rotate) - v1*sin( rotate));
      float v2 = (float)(u1*sin( rotate) + v1*cos( rotate));

      uArray[i] = (float)(u2 + translateu + pivotu);
      vArray[i] = (float)(v2 + translatev + pivotv);
   }

   status = meshFn.setUVs( uArray, vArray, &uvSetName);
   if( !status)
   {
      MGlobal::displayError("Can't set uvs to set " + uvSetName);
      return;
   }
}

MStatus tm_transformUV::compute( const MPlug& plug, MDataBlock& data )
{
   MStatus status = MS::kSuccess;

   MDataHandle stateData = data.outputValue( state, &status );
   MCheckStatus( status, "ERROR getting state" );

   // Check for the HasNoEffect/PassThrough flag on the node.
   //
   // (stateData is an enumeration standard in all depend nodes - stored as short)
   //
   // (0 = Normal)
   // (1 = HasNoEffect/PassThrough)
   // (2 = Blocking)
   // ...
   //
   if( stateData.asShort() == 1 )
   {
      MDataHandle inputMeshData = data.inputValue( inMesh_attrMObj, &status );
      MCheckStatus(status,"ERROR getting inMesh_attrMObj");

      MDataHandle outputMeshData = data.outputValue( outMesh_attrMObj, &status );
      MCheckStatus(status,"ERROR getting outMesh_attrMObj");

      // Simply redirect the inMesh_attrMObj to the outMesh_attrMObj for the PassThrough effect
      //
      outputMeshData.set(inputMeshData.asMesh());
   }
   else
   {
      if (plug == outMesh_attrMObj)
      {
         MDataHandle inputMeshData = data.inputValue( inMesh_attrMObj, &status );
         if( !status) {status.perror("ERROR getting inMesh data"); return status;}
         MDataHandle outputMeshData = data.outputValue( outMesh_attrMObj, &status);
         if( !status) {status.perror("ERROR getting outMesh data"); return status;}
         outputMeshData.set(inputMeshData.asMesh());
         MObject meshObj = outputMeshData.asMesh();

         getTransformValues( data, status);
         if( !status) {status.perror("ERROR getTransformValues"); return status;}

         MFnMesh meshFn(meshObj, &status);
         if( !status) {status.perror("ERROR getting MFnMesh"); return status;}

         switch( uvsettype)
         {
            case 0:
            {
               MString currentUVSetName;
               status = meshFn.getCurrentUVSetName( currentUVSetName);
               if( !status)
               {
                  MGlobal::displayError("Can't get current uvset name.");
                  return status;
               }
               transformUVs( meshFn, currentUVSetName, status);
               if( !status) return status;
               break;
            }
            case 1:
            {
               MStringArray uvSetNames;
               status = meshFn.getUVSetNames( uvSetNames);
               if( !status)
               {
                  MGlobal::displayError("Can't get all uvset names.");
                  return status;
               }
               int length = uvSetNames.length();
               for( int i = 0; i < length; i++)
               {
                  transformUVs( meshFn, uvSetNames[i], status);
                  if( !status) return status;
               }
               break;
            }
            case 2:
            {
               transformUVs( meshFn, uvsetname, status);
               if( !status) return status;
               break;
            }
            default:
            {
               MGlobal::displayError("Unknown uvset type parameter.");
               return MStatus::kFailure;
            }
         }
         outputMeshData.setClean();
      }
      else
      {
         status = MS::kUnknownParameter;
      }
   }
   return status;
}

void* tm_transformUV::creator()
{
   return new tm_transformUV();
}

MStatus tm_transformUV::initialize()
{
   MFnNumericAttribute  nAttr;
   MFnTypedAttribute    tAttr;
   MFnEnumAttribute     eAttr;

   MStatus stat;

   outMesh_attrMObj = tAttr.create("outMesh", "om", MFnMeshData::kMesh);
   tAttr.setStorable(true);
   tAttr.setWritable(false);
   stat = addAttribute( outMesh_attrMObj);
   if (!stat) { stat.perror("addAttribute outMesh"); return stat;}

   inMesh_attrMObj = tAttr.create("inMesh", "im", MFnMeshData::kMesh);
   tAttr.setStorable(true);
   stat = addAttribute( inMesh_attrMObj );
   if (!stat) { stat.perror("addAttribute inMesh"); return stat;}
   stat = attributeAffects( inMesh_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects inMesh"); return stat;}

   translateu_attrMObj = nAttr.create( "translateu", "tu", MFnNumericData::kDouble, 0.0 );
#ifndef MAYA_VERSION_6_0
   nAttr.setChannelBox(true);
#endif
   nAttr.setKeyable(true);
   stat = addAttribute( translateu_attrMObj );
   if (!stat) { stat.perror("addAttribute translateu"); return stat;}
   stat = attributeAffects( translateu_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects translateu"); return stat;}

   translatev_attrMObj = nAttr.create( "translatev", "tv", MFnNumericData::kDouble, 0.0 );
#ifndef MAYA_VERSION_6_0
   nAttr.setChannelBox(true);
#endif
   nAttr.setKeyable(true);
   stat = addAttribute( translatev_attrMObj );
   if (!stat) { stat.perror("addAttribute translatev"); return stat;}
   stat = attributeAffects( translatev_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects translatev"); return stat;}

   rotate_attrMObj = nAttr.create( "rotate", "r", MFnNumericData::kDouble, 0.0 );
#ifndef MAYA_VERSION_6_0
   nAttr.setChannelBox(true);
#endif
   nAttr.setKeyable(true);
   stat = addAttribute( rotate_attrMObj );
   if (!stat) { stat.perror("addAttribute rotate"); return stat;}
   stat = attributeAffects( rotate_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects rotate"); return stat;}

   scaleu_attrMObj = nAttr.create( "scaleu", "su", MFnNumericData::kDouble,-1.0 );
#ifndef MAYA_VERSION_6_0
   nAttr.setChannelBox(true);
#endif
   nAttr.setKeyable(true);
   stat = addAttribute( scaleu_attrMObj );
   if (!stat) { stat.perror("addAttribute scaleu"); return stat;}
   stat = attributeAffects( scaleu_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects scaleu"); return stat;}

   scalev_attrMObj = nAttr.create( "scalev", "sv", MFnNumericData::kDouble, 1.0 );
#ifndef MAYA_VERSION_6_0
   nAttr.setChannelBox(true);
#endif
   nAttr.setKeyable(true);
   stat = addAttribute( scalev_attrMObj );
   if (!stat) { stat.perror("addAttribute scalev"); return stat;}
   stat = attributeAffects( scalev_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects scalev"); return stat;}

   pivotu_attrMObj = nAttr.create( "pivotu", "pu", MFnNumericData::kDouble, 0.5 );
#ifndef MAYA_VERSION_6_0
   nAttr.setChannelBox(true);
#endif
   nAttr.setKeyable(true);
   stat = addAttribute( pivotu_attrMObj );
   if (!stat) { stat.perror("addAttribute pivotu"); return stat;}
   stat = attributeAffects( pivotu_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects pivotu"); return stat;}

   pivotv_attrMObj = nAttr.create( "pivotv", "pv", MFnNumericData::kDouble, 0.5 );
   nAttr.setKeyable(true);
#ifndef MAYA_VERSION_6_0
   nAttr.setChannelBox(true);
#endif
   stat = addAttribute( pivotv_attrMObj );
   if (!stat) { stat.perror("addAttribute pivotv"); return stat;}
   stat = attributeAffects( pivotv_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects pivotv"); return stat;}

   uvsettype_attrMObj = eAttr.create("uvsettype", "st", 3);
   eAttr.addField( "current", 0);
   eAttr.addField( "all", 1);
   eAttr.addField( "specified", 2);
   eAttr.setDefault( 0);
   eAttr.setStorable(true);
//   eAttr.setKeyable(true);
#ifndef MAYA_VERSION_6_0
   eAttr.setChannelBox(true);
#endif
   stat = addAttribute( uvsettype_attrMObj );
   if (!stat) { stat.perror("addAttribute uvsettype"); return stat;}
   stat = attributeAffects( uvsettype_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects uvsettype"); return stat;}

   uvsetname_attrMObj = tAttr.create("uvsetname", "sn", MFnData::kString, MObject::kNullObj, &stat);
   tAttr.setStorable(true);
//   eAttr.setKeyable(true);
#ifndef MAYA_VERSION_6_0
   tAttr.setChannelBox(true);
#endif
   stat = addAttribute( uvsetname_attrMObj );
   if (!stat) { stat.perror("addAttribute uvsetname"); return stat;}
   stat = attributeAffects( uvsetname_attrMObj, outMesh_attrMObj );
   if (!stat) { stat.perror("attributeAffects uvsetname"); return stat;}
/**/
   return MS::kSuccess;
}
