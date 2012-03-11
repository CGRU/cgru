#include "clearOutput.h"

tm_clearOutput::~tm_clearOutput(){}
void* tm_clearOutput::creator(){return new tm_clearOutput;}

const char *tm_clearOutput::clr_Flag = "-clr";
const char *tm_clearOutput::clr_LongFlag = "-clear";
const char *tm_clearOutput::cls_Flag = "-cls";
const char *tm_clearOutput::cls_LongFlag = "-close";

MSyntax tm_clearOutput::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( clr_Flag, clr_LongFlag);
	syntax.addFlag( cls_Flag, cls_LongFlag);
	return syntax;
}

#ifdef _WIN32
///**************************************************
bool tm_clearOutput::CloseOutputWindow()
{
	HWND wh = FindWindow(NULL, "Output Window");
	if(!wh) return false;
	PostMessage(wh, WM_CLOSE, 0, 0);
	return true;
}
bool tm_clearOutput::ClearOutputWindow()
{
	HWND wh = FindWindow(NULL, "Output Window");
	if(!wh) return false;
	HWND hEdit = GetTopWindow(wh);
	if(!hEdit) return false;
	SendMessage(hEdit, EM_SETSEL, 0, -1);
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM) "");
	return true;
}
#else///**************************************************
	bool tm_clearOutput::CloseOutputWindow() { return false; }
	bool tm_clearOutput::ClearOutputWindow() { return false; }
///**************************************************
#endif
MStatus tm_clearOutput::doIt(const MArgList &args)
{
	MArgDatabase argData( syntax(), args);
	bool res;
	if(argData.isFlagSet( clr_Flag))
		res = ClearOutputWindow();
	if(argData.isFlagSet( cls_Flag))
		res = CloseOutputWindow();
	return res ? MS::kSuccess : MS::kFailure;
}
