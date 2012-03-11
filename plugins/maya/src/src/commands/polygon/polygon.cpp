#include "polygon.h"

tm_polygon::~tm_polygon(){}

void* tm_polygon::creator()
{
	return new tm_polygon;
}

const char *tm_polygon::isMesh_Flag = "-im";
const char *tm_polygon::isMesh_LongFlag = "-isMesh";

const char *tm_polygon::isEdgeSoft_Flag = "-ies";
const char *tm_polygon::isEdgeSoft_LongFlag = "-isEdgeSoft";

const char *tm_polygon::getUVcoords_Flag = "-uv";
const char *tm_polygon::getUVcoords_LongFlag = "-getUVcoords";

const char *tm_polygon::findPath_Flag = "-fp";
const char *tm_polygon::findPath_LongFlag = "-findPath";

const char *tm_polygon::selectLoop_Flag = "-sl";
const char *tm_polygon::selectLoop_LongFlag = "-selectLoop";

const char *tm_polygon::selectRing_Flag = "-sr";
const char *tm_polygon::selectRing_LongFlag = "-selectRing";

const char *tm_polygon::removeTweaks_Flag = "-rtw";
const char *tm_polygon::removeTweaks_LongFlag = "-removeTweaks";

const char *tm_polygon::edgesToRing_Flag = "-etr";
const char *tm_polygon::edgesToRing_LongFlag = "-edgesToRing";
const char *tm_polygon::edgesToLoop_Flag = "-etl";
const char *tm_polygon::edgesToLoop_LongFlag = "-edgesToLoop";

MSyntax tm_polygon::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( isMesh_Flag, isMesh_LongFlag, MSyntax::kString);
	syntax.addFlag( isEdgeSoft_Flag, isEdgeSoft_LongFlag, MSyntax::kSelectionItem, MSyntax::kLong);
	syntax.addFlag( getUVcoords_Flag, getUVcoords_LongFlag, MSyntax::kSelectionItem, MSyntax::kLong);
	syntax.addFlag( findPath_Flag, findPath_LongFlag, MSyntax::kSelectionItem, MSyntax::kLong, MSyntax::kLong, MSyntax::kDouble);
	syntax.addFlag( selectLoop_Flag, selectLoop_LongFlag, MSyntax::kSelectionItem, MSyntax::kLong, MSyntax::kLong, MSyntax::kDouble, MSyntax::kLong);
	syntax.addFlag( selectRing_Flag, selectRing_LongFlag, MSyntax::kSelectionItem, MSyntax::kLong, MSyntax::kLong, MSyntax::kDouble, MSyntax::kLong);
	syntax.addFlag( removeTweaks_Flag, removeTweaks_LongFlag, MSyntax::kSelectionItem);
	syntax.addFlag( edgesToRing_Flag, edgesToRing_LongFlag);
	syntax.addFlag( edgesToLoop_Flag, edgesToLoop_LongFlag);

	syntax.setObjectType( MSyntax::kSelectionList);
	syntax.useSelectionAsDefault( true);

	return syntax;
}

MStatus tm_polygon::doIt( const MArgList& args )
{
	MStatus stat = MS::kSuccess;
	clearResult();

	MArgDatabase argData( syntax(), args);
	if(argData.isFlagSet( isMesh_Flag))
	{
		MString mString;
		argData.getFlagArgument( isMesh_Flag, 0, mString);
		bool bool_result = isMesh_Func( mString);
		setResult( bool_result );
		return stat;
	}
	if(argData.isFlagSet( isEdgeSoft_Flag))
	{
		MSelectionList selectionList;
		argData.getFlagArgument( isEdgeSoft_Flag, 0, selectionList);
		int index;
		argData.getFlagArgument( isEdgeSoft_Flag, 1, index);
		bool bool_result = isEdgeSoft_Func( selectionList, index);
		setResult( bool_result );
		return stat;
	}
	if(argData.isFlagSet( getUVcoords_Flag))
	{
		MSelectionList selectionList;
		argData.getFlagArgument( getUVcoords_Flag, 0, selectionList);
		float u, v;
		int index;
		argData.getFlagArgument( getUVcoords_Flag, 1, index);
		getUVcoords_Func( selectionList, index, u, v);
		appendToResult( u);
		appendToResult( v);
		return stat;
	}
	if(argData.isFlagSet( findPath_Flag))
	{
		MSelectionList selectionList;
		MIntArray edgesArray;
		argData.getFlagArgument( findPath_Flag, 0, selectionList);
		int index_a, index_b;
		double koeff_a;
		argData.getFlagArgument( findPath_Flag, 1, index_a);
		argData.getFlagArgument( findPath_Flag, 2, index_b);
		argData.getFlagArgument( findPath_Flag, 3, koeff_a);
		findPath_Func( selectionList, index_a, index_b, edgesArray, koeff_a);
		setResult( edgesArray);
		return stat;
	}
	if(argData.isFlagSet( selectLoop_Flag))
	{
		MSelectionList selectionList;
		MIntArray selArray;
		int selIndex, mode, maxCount;
		double angle;
		argData.getFlagArgument( selectLoop_Flag, 0, selectionList);
		argData.getFlagArgument( selectLoop_Flag, 1, selIndex);
		argData.getFlagArgument( selectLoop_Flag, 2, mode);
		argData.getFlagArgument( selectLoop_Flag, 3, angle);
		argData.getFlagArgument( selectLoop_Flag, 4, maxCount);
		selectLoop_Func( selArray, selectionList, selIndex, mode, angle, maxCount);
		setResult( selArray);
		return stat;
	}
	if(argData.isFlagSet( selectRing_Flag))
	{
		MSelectionList selectionList;
		MIntArray selArray;
		int selIndex, mode, maxCount;
		double angle;
		argData.getFlagArgument( selectRing_Flag, 0, selectionList);
		argData.getFlagArgument( selectRing_Flag, 1, selIndex);
		argData.getFlagArgument( selectRing_Flag, 2, mode);
		argData.getFlagArgument( selectRing_Flag, 3, angle);
		argData.getFlagArgument( selectRing_Flag, 4, maxCount);
		if(selectRing_Func( selArray, selectionList, selIndex, mode, angle, maxCount))
		{
			setResult( selArray);
			return stat;
		}
		MGlobal::displayError("tm_polygon::doIt - can't find ring edges.");
		return MStatus::kFailure;
	}
	if(argData.isFlagSet( removeTweaks_Flag))
	{
		MSelectionList selectionList;
		argData.getFlagArgument( removeTweaks_Flag, 0, selectionList);
		int result = removeTweaks_Func( selectionList);
		setResult( result);
		return stat;
	}
	if(argData.isFlagSet( edgesToRing_Flag))
	{
		MSelectionList selectionList;
		MIntArray edgesArray;
		argData.getObjects( selectionList);
		bool result = edgesToRing_Func( edgesArray, selectionList);
		if(!result)
		{
			edgesArray.clear();
			edgesArray.setLength( 1);
			edgesArray[0] = -1;
		}
		setResult( edgesArray);
		return stat;
	}
	if(argData.isFlagSet( edgesToLoop_Flag))
	{
		MSelectionList selectionList;
		MIntArray edgesArray;
		argData.getObjects( selectionList);
		bool result = edgesToLoop_Func( edgesArray, selectionList);
		if(!result)
		{
			edgesArray.clear();
			edgesArray.setLength( 1);
			edgesArray[0] = -1;
		}
		setResult( edgesArray);
		return stat;
	}
	appendToResult( "\n// Type \"help tm_polygon\" to query flags.\n");
	return stat;
}
//##################################################################### some small functions:

bool tm_polygon::isMesh_Func( MString mString)
{
	bool result = false;
	MStatus stat;
	MSelectionList selectionList;
	stat = selectionList.add( mString);
	if(!stat) return result;
	MObject object;
	stat = selectionList.getDependNode( 0, object);
	if(!stat) return result;
	if(MFn::kMesh == object.apiType()) result = true;
	return result;
}

bool tm_polygon::isEdgeSoft_Func( MSelectionList &selectionList, int index)
{
	bool result = false;
	MStatus stat;
	MObject object;
	stat = selectionList.getDependNode( 0, object);
	if(!stat)
	{
      MGlobal::displayError("***### tm_polygon: Can't find object.");
		return result;
	}
	MFnMesh mesh( object, &stat);
	if(!stat)
	{
      MGlobal::displayError("***### tm_polygon: Can't find mesh.");
		return result;
	}
	if(mesh.isEdgeSmooth( index)) result = true;
	if(!stat)
	{
      MGlobal::displayError("***### tm_polygon: Can't find edge.");
		return result;
	}
	return result;
}

bool tm_polygon::getUVcoords_Func( MSelectionList &selectionList, int index, float &u, float &v)
{
	MStatus stat;
	MObject object;
	stat = selectionList.getDependNode( 0, object);
	if(!stat) return false;
	MFnMesh mesh( object, &stat);
	if(!stat) return false;
	stat = mesh.getUV( index, u, v);
	if(!stat) return false;
	return true;
}

#include "../../utilities/polygon_selectring.h"
bool tm_polygon::selectRing_Func( MIntArray &selArray, MSelectionList &selectionList, int selIndex, int mode, double angle, int maxCount)
{
	tm_polygon_selectring ringTool;
	MObject object;
	selectionList.getDependNode( 0, object);
	if(ringTool.setMesh( object))
		return ringTool.calculate( selArray, selIndex, mode, angle, maxCount);
	else
		return false;
}

#include "../../utilities/polygon_selectloop.h"
bool tm_polygon::selectLoop_Func( MIntArray &selArray, MSelectionList &selectionList, int selIndex, int mode, double angle, int maxCount)
{
	tm_polygon_selectloop loopTool;
	MObject object;
	selectionList.getDependNode( 0, object);
	if(loopTool.setMesh( object))
		return loopTool.calculate( selArray, selIndex, mode, angle, maxCount);
	else
		return false;
}

bool tm_polygon::getFloat3PlugValue( MPlug & plug, MFloatVector & value )
{
	MObject object;
	plug.getValue( object );
	MFnNumericData numDataFn( object );
	numDataFn.getData( value[0], value[1], value[2] );
	return true;
}
bool tm_polygon::setFloat3PlugValue( MPlug & plug, MFloatVector & value )
{
	MObject object;
	MFnNumericData numDataFn( object );
	numDataFn.setData( value[0], value[1], value[2] );
	plug.setValue( object );
	return true;
}
