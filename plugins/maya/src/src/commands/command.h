#ifndef command_h
#define command_h

#include "../definitions.h"

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MSyntax.h>

#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MStatus.h>

#include <maya/MIntArray.h>
#include <maya/MDagPath.h>

#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>


class tm_command: public MPxCommand
{
public:
					tm_command();
	virtual			~tm_command();
	static void*	creator();
	MStatus	doIt( const MArgList& );
	MStatus	doIt_( const MArgList& );

	static MSyntax newSyntax();

private:

};

#endif
