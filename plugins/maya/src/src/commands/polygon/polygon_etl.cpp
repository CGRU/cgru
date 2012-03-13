#include "polygon.h"

bool tm_polygon::edgesToLoop_Func( MIntArray &edgesArray, MSelectionList &selectionList)
{
	MStatus status;
	MObject meshObj;
	status = selectionList.getDependNode( 0, meshObj);
	if(!status){MGlobal::displayError("###***   Can't find object !");return false;}
	MFnMesh meshFn( meshObj, &status);
	if(!status){MGlobal::displayError("###***   Non mesh object founded !");return false;}

	MDagPath meshDagPath_first, meshDagPath;
	selectionList.getDagPath( 0, meshDagPath_first);
	MObject multiEdgeComponent;
	MString infoMStr = "###***   input edges: ";
	MIntArray inputEdgesArray;
	inputEdgesArray.clear();
	for (MItSelectionList edgeComponentIter(selectionList, MFn::kMeshEdgeComponent); !edgeComponentIter.isDone(); edgeComponentIter.next())
	{
		edgeComponentIter.getDagPath(meshDagPath, multiEdgeComponent);
		if(!(meshDagPath_first == meshDagPath))
		{
			MGlobal::displayError("###***   Different meshes edges founded !");
			return false;
		}
		if (!multiEdgeComponent.isNull())
		{
			for (MItMeshEdge edgeIter(meshDagPath, multiEdgeComponent); !edgeIter.isDone(); edgeIter.next())
			{
				int edgeIndex = edgeIter.index();
				infoMStr += edgeIndex;
				inputEdgesArray.append( edgeIndex);
				infoMStr += " ";
			}
		}
	}
	MGlobal::displayInfo( infoMStr);

	unsigned numInputEdges = inputEdgesArray.length();
	int *visitedEdges = new int[numInputEdges];
	for( unsigned e = 0; e < numInputEdges; e++) visitedEdges[e] = 0;
	std::list <int> loopEdgesList;
	int vertices[2];
	int nextVtxs[2];

	int edgeIndex = inputEdgesArray[0];
	loopEdgesList.push_back( inputEdgesArray[0]);
	visitedEdges[0] = 1;
	meshFn.getEdgeVertices( edgeIndex, vertices);
	for( unsigned vertex = 0; vertex < 2; vertex++)
	{
		edgeIndex = inputEdgesArray[0];
		int vtx = vertices[vertex];
		unsigned COUNTER = 0;
		while( COUNTER < 100)
		{
		COUNTER++;
			bool founded = false;
			for( unsigned e = 0; e < numInputEdges; e++)
			{
				if( inputEdgesArray[e] == edgeIndex) continue;
				if( visitedEdges[e] == 1) continue;
				meshFn.getEdgeVertices( inputEdgesArray[e], nextVtxs);
				if( nextVtxs[0] == vtx)
				{
					founded = true;
					vtx = nextVtxs[1];
				} else
				if( nextVtxs[1] == vtx)
				{
					founded = true;
					vtx = nextVtxs[0];
				}
				if( !founded) continue;
				edgeIndex = inputEdgesArray[e];
				visitedEdges[e] = 1;
				if( vertex == 0)
					loopEdgesList.push_front( edgeIndex);
				else
					loopEdgesList.push_back( edgeIndex);
				if( founded) break;
			}
			if(!founded) break;
		}
	}
	if (visitedEdges != NULL) delete [] visitedEdges;

	unsigned numLoopEdges = (unsigned)loopEdgesList.size();
	edgesArray.setLength( numLoopEdges);
	for( unsigned e = 0; e < numLoopEdges; e++)
	{
		edgesArray[e] = *loopEdgesList.begin();
		loopEdgesList.pop_front();
	}
	return true;
}
