#pragma once

#include "../definitions.h"

#include <maya/MArrayDataBuilder.h>
#include <maya/MPxNode.h> 
#include <maya/MString.h> 

#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MEulerRotation.h>
#include <maya/MMatrix.h>

#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>

#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MItMeshVertex.h>

#include <math.h>
#include <maya/MIOStream.h>

class tm_makeGeoSphere : public MPxNode
{
	public:

		tm_makeGeoSphere();
		virtual ~tm_makeGeoSphere(); 
		virtual MStatus   		compute( const MPlug& plug, MDataBlock& data );
		static  void *          creator();
		static  MStatus         initialize();

	public: 

		static	MTypeId		id;

		static	MObject primitive;
		static	MObject radius;
		static	MObject frequency;

		static  MObject outMesh;

	private:

		static int AnzahlPunkte(int Freq);
		static MPoint getCenterPoint( MPoint& pointA, MPoint& pointB);
		static int getIndex( int w, int h, int freq);
		static void getIndexWH( int& w, int& h, int index, int freq);
		static void getUpperIndexesWH( int w, int h, int& wa, int& ha, int& wb, int& hb);
		static void getUpperIndexes( int index, int freq, int& w, int& h);
		void rotMPointArray( MPointArray& rotFrom, MPointArray& rotTo, MEulerRotation& rotEuler);

		int segments;
		int segFaceCount;
		int segVtxCount;

        struct VertexStruct
		{
			double x;
			double y;
			double z;
			int pConCount;
			int pConArray[10];
			bool del;
			int rep;
			int newIndex;
		};
};
