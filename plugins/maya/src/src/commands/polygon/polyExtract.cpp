#include "polyExtract.h"

tm_polyExtract::~tm_polyExtract(){}

void* tm_polyExtract::creator()
{
	return new tm_polyExtract;
}

//const char *tm_polyExtract::extractFaces_Flag = "-ext";
//const char *tm_polyExtract::extractFaces_LongFlag = "-extractFaces";

MSyntax tm_polyExtract::newSyntax()
{
	MSyntax syntax;
//	syntax.addFlag( extractFaces_Flag, extractFaces_LongFlag);

	syntax.setObjectType( MSyntax::kSelectionList);
	syntax.useSelectionAsDefault( true);

	return syntax;
}

MStatus tm_polyExtract::doIt( const MArgList& args )
{
	MStatus stat = MS::kSuccess;
	clearResult();

	MArgDatabase argData( syntax(), args);

//	if(argData.isFlagSet( extractFaces_Flag))
	{
		MSelectionList selectionList;
		argData.getObjects( selectionList);
		MStringArray node_names;
		bool result = extractFaces_Func( selectionList, node_names);
		if(!result)
		{
			MGlobal::displayError("tm_polyExtract: extractFaces function call failed.");
			return MStatus::kFailure;
		}
		setResult( node_names);
		return stat;
	}
}

bool tm_polyExtract::extractFaces_Func( MSelectionList &selectionList, MStringArray &node_names)
{
	MStatus status;
	MObject meshObj;
	status = selectionList.getDependNode( 0, meshObj);
	if(!status){MGlobal::displayError("tm_polyExtract::extractFaces_Func:   Can't find object !");return false;}
	MFnMesh meshFn( meshObj, &status);
	if(!status){MGlobal::displayError("tm_polyExtract::extractFaces_Func:   Non mesh object founded !");return false;}

	MDagPath meshDagPath_first, meshDagPath;
	selectionList.getDagPath( 0, meshDagPath_first);
	MObject multiFaceComponent;
	MIntArray inputFacesArray;
	inputFacesArray.clear();
	inputFacesArray.setSizeIncrement( 4096);
	MFnComponentListData compListFn;
	compListFn.create();
	for (MItSelectionList faceComponentIter(selectionList, MFn::kMeshPolygonComponent); !faceComponentIter.isDone(); faceComponentIter.next())
	{
		faceComponentIter.getDagPath(meshDagPath, multiFaceComponent);
		if(!(meshDagPath_first == meshDagPath))
		{
			MGlobal::displayError("tm_polyExtract::extractFaces_Func:   Different meshes faces founded !");
			return false;
		}
		if (!multiFaceComponent.isNull())
		{
			for (MItMeshPolygon faceIter(meshDagPath, multiFaceComponent); !faceIter.isDone(); faceIter.next())
			{
				int faceIndex = faceIter.index();
#ifdef _DEBUG
				infoMStr += faceIndex;
				infoMStr += " ";
#endif
				inputFacesArray.append( faceIndex);
				compListFn.add( multiFaceComponent );
			}
		}
	}
	if( inputFacesArray.length() == 0)
	{
		MGlobal::displayError("tm_polyExtract::extractFaces_Func:   No faces founded !");
		return false;
	}
#ifdef _DEBUG
	MGlobal::displayInfo( infoMStr);
#endif

	meshFn.setObject( meshDagPath_first);
	meshObj = meshFn.object();

//	MDagModifier dagModifier;
	MFnDagNode meshDagNodeFn;
	MFnDependencyNode depNodeFn;
	meshDagNodeFn.setObject( meshDagPath_first);
	MString meshName = meshDagNodeFn.name();
	MObject outMesh_attrObject = meshDagNodeFn.attribute( "outMesh");

// ----------------------------------- duplicate shape

	MObject duplicated_meshObjectA;
	MObject duplicated_meshObjectB;
	MObject inMesh_attrObjectA;
	MObject inMesh_attrObjectB;
/*
	MStringArray commandResult;
	MSelectionList selList;

	MGlobal::executeCommand( "duplicate " + meshDagNodeFn.name(), commandResult, 1, 1);
	selList.add( commandResult[0]);
	selList.getDependNode( 0, duplicated_meshObjectA);
	meshDagNodeFn.setObject( duplicated_meshObjectA);
	meshDagNodeFn.setName( meshName + "_tA");
	duplicated_meshObjectA = meshDagNodeFn.child(0);
	meshDagNodeFn.setObject( duplicated_meshObjectA);
	meshDagNodeFn.setName( meshName + "_sA");
	inMesh_attrObjectA = meshDagNodeFn.attribute( "inMesh");

	meshDagNodeFn.setObject( meshDagPath_first);
	selList.clear();

	MGlobal::executeCommand( "duplicate " + meshDagNodeFn.name(), commandResult, 1, 1);
	selList.add( commandResult[0]);
	selList.getDependNode( 0, duplicated_meshObjectB);
	meshDagNodeFn.setObject( duplicated_meshObjectB);
	meshDagNodeFn.setName( meshName + "_tB");
	duplicated_meshObjectB = meshDagNodeFn.child(0);
	meshDagNodeFn.setObject( duplicated_meshObjectB);
	meshDagNodeFn.setName( meshName + "_sB");
	inMesh_attrObjectB = meshDagNodeFn.attribute( "inMesh");
*/
	duplicated_meshObjectA = meshDagNodeFn.duplicate();
	meshDagNodeFn.setObject( duplicated_meshObjectA);
	meshDagNodeFn.setName( meshName + "_tA");
	duplicated_meshObjectA = meshDagNodeFn.child(0);
	meshDagNodeFn.setObject( duplicated_meshObjectA);
	meshDagNodeFn.setName( meshName + "_sA");
	inMesh_attrObjectA = meshDagNodeFn.attribute( "inMesh");

	meshDagNodeFn.setObject( meshDagPath_first);

	duplicated_meshObjectB = meshDagNodeFn.duplicate();
	meshDagNodeFn.setObject( duplicated_meshObjectB);
	meshDagNodeFn.setName( meshName + "_tB");
	duplicated_meshObjectB = meshDagNodeFn.child(0);
	meshDagNodeFn.setObject( duplicated_meshObjectB);
	meshDagNodeFn.setName( meshName + "_sB");
	inMesh_attrObjectB = meshDagNodeFn.attribute( "inMesh");

// ----------------------------------- create node deleteComponent

	MDGModifier dgModifier;

	MObject deleteComponent_nodeObjectA = dgModifier.createNode( MString("deleteComponent"));
	depNodeFn.setObject( deleteComponent_nodeObjectA );
	MObject deleteComponent_attrObjectA( depNodeFn.attribute( "deleteComponents" ));
	MObject inputGeometry_attrObjectA( depNodeFn.attribute( "inputGeometry" ));
	MObject outputGeometry_attrObjectA( depNodeFn.attribute( "outputGeometry" ));
	dgModifier.doIt();
	depNodeFn.setName( "dfA_" + meshName);
	node_names.append( depNodeFn.name());

	MObject deleteComponent_nodeObjectB = dgModifier.createNode( MString("deleteComponent"));
	depNodeFn.setObject( deleteComponent_nodeObjectB );
	MObject deleteComponent_attrObjectB( depNodeFn.attribute( "deleteComponents" ));
	MObject inputGeometry_attrObjectB( depNodeFn.attribute( "inputGeometry" ));
	MObject outputGeometry_attrObjectB( depNodeFn.attribute( "outputGeometry" ));
	dgModifier.doIt();
	depNodeFn.setName( "dfB_" + meshName);
	node_names.append( depNodeFn.name());

// ----------------------------------- set attribute deleteComponent.deleteComponents

	MObject componentList_object = compListFn.object();

	MPlug deleteComponents_plugA( deleteComponent_nodeObjectA, deleteComponent_attrObjectA );
	status = deleteComponents_plugA.setValue( componentList_object );

	MIntArray invertedFaces;
	int numPolygons = meshFn.numPolygons();
	invertedFaces.setLength( numPolygons - inputFacesArray.length());
	int selFace = 0;
	int invFace = 0;
	for( int f = 0; f < numPolygons; f++)
	{
		if( f == inputFacesArray[selFace])
			selFace++;
		else
			invertedFaces[invFace++] = f;
	}
	MFnSingleIndexedComponent singleIndexedComponentFn( meshObj);
	singleIndexedComponentFn.create( MFn::kMeshPolygonComponent);
	singleIndexedComponentFn.addElements( invertedFaces);
	compListFn.clear();
	compListFn.create();
   componentList_object = singleIndexedComponentFn.object();
	compListFn.add( componentList_object);
	componentList_object = compListFn.object();
	MPlug deleteComponents_plugB( deleteComponent_nodeObjectB, deleteComponent_attrObjectB );
	status = deleteComponents_plugB.setValue( componentList_object );

// ------------------------------------- connecting plugs
/**/
	dgModifier.connect(
			meshObj, outMesh_attrObject,
			deleteComponent_nodeObjectA, inputGeometry_attrObjectA
		);
	dgModifier.connect(
			deleteComponent_nodeObjectA, outputGeometry_attrObjectA,
			duplicated_meshObjectA, inMesh_attrObjectA
		);

	dgModifier.connect(
			meshObj, outMesh_attrObject,
			deleteComponent_nodeObjectB, inputGeometry_attrObjectB
		);
	dgModifier.connect(
			deleteComponent_nodeObjectB, outputGeometry_attrObjectB,
			duplicated_meshObjectB, inMesh_attrObjectB
		);

	dgModifier.doIt();

// ---------------------------------- assigning shading group
/**/
	meshDagNodeFn.setObject( meshDagPath_first);
	MObject instObjGroups_attrObject = meshDagNodeFn.attribute( "instObjGroups");
	MPlug instObjGroups_plug( meshObj, instObjGroups_attrObject);
	instObjGroups_plug = instObjGroups_plug.elementByPhysicalIndex(0);
	MPlugArray instObjGroups_plug_connectionsArray;
	instObjGroups_plug.connectedTo( instObjGroups_plug_connectionsArray, false, true);
	if( instObjGroups_plug_connectionsArray.length() > 0)
	{
		MPlug dagSetMembers_plug = instObjGroups_plug_connectionsArray[0];
		MObject shadingSetNode_object = dagSetMembers_plug.node();
		MFnSet setFn( shadingSetNode_object);
		setFn.addMember( duplicated_meshObjectA);
		setFn.addMember( duplicated_meshObjectB);
//depNodeFn.setObject(shadingSetNode_object);
//MGlobal::displayInfo( depNodeFn.name());
	}

// ------------------------------------------------------------
	return true;
}
/*
MStatus tm_polyExtract::redoIt()
{
	return dgModifier.doIt();
}
MStatus tm_polyExtract::undoIt()
{
	return dgModifier.undoIt();
}
bool tm_polyExtract::isUndoable() const
{
	return true;
}
*/
