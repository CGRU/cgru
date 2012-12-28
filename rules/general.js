RULES = {};
RULES.rules = 'rules';
RULES_TOP = {};

g_elements = ['asset','rules','navig','info','log','assets'];
g_el = {};

g_elCurFolder = null;

g_cycle = 0;

cgru_params.push(['user_title','User Title', 'Coordinator', 'Enter User Title']);

function g_Init()
{
	cgru_Init();
	c_Init();

	var config = n_ReadConfig();
	for( var file in config.config )
		cgru_ConfigJoin( config.config[file].cgru_config );

	var top_dir = n_ReadDir('.');
	if( top_dir )
		for( var attr in top_dir.rules)
			c_RulesMergeObjs( RULES_TOP, top_dir.rules[attr])

	if( RULES_TOP.cgru_config )
		cgru_ConfigJoin( RULES_TOP.cgru_config );

	document.getElementById('afanasy_webgui').innerHTML =
		'<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';

	g_el.navig.m_folder = '/';
	g_el.navig.m_path = '/';

	window.onhashchange = g_PathChanged;
	document.body.onkeydown = g_OnKeyDown;

	g_PathChanged();
}

function g_OnKeyDown(e)
{
	if( e.keyCode == 27 ) cgru_ClosePopus();// ESC
}

function g_GO( i_path)
{
	window.location.hash = i_path;
}

function g_PathChanged()
{
	var path = document.location.hash;
//window.console.log( 'hash = ' + path);
	if( path.indexOf('#') == 0 )
		path = path.substr(1);
	else
		path = '/';

	if( path.charAt(0) != '/' )
		path = '/'+path;

	while( path.indexOf('//') != -1 )
		path = path.replace(/\/\//g,'/');

	if(( path.length > 1 ) && ( path.charAt(path.length-1) == '/'))
		path = path.substring( 0, path.length-1);

	g_Navigate( path);
}

function g_Navigate( i_path)
{
	if( g_elCurFolder )
		g_elCurFolder.classList.remove('current');
	g_elCurFolder = g_el.navig;

	cgru_ClosePopus();

//window.console.log('RULES_TOP='+JSON.stringify(RULES_TOP).replace(/,/g,', '));
	RULES = JSON.parse( JSON.stringify( RULES_TOP));
//	c_RulesMergeObjs( RULES, RULES_TOP);
	ASSETS = {};

	c_Log('Navigating to: '+i_path);
//window.console.log( 'path = ' + i_path);

	folders = i_path.split('/');
//window.console.log( folders);
	var path = '';
	for( var i = 0; i < folders.length; i++ )
	{
		if(( folders[i].length == 0 ) && ( i != 0 )) continue;

		if( path == '/' )
			path += folders[i];
		else
			path += '/' + folders[i];

		if( false == g_Goto( folders[i], path))
			break;
	}

//window.console.log('RULES_TOP='+JSON.stringify(RULES_TOP).replace(/,/g,', '));
	g_elCurFolder.classList.add('current');

	a_Process();
}

function g_Goto( i_folder, i_path)
{
g_cycle++;
//window.console.log('Goto='+i_folder);
/*
window.console.log('Current='+g_elCurFolder.m_folder);
window.console.log('Path='+g_elCurFolder.m_path);
window.console.log('Folders='+g_elCurFolder.m_dir.folders);
*/
	g_OpenFolder( g_elCurFolder );

	if( g_elCurFolder.m_elFolders )
	{
		var founded = false;
		for( var i = 0; i < g_elCurFolder.m_elFolders.length; i++)
		{
			if( g_elCurFolder.m_elFolders[i].m_folder == i_folder)
			{
				g_elCurFolder = g_elCurFolder.m_elFolders[i];
				founded = true;
				break;
			}
		}
//		if( false == founded )
//			return false;
	}

	if( g_elCurFolder.m_dir == null )
		g_elCurFolder.m_dir = n_ReadDir( i_path);
	if( g_elCurFolder.m_dir == null )
		return false;

	c_RulesMerge( g_elCurFolder.m_dir.rules);
	a_Append( i_path, g_elCurFolder.m_dir.rules);

	if( g_elCurFolder == g_el.navig )
		g_OpenFolder( g_elCurFolder );

	return true;
}

function g_OpenFolder( i_elFolder )
{
	if( i_elFolder.classList.contains('opened'))
		return;

	i_elFolder.m_elFolders = [];
	if( i_elFolder.m_dir == null )
		return;

	for( var i = 0; i < i_elFolder.m_dir.folders.length; i++)
	{
		var folder = i_elFolder.m_dir.folders[i];
		if( folder.charAt(0) == '.' ) continue;
		var elFolder = document.createElement('div');
		elFolder.classList.add('folder');
		elFolder.textContent = folder;
		elFolder.m_folder = folder;
		if( i_elFolder.m_path == '/' )
			elFolder.m_path = '/'+folder;
		else
			elFolder.m_path = i_elFolder.m_path+'/'+folder;
		elFolder.onclick = g_FolderClicked;
		elFolder.ondblclick = g_FolderClicked;
		g_elCurFolder.appendChild( elFolder);
		g_elCurFolder.m_elFolders.push( elFolder);
	}
	i_elFolder.classList.add('opened');
}

function g_CloseFolder( i_elFolder )
{
	if( false == i_elFolder.classList.contains('opened'))
		return;

	if( i_elFolder.m_elFolders && i_elFolder.m_elFolders.length )
		for( var i = 0; i < i_elFolder.m_elFolders.length; i++)
			i_elFolder.removeChild( i_elFolder.m_elFolders[i]);
	i_elFolder.m_elFolders = [];
	i_elFolder.classList.remove('opened');
}

function g_FolderClicked( i_evt)
{
	i_evt.stopPropagation();
	var elFolder = i_evt.currentTarget;

	if( elFolder.classList.contains('current'))
	{
		if( elFolder.classList.contains('opened'))
			g_CloseFolder( elFolder);
		else
			g_OpenFolder( elFolder);
		return;
	}

	window.location.hash = elFolder.m_path;
}

