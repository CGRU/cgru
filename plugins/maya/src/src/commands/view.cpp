#include "view.h"

tm_view::~tm_view(){}

void* tm_view::creator(){return new tm_view;}

const char *tm_view::wh_Flag = "-wh";
const char *tm_view::wh_LongFlag = "-widthHeight";

MSyntax tm_view::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( wh_Flag, wh_LongFlag, MSyntax::kString);
	return syntax;
}

MStatus tm_view::doIt( const MArgList& args )
{
	MStatus stat = MS::kSuccess;

	MArgDatabase argData( syntax(), args);
	if(argData.isFlagSet( wh_Flag))
	{
		MString mString;
		argData.getFlagArgument( wh_Flag, 0, mString);
		M3dView view;
		if( mString == "active")
		{
			view = M3dView::active3dView( &stat);
			if(!stat){MGlobal::displayError("###***   can't get active 3d view");return stat;}
		}
		else
		{
			stat = M3dView:: getM3dViewFromModelPanel( mString, view);
			if(!stat)
			{
				stat = M3dView:: getM3dViewFromModelEditor( mString, view);
				if(!stat){MGlobal::displayError("###***   can't get such 3d view ");return stat;}
			}
		}
		int w = view.portWidth();
		int h = view.portHeight();
		appendToResult(w);
		appendToResult(h);
		return stat;
	}
	clearResult();
	appendToResult( "\n// tm_view command usage:\n");
	appendToResult( "//    Synopsis: tm_view [flags]\n");
	appendToResult( "//    Flags:\n");
	appendToResult( "//       -wh -widthHeight  String\n");
	appendToResult( "//          String - name of modelPanel or modelEditor,\n");
	appendToResult( "//             type \"active\" to query active\n");
	appendToResult( "//          RESULT - int2\n");
	return stat;
}
