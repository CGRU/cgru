#pragma once

#include "../definitions.h"

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>

#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>

#include <maya/M3dView.h>

class tm_view: public MPxCommand
{
public:
					tm_view() {};
	virtual			~tm_view();
	static void*	creator();
	MStatus	doIt( const MArgList& );

	static MSyntax newSyntax();
	static const char *wh_Flag, *wh_LongFlag;

private:

};
