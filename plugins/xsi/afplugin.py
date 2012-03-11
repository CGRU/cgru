from win32com.client import constants

#------------------------------------------------------------------
# Install callback for the plug-in
#------------------------------------------------------------------

def XSILoadPlugin(in_reg):

	in_reg.Name = "CGRU Afanasy Submit"
	in_reg.URL = "http://cgru.sourceforge.net/"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterCommand("SubmitToAfanasy_PY", "SubmitToAfanasy")
	in_reg.RegisterMenu(constants.siMenuTbRenderRenderID , "AfanasyPYMenu", False, False)

	return True

#------------------------------------------------------------------
# Initialize the command
#------------------------------------------------------------------

def SubmitToAfanasy_PY_Init(in_context):
	oCmd = in_context.Source
	oCmd.ReturnValue = False
	return True

#------------------------------------------------------------------
# Implement the command
#------------------------------------------------------------------

def SubmitToAfanasy_PY_Execute(in_context):
	Main(in_context)

def AfanasyPYMenu_Init(in_context):
	oMenu = in_context.Source
	menuitem = oMenu.AddCallbackItem( "Submit XSI to Afanasy", "Main" )
	return True

def Main(in_context):
	import afxsisubmit
	reload(afxsisubmit)
	afxsisubmit.Main()
