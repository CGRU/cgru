#pragma once

#include "../definitions.h"

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>

#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>

class tm_clearOutput: public MPxCommand
{
public:
					tm_clearOutput() {};
	virtual			~tm_clearOutput();
	static void*	creator();
	MStatus	doIt( const MArgList& );

	static MSyntax newSyntax();
	static const char *clr_Flag, *clr_LongFlag;
	static const char *cls_Flag, *cls_LongFlag;

	static bool CloseOutputWindow();
	static bool ClearOutputWindow();
	static bool DisplayHelp();

private:
};
