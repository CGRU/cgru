#ifndef tm_polygon_selectring_h
#define tm_polygon_selectring_h

#include "../definitions.h"

#include <maya/MIntArray.h>

#include <maya/MObject.h>

#include <maya/MFnMesh.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>

class tm_polygon_selectring
{
public:
	
	tm_polygon_selectring();
	virtual ~tm_polygon_selectring();

	bool setMesh( MObject object);
	bool calculate( MIntArray &ringEdgesArray, const int selIndex, const int mode, const double angle_deg, const int maxCount);

private:

	MObject meshObject;
	bool objectIsSet;
	bool findNextEdge( int &edge, int &face, const int &mode, const double &angle);

	MFnMesh *pMesh;
	MItMeshVertex *pVtxIt;
	MItMeshEdge *pEdgeIt;
	MItMeshPolygon *pFaceIt;

};

#endif
