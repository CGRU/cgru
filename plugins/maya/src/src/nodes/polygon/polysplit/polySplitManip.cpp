#include "polySplitManip.h"
#include "../../../definitions.h"

#include <maya/MObject.h>
#include <maya/MPlugArray.h>

#include <maya/MFnDagNode.h>
#include <maya/MFnTransform.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnDistanceManip.h>
#include <maya/MFnDirectionManip.h>

MTypeId tm_polySplitNodeManip::id( tm_polySplitNode_Manip__id );

MManipData tm_polySplitNodeManip::startPointCallback(unsigned /*index*/) 
const
{
	MManipData manipData;
	MFnNumericData numData;
	MObject numDataObj = numData.create(MFnNumericData::k3Double);
	MVector vec = nodeTranslation();
	numData.setData(vec.x, vec.y, vec.z);
	manipData = numDataObj;
	return manipData;
}
MVector tm_polySplitNodeManip::nodeTranslation() const
{
	MFnDagNode dagFn(fNodePath);
	MDagPath path;
	dagFn.getPath(path);
	path.pop();  // pop from the shape to the transform
	MFnTransform transformFn(path);
	MVector vector = transformFn.translation(MSpace::kWorld);
	vector += edgePos;
	return vector;
}
tm_polySplitNodeManip::tm_polySplitNodeManip()
{ 
    // Do not call createChildren from here 
}
tm_polySplitNodeManip::~tm_polySplitNodeManip(){}

void* tm_polySplitNodeManip::creator() { return new tm_polySplitNodeManip();}

MStatus tm_polySplitNodeManip::initialize()
{ 
    MStatus stat;
    stat = MPxManipContainer::initialize();
    return stat;
}
MStatus tm_polySplitNodeManip::createChildren()
{
    MStatus stat = MStatus::kSuccess;
#ifdef _DEBUG
cout<<endl<<"tm_polySplitNodeManip::createChildren"<<endl;
#endif
	// DistanceManip
	//
    fDistanceManip = addDistanceManip("SplitDistanceManip", "SplitDistance");
	MFnDistanceManip distanceManipFn(fDistanceManip);

    return stat;
}

MStatus tm_polySplitNodeManip::connectToDependNode(const MObject &node)
{
    MStatus stat;

    MFnDependencyNode nodeFn;
    nodeFn.setObject(node);
	// Get edgePos
	MPlug vectorPlug;
	MObject vectorObject;
	MFnNumericData numDataFn;
	vectorPlug = nodeFn.findPlug( "edgePos", &stat);
	vectorPlug.getValue( vectorObject);
	numDataFn.setObject( vectorObject);
	numDataFn.getData( edgePos[0], edgePos[1], edgePos[2]);
#ifdef _DEBUG
cout<<endl<<"tm_polySplit_Manip::connectToDependNode"<<endl;
cout<<"edgePos = "<< edgePos[0] << "; " << edgePos[1] << "; " << edgePos[2] << endl;
#endif
	// Get the DAG path
	MPlug outPlug = nodeFn.findPlug( "outMesh", &stat);
	if(!stat){ MGlobal::displayError( "Can't find plug tm_polySplit_.outMesh"); return stat;}
	MPlugArray connectedPlugsArray;
	if(!outPlug.connectedTo( connectedPlugsArray, 0, 1, &stat)) {
		MGlobal::displayError("tm_polySplitNode has no connections");
		return MStatus::kFailure;}
	if(!stat){ MGlobal::displayError("can't find connections from tm_polySplitNode"); return stat;}
	MObject mesh = connectedPlugsArray[0].node( &stat);
	if(!stat){ MGlobal::displayError("invalid connections from tm_polySplitNode"); return stat;}
	MFnDagNode dagNodeFn(mesh);
	dagNodeFn.getPath(fNodePath);
	MObject parentNode = dagNodeFn.parent(0);
	MFnDagNode parentNodeFn(parentNode);
    // Connect the plugs
    //

	// DistanceManip
	//
    MFnDistanceManip distanceManipFn;
    distanceManipFn.setObject(fDistanceManip);
	MPlug factorPlug = nodeFn.findPlug("factor", &stat);
    if (MStatus::kFailure != stat) {
	    distanceManipFn.connectToDistancePlug(factorPlug);
		unsigned startPointIndex = distanceManipFn.startPointIndex();
	    addPlugToManipConversionCallback(startPointIndex, 
										 (plugToManipConversionCallback) 
										 &tm_polySplitNodeManip::startPointCallback);
	}
	else stat.perror("can't connectToDependNode DistanceManip");
//	distanceManipFn.setStartPoint(edgePos);
//	distanceManipFn.setDirection(edgeDir);
	distanceManipFn.setDirection( MVector( 0.0, 1.0, 0.0));

	finishAddingManips();
//	MPxManipContainer::connectToDependNode(node);
    return stat;
}

void tm_polySplitNodeManip::draw(M3dView & view, 
								 const MDagPath &path, 
								 M3dView::DisplayStyle style,
								 M3dView::DisplayStatus status)
{
    MPxManipContainer::draw(view, path, style, status);
    MPoint textPos = nodeTranslation();
	char str[100];
    sprintf(str, "factor"); 
    MString distanceText(str);

    view.beginGL(); 
	    view.drawText(distanceText, textPos, M3dView::kLeft);
    view.endGL();
}
