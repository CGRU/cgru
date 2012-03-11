#ifndef MAKEHGEO_H
#define MAKEHGEO_H

#include "../definitions.h"

#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h>

#include <maya/MTypes.h>

#include <maya/MIntArray.h>
#include <maya/MDataBlock.h>
#include <maya/MFloatArray.h>
#include <maya/MPointArray.h>
#include <maya/MPxNode.h>
#include <maya/MString.h>

#define FILENAME_MAXLENGTH 255
#define MIN_SUBFRAME 0.0001

class tm_makeHgeo : public MPxNode
{
public:
   tm_makeHgeo();
   virtual ~tm_makeHgeo();
   virtual MStatus compute( const MPlug& plug, MDataBlock& data );
   static void * creator();
   static MStatus initialize();

public:
   static MTypeId id;
   static MObject dummy_attrMObj;
   static MObject fileName_attrMObj;
   static MObject frame_attrMObj;
   static MObject padding_attrMObj;
   static MObject reconstruct_attrMObj;
   static MObject cacheFrames_attrMObj;
   static MObject reverse_attrMObj;
   static MObject outMesh_attrMObj;

private:
   bool     getNodeAttributes( const MPlug& plug, MDataBlock& data );
   MString  fileBody;
   float    frameFloat;
   int      padding;
   bool     recreate;
   int      cacheFrames;
   bool     reverse;

   MString  fileBodyPrev;
   int      cacheFramesPrev;
   bool     constructed;

   char  fileChar[FILENAME_MAXLENGTH];
   char  *file_buf;
   long  file_buf_len;
   long  file_len;
   void  createFileName( char *fileName, const MString &file_boby, const int number, const int padding);
   bool  readFileInBuffer( char *fileName);
   bool  parseFileBuffer( MPointArray &vertexArray, bool readVerticesOnly = false);

   int ffind( const char* str_to_seek, char* data, const int pos);
   int ffind( const char char_to_seek, char* data, const int pos);

   bool createMesh( const MPointArray &vertexArray, const MPlug& plug, MDataBlock& data );
   bool deformMesh( MPointArray &vertexArray, const MPlug& plug, MDataBlock& data );
   void linearStep( MPointArray &vertexArray, const MPointArray &vertexArray_A, const MPointArray &vertexArray_B, float sub);

   int verticesCount;
   int numPolygons;
   MIntArray polygonCounts;
   MIntArray polygonConnects;
   bool has_uvs;
   MIntArray uvIds;
   MFloatArray uArray, vArray;

   int frameIntNext;
   int frameIntPrev;
   MPointArray vertexArrayNext;
   MPointArray vertexArrayPrev;

   MPointArray* cache;
   MIntArray cacheFramesNums;
   void initCache();
   MPointArray* getPoints( const int frame);
   MPointArray mPointArray;
};
#endif
