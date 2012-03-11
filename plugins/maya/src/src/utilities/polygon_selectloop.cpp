#include "polygon_selectloop.h"

#include <maya/MIOStream.h>
#include <maya/MStatus.h>
#include <maya/MGlobal.h>
#include <math.h>
#include <list>

tm_polygon_selectloop::tm_polygon_selectloop():
						objectIsSet(false)
{
}

tm_polygon_selectloop::~tm_polygon_selectloop()
{
}

bool tm_polygon_selectloop::setMesh( MObject object)
{
	if(!(object.hasFn( MFn::kMesh)))
	{
		MGlobal::displayError("tm_polygon_selectloop::setMesh - Object is not a mesh.");
		return false;
	}
	meshObject = object;
	objectIsSet = true;
	return true;
}

bool tm_polygon_selectloop::calculate( MIntArray &edgesArray, const int &selIndex, const int &mode, const double &angle_deg, const int &maxCount)
{
	if(!objectIsSet)
	{
		MGlobal::displayError("tm_polygon_selectloop::calculate - Object is not set.");
		return false;
	}

	double angle = angle_deg*M_PI/180;
	MStatus stat;
	pMesh = new MFnMesh( meshObject, &stat);				if (!stat) return false;
	int meshEdgesCount = pMesh->numEdges( &stat);		if (!stat) return false;
	int *pVisitedEdges = new int[meshEdgesCount];
	for( int i = 0; i < meshEdgesCount; i++) pVisitedEdges[i] = 0;

	pVtxIt = new MItMeshVertex( meshObject, &stat);			if (!stat) return false;
	pEdgeIt = new MItMeshEdge( meshObject, &stat);			if (!stat) return false;
	pFaceIt = new MItMeshPolygon( meshObject, &stat);		if (!stat) return false;

	int edgeVtx[2];
	int edge, vtx;
	stat = pMesh->getEdgeVertices( selIndex, edgeVtx);	if (!stat) return false;
	std::list <int> edgesList;
	edgesList.push_front( selIndex);

	int count = 1;
	edge = selIndex;
	pVisitedEdges[selIndex] = 1;
	vtx = edgeVtx[0];
	while( findNextEdge( edge, vtx, mode, angle))
	{
		if (pVisitedEdges[edge]) break;
		if ((++count) > maxCount) break;
		edgesList.push_front( edge);
		pVisitedEdges[edge] = 1;
	}
	edge = selIndex;
	vtx = edgeVtx[1];
	while( findNextEdge( edge, vtx, mode, angle))
	{
		if (pVisitedEdges[edge]) break;
		if ((++count) > maxCount) break;
		edgesList.push_back( edge);
		pVisitedEdges[edge] = 1;
	}

	int len = (int)edgesList.size();
	edgesArray.setLength( len);
	for( int i = 0; i < len; i++)
	{
		edgesArray[i] = *edgesList.begin();
		edgesList.pop_front();
	}

	if( pMesh != NULL) delete pMesh;
	if( pVtxIt != NULL) delete pVtxIt;
	if( pEdgeIt != NULL) delete pEdgeIt;
	if( pFaceIt != NULL) delete pFaceIt;
	if( pVisitedEdges != NULL) delete [] pVisitedEdges;

	return true;
}

bool tm_polygon_selectloop::findNextEdge( int &edge, int &vtx, const int &mode, double &angle)
{
	bool result = false;
	int prevIndex;
	int edgeVtx[2];

	MIntArray conEdgesArray;
	pVtxIt->setIndex( vtx, prevIndex);
	pVtxIt->getConnectedEdges( conEdgesArray);
	unsigned int numConEdges = conEdgesArray.length();
	int *pEdgeFlags = new int[numConEdges];
	for( unsigned i = 0; i < numConEdges; i++) pEdgeFlags[i] = 0;

	MIntArray conFacesArray;
	pEdgeIt->setIndex( edge, prevIndex);
	pEdgeIt->getConnectedFaces( conFacesArray);
	unsigned int numConFaces = conFacesArray.length();

//################################################################################################
	if( mode != 2)											// setting topology flags
	{
		for( unsigned f = 0; f < numConFaces; f++)
		{
			pFaceIt->setIndex( conFacesArray[f], prevIndex);
			MIntArray curFaceEdgesArray;
			pFaceIt->getEdges( curFaceEdgesArray);
			unsigned int numCurFaceEdges = curFaceEdgesArray.length();
			for( unsigned e = 0; e < numConEdges; e++)
			{
				for( unsigned ce = 0; ce < numCurFaceEdges; ce++)
				{
						if (conEdgesArray[e] == curFaceEdgesArray[ce])
							pEdgeFlags[e] = -1;
				}
			}
		}
	}
//################################################################################################
	if( (mode == 1) || (mode == 3))							// finding one topology edge
	{
		int bestEdge = -1;
		for( unsigned e = 0; e < numConEdges; e++)
		{
			if( pEdgeFlags[e] != -1)
			{
				if ( result == false)
				{
					bestEdge = conEdgesArray[e];
					result = true;
				}
				else
				{
					result = false;
					break;
				}
			}
		}
		if( (bestEdge != -1) && (result == true)) edge = bestEdge;
	}
//################################################################################################
	if( ((mode == 3) && (result == false)) || (mode == 2))	// finding the minimum angle edge
	{
		int bestEdge = -1;
		double minAngle = M_PI;
		pMesh->getEdgeVertices( edge, edgeVtx);
		MPoint pointA, pointB;
		if( edgeVtx[0] == vtx)
		{
			pMesh->getPoint( edgeVtx[1], pointA);
			pMesh->getPoint( edgeVtx[0], pointB);
		}
		else
		{
			pMesh->getPoint( edgeVtx[0], pointA);
			pMesh->getPoint( edgeVtx[1], pointB);
		}
		double x0 = pointB.x - pointA.x;
		double y0 = pointB.y - pointA.y;
		double z0 = pointB.z - pointA.z;
		double d0 = sqrt(x0*x0 + y0*y0 + z0*z0);				if( d0 == 0) d0 = 1e99;
		for( unsigned e = 0; e < numConEdges; e++)
		{
			pMesh->getEdgeVertices( conEdgesArray[e], edgeVtx);
			if( edgeVtx[0] == vtx)
			{
				pMesh->getPoint( edgeVtx[0], pointA);
				pMesh->getPoint( edgeVtx[1], pointB);
			}
			else
			{
				pMesh->getPoint( edgeVtx[1], pointA);
				pMesh->getPoint( edgeVtx[0], pointB);
			}
			double x1 = pointB.x - pointA.x;
			double y1 = pointB.y - pointA.y;
			double z1 = pointB.z - pointA.z;
			double d1 = sqrt(x1*x1 + y1*y1 + z1*z1);			if( d0 == 0) continue;
			double curAngle = acos(((x0*x1) + (y0*y1) + (z0*z1)) / (d0*d1));
			if((curAngle < angle) && (curAngle < minAngle))
			{
				if ( curAngle < minAngle)
				{
					minAngle = curAngle;
					bestEdge = e;
				}
			}
		}
		if( bestEdge != -1)
		{
			result = true;
			edge = conEdgesArray[bestEdge];
		}
	}
//################################################################################################
	if( result)										// setting the next vertex
	{
		pMesh->getEdgeVertices( edge, edgeVtx);
		if(edgeVtx[0] == vtx)
			vtx = edgeVtx[1];
		else
			vtx = edgeVtx[0];
	}
//################################################################################################
	delete [] pEdgeFlags;
	return result;
}
