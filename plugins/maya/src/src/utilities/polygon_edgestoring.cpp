#include "polygon_edgestoring.h"

#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MStatus.h>

#include <maya/MDagPath.h>

#include <maya/MFnMesh.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>

#include <list>

tm_polygon_edgestoring::tm_polygon_edgestoring():
						objectIsSet(false)
{
}

tm_polygon_edgestoring::~tm_polygon_edgestoring()
{
}

bool tm_polygon_edgestoring::setMesh( MObject object)
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

bool tm_polygon_edgestoring::calculate( MIntArray &edgesArray)
{

	if(!objectIsSet)
	{
		MGlobal::displayError("tm_polygon_edgestoring::calculate - Object is not set.");
		return false;
	}

	MFnMesh meshFn( meshObject);
	MItMeshEdge edgeIt(meshObject);
	MItMeshPolygon faceIt(meshObject);
	unsigned numInputEdges = edgesArray.length();
	int *visitedEdges = new int[numInputEdges];
	for( unsigned e = 0; e < numInputEdges; e++) visitedEdges[e] = 0;
	std::list <int> ringEdgesList;
	int prevIndex;
	MIntArray faces;
	MIntArray edgeFaces;
	MIntArray faceEdges;

	int edgeIndex = edgesArray[0];
	ringEdgesList.push_back( edgeIndex);
	visitedEdges[0] = 1;
	edgeIt.setIndex( edgeIndex, prevIndex);
	edgeIt.getConnectedFaces( faces);
	unsigned numFaces = faces.length();
	unsigned numFaceEdges;
	for( unsigned face = 0; face < numFaces; face++)
	{
		edgeIndex = edgesArray[0];
		int lastFace, newFace;
		if( face == 1)	lastFace = faces[0];
		else			lastFace = -1;

		unsigned COUNTER = 0;
		while( COUNTER < 32000)
		{
		COUNTER++;
			edgeIt.setIndex( edgeIndex, prevIndex);
			edgeIt.getConnectedFaces( edgeFaces);
			if(edgeFaces.length() > 1)
			{
				if( edgeFaces[0] == lastFace)
					newFace = edgeFaces[1];
				else
					newFace = edgeFaces[0];
			}
			else
				newFace = edgeFaces[0];
			faceIt.setIndex( newFace, prevIndex);

			lastFace = newFace;

			bool founded = false;
			for( unsigned e = 0; e < numInputEdges; e++)
			{
				if( edgesArray[e] == edgeIndex) continue;
				if( visitedEdges[e] == 1) continue;
				faceIt.getEdges( faceEdges);
				numFaceEdges = faceEdges.length();
				for( unsigned fe = 0; fe < numFaceEdges; fe++)
				{
					if( faceEdges[fe] == edgesArray[e])
					{
						founded = true;
						edgeIndex = edgesArray[e];
						visitedEdges[e] = 1;
						if( face == 0)
							ringEdgesList.push_front( edgeIndex);
						else
							ringEdgesList.push_back( edgeIndex);
					}
					if( founded) break;
				}
				if( founded) break;
			}
			if(!founded) break;
		}
	}
	if (visitedEdges != NULL) delete [] visitedEdges;

	unsigned numRingEdges = (unsigned)ringEdgesList.size();
	edgesArray.setLength( numRingEdges);
	for( unsigned e = 0; e < numRingEdges; e++)
	{
		edgesArray[e] = *ringEdgesList.begin();
		ringEdgesList.pop_front();
	}
	return true;
}
