#include "polygon_selectring.h"

#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MStatus.h>

#include <maya/MPoint.h>

#include <math.h>
#define M_PId2 1.5707963267948966192313216916398
#include <list>

tm_polygon_selectring::tm_polygon_selectring():
						objectIsSet(false)
{
}

tm_polygon_selectring::~tm_polygon_selectring()
{
}

bool tm_polygon_selectring::setMesh( MObject object)
{
	if(!(object.hasFn( MFn::kMesh)))
	{
		MGlobal::displayError("tm_polygon_selectring::setMesh - Object is not a mesh.");
		return false;
	}
	meshObject = object;
	objectIsSet = true;
	return true;
}

bool tm_polygon_selectring::calculate( MIntArray &ringEdgesArray, const int selIndex, const int mode, const double angle_deg, const int maxCount)
{

	if(!objectIsSet)
	{
		MGlobal::displayError("tm_polygon_selectring::calculate - Object is not set.");
		return false;
	}

	double angle = angle_deg*M_PI/180;
	MStatus stat;
	pMesh = new MFnMesh( meshObject, &stat);				if (!stat) return false;
	int meshEdgesCount = pMesh->numEdges( &stat);			if (!stat) return false;
	int *pVisitedEdges = new int[meshEdgesCount];
	for( int i = 0; i < meshEdgesCount; i++) pVisitedEdges[i] = 0;

	pVtxIt = new MItMeshVertex( meshObject, &stat);			if (!stat) return false;
	pEdgeIt = new MItMeshEdge( meshObject, &stat);			if (!stat) return false;
	pFaceIt = new MItMeshPolygon( meshObject, &stat);		if (!stat) return false;
	std::list <int> edgesList;
	edgesList.push_front( selIndex);

	MIntArray facesArray;
	int edge, face, p;
	int count = 1;
	edge = selIndex;
	stat = pEdgeIt->setIndex( edge, p);
	if (!stat)
	{
		MGlobal::displayError("tm_polygon_selectring::calculate - can't find selected edge.");
		return false;
	}
	int firstNumFaces = pEdgeIt->getConnectedFaces( facesArray, &stat);
														if (!stat) return false;
														if (firstNumFaces < 1) return false;
	pVisitedEdges[selIndex] = 1;
	face = facesArray[0];
	while( findNextEdge( edge, face, mode, angle))
	{
		if (pVisitedEdges[edge]) break;
		if ((++count) > maxCount) break;
		edgesList.push_front( edge);
		pVisitedEdges[edge] = 1;
	}
	if( firstNumFaces > 1)
	{
		edge = selIndex;
		face = facesArray[1];
		while( findNextEdge( edge, face, mode, angle))
		{
			if (pVisitedEdges[edge]) break;
			if ((++count) > maxCount) break;
			edgesList.push_back( edge);
			pVisitedEdges[edge] = 1;
		}
	}

	int len = (int)edgesList.size();
	ringEdgesArray.setLength( len);
	for( int i = 0; i < len; i++)
	{
		ringEdgesArray[i] = *edgesList.begin();
		edgesList.pop_front();
	}

	if( pMesh != NULL) delete pMesh;
	if( pVtxIt != NULL) delete pVtxIt;
	if( pEdgeIt != NULL) delete pEdgeIt;
	if( pFaceIt != NULL) delete pFaceIt;
	if( pVisitedEdges != NULL) delete [] pVisitedEdges;
	return true;
}

bool tm_polygon_selectring::findNextEdge( int &edge, int &face, const int &mode, const double &angle)
{
	bool result = false;
	int p;
	int edgeVtx[2];

	MIntArray faceEdgesArray;
	pFaceIt->setIndex( face, p);
	pFaceIt->getEdges( faceEdgesArray);
	unsigned int numFaceEdges = faceEdgesArray.length();
	int *pEdgeFlags = new int[numFaceEdges];
	for( unsigned i = 0; i < numFaceEdges; i++) pEdgeFlags[i] = 0;
	pMesh->getEdgeVertices( edge, edgeVtx);
//################################################################################################
	if( mode != 2)											// setting topology flags
	{
		int faceEdgeVtx[2];
		for( unsigned e = 0; e < numFaceEdges; e++)
		{
			pMesh->getEdgeVertices( faceEdgesArray[e], faceEdgeVtx);
			if(		(faceEdgeVtx[0] == edgeVtx[0]) ||
					(faceEdgeVtx[0] == edgeVtx[1]) ||
					(faceEdgeVtx[1] == edgeVtx[0]) ||
					(faceEdgeVtx[1] == edgeVtx[1]))
				pEdgeFlags[e] = -1;
		}
	}

//################################################################################################
	if( (mode == 1) || (mode == 3))							// finding one topology edge
	{
		int bestEdge = -1;
		for( unsigned e = 0; e < numFaceEdges; e++)
		{
			if( pEdgeFlags[e] != -1)
			{
				if ( result == false)
				{
					bestEdge = faceEdgesArray[e];
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
		MPoint pointA, pointB;
		pMesh->getPoint( edgeVtx[0], pointA);
		pMesh->getPoint( edgeVtx[1], pointB);
		double x0 = pointB.x - pointA.x;
		double y0 = pointB.y - pointA.y;
		double z0 = pointB.z - pointA.z;
		double d0 = sqrt(x0*x0 + y0*y0 + z0*z0);				if( d0 == 0) d0 = 1e99;
		for( unsigned e = 0; e < numFaceEdges; e++)
		{
			if( faceEdgesArray[e] == edge) continue;
			pMesh->getEdgeVertices( faceEdgesArray[e], edgeVtx);
			pMesh->getPoint( edgeVtx[0], pointA);
			pMesh->getPoint( edgeVtx[1], pointB);
			double x1 = pointB.x - pointA.x;
			double y1 = pointB.y - pointA.y;
			double z1 = pointB.z - pointA.z;
			double d1 = sqrt(x1*x1 + y1*y1 + z1*z1);			if( d0 == 0) continue;
			double curAngle = acos(((x0*x1) + (y0*y1) + (z0*z1)) / (d0*d1));
			if( curAngle < 0) curAngle = -curAngle;
			if( curAngle > M_PId2) curAngle = M_PI - curAngle;
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
			edge = faceEdgesArray[bestEdge];
		}
	}
//################################################################################################
	if( result)										// setting the next vertex
	{
		MIntArray conFacesArray;
		pEdgeIt->setIndex( edge, p);
		int numConFaces = pEdgeIt->getConnectedFaces( conFacesArray);
		if( numConFaces > 1)
		{
			if( conFacesArray[0] == face)
				face = conFacesArray[1];
			else
				face = conFacesArray[0];
		}
	}
//################################################################################################
	delete [] pEdgeFlags;
	return result;
}
