
//
#include "polySlotFty.h"
#include "../../../utilities/utilities.h"

// General Includes
//
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

// Function Sets
//
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>

// Iterators
//
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>

MStatus tm_polySlotFty::doIt()
//
//	Description:
//		Performs the actual tm_polySlot operation on the given object and UVs
//
{
	MStatus status = MS::kSuccess;

	if( firstTime )
	{
		if(!validateEdges())
		{
			MGlobal::displayError( "tm_polySlot command failed: Bad edges selected." );
			return MStatus::kFailure;
		}
	}

	unsigned i; int index, p;
	int selEdgesCount = fSelEdges.length();

	MFnMesh meshFn( fMesh );


	MItMeshEdge edgeIt( fMesh );
	MItMeshVertex vtxIt( fMesh);
	MItMeshPolygon faceIt( fMesh);

#ifdef _DEBUG
cout << endl << "##########################tm_polySlotFty::doIt" << endl;
cout << endl << "fSelEdges = ";for(i=0;i<fSelEdges.length();i++) cout << fSelEdges[i] << " ";cout << endl;
#endif
//#################################################### getting mesh information
	MPointArray oldPoints, newPoints;
	meshFn.getPoints( oldPoints);
	MIntArray facesSmooth_a;
	facesSmooth_a.setSizeIncrement( 100);
	MIntArray facesSmooth_b;
	facesSmooth_b.setSizeIncrement( 100);
	int numSmoothFaces;


	// getting UVs:
	MFloatArray uArray;
	MFloatArray vArray;
	MIntArray uvCounts;
	MIntArray uvIds;
	MString uvSetName;
	unsigned uvSetsCount = meshFn.numUVSets();
	if(uvSetsCount)
	{
		meshFn.getCurrentUVSetName( uvSetName);
		meshFn.getUVs( uArray, vArray, &uvSetName);
		meshFn.getAssignedUVs( uvCounts, uvIds, &uvSetName); 
	}

	{// getting edges smoothing
	MIntArray conFaces;
	edgeIt.reset();
	while( !edgeIt.isDone())
	{
		if( !edgeIt.isSmooth())
		{
			edgeIt.getConnectedFaces( conFaces);
			if( conFaces.length() > 1)
			{
				facesSmooth_a.append( conFaces[0]);
				facesSmooth_b.append( conFaces[1]);
			}
		}
		edgeIt.next();
	}
	numSmoothFaces = facesSmooth_a.length();
	}

	if(firstTime)
	{
	polyConnects.clear();
	polyCount = meshFn.numPolygons();
	polyCounts.setLength( polyCount);
	oldVtxCount = meshFn.numVertices();
	MIntArray vertexList; 
	for( i = 0; i < polyCount; i++)
	{
		meshFn.getPolygonVertices( i, vertexList);
		unsigned vertexCount = vertexList.length();
		polyCounts[i] = vertexCount;
		for( unsigned j = 0; j < vertexCount; j++)
			polyConnects.append(vertexList[j]);
	}
	}

#ifdef _DEBUG
cout<<endl<<"facesSmooth_a:";for(i=0;i<facesSmooth_a.length();i++)cout<<facesSmooth_a[i]<<" ";
cout<<endl<<"facesSmooth_b:";for(i=0;i<facesSmooth_b.length();i++)cout<<facesSmooth_b[i]<<" ";
#endif

	// getting split vertices:
	if( firstTime)
	{
	slotVerticesIds.clear();
	int twoVertices_a[2];
	int twoVertices_b[2];
   int firstVertex;
	for( int se = 0; se < selEdgesCount; se++)
	{
		if( se == 0)
		{
			meshFn.getEdgeVertices( fSelEdges[se], twoVertices_a);
			meshFn.getEdgeVertices( fSelEdges[se+1], twoVertices_b);
			if( (twoVertices_a[0] != twoVertices_b[0]) &&
				(twoVertices_a[0] != twoVertices_b[1]))
				firstVertex = twoVertices_a[0];
			else
				firstVertex = twoVertices_a[1];
			vtxIt.setIndex( firstVertex, p);
			MIntArray edges;
			vtxIt.getConnectedEdges( edges);
			int numEdges = edges.length();
			for( int e = 0; e < numEdges; e++)
			{
				edgeIt.setIndex( edges[e], p);
				if( edgeIt.onBoundary())
				{
					slotVerticesIds.append( firstVertex);
					break;
				}
			}
			continue;
		}
		bool founded = false;
		meshFn.getEdgeVertices( fSelEdges[se], twoVertices_a);
		meshFn.getEdgeVertices( fSelEdges[se-1], twoVertices_b);
		if( (twoVertices_a[0] == twoVertices_b[0]) ||
			(twoVertices_a[0] == twoVertices_b[1]))
		{
				slotVerticesIds.append( twoVertices_a[0]);
				founded = true;
		} else
		if( (twoVertices_a[1] == twoVertices_b[0]) ||
			(twoVertices_a[1] == twoVertices_b[1]))
		{
				slotVerticesIds.append( twoVertices_a[1]);
				founded = true;
		}
		if( se == (selEdgesCount-1))
		{
			int lastVertex;
			meshFn.getEdgeVertices( fSelEdges[se], twoVertices_a);
			meshFn.getEdgeVertices( fSelEdges[se-1], twoVertices_b);
			if( (twoVertices_a[0] != twoVertices_b[0]) &&
				(twoVertices_a[0] != twoVertices_b[1]))
				lastVertex = twoVertices_a[0];
			else
				lastVertex = twoVertices_a[1];
			vtxIt.setIndex( lastVertex, p);
			MIntArray edges;
			vtxIt.getConnectedEdges( edges);
			int numEdges = edges.length();
			for( int e = 0; e < numEdges; e++)
			{
				edgeIt.setIndex( edges[e], p);
				if( edgeIt.onBoundary())
				{
					slotVerticesIds.append( lastVertex);
					break;
				}
			}
         if( lastVertex == firstVertex)
            slotVerticesIds.append( lastVertex);
		}
		if( founded) continue;
		MGlobal::displayError(" can`t find vertices to split");
		return MStatus::kFailure;
	}
	slotVtxCount = slotVerticesIds.length();
	offsetVerticesIds_a.setLength( slotVtxCount);
	offsetVerticesStart_a.setLength( slotVtxCount);
	offsetVerticesDir_a.setLength( slotVtxCount);
	offsetVerticesCont_a = slotVtxCount;
	offsetVerticesIds_b.setLength( slotVtxCount);
	offsetVerticesStart_b.setLength( slotVtxCount);
	offsetVerticesDir_b.setLength( slotVtxCount);
	offsetVerticesCont_b = slotVtxCount;
	}
#ifdef _DEBUG
cout<<endl<<"slotVerticesIds:";for(i=0;i<slotVerticesIds.length();i++)cout<<slotVerticesIds[i]<<" ";
cout<<endl<<"slotVtxCount = " << slotVtxCount;
#endif

	if( firstTime)
	{// getting right faces:
	rightFaces.clear();
	MIntArray conFaces;
	status = edgeIt.setIndex( fSelEdges[0], p);
	edgeIt.getConnectedFaces( conFaces, &status);
	int lastFaceIndex = conFaces[0];
	rightFaces.append(lastFaceIndex);
	rightFacesCount = 1;
	for( int sv = 0; sv < slotVtxCount; sv++)
	{
//cout<<endl<<"rightFaces:";for(i=0;i<rightFaces.length();i++)cout<<rightFaces[i]<<" ";cout<<endl;
		vtxIt.setIndex( slotVerticesIds[sv], p);
//cout<<endl<<" vtxIt = "<<slotVerticesIds[sv];
		vtxIt.getConnectedFaces( conFaces);
		int numConFaces = conFaces.length();
		for( int f = 0; f < numConFaces; f++)
		{
			if ( conFaces[f] == lastFaceIndex) continue;
			if (inMIntArray( conFaces[f], rightFaces, index)) continue;
//cout<<endl;cout<<"isFaceConnected( "<<conFaces[f]<<" - ";
			if( isFaceConnected( conFaces[f]))
			{
//cout<<"yes"<<endl;
				lastFaceIndex = conFaces[f];
				rightFaces.append(lastFaceIndex);
				rightFacesCount++;
				sv--;
				break;
			}
		}
	}
//cout<<endl<<"- rightFaces: ";for(i=0;i<rightFaces.length();i++)cout<<rightFaces[i]<<" ";cout<<endl;
	sortMIntArray( rightFaces);
	}
#ifdef _DEBUG
cout<<endl<<"rightFaces: ";for(i=0;i<rightFaces.length();i++)cout<<rightFaces[i]<<" ";cout<<endl;
#endif
//	if( firstTime)
	{// getting vertices offsets:
	MIntArray conFaces;
	MIntArray conEdges;
	MIntArray faceVertices;
	MPoint pointA, pointB;
	int opVtx;

	averagePos = MPoint( 0.0, 0.0, 0.0);
	averageDir = MVector( 0.0, 0.0, 0.0);
	for( int sv = 0; sv < slotVtxCount; sv++)
	{
//cout<<endl<<"* Vtx= " << slotVerticesIds[sv] <<endl;
		offsetVerticesIds_a[sv] = sv + oldVtxCount;
		offsetVerticesIds_b[sv] = slotVerticesIds[sv];
		meshFn.getPoint( slotVerticesIds[sv], offsetVerticesStart_a[sv]);
		offsetVerticesStart_b[sv] = offsetVerticesStart_a[sv];
		averagePos += offsetVerticesStart_a[sv];
		MVector vector( 0.0, 0.0, 0.0);
		MVector v( 0.0, 0.0, 0.0);
		meshFn.getPoint( slotVerticesIds[sv], pointB);

		vtxIt.setIndex( slotVerticesIds[sv], p);

		vtxIt.getConnectedEdges( conEdges);
		unsigned numConEdges = conEdges.length();
		bool founded = false;
		for( unsigned ce = 0; ce < numConEdges; ce++)
		{
//cout<<",   ce= " << conEdges[ce];
			if ( inMIntArray( conEdges[ce], fSelEdges, index)) continue;
//cout<<",notSE ";
			edgeIt.setIndex( conEdges[ce], p);
			edgeIt.getConnectedFaces( conFaces);
			if ( !inMIntArray( conFaces[0], rightFaces, index)) continue;
//cout<<",onRF ";
			founded = true;
//cout<<" look for edges ";
			vtxIt.getOppositeVertex( opVtx, conEdges[ce]);
			meshFn.getPoint( opVtx, pointA);
			v = pointB - pointA;
			v.normalize();
			vector += v;
		}
		if(!founded)
		{
//cout<<" look for faces: "<< endl;
			vtxIt.getConnectedFaces( conFaces);
			unsigned numConFaces = conFaces.length();
			for( unsigned cf = 0; cf < numConFaces; cf++)
			{
//cout<<",   cf= " << conFaces[cf];
				if (!inMIntArray( conFaces[cf], rightFaces, index)) continue;
//cout<<",RF ";
				faceIt.setIndex( conFaces[cf], p);
				faceIt.getConnectedVertices( faceVertices);
				unsigned numConVertices = faceVertices.length();
				for( unsigned fv = 0; fv < numConVertices; fv++)
				{
//cout<<", fv= " << faceVertices[fv];
//					if ( inMIntArray( faceVertices[fv], slotVerticesIds, index)) continue;
					if ( faceVertices[fv] == slotVerticesIds[sv]) continue;
//cout<<",NS ! ";
					meshFn.getPoint( faceVertices[fv], pointA);
					v = pointB - pointA;
//					v = pointA - pointB;
					v.normalize();
					vector += v;
				}
			}
		}
		vector.normalize();
		offsetVerticesDir_a[sv] = vector;
		offsetVerticesDir_b[sv] = -vector;

//		meshFn.getVertexNormal( slotVerticesIds[sv], offsetVerticesDir_a[sv]);

		averageDir += offsetVerticesDir_a[sv];
	}
	averagePos = averagePos / double(slotVtxCount);
	averageDir.normalize();
	}
	{// adding new vertices:
	newVtxCount = oldVtxCount + slotVtxCount;
	newPoints = oldPoints;
	newPoints.setLength( newVtxCount);
	i = 0;
	for( int v = oldVtxCount; v < newVtxCount; v++)
		newPoints[v] = oldPoints[slotVerticesIds[i++]];
	}

	if(firstTime)
	{// modifying polyConnects:
	int pc = 0; int rf = 0;
	for( unsigned f = 0; f < polyCount; f++)
	{
//cout<<endl<<"face = " << f;
		int numVtx = polyCounts[f];
		if(rightFaces[rf] != f)
		{
			pc += numVtx;
			continue;
		}
		for( int vtx = 0; vtx < numVtx; vtx++)
		{
			if( inMIntArray( polyConnects[ pc + vtx], slotVerticesIds, index))
			{
//cout<<endl<<"polyConnects["<<(pc + vtx)<<"] = "<<polyConnects[ pc + vtx]<<" in slotVerticesIds - "<<index;
				polyConnects[ pc + vtx] = oldVtxCount + index;
			}
		}
		pc += numVtx;
		rf++;
	}
	}
#ifdef _DEBUG
cout<<endl<<"offsetVerticesIds_a: ";for(i=0;i<offsetVerticesIds_a.length();i++)cout<<offsetVerticesIds_a[i]<<", ";
cout<<endl<<"offsetVerticesStart_a: ";for(i=0;i<offsetVerticesStart_a.length();i++)cout<<offsetVerticesStart_a[i].x<<","<<offsetVerticesStart_a[i].y<<","<<offsetVerticesStart_a[i].z<<"; ";
cout<<endl<<"offsetVerticesDir_a: ";for(i=0;i<offsetVerticesDir_a.length();i++)cout<<offsetVerticesDir_a[i].x<<","<<offsetVerticesDir_a[i].y<<","<<offsetVerticesDir_a[i].z<<"; ";
#endif
//######################################## creating new mesh:
	meshFn.create( newVtxCount, polyCount, newPoints, polyCounts, polyConnects, fMesh, &status);
	if( !status)
	{
		MGlobal::displayError(" can`t create new mesh");
		return status;
	}
	{// setting back edges smoothing:
	MItMeshPolygon fIt(fMesh);
	MItMeshEdge eIt(fMesh);
	MIntArray conEdges, conFaces;
	for( int sf = 0; sf < numSmoothFaces; sf++)
	{
//cout << endl<<"facesSmooth_a[" << sf << "] = " << facesSmooth_a[sf];
		fIt.setIndex( facesSmooth_a[sf], p);
		fIt.getEdges( conEdges);
//cout<<endl<<"conEdges: ";for(i=0;i<conEdges.length();i++)cout<<conEdges[i]<<" ";
		unsigned numConEdges = conEdges.length();
		for( unsigned ce = 0; ce < numConEdges; ce++)
		{
//cout << endl<<"conEdges[" << ce << "] = " << conEdges[ce];
			eIt.setIndex( conEdges[ce], p);
			eIt.getConnectedFaces( conFaces);
//cout<<endl<<"conFaces: ";for(i=0;i<conFaces.length();i++)cout<<conFaces[i]<<" ";
			unsigned numConFaces = conFaces.length();
			for( unsigned cf = 0; cf < numConFaces; cf++)
			{
//cout << endl<<"conFaces[" << cf << "] = " << conFaces[cf];
				if( conFaces[cf] == facesSmooth_a[sf]) continue;
				if( conFaces[cf] == facesSmooth_b[sf])
				{
//cout<<"yes";
					eIt.setSmoothing( false);
				}
			}
		}
	}
	}
	// setting UVs:
	if(uvSetsCount)
	{
		meshFn.setUVs( uArray, vArray, &uvSetName);
		meshFn.assignUVs( uvCounts, uvIds, &uvSetName);
	}
//##################################################################################
	return status;
}

bool tm_polySlotFty::isFaceConnected( int face)
{
	MItMeshPolygon faceIt( fMesh);
	MItMeshEdge edgeIt( fMesh);
	int p, index;
	faceIt.setIndex( face, p);
	MIntArray faceEdges;
	MIntArray conFaces;
	faceIt.getEdges( faceEdges);
	int numFaceEdges = faceEdges.length();
	for( int fe = 0; fe < numFaceEdges; fe++)
	{
//cout << endl<<"faceEdge = "<< faceEdges[fe];
		bool selEdge = false;
		if(inMIntArray( faceEdges[fe], fSelEdges, index)) continue;
		edgeIt.setIndex( faceEdges[fe], p);
		edgeIt.getConnectedFaces( conFaces);
		int numConFaces = conFaces.length();
		for( int cf = 0; cf < numConFaces; cf++)
		{
			if (conFaces[cf] == face) continue;
//cout << endl<<"conFace = "<< conFaces[cf];
			if( inMIntArray( conFaces[cf], rightFaces, index))
				return true;
		}
	}
	return false;
}

#include <list>
bool tm_polySlotFty::validateEdges()
{
	MFnMesh meshFn( fMesh);

	unsigned numInputEdges = fSelEdges.length();
	int *visitedEdges = new int[numInputEdges];
	for( unsigned e = 0; e < numInputEdges; e++) visitedEdges[e] = 0;
	std::list <int> loopEdgesList;
	int vertices[2];
	int nextVtxs[2];

	int edgeIndex = fSelEdges[0];
	loopEdgesList.push_back( fSelEdges[0]);
	visitedEdges[0] = 1;
	meshFn.getEdgeVertices( edgeIndex, vertices);
	for( unsigned vertex = 0; vertex < 2; vertex++)
	{
		edgeIndex = fSelEdges[0];
		int vtx = vertices[vertex];
		unsigned COUNTER = 0;
		while( COUNTER < 10000)
		{
		COUNTER++;
#ifdef _DEBUG
cout << endl << "edge = " << edgeIndex << ", vtx = " << vtx << " : ";
#endif
			bool founded = false;
			for( unsigned e = 0; e < numInputEdges; e++)
			{
#ifdef _DEBUG
cout << " e" << fSelEdges[e];
#endif
				if( fSelEdges[e] == edgeIndex) continue;
				if( visitedEdges[e] == 1) continue;
				meshFn.getEdgeVertices( fSelEdges[e], nextVtxs);
#ifdef _DEBUG
cout << " v" << nextVtxs[0] << " v" << nextVtxs[1] << ";";
#endif
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
				edgeIndex = fSelEdges[e];
#ifdef _DEBUG
cout << " *" << edgeIndex << "v" << vtx;
#endif
				visitedEdges[e] = 1;
				if( vertex == 0)
					loopEdgesList.push_front( edgeIndex);
				else
					loopEdgesList.push_back( edgeIndex);
				if( founded) break;
			}
			if(!founded) break;
		}
#ifdef _DEBUG
cout << endl << "COUNTER = " << COUNTER;
#endif
	}
	if (visitedEdges != NULL) delete [] visitedEdges;

	unsigned numLoopEdges = (unsigned)loopEdgesList.size();
#ifdef _DEBUG
cout << endl << "numLoopEdges = " << numLoopEdges << " : ";
#endif
	fSelEdges.setLength( numLoopEdges);
	for( unsigned e = 0; e < numLoopEdges; e++)
	{
		fSelEdges[e] = *loopEdgesList.begin();
#ifdef _DEBUG
cout << fSelEdges[e] << " ";
#endif
		loopEdgesList.pop_front();
	}
	if(fSelEdges.length() < 2)
	{
		MGlobal::displayError("Select at least two continuous edges.");
		return false;
	}
	if(numLoopEdges < numInputEdges)
		MGlobal::displayWarning("Some selected edges are not continuous. Ignored.");
	return true;
}
