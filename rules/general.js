p_PLAYER = false;

g_auth_user = null;
g_admin = false;
g_users = null;
g_groups = null;

g_elCurFolder = null;
g_nav_clicked = false;
g_arguments = null;

function cgru_params_OnChange( i_param, i_value) { u_ApplyStyles();}

function g_Init()
{
	cgru_Init();
	u_Init();
	c_Init();

	SERVER = c_Parse( n_Request({"start":{}}));
	if( SERVER == null ) return;
	if( SERVER.error )
	{
		c_Error( SERVER.error);
		return;
	}
	if( SERVER.version )
		$('version').innerHTML = c_Strip( SERVER.version);

	var config = c_Parse( n_Request({"initialize":{}}));
	if( config == null ) return;
	if( config.error )
	{
		c_Error( config.error);
		if( config.auth_error )
		{
			c_Error( config.auth_status);
			localStorage.auth_user = '';
			localStorage.auth_digest = '';
		}
		return;
	}

	if( config.admin ) g_admin = true;

	for( var file in config.config )
		cgru_ConfigJoin( config.config[file].cgru_config );
	g_users = config.users;
	if( config.user )
	{
		g_auth_user = config.user;
		ad_Init();
		nw_Init();
		u_InitAuth();
		up_Init();
	}

	c_RulesMergeDir( RULES_TOP, n_WalkDir(['.'], 0, RULES.rufolder, ['rules'])[0]);

	if( RULES_TOP.cgru_config )
		cgru_ConfigJoin( RULES_TOP.cgru_config );

	RULES = RULES_TOP;
	p_Init();
	nw_InitConfigured();

	document.getElementById('afanasy_webgui').innerHTML =
		'<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';
	$('rules_label').textContent = RULES_TOP.company+'-RULES';

	u_el.navig.m_folder = '/';
	u_el.navig.m_path = '/';

	window.onhashchange = g_PathChanged;
	document.body.onkeydown = g_OnKeyDown;

	g_NavigShowInfo();

	g_PathChanged();
}

function g_CurPath() { if( g_elCurFolder ) return g_elCurFolder.m_path; else return null;}

function g_OnKeyDown(e)
{
	if( e.keyCode == 27 ) cgru_EscapePopus();// ESC
}

function g_GO( i_path)
{
	if( i_path == '..' )
	{
		i_path = g_elCurFolder.m_path;
		i_path = i_path.substr( 0, i_path.lastIndexOf('/'));
	}
	window.location.hash = i_path;
}

function g_GetLocationArgs( i_args, i_absolute)
{
	var link = '#' + g_elCurFolder.m_path + '?' + encodeURI( JSON.stringify( i_args));
	if( i_absolute ) link = window.location.protocol + '//' + window.location.host + window.location.pathname + link;
	return link;
}

function g_SetLocationArgs( i_args)
{
	g_GO( g_elCurFolder.m_path + '?' + encodeURI( JSON.stringify( i_args)));
}

function g_ClearLocationArgs()
{
	g_GO( g_elCurFolder.m_path);
}

function g_PathChanged()
{
	var old_path = null;
	if( g_elCurFolder ) old_path = g_elCurFolder.m_path;

	var new_path = c_GetHashPath();
	g_arguments = null;
	if( new_path.indexOf('?') != -1 )
	{
		new_path = new_path.split('?');
		if( new_path.length > 0 )
			g_arguments = new_path[1];
		new_path = new_path[0];
	}

	if( new_path != old_path )
		g_Navigate( new_path);

	if( g_nav_clicked == false )
	{
		g_elCurFolder.scrollIntoView();
	}
	g_nav_clicked = false;

	if( g_arguments == null ) return;

	g_arguments = c_Parse( decodeURI( g_arguments));
	for( var func in g_arguments)
		if( window[func])
			window[func](g_arguments[func]);
		else
			c_Error('Function "'+func+'" does not exist.');
}

function g_Navigate( i_path)
{
	if( g_elCurFolder )
		g_elCurFolder.classList.remove('current');
	g_elCurFolder = u_el.navig;

	cgru_ClosePopus();
	u_Finish();
	ad_PermissionsFinish();

	RULES = c_CloneObj( RULES_TOP);

	c_Log('Navigating to: '+i_path);

	folders = i_path.split('/');
//window.console.log( folders);
	var walk = {};
	walk.paths = [];
	walk.folders = [];
	var path = '';
	for( var i = 0; i < folders.length; i++ )
	{
		if(( folders[i].length == 0 ) && ( i != 0 )) continue;
		if( path == '/' )
			path += folders[i];
		else
			path += '/' + folders[i];
		walk.folders.push( folders[i]);
		walk.paths.push( path);
	}

//	var rufiles = ['rules'];
//	if( i_last )
//		rufiles.push('status');

	walk.walks = n_WalkDir( walk.paths, 0, RULES.rufolder, ['rules','status'], ['status']);

	for( var i = 0; i < walk.paths.length; i++ )
	{
		RULES.status = null;
		if( false == g_Goto( walk.folders[i], walk.paths[i], walk.walks[i], i == (walk.paths.length-1)))
			return;
	}

//window.console.log('RULES_TOP='+JSON.stringify(RULES_TOP).replace(/,/g,', '));
	g_elCurFolder.classList.add('current');

	if( g_elCurFolder.m_path == '/')
		window.document.title = RULES_TOP.company+'-RULES';
	else
		window.document.title = g_elCurFolder.m_path;

	u_Process();
	ad_PermissionsProcess();
}

function g_Goto( i_folder, i_path, i_walk, i_last)
{
	c_Log('Goto "'+i_folder+'"'+(i_last?'*':''));

	if( i_walk.error )
	{
		c_Error( i_walk.error);
//		return false;
	}
//window.console.log('Goto='+i_folder);
/*
window.console.log('Current='+g_elCurFolder.m_folder);
window.console.log('Path='+g_elCurFolder.m_path);
window.console.log('Folders='+g_elCurFolder.m_dir.folders);
*/
	g_OpenFolder( g_elCurFolder );
	
	var exists = false;
	if( g_elCurFolder.m_elFolders )
	{
		for( var i = 0; i < g_elCurFolder.m_elFolders.length; i++)
		{
			if( g_elCurFolder.m_elFolders[i].m_folder == i_folder)
			{
				g_elCurFolder = g_elCurFolder.m_elFolders[i];
				exists = true;
				break;
			}
		}
	}

	if(( false == exists ) && ( g_elCurFolder.m_dir != null ) && ( i_folder != '' ))
	{
//		if( g_elCurFolder.m_dir.folders == null ) g_elCurFolder.m_dir.folders = [];
//		g_elCurFolder.m_dir.folders.push( {"name":i_folder});
		g_elCurFolder = g_AppendFolder( g_elCurFolder, {"name":i_folder});
		c_Info('Dummy folder "'+i_folder+'" pushed: "'+g_elCurFolder.m_path+'"');
	}

	if( i_walk.error || i_walk.denied )
	{
		g_elCurFolder.classList.add('dummy');
	}
/*	else
	{
		g_access = true;
		g_denied = false;
	}*/

	g_elCurFolder.m_dir = i_walk;
	if( g_elCurFolder.m_dir.folders == null ) g_elCurFolder.m_dir.folders = [];
	g_elCurFolder.m_dir.folders.sort( c_CompareFiles );

	c_RulesMergeDir( RULES, g_elCurFolder.m_dir);
	a_Append( i_path, g_elCurFolder.m_dir.rules);

	if( g_elCurFolder == u_el.navig )
		g_OpenFolder( g_elCurFolder );

	return true;
}

function g_OpenCloseFolder( i_elFolder)
{
	if( i_elFolder.classList.contains('opened')) g_CloseFolder( i_elFolder);
	else g_OpenFolder( i_elFolder);
}

function g_OpenFolder( i_elFolder )
{
	if( i_elFolder.classList.contains('opened'))
		return;

	i_elFolder.m_elFolders = [];

	if( i_elFolder.m_dir == null )
	{
		i_elFolder.m_dir = n_WalkDir( [i_elFolder.m_path], 0, RULES.rufolder, [], ['status'])[0];
		i_elFolder.m_dir.folders.sort( c_CompareFiles );
	}

	i_elFolder.classList.add('opened');

	if( i_elFolder.m_dir.folders == null ) return;

	for( var i = 0; i < i_elFolder.m_dir.folders.length; i++)
	{
		var fobject = i_elFolder.m_dir.folders[i];
		var folder = fobject.name;
		if( folder.charAt(0) == '.' ) continue;
		g_AppendFolder( i_elFolder, fobject);
	}
}

function g_AppendFolder( i_elParent, i_fobject)
{
	var folder = i_fobject.name;

	var elFolder = document.createElement('div');
	elFolder.classList.add('folder');
	elFolder.m_fobject = i_fobject;

	var elColor = document.createElement('div');
	elFolder.appendChild( elColor);
	elFolder.m_elColor = elColor;
	elColor.classList.add('fcolor');

	var elName = document.createElement('div');
	elFolder.appendChild( elName);
	elName.classList.add('fname');
	elName.textContent = folder;

	var elPercent = document.createElement('div');
	elFolder.appendChild( elPercent);
	elFolder.m_elPercent = elPercent;
	elPercent.classList.add('percent');
	elPercent.classList.add('info');

	var elAnn = document.createElement('div');
	elFolder.appendChild( elAnn);
	elFolder.m_elAnn = elAnn;
	elAnn.classList.add('annotation');
	elAnn.classList.add('info');

	var elTags = document.createElement('div');
	elFolder.appendChild( elTags);
	elFolder.m_elTags = elTags;
	elTags.classList.add('tags');
	elTags.classList.add('info');

	var elArtists = document.createElement('div');
	elFolder.appendChild( elArtists);
	elFolder.m_elArtists = elArtists;
	elArtists.classList.add('artists');
	elArtists.classList.add('info');

	elFolder.m_elProgress = document.createElement('div');
	elFolder.appendChild( elFolder.m_elProgress);
	elFolder.m_elProgress.classList.add('progress');
	elFolder.m_elProgressBar = document.createElement('div');
	elFolder.m_elProgress.appendChild( elFolder.m_elProgressBar);
	elFolder.m_elProgressBar.classList.add('progressbar');

	elFolder.m_folder = folder;
	if( i_elParent.m_path == '/' )
		elFolder.m_path = '/'+folder;
	else
		elFolder.m_path = i_elParent.m_path+'/'+folder;

	
	elFolder.onclick = g_FolderOnClick;
	elFolder.oncontextmenu = function(e){ e.stopPropagation(); g_OpenCloseFolder( e.currentTarget); return false;};
//	elFolder.ondblclick = g_FolderOnDblClick;

	g_FolderSetStatus( i_fobject.status, elFolder);

	i_elParent.appendChild( elFolder);
	i_elParent.m_elFolders.push( elFolder);

	return elFolder;
}

function g_FolderSetStatus( i_status, i_elFolder)
{
	if( i_elFolder == null ) i_elFolder = g_elCurFolder;

	i_elFolder.m_fobject.status = i_status;

	st_SetElLabel( i_status, i_elFolder.m_elAnn);
	st_SetElColor( i_status, i_elFolder.m_elColor, i_elFolder);
	st_SetElProgress( i_status, i_elFolder.m_elProgressBar, i_elFolder.m_elProgress);
	st_SetElArtists( i_status, i_elFolder.m_elArtists, true);
	st_SetElTags( i_status, i_elFolder.m_elTags, true);

	if( i_status && ( i_status.progress != null ) && ( i_status.progress >= 0 ))
		i_elFolder.m_elPercent.textContent = i_status.progress + '%';
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

//g_clicked_folder = null;
function g_FolderOnClick( i_evt, i_double)
{
//if( i_double !== true ) window.console.log('Clicked');

	i_evt.stopPropagation();
	var elFolder = i_evt.currentTarget;
	if( elFolder.classList.contains('dummy')) return;
/*	g_clicked_folder = elFolder;
	setTimeout( g_FolderClicked, 100);
}

function g_FolderClicked()
{
	elFolder = g_clicked_folder;*/
	if( elFolder.classList.contains('current'))
	{
		if( elFolder.classList.contains('opened'))
			g_CloseFolder( elFolder);
		else
			g_OpenFolder( elFolder);
		return;
	}

	g_nav_clicked = true;
	window.location.hash = elFolder.m_path;
}

function g_FolderOnDblClick( i_evt)
{
window.console.log('Double Clicked');
//g_FolderOnClick( i_evt, true);
return;
//return;
	i_evt.stopPropagation();
	var elFolder = i_evt.currentTarget;

	if( elFolder.classList.contains('opened'))
		g_CloseFolder( elFolder);
	else
		g_OpenFolder( elFolder);
}

function g_NavigShowInfo( i_toggle)
{
	var infos = ['annotation','artists','tags','percent'];
	for( var i = 0; i < infos.length; i++ )
	{
		var name = 'navig_show_'+infos[i];
		if( localStorage[name] == null )
		{
			if( i == 0 )
				localStorage[name] = 'true';
			else
				localStorage[name] = 'false';
		}
		if( i_toggle && ( i_toggle.id == name ))
		{
			if( localStorage[name] == 'true' ) localStorage[name] = 'false';
			else localStorage[name] = 'true';
		}
		if( localStorage[name] == 'true' )
		{
			$(name).classList.add('selected');
			u_el.navig.classList.add( name);
		}
		else
		{
			$(name).classList.remove('selected');
			u_el.navig.classList.remove( name);
		}
	}
}

function n_MessageReceived( i_msg)
{
	if( i_msg == null ) return;

	if( i_msg.thumbnail )
		u_UpdateThumbnail( i_msg);

	if( i_msg.convert )
		fv_Converted( i_msg);

	if( i_msg.cmdexec )
		for( var i = 0; i < i_msg.cmdexec.length; i++)
			n_MessageReceived( i_msg.cmdexec[i]);
}

