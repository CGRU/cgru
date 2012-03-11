#ifndef polygon_selectloop_h
#define polygon_selectloop_h

#include "../definitions.h"

#include <maya/MObject.h>
#include <maya/MIntArray.h>

#include <maya/MFnMesh.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>

class tm_polygon_selectloop
{
public:
					tm_polygon_selectloop();
	virtual			~tm_polygon_selectloop();

	bool setMesh( MObject object);
	bool calculate( MIntArray &edgesArray, const int &selIndex, const int &mode, const double &angle_deg, const int &maxCount);

private:

	MObject meshObject;
	bool objectIsSet;

	MFnMesh *pMesh;
	MItMeshVertex *pVtxIt;
	MItMeshEdge *pEdgeIt;
	MItMeshPolygon *pFaceIt;

	bool findNextEdge( int &edge, int &vtx, const int &mode, double &angle);

};
#endif
