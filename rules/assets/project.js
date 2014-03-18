function prj_Show()
{
	a_SetLabel('Project');
	$('asset').innerHTML = n_Request({"send":{"getfile":'rules/assets/project.html'}});
}

if( ASSETS.project && ( ASSETS.project.path == g_CurPath()))
{
	prj_Show();
}

