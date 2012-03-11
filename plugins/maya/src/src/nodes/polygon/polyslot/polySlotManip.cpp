#include "polySlotManip.h"
#include "../../../definitions.h"

#include <maya/MObject.h>
#include <maya/MPlugArray.h>

#include <maya/MFnDagNode.h>
#include <maya/MFnTransform.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnDistanceManip.h>
#include <maya/MFnDirectionManip.h>

MTypeId tm_polySlotNodeManip::id( tm_polySlotNode_Manip__id );

MManipData tm_polySlotNodeManip::startPointCallback(unsigned /*index*/) 
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
MVector tm_polySlotNodeManip::nodeTranslation() const
{
	MFnDagNode dagFn(fNodePath);
	MDagPath path;
	dagFn.getPath(path);
	path.pop();  // pop from the shape to the transform
	MFnTransform transformFn(path);
	MVector vector = transformFn.translation(MSpace::kWorld);
	vector += edgesPos;
	return vector;
}
tm_polySlotNodeManip::tm_polySlotNodeManip()
{ 
    // Do not call createChildren from here 
}
tm_polySlotNodeManip::~tm_polySlotNodeManip(){}

void* tm_polySlotNodeManip::creator() { return new tm_polySlotNodeManip();}

MStatus tm_polySlotNodeManip::initialize()
{ 
    MStatus stat;
    stat = MPxManipContainer::initialize();
    return stat;
}
MStatus tm_polySlotNodeManip::createChildren()
{
    MStatus stat = MStatus::kSuccess;
#ifdef _DEBUG
cout<<endl<<"tm_polySlotNodeManip::createChildren"<<endl;
#endif
	// DistanceManip
	//
    fDistanceManip = addDistanceManip("slotDistanceManip", "slotDistance");
	MFnDistanceManip distanceManipFn(fDistanceManip);
	// DirectionManip
	//
	fDirectionManip = addDirectionManip( "slotDirectionManip", "slotDirection");
	MFnDirectionManip directionManipFn(fDirectionManip);

    return stat;
}

MStatus tm_polySlotNodeManip::connectToDependNode(const MObject &node)
{
    MStatus stat;

    MFnDependencyNode nodeFn;
    nodeFn.setObject(node);
	// Get edgesPos and edgesDir
	MPlug vectorPlug;
	MObject vectorObject;
	MFnNumericData numDataFn;
	vectorPlug = nodeFn.findPlug( "edgesPos", &stat);
	vectorPlug.getValue( vectorObject);
	numDataFn.setObject( vectorObject);
	numDataFn.getData( edgesPos[0], edgesPos[1], edgesPos[2]);
	vectorPlug = nodeFn.findPlug( "edgesDir", &stat);
	vectorPlug.getValue( vectorObject);
	numDataFn.setObject( vectorObject);
	numDataFn.getData( edgesDir[0], edgesDir[1], edgesDir[2]);
#ifdef _DEBUG
cout<<endl<<"tm_polySlot_Manip::connectToDependNode"<<endl;
cout<<"edgesPos = "<< edgesPos[0] << "; " << edgesPos[1] << "; " << edgesPos[2] << endl;
cout<<"edgesDir = "<< edgesDir[0] << "; " << edgesDir[1] << "; " << edgesDir[2] << endl;
#endif
	// Get the DAG path
	MPlug outPlug = nodeFn.findPlug( "outMesh", &stat);
	if(!stat){ MGlobal::displayError( "Can't find plug tm_polySlot_.outMesh"); return stat;}
	MPlugArray connectedPlugsArray;
	if(!outPlug.connectedTo( connectedPlugsArray, 0, 1, &stat)) {
		MGlobal::displayError("tm_polySlot_ has no connections");
		return MStatus::kFailure;}
	if(!stat){ MGlobal::displayError("can't find connections from tm_polySlot_"); return stat;}
	MObject mesh = connectedPlugsArray[0].node( &stat);
	if(!stat){ MGlobal::displayError("invalid connections from tm_polySlot_"); return stat;}
	MFnDagNode dagNodeFn(mesh);
	dagNodeFn.getPath(fNodePath);
	MObject parentNode = dagNodeFn.parent(0);
	MFnDagNode parentNodeFn(parentNode);
    // Connect the plugs
    //
	// DirectionManip
	//
    MFnDirectionManip directionManipFn;
    directionManipFn.setObject(fDirectionManip);
	MPlug offsetDirPlug = nodeFn.findPlug("offsetDir", &stat);
    if (MStatus::kFailure != stat) {
	    directionManipFn.connectToDirectionPlug(offsetDirPlug);
		unsigned startPointIndex = directionManipFn.startPointIndex();
	    addPlugToManipConversionCallback(startPointIndex, 
										 (plugToManipConversionCallback) 
										 &tm_polySlotNodeManip::startPointCallback);
	}
	else stat.perror("can't connectToDependNode DirectionManip");
//	directionManipFn.setStartPoint(edgesPos);
//	directionManipFn.setDirection(tm_polySlot_::manipDirVector);

	// DistanceManip
	//
    MFnDistanceManip distanceManipFn;
    distanceManipFn.setObject(fDistanceManip);
	MPlug offsetPlug = nodeFn.findPlug("offset", &stat);
    if (MStatus::kFailure != stat) {
	    distanceManipFn.connectToDistancePlug(offsetPlug);
		unsigned startPointIndex = distanceManipFn.startPointIndex();
	    addPlugToManipConversionCallback(startPointIndex, 
										 (plugToManipConversionCallback) 
										 &tm_polySlotNodeManip::startPointCallback);
	}
	else stat.perror("can't connectToDependNode DistanceManip");
//	distanceManipFn.setStartPoint(edgesPos);
//	distanceManipFn.setDirection(edgesDir);
	distanceManipFn.setDirection( MVector( 0.0, 0.5, 0.0));

	finishAddingManips();
	MPxManipContainer::connectToDependNode(node);
    return stat;
}

void tm_polySlotNodeManip::draw(M3dView & view, 
								 const MDagPath &path, 
								 M3dView::DisplayStyle style,
								 M3dView::DisplayStatus status)
{
    MPxManipContainer::draw(view, path, style, status);
    MPoint textPos = nodeTranslation();
	char str[100];
    sprintf(str, "offset"); 
    MString distanceText(str);

    view.beginGL(); 
	    view.drawText(distanceText, textPos, M3dView::kLeft);
    view.endGL();
}
