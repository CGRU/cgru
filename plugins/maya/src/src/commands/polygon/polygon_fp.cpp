#include "polygon.h"

bool tm_polygon::findPath_Func( MSelectionList &selectionList, int vtxIndex_a, int vtxIndex_b, MIntArray &returnArray, double koeff_a)
{
//printf("\n##########################################################\n");
	pObject = NULL;
	pMesh = NULL;
	pPointArray = NULL;
	pVtxFlags = NULL;

	bool result = false;
	MStatus stat;
	pObject = new MObject;
	stat = selectionList.getDependNode( 0, *pObject);
	if(!stat)
	{
      MGlobal::displayError("***### tm_polygon: Can't find object.");
		return false;
	}
	pMesh = new MFnMesh( *pObject, &stat);
	if(!stat)
	{
      MGlobal::displayError("***### tm_polygon: Can't find mesh.");
		return false;
	}

	int numVertices = pMesh->numVertices();
	pVtxFlags = new int[numVertices];
	for( int i = 0; i < numVertices; i++) pVtxFlags[i] = 1;

	MItMeshVertex vtxIt( *pObject);
	MIntArray vtxArray;
	pPointArray = new MPointArray();
	pMesh->getPoints( *pPointArray);

	int index;
	std::list <int> vtxList;
	int vtx = vtxIndex_a;
	pVtxFlags[vtx] = 0;
	vtxList.push_front( vtx);

	unsigned long counter = 0;
	while(( result == false)&&(counter++ < 1000))
	{
//printf("*vervex #%d :", vtx);
		stat = vtxIt.setIndex( vtx, index);
		stat = vtxIt.getConnectedVertices( vtxArray);
		if( !stat)
		{
			result = false;
			break;
		}
for( unsigned int i = 0; i < vtxArray.length(); i++) printf(" %d,", vtxArray[i]);
		vtx = findPath_findBestVtx( vtx, vtxArray, (*pPointArray)[vtxIndex_b], koeff_a);
//printf(" - %d\n", vtx);
		if( vtx < 0)
		{
			if(vtxList.empty())
			{
				result = false;
				break;
			}
			vtxList.pop_front();
			vtx = *vtxList.begin();
		}
		else
		{
			vtxList.push_front( vtx);
			pVtxFlags[vtx] = 0;
			if(vtx == vtxIndex_b) result = true;
		}
	}
	int numVtx = (int)vtxList.size();
	if(result && (numVtx > 1))
	{
		returnArray.setLength( numVtx - 1);
		MIntArray edgeArray;
		int twoVertices[2];
		int vtx_a, vtx_b;
		int edgeCount = 0;
		vtx_a = *vtxList.begin();
		vtxList.pop_front();
		while( !vtxList.empty())
		{
			vtxIt.setIndex( vtx_a, index);
			vtxIt.getConnectedEdges( edgeArray);
			int numEdges = edgeArray.length();
			vtx_b = *vtxList.begin();
			for( int i = 0; i < numEdges; i++)
			{
				pMesh->getEdgeVertices( edgeArray[i], twoVertices);
				if((twoVertices[0] == vtx_b) || (twoVertices[1] == vtx_b))
				{
					returnArray[edgeCount] = edgeArray[i];
					continue;
				}
			}
			vtx_a = vtx_b;
			vtxList.pop_front();
			edgeCount++;
		}
	}
	else
	{
		returnArray.setLength( 1);
		returnArray[0] = -1;
	}

	if( pObject != NULL) delete pObject;
	if( pMesh != NULL) delete pMesh;
	if( pPointArray != NULL) delete pPointArray;
	if( pVtxFlags != NULL) delete pVtxFlags;

	return result;
}

int tm_polygon::findPath_findBestVtx( int startId, MIntArray &vtxArray, MPoint &aimPoint, double koeff_a)
{
	int result = -1;
	double distance = 1e100;

	unsigned int vtxCount = vtxArray.length();
	for( unsigned int i = 0; i < vtxCount; i++)
	{
		if(pVtxFlags[vtxArray[i]])
		{
			double x = (*pPointArray)[vtxArray[i]].x;
			double y = (*pPointArray)[vtxArray[i]].y;
			double z = (*pPointArray)[vtxArray[i]].z;

			double dX = aimPoint.x - x;
			double dY = aimPoint.y - y;
			double dZ = aimPoint.z - z;
			double dist =dX*dX + dY*dY + dZ*dZ;

			if( dist < distance)
			{
				result = vtxArray[i];
				distance = dist;
			}
		}
	}
	return result;
}
