#include "polygon.h"

#include "../../utilities/polygon_edgestoring.h"
bool tm_polygon::edgesToRing_Func( MIntArray &edgesArray, MSelectionList &selectionList)
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
				edgesArray.append( edgeIndex);
				infoMStr += " ";
			}
		}
	}
	MGlobal::displayInfo( infoMStr);

	tm_polygon_edgestoring edgestoring_tool;
	MObject object;
	if(edgestoring_tool.setMesh( meshObj))
		return edgestoring_tool.calculate( edgesArray);
	else
		return false;
}
