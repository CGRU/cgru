
//
#include "polySplitFty.h"
#include "../../../utilities/utilities.h"

// General Includes
//
#include "math.h"
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MIOStream.h>
#include <maya/MFloatPointArray.h>

// Function Sets
//
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>

// Iterators
//
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>

MStatus tm_polySplitFty::doIt()
//
//	Description:
//		Performs the actual tm_polySplit operation on the given object and UVs
//
{
	MStatus status = MS::kSuccess;

	MVector vector;
	MPoint point;
	MFnMesh meshFn( fMesh);
	int edgeVertices[2];
	MItMeshEdge edgeIt( fMesh);
	int prevIndex;

	meshFn.getEdgeVertices( fSelEdges[0], edgeVertices);
	meshFn.getPoint( edgeVertices[0], point);
	meshFn.getPoint( edgeVertices[1], averagePos);
	averagePos = (point + averagePos) * 0.5;

#ifdef _DEBUG
cout << endl << "##########################tm_polySplitFty::doIt" << endl;
cout<<"loop="<<fa_loop<<" loopMode="<<fa_loop_mode<<" loop_angle="<<fa_loop_angle<<" loop_maxcount="<<fa_maxcount<<endl;
#endif
	//getting valid edges
	{
		if( fa_loop)
		{
			if(!getLoopFromFirst( fa_loop_mode, fa_loop_angle, fa_maxcount))
			{
				MGlobal::displayError( "tm_polySplit command failed: Bad edges selected." );
				return MStatus::kFailure;
			}
		}
		else
		{
			if(!getRing())
			{
				MGlobal::displayError( "tm_polySplit command failed: Bad edges selected." );
				return MStatus::kFailure;
			}
		}
		{// close edges:
		MIntArray conFacesA;
		edgeIt.setIndex( fSelEdges[0], prevIndex);
		edgeIt.getConnectedFaces( conFacesA);
		if( conFacesA.length() > 1)
		{
			MIntArray conFacesB;
			edgeIt.setIndex( fSelEdges[fSelEdges.length() - 1], prevIndex);
			edgeIt.getConnectedFaces( conFacesB);
			if( conFacesB.length() > 1)
			{
				if( conFacesA[0] == conFacesB[0]) fSelEdges.append( fSelEdges[0]);
				else if( conFacesA[0] == conFacesB[1]) fSelEdges.append( fSelEdges[0]);
				else if( conFacesA[1] == conFacesB[0]) fSelEdges.append( fSelEdges[0]);
				else if( conFacesA[1] == conFacesB[1]) fSelEdges.append( fSelEdges[0]);
			}
		}
		}
	}

#ifdef _DEBUG
cout << endl << "fSelEdges = ";for( unsigned i=0;i<fSelEdges.length();i++) cout << fSelEdges[i] << " ";cout << endl;
#endif

	edgesCount = fSelEdges.length();
	if(edgesCount < 2)
	{
		MGlobal::displayError( "tm_polySplit command failed: Can't find more than one ring edge." );
		return MStatus::kFailure;
	}

	MItMeshVertex vtxIt( fMesh);
	MItMeshPolygon faceIt( fMesh);

	MIntArray conFaces;
	MIntArray conEdges;
	MIntArray faceEdges;

	unsigned numConFaces;

	if( firstTime)
	{
	splitedPoints_start_N.setLength( edgesCount);
	splitedPoints_dir_N.setLength( edgesCount);
	splitedPoints_ndir_N.setLength( edgesCount);
	splitedPoints_start_R.setLength( edgesCount);
	splitedPoints_dir_R.setLength( edgesCount);
	splitedPoints_ndir_R.setLength( edgesCount);

	oldVtxCount = meshFn.numVertices();
	oldUVsCount = meshFn.numUVs();

#ifdef _DEBUG
cout << endl << "oldVtxCount = " << oldVtxCount << endl;
cout << endl << "oldUVsCount = " << oldUVsCount << endl;
#endif

//######################################## finding inverted edges
	invEdge.setLength(edgesCount);
#ifdef _DEBUG
cout << "### finding inverted edges:" << endl;
#endif
	for( unsigned i = 0; i < edgesCount; i++) invEdge[i] = 0;
	int zero_vtx_index = 0;
	edgeIt.setIndex( fSelEdges[0], prevIndex);
	edgeIt.getConnectedFaces( conFaces);
	numConFaces = conFaces.length();
	int nextEdgeId = -1;
	int nextFaceId = -1;
	for( unsigned cf = 0; cf < numConFaces; cf++)
	{
		faceIt.setIndex( conFaces[cf], prevIndex);
		faceIt.getEdges( faceEdges);
		unsigned numFaceEdges = faceEdges.length();
		for( unsigned fe = 0; fe < numFaceEdges; fe++)
		{
			if( faceEdges[fe] == fSelEdges[1])
			{
				nextFaceId = conFaces[cf];
				break;
			}
		}
		if( nextEdgeId != -1) break;
	}
	if( nextFaceId == -1)
	{
#ifdef _DEBUG
cout << "nextFaceId == -1 (edgeId[" << fSelEdges[0] << "]);" << endl;
#endif
		return MStatus::kFailure;
	}
	meshFn.getEdgeVertices( fSelEdges[0], edgeVertices);
	int vtxIndex = edgeVertices[zero_vtx_index];
	unsigned e = 1;
	bool founded = true;
	int nextEdgeVertices[2];
	int COUNTER = 0;
	while( e < edgesCount)
	{
		COUNTER++;
		if(COUNTER > 32000)
		{
#ifdef _DEBUG
cout << "(COUNTER > 32000) on finding inverted edges!" << endl;
#endif
			break;
		}
		meshFn.getEdgeVertices( fSelEdges[e], nextEdgeVertices);
		vtxIt.setIndex( vtxIndex, prevIndex);
		vtxIt.getConnectedEdges( conEdges);
		unsigned numConEdges = conEdges.length();
		faceIt.setIndex( nextFaceId, prevIndex);
		faceIt.getEdges( faceEdges);
		unsigned numFaceEdges = faceEdges.length();
#ifdef _DEBUG
cout << COUNTER << ") edgeId = " << fSelEdges[e-1] << ", numConEdges = " << numConEdges;
cout << ", vtxIndex = " << vtxIndex << ", nextFaceId = " << nextFaceId << ":" << endl;
#endif
		bool nextEdgeId_founded = false;
		for( unsigned ce = 0; ce < numConEdges; ce++)
		{
			if((conEdges[ce] == fSelEdges[e-1]) || (conEdges[ce] == nextEdgeId)) continue;
			for( unsigned fe = 0; fe < numFaceEdges; fe++)
			{
				if( conEdges[ce] == faceEdges[fe])
				{
					nextEdgeId = conEdges[ce];
					nextEdgeId_founded = true;
					break;
				}
			}
			if( nextEdgeId_founded) break;
		}
if(!nextEdgeId_founded)
{
#ifdef _DEBUG
cout << "nextEdgeId was not founded, edge " <<  fSelEdges[e-1] << endl;
cout << "connected edges: ";
for( unsigned ce = 0; ce < numConEdges; ce++) cout << conEdges[ce] << ", "; cout << endl;
cout << "connected face edges: ";
for( unsigned fe = 0; fe < numFaceEdges; fe++) cout << faceEdges[fe] << ", "; cout << endl;
#endif
break;
}
		int cEdgeVertices[2];
		meshFn.getEdgeVertices( nextEdgeId, cEdgeVertices);
		int cEdge_oppVtx;
		int searchVtxIndex = 1;
		if( nextEdgeId == fSelEdges[e]) searchVtxIndex = 0;
		if( cEdgeVertices[0] == vtxIndex) cEdge_oppVtx = cEdgeVertices[1];
		else cEdge_oppVtx = cEdgeVertices[0];
#ifdef _DEBUG
cout << "nextEdgeId = " << nextEdgeId << ", cEdge_oppVtx = " << cEdge_oppVtx << endl;
#endif
		founded = false;
		if(cEdge_oppVtx == nextEdgeVertices[searchVtxIndex])
		{
			invEdge[e] = 1;
			zero_vtx_index = 1;
			founded = true;
		}
		if(cEdge_oppVtx == nextEdgeVertices[1-searchVtxIndex])
		{
			zero_vtx_index = 0;
			founded = true;
		}
		if(!founded)
		{
			vtxIndex = cEdge_oppVtx;
			continue;
		}
		if(e == (edgesCount-1)) break;
		edgeIt.setIndex( fSelEdges[e], prevIndex);
		edgeIt.getConnectedFaces( conFaces);
		numConFaces = conFaces.length();
		if( numConFaces < 2)
		{
#ifdef _DEBUG
cout << "numConFaces < 2 (edgeId[" << fSelEdges[e] << "]);" << endl;
#endif
			break;
		}
		if( conFaces[0] == nextFaceId) nextFaceId = conFaces[1];
		else nextFaceId = conFaces[0];
		vtxIndex = nextEdgeVertices[zero_vtx_index];
		e++;
#ifdef _DEBUG
cout << "founded, conFaces = " << conFaces[0] << ", " << conFaces[1] << " => " << nextFaceId << endl;
#endif
	}
	}
//######################################## getting edges vertices UVs information
	MFloatArray edgeUVs_u( edgesCount*4, -1.0f);
	MFloatArray edgeUVs_v( edgesCount*4, -1.0f);
	MIntArray edge_conFacesIds( edgesCount*2, -1);
	MIntArray edge_conFacesNum( edgesCount, -1);
	for( unsigned e = 0; e < edgesCount; e++)
	{
//#ifdef _DEBUG
//cout << "edgeId #" << fSelEdges[e] << ":" << endl;
//#endif
		meshFn.getEdgeVertices( fSelEdges[e], edgeVertices);
		edgeIt.setIndex( fSelEdges[e], prevIndex);
		edgeIt.getConnectedFaces( conFaces);
		numConFaces = conFaces.length();
		edge_conFacesNum[e] = numConFaces;
		bool swap = false;
		if((numConFaces > 1) && (conFaces[0] > conFaces[1])) swap = true;
		for( unsigned cf = 0; cf < numConFaces; cf++)
		{
			int cFaceId = cf;
			if( swap) cFaceId = 1 - cf;
			edge_conFacesIds[e*2 + cf] = conFaces[cFaceId];
			for( int ev = 0; ev < 2 ; ev++)
			{
				int numVtxUVs;
				float uvPiont[2];
				vtxIt.setIndex( edgeVertices[ev], prevIndex);
				vtxIt.numUVs( numVtxUVs);
				if( numVtxUVs <= 0) continue;
				MStatus stat = vtxIt.getUV( conFaces[cFaceId], uvPiont);
				if(!stat) continue;
				unsigned uvArrayIndex = (e*4) + (cf*2) + ev;
				edgeUVs_u.set( uvPiont[0], uvArrayIndex);
				edgeUVs_v.set( uvPiont[1], uvArrayIndex);
//#ifdef _DEBUG
//cout << "faceId #" << conFaces[cFaceId] << ", vtxId #" << edgeVertices[ev] << " = (";
//cout << edgeUVs_u[uvArrayIndex] << ", " << edgeUVs_v[uvArrayIndex] << ");" << endl;
//#endif
			}
		}
	}
//###################################### get points starts and vectors:
	for( unsigned i = 0; i < edgesCount; i++)
	{
		meshFn.getEdgeVertices( fSelEdges[i], edgeVertices);
		if ( invEdge[i] == 1)
		{
			meshFn.getPoint( edgeVertices[1], splitedPoints_start_N[i]);
			meshFn.getPoint( edgeVertices[0], point);
		}
		else
		{
			meshFn.getPoint( edgeVertices[0], splitedPoints_start_N[i]);
			meshFn.getPoint( edgeVertices[1], point);
		}
		splitedPoints_dir_N[i] = point - splitedPoints_start_N[i];
		splitedPoints_ndir_N[i] = splitedPoints_dir_N[i];
		splitedPoints_ndir_N[i].normalize();
		splitedPoints_start_R[i] = point;
		splitedPoints_dir_R[i] = splitedPoints_start_N[i] - point;
		splitedPoints_ndir_R[i] = splitedPoints_dir_R[i];
		splitedPoints_ndir_R[i].normalize();
	}
/*
#ifdef _DEBUG
cout << endl << "splitedPoints_start_N = ";
for( unsigned i=0;i<splitedPoints_start_N.length();i++)
	cout << splitedPoints_start_N[0]<<","<<splitedPoints_start_N[1]<<","<<splitedPoints_start_N[2] << " ";
cout << endl << "splitedPoints_dir_N = ";
for( unsigned i=0;i<splitedPoints_dir_N.length();i++)
	cout << splitedPoints_dir_N[0]<<","<<splitedPoints_dir_N[1]<<","<<splitedPoints_dir_N[2] << " ";
cout << endl;
#endif
*/
//######################################## do the split:
	MFloatPointArray internalPoints;
	MIntArray placements( edgesCount, MFnMesh::kOnEdge);
	MFloatArray edgeFactors( edgesCount, 0.5);
status = meshFn.split( placements, fSelEdges, edgeFactors, internalPoints);
	if( !status)
	{
		MGlobal::displayError( "can't split with given data");
		return status;
	}
#ifdef _DEBUG
cout << endl << " ! split success ! " << endl;
#endif
	newVtxCount = meshFn.numVertices();
	newUVsCount = meshFn.numUVs();
//###################################### get UVs starts and vectors:
#ifdef _DEBUG
cout << endl << "newVtxCount = " << newVtxCount << endl;
cout << endl << "newUVsCount = " << newUVsCount << endl;
#endif

	unsigned edgeNum = 0;
	unsigned uvNum = 0;
	unsigned numNewUVs = newUVsCount - oldUVsCount;
	splitedUVsU_start_N.setLength( numNewUVs);
	splitedUVsV_start_N.setLength( numNewUVs);
	splitedUVsU_start_R.setLength( numNewUVs);
	splitedUVsV_start_R.setLength( numNewUVs);
	splitedUVsU_dir_N.setLength( numNewUVs);
	splitedUVsV_dir_N.setLength( numNewUVs);
	splitedUVsU_dir_R.setLength( numNewUVs);
	splitedUVsV_dir_R.setLength( numNewUVs);
	splitedUVsU_ndir_N.setLength( numNewUVs);
	splitedUVsV_ndir_N.setLength( numNewUVs);
	splitedUVsU_ndir_R.setLength( numNewUVs);
	splitedUVsV_ndir_R.setLength( numNewUVs);
	for( int j = oldVtxCount; j < newVtxCount; j++)
	{
		MIntArray conFaces;
		int numVtxUVs;
		vtxIt.setIndex( j, prevIndex);
		vtxIt.numUVs( numVtxUVs);
		if( numVtxUVs > 0)
		{
//#ifdef _DEBUG
//cout << "vtx #" << j << " (edge #" << fSelEdges[edgeNum] << ") :" << endl;
//#endif
			bool swap = false;
			if((edge_conFacesNum[edgeNum] > 1) && (numVtxUVs > 1))
			{
				MItMeshPolygon faceIt( fMesh);
				faceIt.setIndex( edge_conFacesIds[edgeNum*2], prevIndex);
				int numFaceVtx = faceIt.polygonVertexCount();
				bool has = false;
//#ifdef _DEBUG
//cout << "((numConFaces > 1) && (numVtxUVs > 1)) : ( ";
//#endif
				for( int fVtx = 0; fVtx < numFaceVtx; fVtx++)
				{
					int uvIndex;
					faceIt.getUVIndex( fVtx, uvIndex);
//#ifdef _DEBUG
//cout << uvIndex << " ";
//#endif
					if( uvIndex == (uvNum + oldUVsCount)) has = true;
				}
				if( !has) swap = true;
//#ifdef _DEBUG
//cout << ") uv = " << (uvNum + oldUVsCount) << " => swap = " << swap << endl;
//#endif
			}
			for( int uvi = 0; uvi < numVtxUVs; uvi++)
			{
				unsigned uvArrayIndex_a;
				if( swap) uvArrayIndex_a = (edgeNum*4) + ((1-uvi)*2);
				else uvArrayIndex_a = (edgeNum*4) + (uvi*2);
				unsigned uvArrayIndex_b = uvArrayIndex_a;
				if(invEdge[edgeNum] == 1) uvArrayIndex_a++;
				else uvArrayIndex_b++;
				splitedUVsU_start_N[uvNum] = edgeUVs_u[uvArrayIndex_a];
				splitedUVsV_start_N[uvNum] = edgeUVs_v[uvArrayIndex_a];
				splitedUVsU_start_R[uvNum] = edgeUVs_u[uvArrayIndex_b];
				splitedUVsV_start_R[uvNum] = edgeUVs_v[uvArrayIndex_b];
				splitedUVsU_dir_N[uvNum] = edgeUVs_u[uvArrayIndex_b] - edgeUVs_u[uvArrayIndex_a];
				splitedUVsV_dir_N[uvNum] = edgeUVs_v[uvArrayIndex_b] - edgeUVs_v[uvArrayIndex_a];
				splitedUVsU_dir_R[uvNum] = edgeUVs_u[uvArrayIndex_a] - edgeUVs_u[uvArrayIndex_b];
				splitedUVsV_dir_R[uvNum] = edgeUVs_v[uvArrayIndex_a] - edgeUVs_v[uvArrayIndex_b];
				float dist = sqrt((splitedUVsU_dir_N[uvNum]*splitedUVsU_dir_N[uvNum]) + (splitedUVsV_dir_N[uvNum]*splitedUVsV_dir_N[uvNum]));
				if(dist == 0)
				{
					splitedUVsU_ndir_N[uvNum] = 0;
					splitedUVsV_ndir_N[uvNum] = 0;
					splitedUVsU_ndir_R[uvNum] = 0;
					splitedUVsV_ndir_R[uvNum] = 0;
				}
				else
				{
					splitedUVsU_ndir_N[uvNum] = splitedUVsU_dir_N[uvNum] / dist;
					splitedUVsV_ndir_N[uvNum] = splitedUVsV_dir_N[uvNum] / dist;
					splitedUVsU_ndir_R[uvNum] = splitedUVsU_dir_R[uvNum] / dist;
					splitedUVsV_ndir_R[uvNum] = splitedUVsV_dir_R[uvNum] / dist;
				}
/*#ifdef _DEBUG
cout << "uvNum #" << (uvNum + oldUVsCount) << " (uvi=" << uvi << ") : ( ";
cout << edgeUVs_u[uvArrayIndex_a] << ", ";
cout << edgeUVs_v[uvArrayIndex_a] << ") - ( ";
cout << edgeUVs_u[uvArrayIndex_b] << ", ";
cout << edgeUVs_v[uvArrayIndex_b] << ");" << endl;
#endif
*/						uvNum++;
			}
		}
		edgeNum++;
	}
//##################################################################################
	return status;
}

#include "../../../utilities/polygon_selectring.h"
bool tm_polySplitFty::getLoopFromFirst( const int &mode, const double &angleDeg, const int &maxCount)
{
#ifdef _DEBUG
cout << endl << "#####################   tm_polySplitFty::getLoopFromFirst( " << mode << ", " << angleDeg << ", " << maxCount << ")" << endl;
cout << endl << "fSelEdges = ";for( unsigned i=0;i<fSelEdges.length();i++) cout << fSelEdges[i] << " ";cout << endl;
#endif
	tm_polygon_selectring ringTool;
	int selIndex = fSelEdges[0];
	if(!ringTool.setMesh( fMesh)) return false;
    if(!ringTool.calculate( fSelEdges, selIndex, mode, angleDeg, maxCount)) return false;
	return true;
}

#include "../../../utilities/polygon_edgestoring.h"
bool tm_polySplitFty::getRing()
{
#ifdef _DEBUG
cout << endl << "#####################   tm_polySplitFty::getRing()" << endl;
cout << endl << "fSelEdges = ";for( unsigned i=0;i<fSelEdges.length();i++) cout << fSelEdges[i] << " ";cout << endl;
#endif
	tm_polygon_edgestoring edgestoring_tool;
	if(!edgestoring_tool.setMesh( fMesh))  return false;
	if(!edgestoring_tool.calculate( fSelEdges)) return false;
	return true;
}
