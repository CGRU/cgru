#define McheckErr(stat,msg)         \
    if ( MS::kSuccess != stat ) {   \
        stat.perror(msg);                \
        return false;        \
    }

#include "makehgeo.h"

#include <maya/MDataHandle.h>
#include <maya/MDoubleArray.h>
//#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MPlug.h>
//#include <maya/MTypeId.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef NT_PLUGIN
#include <io.h>
#define open _open
#define close _close
#define read _read
#define O_RDONLY _O_RDONLY
#define fstat _fstat
#define stat _stat
#ifndef MAYA_VERSION_6_0
#define atof atofwin
double atofwin( char *str)
{ _CRT_DOUBLE value; _atodbl( &value, str); return value.x; }
#endif
#endif
#include <iostream>

#define _STDOUT_ON
#undef _STDOUT_ON

//
//
/*-------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/
// node attributes:
/*-------------------------------------------------------------------------------------------------------------------*/
MTypeId tm_makeHgeo::id( tm_makeHgeo__id );
MObject tm_makeHgeo::dummy_attrMObj;
MObject tm_makeHgeo::fileName_attrMObj;
MObject tm_makeHgeo::frame_attrMObj;
MObject tm_makeHgeo::padding_attrMObj;
MObject tm_makeHgeo::reconstruct_attrMObj;
MObject tm_makeHgeo::cacheFrames_attrMObj;
MObject tm_makeHgeo::reverse_attrMObj;
MObject tm_makeHgeo::outMesh_attrMObj;
//
//
/*-------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/
// class constructor & dectructor:
/*-------------------------------------------------------------------------------------------------------------------*/
tm_makeHgeo::tm_makeHgeo()
{
   file_len = 0;
   file_buf = NULL;
   file_buf_len = 0;
   constructed = false;
   frameIntNext = -1;
   frameIntPrev = -1;
   cacheFramesPrev = -1;
   cache = NULL;
}
tm_makeHgeo::~tm_makeHgeo()
{
   if( file_buf != NULL ) delete [] file_buf;
   if( cache != NULL) delete [] cache;
}
//
//
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/
// static attributes:
/*-------------------------------------------------------------------------------------------------------------------*/
//
//
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/
// member functions:
/*-------------------------------------------------------------------------------------------------------------------*/
bool tm_makeHgeo::getNodeAttributes( const MPlug& plug, MDataBlock& data )
{
   MStatus stat;

   MDataHandle dummyData = data.inputValue( dummy_attrMObj, &stat);

   MDataHandle fileNameData = data.inputValue( fileName_attrMObj, &stat);
   McheckErr( stat, "Error reading fileName dattribute.\n");
   fileBody = fileNameData.asString();

   MDataHandle frameData = data.inputValue( frame_attrMObj, &stat);
   McheckErr( stat, "Error reading frame attribute.\n");
   frameFloat = frameData.asFloat();

   MDataHandle paddingData = data.inputValue( padding_attrMObj, &stat);
   McheckErr( stat, "Error reading padding attribute.\n");
   padding = paddingData.asInt();

   MDataHandle reconstructData = data.inputValue( reconstruct_attrMObj, &stat);
   McheckErr( stat, "Error reading reconstruct attribute.\n");
   recreate = reconstructData.asBool();

   MDataHandle cacheFramesData = data.inputValue( cacheFrames_attrMObj, &stat);
   McheckErr( stat, "Error reading padding attribute.\n");
   cacheFrames = cacheFramesData.asInt();

   MDataHandle reverseData = data.inputValue( reverse_attrMObj, &stat);
   McheckErr( stat, "Error reading reverse attribute.\n");
   reverse = reverseData.asBool();

   return true;
}
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
void tm_makeHgeo::createFileName( char *fileName, const MString &file_boby, const int number, const int padding)
{
   int fileBody_l = file_boby.length();
   memcpy( fileName, file_boby.asChar(), fileBody_l);
   char fileSuffix[FILENAME_MAXLENGTH];
   // setting mask
   char fileSuffix_temp[] = ".%04d.geo";
   // changing padding
   fileSuffix_temp[3] = padding + '0';
   // constructing result
   sprintf( fileSuffix, fileSuffix_temp, number);
   int fileSuffix_l = (int)strlen( fileSuffix);
   memcpy( fileName+fileBody_l, fileSuffix, fileSuffix_l);
   fileName[ fileBody_l + fileSuffix_l] = '\0';
}
/*-------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/
bool tm_makeHgeo::readFileInBuffer( char *fileName)
{
   // open the file
#ifdef _STDOUT_ON
printf("\nreading file \"%s\"",fileName);
#endif
   int fd = open( fileName, O_RDONLY);
   if( fd < 0)
   {
      perror( fileName);
      MString mstr_err = "Can't open \"";
      mstr_err += fileName;
      mstr_err += "\" file.";
      MGlobal::displayError( mstr_err);
      return false;
   }
   // get file length
   int status;
   struct stat file_stat;
   status = fstat( fd, &file_stat);
   if( status < 0)
   {
      perror( fileName);
      MGlobal::displayError("Can't get file lenght.");
      return false;
   }
   file_len = file_stat.st_size;
#ifdef _STDOUT_ON
printf(", size = %d\n", file_len );
#endif
   // allocate memory for file_buf
   if( file_len > file_buf_len )
   {
      if( file_buf != NULL ) delete [] file_buf;
      file_buf_len = file_len * 2;
      file_buf = new char[file_buf_len];
      if( file_buf == NULL)
      {
         MGlobal::displayError(MString("Can't allocate ") + file_buf_len + "bytes of memory to cache file \"" + fileName + "\"");
         return false;
      }
   }
   // read whole file data in file_buf
#ifdef _STDOUT_ON
printf("begin reading %d bytes in %d\n", file_len, file_buf_len);
#endif
   if( read( fd, file_buf, file_len) != file_len)
   {
      MGlobal::displayError(MString("Can't read ") + file_len + "bytes from file \"" + fileName + "\"");
      return false;
   }
#ifdef _STDOUT_ON
printf("read successfuly\n");
#endif
   close( fd);
   return true;
}
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
bool tm_makeHgeo::parseFileBuffer( MPointArray &vertexArray, bool readVerticesOnly)
{
//-----------------------------------------------------------------------------------------
char* NPoints = "NPoints"   ;
char* NPrims =  "NPrims"    ;
char* Poly =    "Poly"      ;
char* UVs =     "uv 3 float";
//-----------------------------------------------------------------------------------------
#ifdef _STDOUT_ON
printf("Parsing file started.\n");
#endif
   long pos = 0;
   if( !readVerticesOnly)
   {
      // get vertices count (NPoints -> verticesCount)
      pos = ffind( NPoints, file_buf, pos) + 1;
      if( pos < 1)
      {
         MGlobal::displayError(MString("Can't find ") + NPoints);
         return false;
      }
      verticesCount = atoi( file_buf+pos);
      vertexArray.setLength( verticesCount);
      // get polygons count (NPrims -> numPolygons)
      pos = ffind( NPrims, file_buf, pos) + 1;
      if( pos < 1)
      {
         MGlobal::displayError(MString("Can't find ") + NPrims);
         return false;
      }
      numPolygons = atoi( file_buf+pos);
      polygonCounts.setLength( numPolygons);
#ifdef _STDOUT_ON
printf("verticesCount=%d, numPolygons = %d.\n", verticesCount, numPolygons);
#endif
   }
   // get points coordinates
#ifdef _STDOUT_ON
printf("points coordinates...   ");
#endif
   if( vertexArray.length() != verticesCount) vertexArray.setLength( verticesCount);
   pos = ffind( 10, file_buf, pos) + 1;
   if( pos < 1) return false;
   while( !(((file_buf[pos]>='0')&&(file_buf[pos]<='9'))||(file_buf[pos]=='-')) )
   {
      pos = ffind( 10, file_buf, pos) + 1;
      if( pos < 1) return false;
   }
   for( int p = 0; p < verticesCount; p++)
   {
      vertexArray[p].x = atof( file_buf+pos);
      pos = ffind( ' ', file_buf, pos) + 1;
      if( pos < 1) return false;
      vertexArray[p].y = atof( file_buf+pos);
      pos = ffind( ' ', file_buf, pos) + 1;
      if( pos < 1) return false;
      vertexArray[p].z = atof( file_buf+pos);
      pos = ffind(  10, file_buf, pos) + 1;
      if( pos < 1) return false;
   }
#ifdef _STDOUT_ON
printf("parsed.\n");
#endif
   if( !readVerticesOnly)
   {
      if( ffind( UVs, file_buf, pos) > 0)
         has_uvs = true;
      else
         has_uvs = false;
      // get polygonCounts and polygonConnects
#ifdef _STDOUT_ON
printf("topology...   ");
#endif
      pos = ffind( Poly, file_buf, pos) + 2;
      if( pos < 2)
      {
         MGlobal::displayError(MString("Can't find ") + Poly);
         return false;
      }
      long poly_pos = pos;
      polygonConnects.clear();
      polygonConnects.setSizeIncrement( 65535);
      MIntArray connects(16);
      for( int p = 0; p < numPolygons; p++)
      {
         while( !((file_buf[pos]>='0')&&(file_buf[pos]<='9')))
         {
            pos = ffind( 10, file_buf, pos) + 2;
            if( pos < 2) return false;
         }
         int pcount = atoi( file_buf+pos);
         polygonCounts[p] = pcount;
         pos = ffind( '<', file_buf, pos) + 2;
         if( pos < 2) return false;

         if( (int)connects.length() < pcount) 
			 connects.setLength( pcount * 2);
         for( int v = 0; v < pcount; v++)
         {
            connects[v] = atoi( file_buf+pos);
            if( v != pcount-1)
            {
               pos = ffind( ' ', file_buf, pos) + 1;
               if( pos < 1) return false;
               if( file_buf[pos] == '(')
               {
                  pos = ffind( ')', file_buf, pos) + 2;
                  if( pos < 2) return false;
               }
            }
            else
            {
               pos = ffind( 10, file_buf, pos) + 2;
               if( pos < 2) return false;
            }
         }
         if( reverse)
            for( int v = pcount-1; v >= 0; v--)
               polygonConnects.append( connects[v]);
         else
            for( int v = 0; v < pcount; v++)
               polygonConnects.append( connects[v]);
      }
#ifdef _STDOUT_ON
printf("parsed.\n");
#endif
      if( has_uvs)
      {
#ifdef _STDOUT_ON
printf("UVs...   ");
#endif
         int uvIds_len = polygonConnects.length();
         uvIds.setLength( uvIds_len);
         uArray.setLength( uvIds_len);
         vArray.setLength( uvIds_len);
         for( int i = 0; i < uvIds_len; i++) uvIds[i] = i;
         pos = poly_pos;
         int uv = 0;
         for( int p = 0; p < numPolygons; p++)
         {
            int numVtx = polygonCounts[p];
            for( int v = 0; v < numVtx; v++)
            {
               pos = ffind( '(', file_buf, pos) + 1;
               if( pos < 1) return false;
               uArray[uv] = (float)atof( file_buf+pos);
               pos = ffind( ' ', file_buf, pos) + 1;
               if( pos < 1) return false;
               vArray[uv] = (float)atof( file_buf+pos);
               uv++;
            }
         }
#ifdef _STDOUT_ON
printf("parsed.\n");
#endif
      }
/////////////////////////////////////////////////////////////////////////////////////////
#ifdef _STDOUT_ON
/*
printf("verticesCount=%d\n",verticesCount);
printf("numPolygons=%d\n",numPolygons);
for( int p = 0; p < verticesCount; p++)
   printf( "%f %f %f\n", vertexArray[p].x, vertexArray[p].y, vertexArray[p].z);
int pc = 0;
for( int p = 0; p < numPolygons; p++)
{
   printf(" %d < ", polygonCounts[p]);
   for( int v = 0; v < polygonCounts[p]; v++)
      printf(" %d", polygonConnects[pc++]);
   printf("\n");
}
*/
#endif
/////////////////////////////////////////////////////////////////////////////////////////
#ifdef _STDOUT_ON
printf("Parsing file successfuly finished.\n");
#endif
   }
   return true;
}
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
int tm_makeHgeo::ffind( const char* str_to_seek, char* data, const int pos)
{
   int str_to_seek_l = (int)strlen( str_to_seek);
   int equal = 0;
   for( int i = pos; i < file_len; i++)
   {
      if( data[i] == str_to_seek[equal]) equal++;
      else equal = 0;
      if( equal == str_to_seek_l) return i+1;
   }
   return -1;
}
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
int tm_makeHgeo::ffind( const char char_to_seek, char* data, const int pos)
{
   for( int i = pos; i < file_len; i++)
   {
      if( data[i] == char_to_seek)
      {
         return i;
      }
   }
   return -1;
}
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
bool tm_makeHgeo::createMesh(  const MPointArray &vertexArray, const MPlug& plug, MDataBlock& data )
{
   MStatus stat;

   MFnMeshData dataCreator;
   MObject newOutputData = dataCreator.create( &stat);
   McheckErr( stat, "ERROR creating outputData.\n");

   MFnMesh meshFn;
   MObject mesh = meshFn.create(
         verticesCount,
         numPolygons,
         vertexArray,
         polygonCounts,
         polygonConnects,
         newOutputData,
         &stat
      );
   McheckErr( stat, "ERROR creating mesh.\n");

   stat = meshFn.setObject( newOutputData);
   McheckErr( stat, "ERROR reading created mesh.\n");

   if( has_uvs )
   {
      MString uvSetName;
      meshFn.getCurrentUVSetName( uvSetName);
         McheckErr( stat, "ERROR getting current uvset name.\n");
      stat = meshFn.setUVs( uArray, vArray, &uvSetName);
         McheckErr( stat, "ERROR setting UVs.\n");
      stat = meshFn.assignUVs( polygonCounts, uvIds, &uvSetName);
      if( stat==MS::kInvalidParameter) stat.perror("meshFn.assignUVs() == MS::kInvalidParameter.");
         McheckErr( stat, "ERROR assigning UVs.\n");
   }

   MDataHandle outputHandle = data.outputValue( outMesh_attrMObj, &stat);
   outputHandle.set( newOutputData);
   data.setClean( plug);

   return true;
}
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
bool tm_makeHgeo::deformMesh( MPointArray &vertexArray, const MPlug& plug, MDataBlock& data )
{
   MStatus stat;

   MDataHandle outputHandle = data.outputValue( outMesh_attrMObj, &stat);
   McheckErr( stat, "ERROR reading polygon attribute.\n");

   MObject mesh = outputHandle.asMesh();
   MFnMesh meshFn( mesh);
   meshFn.setPoints( vertexArray);

   data.setClean( plug);
   return true;
}
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
void tm_makeHgeo::linearStep( MPointArray &vertexArray, const MPointArray &vertexArray_A, const MPointArray &vertexArray_B, float sub)
{
   int length = vertexArray_A.length();
   if( vertexArray.length() != length) vertexArray.setLength( length);
   for( int v = 0; v < length; v++)
   {
      double x = vertexArray_A[v].x;
      double y = vertexArray_A[v].y;
      double z = vertexArray_A[v].z;

      vertexArray[v].x = x + sub*( vertexArray_B[v].x - x);
      vertexArray[v].y = y + sub*( vertexArray_B[v].y - y);
      vertexArray[v].z = z + sub*( vertexArray_B[v].z - z);
   }
}
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
void tm_makeHgeo::initCache()
{
   if( cache != NULL) delete [] cache;
   cache = new MPointArray[ cacheFrames];
   cacheFramesNums.setLength( cacheFrames);
   for( int i = 0; i < cacheFrames; i++)
   {
      cacheFramesNums[i] = -1;
   }
}
MPointArray* tm_makeHgeo::getPoints( const int frame)
{
#ifdef _STDOUT_ON
printf("Trying to get points for %d frame.\n", frame);
#endif
   for( int i = 0; i < cacheFrames; i++)
      if( cacheFramesNums[i] == frame)
         return &cache[i];

   float difference = 0;
   int index = -1;
   for( int i = 0; i < cacheFrames; i++)
   {
      if( cacheFramesNums[i] == -1)
      {
         index = i;
         break;
      }
      float d = frameFloat - cacheFramesNums[i];
      if( d < 0) d = -d;
      if( d > difference)
      {
         difference = d;
         index = i;
      }
   }

   if( index == -1) return NULL;

   cacheFramesNums[index] = frame;

   createFileName( fileChar, fileBody, frame, padding);
   if( !readFileInBuffer( fileChar)) return NULL;
   if( !parseFileBuffer( cache[index], false ))
   {
      MGlobal::displayError("Parsing file failed. Export polygons only.");
      return false;
   }

#ifdef _STDOUT_ON
printf("Readed NEW points for position %d.\n", index);
#endif

   return &cache[index];
}
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/
// compute output node attribute:
/*-------------------------------------------------------------------------------------------------------------------*/
MStatus tm_makeHgeo::compute( const MPlug& plug, MDataBlock& data )
{
   MStatus stat;

   if (plug == outMesh_attrMObj)
   {
      if( !getNodeAttributes( plug, data )) return MS::kFailure;

      if( fileBody == "" ) return MS::kSuccess;

      if( frameFloat < 0)
      {
         MGlobal::displayError("Can't work with frames < 0.");
         return MS::kFailure;
      }

      if( cacheFrames != cacheFramesPrev)
      {
         initCache();
         cacheFramesPrev = cacheFrames;
      }

      if( fileBody != fileBodyPrev)
      {                          // if it is an another cache
         constructed = false;    // we need to force mesh reconstruct
         file_buf_len = 0;       // and file buffer memory reallocation
         initCache();
      }
      fileBodyPrev = fileBody;

      if ( recreate ) constructed = false;   // force mesh every frame reconstruction

      // check whether we need inferpolation and its direction

      int frameInt = (int)frameFloat;
      float subFrame = frameFloat - frameInt;
      bool interpolate = false;
      if( subFrame > MIN_SUBFRAME )
      {
         if( subFrame > (1-MIN_SUBFRAME) )
            frameInt++;
         else
            interpolate = true;
      }

      if( ! constructed )
      {
         createFileName( fileChar, fileBody, frameInt, padding);
         if( !readFileInBuffer( fileChar)) return MS::kFailure;
         if( !parseFileBuffer( vertexArrayPrev, false ))
         {
            MGlobal::displayError("Parsing file failed. Export polygons only.");
            return MS::kFailure;
         }
         if( !createMesh( vertexArrayPrev, plug, data ))
         {
            MGlobal::displayError("Creating mesh failed. Invalid number of vertices and faces. Export polygons only.");
            return MS::kFailure;
         }
         constructed = true;
      }

      if( recreate ) return MS::kSuccess;

      if( ! interpolate )
      {
         MPointArray *vertexArray = getPoints( frameInt);
         if( vertexArray == NULL)
         {
            MGlobal::displayError( "Error getting points.");
            return MS::kFailure;
         }
         if( !deformMesh( *vertexArray, plug, data )) return MS::kFailure;
         return MS::kSuccess;
      }

      MPointArray *vertexArray_A = getPoints( frameInt);
      MPointArray *vertexArray_B = getPoints( frameInt + 1);
      if(( vertexArray_A == NULL ) || ( vertexArray_B == NULL ))
      {
         MGlobal::displayError( "Error getting points.");
         return MS::kFailure;
      }

      linearStep( mPointArray, *vertexArray_A, *vertexArray_B, subFrame);

      if( !deformMesh( mPointArray, plug, data )) return MS::kFailure;


   }
   return MS::kSuccess;
}
//
//
/*-------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/
// node initialization:
/*-------------------------------------------------------------------------------------------------------------------*/
void* tm_makeHgeo::creator(){ return new tm_makeHgeo();}
MStatus tm_makeHgeo::initialize()
{
   MStatus stat;

   MFnTypedAttribute   typedFn;
   outMesh_attrMObj = typedFn.create( "outMesh", "out", MFnData::kMesh, &stat);
   if ( MS::kSuccess != stat )
   {
      stat.perror("ERROR creating tm_makeHgeo output attribute");
      return stat;
   }
   typedFn.setStorable( false);
   typedFn.setWritable( false);
   stat = addAttribute( outMesh_attrMObj);

   MFnNumericAttribute nAttr;
   MFnTypedAttribute tAttr;

   dummy_attrMObj = nAttr.create( "dummy", "dm",   MFnNumericData::kInt, 0, &stat);
   stat = nAttr.setCached( false);
   stat = nAttr.setStorable( false);
   stat = nAttr.setHidden( true);
   stat = addAttribute( dummy_attrMObj);

   fileName_attrMObj = tAttr.create( "fileName", "fn",  MFnData::kString, MObject::kNullObj, &stat);
   stat = tAttr.setCached( true);
   stat = tAttr.setStorable( true);
#ifndef MAYA_VERSION_6_0
   stat = nAttr.setChannelBox( true);
#endif
   stat = addAttribute( fileName_attrMObj);
   stat = attributeAffects( fileName_attrMObj, outMesh_attrMObj);

   frame_attrMObj = nAttr.create( "frame", "fr",   MFnNumericData::kFloat, 0, &stat);
   stat = nAttr.setCached( true);
   stat = nAttr.setStorable( true);
   stat = nAttr.setKeyable( true);
   stat = addAttribute( frame_attrMObj);
   stat = attributeAffects( frame_attrMObj, outMesh_attrMObj);

   padding_attrMObj = nAttr.create( "padding", "pd",   MFnNumericData::kInt, 4, &stat);
   stat = nAttr.setCached( true);
   stat = nAttr.setStorable( true);
#ifndef MAYA_VERSION_6_0
   stat = nAttr.setChannelBox( true);
#endif
   stat = nAttr.setMin( 1);
   stat = nAttr.setMax( 9);
   stat = addAttribute( padding_attrMObj);
   stat = attributeAffects( padding_attrMObj, outMesh_attrMObj);

   reconstruct_attrMObj = nAttr.create( "reconstruct", "rc",   MFnNumericData::kBoolean, 0, &stat);
   stat = nAttr.setCached( true);
   stat = nAttr.setStorable( true);
#ifndef MAYA_VERSION_6_0
   stat = nAttr.setChannelBox( true);
#endif
   stat = addAttribute( reconstruct_attrMObj);
   stat = attributeAffects( reconstruct_attrMObj, outMesh_attrMObj);

   cacheFrames_attrMObj = nAttr.create( "cacheFrames", "cf",   MFnNumericData::kInt, 4, &stat);
   stat = nAttr.setCached( true);
   stat = nAttr.setStorable( true);
#ifndef MAYA_VERSION_6_0
   stat = nAttr.setChannelBox( true);
#endif
   stat = nAttr.setMin( 2);
   stat = addAttribute( cacheFrames_attrMObj);

   reverse_attrMObj = nAttr.create( "reverse", "rv",   MFnNumericData::kBoolean, 1, &stat);
   stat = nAttr.setCached( true);
   stat = nAttr.setStorable( true);
#ifndef MAYA_VERSION_6_0
   stat = nAttr.setChannelBox( true);
#endif
   stat = addAttribute( reverse_attrMObj);
   stat = attributeAffects( reverse_attrMObj, outMesh_attrMObj);

   return MS::kSuccess;
}
/*-------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/
