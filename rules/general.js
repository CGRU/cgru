RULES = {};
RULES.rules = 'rules';
RULES_TOP = {};

g_elCurFolder = null;

g_cycle = 0;

cgru_params.push(['user_title','User Title', 'Coordinator', 'Enter User Title']);

function g_Init()
{
	cgru_Init();
	u_Init();
	p_Init();
	c_Init();

	var config = n_ReadConfig();
	for( var file in config.config )
		cgru_ConfigJoin( config.config[file].cgru_config );

	c_RulesMergeDir( RULES_TOP, n_ReadDir('.'));

	if( RULES_TOP.cgru_config )
		cgru_ConfigJoin( RULES_TOP.cgru_config );

	document.getElementById('afanasy_webgui').innerHTML =
		'<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';

	if( RULES_TOP.company )
		document.getElementById('rules_button').textContent = RULES_TOP.company+'-RULES';

	u_el.navig.m_folder = '/';
	u_el.navig.m_path = '/';

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
	g_elCurFolder = u_el.navig;

	cgru_ClosePopus();
	u_Finish();

	RULES = c_CloneObj( RULES_TOP);
	ASSETS = {};

	c_Log('Navigating to: '+i_path);

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

		RULES.status = null;

		if( false == g_Goto( folders[i], path))
			break;
	}

//window.console.log('RULES_TOP='+JSON.stringify(RULES_TOP).replace(/,/g,', '));
	g_elCurFolder.classList.add('current');

	if( g_elCurFolder.m_path == '/')
		window.document.title = 'CG-RULES';
	else
		window.document.title = g_elCurFolder.m_path;

	u_el.thumbnail.setAttribute('src', RULES.root+path+'/'+RULES.rules+'/thumbnail.jpg');

	u_Process();
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
		g_elCurFolder.m_dir = n_ReadDir( i_path, ['status']);
	if( g_elCurFolder.m_dir == null )
		return false;

	c_RulesMergeDir( RULES, g_elCurFolder.m_dir);
	a_Append( i_path, g_elCurFolder.m_dir.rules);

	if( g_elCurFolder == u_el.navig )
		g_OpenFolder( g_elCurFolder );

	return true;
}

function g_CompareFolders(a,b)
{
	var attr = 'name';
	if( a[attr] < b[attr]) return -1;
	if( a[attr] > b[attr]) return 1;
	return 0;
}
function g_OpenFolder( i_elFolder )
{
	if( i_elFolder.classList.contains('opened'))
		return;

	i_elFolder.m_elFolders = [];
	if( i_elFolder.m_dir == null )
		return;

	i_elFolder.m_dir.folders.sort( g_CompareFolders );

	for( var i = 0; i < i_elFolder.m_dir.folders.length; i++)
	{
		var fobject = i_elFolder.m_dir.folders[i];
		var folder = fobject.name;
		if( folder.charAt(0) == '.' ) continue;

		var elFolder = document.createElement('div');
		elFolder.classList.add('folder');

		var fstatus = fobject.status;
		if( fstatus )
		{
			var elStatus = document.createElement('div');
			elFolder.appendChild( elStatus);
			elStatus.classList.add('fstatus');
			elStatus.textContent = fstatus.annotation.split(' ')[0];
		}

		var elName = document.createElement('div');
		elFolder.appendChild( elName);
		elFolder.classList.add('fname');
		elName.textContent = folder;

		elFolder.m_folder = folder;
		if( i_elFolder.m_path == '/' )
			elFolder.m_path = '/'+folder;
		else
			elFolder.m_path = i_elFolder.m_path+'/'+folder;
		elFolder.onclick = g_FolderOnClick;
//		elFolder.ondblclick = g_FolderOnDblClick;
//		elFolder.ondblclick = g_FolderOnClick;

		i_elFolder.appendChild( elFolder);
		i_elFolder.m_elFolders.push( elFolder);
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

function g_FolderOnClick( i_evt)
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

function g_FolderOnDblClick( i_evt)
{
window.console.log('g_FolderOnDblClick( i_evt)');
return;
g_FolderOnClick( i_evt);
	i_evt.stopPropagation();
	var elFolder = i_evt.currentTarget;

	if( elFolder.classList.contains('opened'))
		g_CloseFolder( elFolder);
	else
	{
		if( elFolder.m_dir == null )
			elFolder.m_dir = n_ReadDir( elFolder.m_path, ['status']);
		g_OpenFolder( elFolder);
	}
}

