/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	general.js - TODO: description
*/

"use strict";

var p_PLAYER = false;

var g_auth_user = null;
var g_admin = false;
var g_users = null;
var g_groups = null;

var g_elCurFolder = null;
var g_elFolders = {};

var g_nav_clicked = false;
var g_navigating_path = null;
var g_arguments = null;

var g_sort_aux_bottom = false;

var g_navig_infos = {
	all     : ['annotation', 'size', 'flags', 'artists', 'tags', 'thumbs', 'tasks','tasks_only_my', 'duration', 'price', 'frames', 'percent'],
	default : ['annotation', 'flags', 'tags', 'artists', 'thumbs', 'tasks', 'percent'],
	current : []
};

function cgru_params_OnChange(i_param, i_value)
{
	u_ApplyStyles();
}

/* ---------------- [ init functions ] ------------------------------------------------------------------- */

function g_Init()
{
	cgru_Init();
	u_Init();
	c_Init();
	n_Init();
	activity_Init();
	cm_Init();

	n_Request({"send": {"start": {}}, "func": g_Init_Server, "info": 'start', "force_log": true});

	window.onbeforeunload = g_OnClose;
}

function g_Init_Server(i_data)
{
	SERVER = i_data;
	if (SERVER == null)
		return;
	if (SERVER.error)
	{
		c_Error(SERVER.error);
		return;
	}
	if (SERVER.version)
		$('version').innerHTML = c_Strip(SERVER.version);

	if (SERVER.upload_max_filesize)
		RULES_TOP.upload_max_filesize = SERVER.upload_max_filesize;

	var url = decodeURI(document.location.href);

	n_Request({"send": {"initialize": {'url': url}}, "func": g_Init_Config, "info": 'init'});
}

function g_Init_Config(i_data)
{
	var config = i_data;
	if (config == null)
		return;
	if (config.error)
	{
		c_Error(config.error);
		if (config.auth_error)
		{
			c_Error(config.auth_status);
			localStorage.auth_user = '';
			localStorage.auth_digest = '';
		}
		return;
	}

	if (config.admin)
		g_admin = true;

	for (var file in config.config)
		cgru_ConfigJoin(config.config[file].cgru_config);
	g_users = config.users;
	if (config.user)
	{
		g_auth_user = config.user;
		ad_Init();
		u_InitAuth();
		up_Init();
	}

	nw_Init();
	bm_Init();
	st_Init();
	ab_Init();

	c_RulesMergeObjs(RULES_TOP, config.rules_top);
    if (RULES_TOP.ruerror)
    {
        if (RULES_TOP.ruerror.info)
            c_Log(RULES_TOP.ruerror.info);
        c_Error(RULES_TOP.ruerror.error);
        c_ConstantError(RULES_TOP.ruerror.error);
    }

	if (RULES_TOP.cgru_config)
		cgru_ConfigJoin(RULES_TOP.cgru_config);

	ROOT = config.root;

	RULES = RULES_TOP;
	p_Init();
	u_InitConfigured();
	nw_InitConfigured();
	bm_InitConfigured();
	up_InitConfigured();

	$('afanasy_webgui').href = 'http://' + cgru_Config.af_servername + ':' + cgru_Config.af_serverport;
	$('rules_label').textContent = RULES_TOP.company + '-RULES';

	$('panel_logo').style.backgroundImage = 'url(' + RULES_TOP.panel_logo_img + ')';

	u_el.navig.m_folder = '/';
	u_el.navig.m_path = '/';

	window.onhashchange = g_PathChanged;
	document.body.onkeydown = g_OnKeyDown;

	g_NavigShowInfo();

	if (localStorage.sort_aux_bottom == null)
		localStorage.sort_aux_bottom = 'ON';
	$('sort_aux_bottom').textContent = localStorage.sort_aux_bottom;
	g_sort_aux_bottom = (localStorage.sort_aux_bottom == 'ON')

	g_PathChanged();

	$('navigate_root').href = document.location.href.replace(/#.*/, '');
}

function g_OnClose()
{
	st_OnClose();
}

/* ---------------- [ path functions ] ------------------------------------------------------------------- */

function g_CurPath()
{
	if (g_elCurFolder)
		return g_elCurFolder.m_path;
	else
		return null;
}

function g_CurPathDummy()
{
	if (g_elCurFolder)
		return g_elCurFolder.m_fobject.dummy;
	else
		return true;
}


function g_OnKeyDown(e)
{
	if (e.keyCode == 27)  // ESC
	{
		// Close all cgru popups
		cgru_EscapePopus();

		// Close comments images processing
		if (ec_process_image && (ec_process_image.uploading != true))
			ec_ProcessImageClose();
	}
	else if (e.keyCode == 13) // ENTER
	{
		// Close comments images processing
		if (ec_process_image && (ec_process_image.uploading != true))
			ec_ProcessImageUpload();
	}
}

function g_GO(i_path)
{
	if (i_path == '..')
	{
		i_path = g_elCurFolder.m_path;
		i_path = i_path.substr(0, i_path.lastIndexOf('/'));
	}
	window.location.hash = i_path;
}

/* ---------------- [ location args functions ] ---------------------------------------------------------- */

function g_GetLocationArgs(i_args, i_absolute, i_path)
{
	var path = i_path;
	if (path == null)
		path = g_CurPath();
	var link = '#' + path + '?' + encodeURI(JSON.stringify(i_args));
	if (i_absolute)
		link = window.location.protocol + '//' + window.location.host + window.location.pathname + link;
	return link;
}

function g_SetLocationArgs(i_args)
{
	g_GO(g_elCurFolder.m_path + '?' + encodeURI(JSON.stringify(i_args)));
}

function g_ClearLocationArgs()
{
	g_GO(g_elCurFolder.m_path);
}


function g_PathChanged()
{
	var old_path = null;
	if (g_elCurFolder)
		old_path = g_elCurFolder.m_path;

	var new_path = c_GetHash();
	g_arguments = null;
	if (new_path.indexOf('?') != -1)
	{
		new_path = new_path.split('?');
		if (new_path.length > 0)
			g_arguments = c_Parse(new_path[1]);
		new_path = new_path[0];
	}

	if (new_path != old_path)
	{
		g_Navigate(new_path);
		return;
	}

	g_POST('hash')
}

function g_NavigatePost()
{
	// console.log('g_NavigatePost');
	a_Process();
	u_Process();
	nw_Process();
	ad_PermissionsProcess();

	if (g_nav_clicked == false)
	{
		g_elCurFolder.scrollIntoView({behavior:'auto',block:'center',inline:'center'});
	}

	g_nav_clicked = false;

	// Do bookmarks Post first.
	// As we should scroll to current bookmark if clicked not news and not bookmark item
	bm_NavigatePost();
	nw_NavigatePost();
	p_NavigatePost();

	$('navigate_up').href = '#' + c_PathDir(g_CurPath());

	if (g_elCurFolder.m_elNext)
		$('navigate_next').href = '#' + g_elCurFolder.m_elNext.m_path;
	else
		$('navigate_next').href = '#' + g_elCurFolder.m_path;

	if (g_elCurFolder.m_elPrev)
		$('navigate_prev').href = '#' + g_elCurFolder.m_elPrev.m_path;
	else
		$('navigate_prev').href = '#' + g_elCurFolder.m_path;

	g_POST('navig');
	g_navigating_path = null;
}

function g_POST(i_msg)
{
	if (ASSET && ((ASSET.path == g_CurPath()) || (ASSET.show_in_subdirectories)) && ASSET.post)
	{
		// Asset script should launch post function itself,
		// as all its data load (GUI construction) can be delayed (async).
		// But if post function runs because only hash arguments changes (not the entire location)
		// it should be run here, because asset was loaded before.
		if (i_msg == 'hash')
			window[ASSET.post]();
		return;
	}
	g_PostLaunchFunc(i_msg);
}

function g_PostLaunchFunc(i_msg)
{
	c_Log('Post: ' + i_msg);
	// console.log('Post: ' + i_msg);
	if (g_arguments == null)
		return;

	for (var func in g_arguments)
	{
		if (window[func])
			window[func](g_arguments[func]);
		else
			c_Error('Function "' + func + '" does not exist.');
	}
}

function g_Navigate(i_path)
{
	if (g_navigating_path != null)
	{
		c_Error('Already navigating to ' + g_navigating_path);
		return;
	}

	g_navigating_path = i_path;
	g_WaitingSet();

	if (g_elCurFolder)
		g_elCurFolder.classList.remove('current');
	g_elCurFolder = u_el.navig;

	cgru_ClosePopus();
	u_Finish();
	ad_PermissionsFinish();

	RULES = c_CloneObj(RULES_TOP);

	c_Info('Navigating to: ' + i_path);

	var folders = i_path.split('/');
	// window.console.log( folders);
	var walk = {};
	walk.paths = [];
	walk.folders = [];
	var path = '';
	for (var i = 0; i < folders.length; i++)
	{
		if ((folders[i].length == 0) && (i != 0))
			continue;
		if (path == '/')
			path += folders[i];
		else
			path += '/' + folders[i];
		walk.folders.push(folders[i]);
		walk.paths.push(path);
	}

	walk.rufiles = ['rules', 'status'];
	walk.cache_time = RULES.cache_time;
	walk.lookahead = ['status'];
	walk.wfunc = g_WalksReceived;
	walk.info = 'walk GO';
	n_WalkDir(walk);
}

function g_WalksReceived(i_data, i_args)
{
	var walk = i_args;
	walk.walks = i_data;
	g_WaitingReset();

	for (var i = 0; i < walk.paths.length; i++)
	{
		// Store parent status annotations:
		if (i && RULES.status && RULES.status.annotation)
		{
			if (RULES.annotations == null)
				RULES.annotations = {};
			var name = c_PathBase(walk.paths[i - 1]);
			if (name == '')
				name = '/';
			RULES.annotations[name] = RULES.status.annotation;
		}

		// Unlike other rules, status should not be merged,
		// it is unique for each location,
		// no matter what parent folders status is
		RULES.status = null;

		if (false == g_Goto(walk.folders[i], walk.paths[i], walk.walks[i]))
			return;
		// console.log('STATUS:'+g_CurPath()+':'+JSON.stringify(RULES.status));
	}

	// console.log( JSON.stringify( RULES.annotations ));
	// console.log('RULES_TOP='+JSON.stringify(RULES_TOP).replace(/,/g,', '));
	g_elCurFolder.classList.add('current');

	if (g_elCurFolder.m_path == '/')
		window.document.title = RULES_TOP.company + '-RULES';
	else
	{
		window.document.title = g_elCurFolder.m_path;

		// Update status of the current folder,
		// as it can be created before (user navigating shots of the same scene),
		// and now it can be changed
		g_FolderSetStatus(RULES.status);
	}

	g_NavigatePost();
}

function g_Goto(i_folder, i_path, i_walk)
{
	c_Log('Goto "' + i_folder + '"');

	if (i_walk.error)
	{
		c_Error(i_walk.error);
		//		return false;
	}
	// window.console.log('Goto='+i_folder);
	/*
	window.console.log('Current='+g_elCurFolder.m_folder);
	window.console.log('Path='+g_elCurFolder.m_path);
	window.console.log('Folders='+g_elCurFolder.m_dir.folders);
	*/
	if (g_elCurFolder != u_el.navig)
		g_OpenFolder(g_elCurFolder);

	var exists = false;
	if (g_elCurFolder.m_elFolders)
	{
		for (var i = 0; i < g_elCurFolder.m_elFolders.length; i++)
		{
			if (g_elCurFolder.m_elFolders[i].m_folder == i_folder)
			{
				g_elCurFolder = g_elCurFolder.m_elFolders[i];
				exists = true;
				break;
			}
		}
	}

	var dummy = false;
	if (i_walk.error || i_walk.denied)
	{
		//		g_elCurFolder.classList.add('dummy');
		dummy = true;
		c_Info('Dummy folder "' + i_folder + '" pushed: "' + g_elCurFolder.m_path + '"');
	}
	/*	else
		{
			g_access = true;
			g_denied = false;
		}*/

	if ((false == exists) && (g_elCurFolder.m_dir != null) && (i_folder != ''))
	{
		//		if( g_elCurFolder.m_dir.folders == null ) g_elCurFolder.m_dir.folders = [];
		//		g_elCurFolder.m_dir.folders.push( {"name":i_folder});

		//		g_elCurFolder = g_AppendFolder( g_elCurFolder, {"name":i_folder,"dummy":true});
		//		c_Info('Dummy folder "'+i_folder+'" pushed: "'+g_elCurFolder.m_path+'"');

		g_elCurFolder = g_AppendFolder(g_elCurFolder, {"name": i_folder, "dummy": dummy});
	}

	g_elCurFolder.m_dir = i_walk;
	if (g_elCurFolder.m_dir.folders == null)
		g_elCurFolder.m_dir.folders = [];
	g_elCurFolder.m_dir.folders.sort(g_CompareFolders);

	// console.log('RULES:'+g_CurPath()+':'+JSON.stringify( g_elCurFolder.m_dir.rules))

	c_RulesMergeDir(RULES, g_elCurFolder.m_dir);
	a_Append(i_path, g_elCurFolder.m_dir.rules);

	if (g_elCurFolder == u_el.navig)
		g_OpenFolder(g_elCurFolder);

	if (g_elCurFolder.m_elGroup)
		g_GroupShowFolders(g_elCurFolder.m_elGroup);

	return true;
}

/* ---------------- [ folder functions ] ----------------------------------------------------------------- */

function g_OpenCloseFolder(i_elFolder)
{
	if (i_elFolder.classList.contains('opened'))
		g_CloseFolder(i_elFolder);
	else
		g_OpenFolder(i_elFolder);
}

function g_OpenFolder(i_elFolder)
{
	if (i_elFolder.classList.contains('opened'))
		return;

	i_elFolder.m_elFolders = [];
	i_elFolder.m_elGroups = [];

	if (i_elFolder.m_dir == null)
	{
		n_WalkDir({
			'paths': [i_elFolder.m_path],
			'rufiles': ['rules', 'status'],
			'lookahead': ['status'],
			'wfunc': g_OpenFolderDo,
			'element': i_elFolder
		});
		g_WaitingSet();
		return;
	}
	g_OpenFolderDo(null, {"element": i_elFolder});
}

function g_OpenFolderDo(i_data, i_args)
{
	var el = i_args.element;
	el.classList.add('opened');

	g_WaitingReset();

	if (i_data)
	{
		el.m_dir = i_data[0];
		el.m_dir.folders.sort(g_CompareFolders);
	}

	if (el.m_dir.folders == null)
		return;

	for (let i = 0; i < el.m_dir.folders.length; i++)
	{
		let fobject = el.m_dir.folders[i];

		// Skip hidden folders
		if (fobject.name.charAt(0) == '.')
			continue;

		let elFolder = g_AppendFolder(el, fobject);

		// This can happen if the parent of the current folder was closed and than opened
		// without current folder change
		if (elFolder.m_path == g_CurPath())
		{
			g_elCurFolder = elFolder;
			elFolder.classList.add('current');
		}
	}

	if (el.m_elGroup)
		g_GroupShowFolders(el.m_elGroup);

	if (null == RULES.group_folders_levels)
		return;

	if (RULES.group_folders_levels.indexOf(el.m_path.split('/').length) == -1)
		return;

	// Group folders:
	let prefix = null;
	let elPrevFolder = null;
	let elFolders = [];
	for (let i = 0; i < el.m_elFolders.length; i++)
	{
		let elFolder = el.m_elFolders[i];
		let name = elFolder.m_folder;
		let parts = name.split('_');

		if ((name.length < 3) || (name.indexOf('_') < 1) || (parts.length < 2) || (parts[0] != prefix))
		{
			if (elFolders.length > 1)
			{
				elPrevFolder = g_GroupCreate(elFolders, el, elPrevFolder, prefix);
				elFolders = [];
				prefix = null;
			}
		}

		// Name is not suitable for grouping
		if ((name.length < 3) || (name.indexOf('_') < 1) || (parts.length < 2))
		{
			prefix = null;
			elPrevFolder = null;
			elFolders = [];
			continue;
		}

		// Some new prefix found
		if (parts[0] != prefix)
		{
			prefix = parts[0];
			elFolders = [];
			elPrevFolder = elFolder;
		}

		elFolders.push(elFolder);
	}

	if (elFolders.length > 1)
		g_GroupCreate(elFolders, el, elPrevFolder, prefix);
}

function g_GroupCreate(i_elFolders, i_elParent, i_elPrevFolder, i_prefix)
{
	let elGroup = document.createElement('div');
	elGroup.classList.add('group');
	elGroup.m_elFolders = [];
	elGroup.m_hidden_folders = false;

	i_elParent.m_elFBody.insertBefore(elGroup, i_elPrevFolder);
	i_elParent.m_elGroups.push(elGroup);

	let elName = document.createElement('div');
	elGroup.appendChild(elName);
	elName.classList.add('name');
	elName.textContent = i_prefix;

	let elFBody = document.createElement('div');
	elGroup.appendChild(elFBody);
	elFBody.classList.add('fbody');
	elGroup.m_elFBody = elFBody;

	for (let i = 0; i < i_elFolders.length; i++)
	{
		let elFolder = i_elFolders[i];
		elFolder.m_elGroup = elGroup;
		elFBody.appendChild(elFolder);
		elGroup.m_elFolders.push(elFolder);
	}

	c_Log('Group "' + i_elParent.m_path + ': ' + i_prefix + '" created"');

	g_GroupHideFolders(elGroup);

	elGroup.onclick = g_GroupShowHideFolders;
	elGroup.oncontextmenu = g_GroupShowHideFolders;

	return elGroup;
}

function g_GroupHideFolders(i_elGroup)
{
	if (true == i_elGroup.m_hidden_folders)
		return;

	for (let i = 0; i < i_elGroup.m_elFolders.length; i++)
	{
		let elFolder = i_elGroup.m_elFolders[i];
		elFolder.style.display = 'none';
	}

	i_elGroup.classList.add('closed');
	i_elGroup.classList.remove('opened');
	i_elGroup.m_hidden_folders = true;
}

function g_GroupShowFolders(i_elGroup)
{
	if (false == i_elGroup.m_hidden_folders)
		return;

	for (let i = 0; i < i_elGroup.m_elFolders.length; i++)
	{
		let elFolder = i_elGroup.m_elFolders[i];
		elFolder.style.display = 'block';
	}

	i_elGroup.classList.remove('closed');
	i_elGroup.classList.add('opened');
	i_elGroup.m_hidden_folders = false;
}

function g_GroupShowHideFolders(i_evt)
{
	i_evt.stopPropagation();

	let elGroup = i_evt.currentTarget;

	if (elGroup.m_hidden_folders)
		g_GroupShowFolders(elGroup);
	else
		g_GroupHideFolders(elGroup);

	return false;
}

function g_WaitingSet()
{
	$('navig_loading').style.display = 'block';
}

function g_WaitingReset()
{
	$('navig_loading').style.display = 'none';
}

function g_AppendFolder(i_elParent, i_fobject)
{
	var folder = i_fobject.name;

	var elFolder = document.createElement('div');
	elFolder.classList.add('folder');
	elFolder.m_fobject = i_fobject;
	elFolder.m_elParent = i_elParent;

	var elFBody = document.createElement('div');
	elFolder.appendChild(elFBody);
	elFolder.m_elFBody = elFBody;
	elFBody.classList.add('fbody');

	if (c_AuxFolder(i_fobject))
	{
		i_fobject.auxiliary = true;
		elFolder.classList.add('auxiliary');
		elFBody.classList.add('auxiliary');
	}
	if (i_fobject.dummy)
	{
		elFolder.classList.add('dummy');
		elFBody.classList.add('dummy');
	}

	var elName = document.createElement('a');
	elFBody.appendChild(elName);
	elName.classList.add('fname');
	elName.innerHTML = c_HighlightBadChars(folder);

	var elPercent = document.createElement('div');
	elFBody.appendChild(elPercent);
	elFolder.m_elPercent = elPercent;
	elPercent.classList.add('percent');
	elPercent.classList.add('info');

	var elFrames = document.createElement('div');
	elFBody.appendChild(elFrames);
	elFolder.m_elFrames = elFrames;
	elFrames.classList.add('frames');
	elFrames.classList.add('info');

	var elPrice = document.createElement('div');
	elFBody.appendChild(elPrice);
	elFolder.m_elPrice = elPrice;
	elPrice.classList.add('price');
	elPrice.classList.add('info');

	var elDuration = document.createElement('div');
	elFBody.appendChild(elDuration);
	elFolder.m_elDuration = elDuration;
	elDuration.classList.add('duration');
	elDuration.classList.add('info');

	var elSize = document.createElement('div');
	elFBody.appendChild(elSize);
	elFolder.m_elSize = elSize;
	elSize.classList.add('size');
	elSize.classList.add('info');

	var elAnn = document.createElement('div');
	elFBody.appendChild(elAnn);
	elFolder.m_elAnn = elAnn;
	elAnn.classList.add('annotation');
	elAnn.classList.add('info');

	var elTags = document.createElement('div');
	elFBody.appendChild(elTags);
	elFolder.m_elTags = elTags;
	elTags.classList.add('tags');
	elTags.classList.add('info');

	var elArtists = document.createElement('div');
	elFBody.appendChild(elArtists);
	elFolder.m_elArtists = elArtists;
	elArtists.classList.add('artists');
	elArtists.classList.add('info');

	var elFlags = document.createElement('div');
	elFBody.appendChild(elFlags);
	elFolder.m_elFlags = elFlags;
	elFlags.classList.add('flags');
	elFlags.classList.add('info');

	if (i_fobject.thumbnail)
	{
		let elThumb = document.createElement('div');
		elFBody.appendChild(elThumb);
		elFolder.m_elThumb = elThumb;
		elThumb.classList.add('thumb');
		elThumb.classList.add('info');
	}

	var elTasks = document.createElement('div');
	elFBody.appendChild(elTasks);
	elFolder.m_elTasks = elTasks;
	elTasks.classList.add('tasks');
	elTasks.classList.add('info');

	elFolder.m_elProgress = document.createElement('div');
	elFBody.appendChild(elFolder.m_elProgress);
	elFolder.m_elProgress.classList.add('progress');
	elFolder.m_elProgressBar = document.createElement('div');
	elFolder.m_elProgress.appendChild(elFolder.m_elProgressBar);
	elFolder.m_elProgressBar.classList.add('progressbar');

	elFolder.m_folder = folder;
	if (i_elParent.m_path == '/')
		elFolder.m_path = '/' + folder;
	else
		elFolder.m_path = i_elParent.m_path + '/' + folder;

	elName.href = '#' + elFolder.m_path;
	if (i_fobject.thumbnail)
		elFolder.m_elThumb.style.backgroundImage = 'url(' + c_GetRuFilePath(RULES.thumbnail.filename, elFolder.m_path) + ')';

	elFolder.onclick = g_FolderOnClick;
	elFolder.oncontextmenu = function(e) {
		e.stopPropagation();
		g_OpenCloseFolder(e.currentTarget);
		return false;
	};
	//	elFolder.ondblclick = g_FolderOnDblClick;

	g_FolderSetStatus(i_fobject.status, elFolder);
	if (i_fobject.space != null)
	{
		elFolder.m_elSize.textContent = c_Bytes2KMG(i_fobject.space);

		var title = '';
		if (i_fobject.size_total)
			title += 'Total Size: ' + c_Bytes2KMG(i_fobject.size_total);
		if (i_fobject.space)
			title += '\nSpace on device: ' + c_Bytes2KMG(i_fobject.space);
		if (i_fobject.num_folders_total)
			title += '\nFolders count: ' + i_fobject.num_folders_total;
		if (i_fobject.num_files_total)
			title += '\nFiles count: ' + i_fobject.num_files_total;

		elFolder.m_elSize.title = title;
	}

	// Search index to place new folder alphabetically:
	var index = 0;
	for (var i = 0; i < i_elParent.m_elFolders.length; i++, index++)
		if (g_CompareFolders(elFolder.m_fobject, i_elParent.m_elFolders[i].m_fobject) < 0)
			break;

	// Get parent element body to insert new child (root navig element has no body):
	var elBody = i_elParent.m_elFBody;
	if (elBody == null)
		elBody = i_elParent;

	// Insert element to parent body:
	if (i_elParent.m_elFolders.length && i_elParent.m_elFolders[index])
	{
		// There can be a folders group:
		// (this is most probably a dummy folder)
		elBody = i_elParent.m_elFolders[index].parentNode;
		elBody.insertBefore(elFolder, i_elParent.m_elFolders[index]);
	}
	else
		elBody.appendChild(elFolder);

	// Insert element in a parent array:
	i_elParent.m_elFolders.splice(index, 0, elFolder);

	// Store element in a global array:
	g_elFolders[elFolder.m_path] = elFolder;

	// Set next and previous folders:
	if (index - 1 >= 0)
	{
		var elPrev = i_elParent.m_elFolders[index - 1];
		elPrev.m_elNext = elFolder;
		elFolder.m_elPrev = elPrev;
	}
	if (index + 1 < i_elParent.m_elFolders.length)
	{
		var elNext = i_elParent.m_elFolders[index + 1];
		elNext.m_elPrev = elFolder;
		elFolder.m_elNext = elNext;
	}

	return elFolder;
}

function g_RemoveFolder(i_elFolder)
{
	var elParent = i_elFolder.m_elParent;

	for (var i = 0; i < elParent.m_elFolders.length; i++)
	{
		if (elParent.m_elFolders[i] == i_elFolder)
		{
			elParent.m_elFolders.splice(i, 1);

			// Set prev folder next link:
			if (i > 0)
			{
				if (i < elParent.m_elFolders.length)
					elParent.m_elFolders[i - 1].m_elNext = elParent.m_elFolders[i];
				else
					elParent.m_elFolders[i - 1].m_elNext = null;
			}

			// Set next folder prev link:
			if (i < elParent.m_elFolders.length)
			{
				if (i > 0)
					elParent.m_elFolders[i].m_elPrev = elParent.m_elFolders[i - 1];
				else
					elParent.m_elFolders[i].m_elPrev = null;
			}

			break;
		}
	}

	// Remove element from a global array:
	g_elFolders[i_elFolder.m_path] = null;

	// Get parent element body to insert new child (root navig element has no body):
	var elBody = elParent.m_elFBody;
	if (elBody == null)
		elBody = elParent;

	elBody.removeChild(i_elFolder);
}

function g_NavigScrollCurrent()
{
	if (g_elCurFolder)
		g_elCurFolder.scrollIntoView();
}

function g_FolderSetStatusPath(i_status, i_path, i_up_params)
{
	var elFolder = g_elFolders[i_path];
	if (elFolder)
		g_FolderSetStatus(i_status, elFolder, i_up_params);
}

function g_FolderSetStatus(i_status, i_elFolder, i_up_params)
{
	if (i_elFolder == null)
		i_elFolder = g_elCurFolder;
	if (i_elFolder.m_fobject.status == null)
		i_elFolder.m_fobject.status = {};

	if (i_up_params)
		// Update only specified parameters:
		for (let parm in i_up_params)
		{
			if (parm == 'tasks')
			{
				// On tasks, update each task
				if (null == i_elFolder.m_fobject.status.tasks)
					i_elFolder.m_fobject.status.tasks = {};
				for (let t in i_status.tasks)
					i_elFolder.m_fobject.status.tasks[t] = i_status.tasks[t];
			}
			else
				i_elFolder.m_fobject.status[parm] = i_status[parm];
		}
	else
		i_elFolder.m_fobject.status = i_status;

	if ((i_up_params == null) || i_up_params.annotation)
		st_SetElLabel(i_status, i_elFolder.m_elAnn);
	if ((i_up_params == null) || i_up_params.color)
		st_SetElColor(i_status, i_elFolder.m_elFBody);
	if ((i_up_params == null) || i_up_params.artists)
		st_SetElArtists(i_status, i_elFolder.m_elArtists, true);
	if ((i_up_params == null) || i_up_params.duration)
		st_SetElDuration(i_status, i_elFolder.m_elDuration);
	if ((i_up_params == null) || i_up_params.price)
		st_SetElPrice(i_status, i_elFolder.m_elPrice);
	if ((i_up_params == null) || i_up_params.tags)
		st_SetElTags(i_status, i_elFolder.m_elTags, true);
	if ((i_up_params == null) || i_up_params.flags)
		st_SetElFlags(i_status, i_elFolder.m_elFlags, true);
	if ((i_up_params == null) || i_up_params.tasks)
		task_DrawBadges(i_status, i_elFolder.m_elTasks, {'update':(i_up_params && i_up_params.tasks)});

	if (i_elFolder.m_fobject.auxiliary)
	{
		i_elFolder.m_elFrames.style.display = 'none';
		i_elFolder.m_elPercent.style.display = 'none';
		i_elFolder.m_elProgress.style.display = 'none';
		return;
	}

	if ((i_up_params == null) || i_up_params.frames_num)
		st_SetElFramesNum(i_status, i_elFolder.m_elFrames, false);
	if ((i_up_params == null) || i_up_params.progress)
	{
		st_SetElProgress(
			i_status, i_elFolder.m_elProgressBar, i_elFolder.m_elProgress, i_elFolder.m_elPercent);
	}
}

function g_SortAuxBottom()
{
	if (localStorage.sort_aux_bottom == 'OFF')
		localStorage.sort_aux_bottom = 'ON';
	else
		localStorage.sort_aux_bottom = 'OFF';
	$('sort_aux_bottom').textContent = localStorage.sort_aux_bottom;
	g_sort_aux_bottom = (localStorage.sort_aux_bottom == 'ON')
}
function g_CompareFolders(a, b)
{
	// Compare folders sizes if size is shown in the navigation:
	if (g_navig_infos.current.navig_show_size)
	{
		if ((a.space != null) && (b.space == null))
			return -1;
		if ((b.space != null) && (a.space == null))
			return 1;
		if ((a.space != null) && (b.space != null))
		{
			if (a.space < b.space)
				return 1;
			else if (a.space > b.space)
				return -1;
		}
	}

	// Move COMMON folder to top:
	let a_cmn = c_PathBase(a.name) == 'COMMON';
	let b_cmn = c_PathBase(b.name) == 'COMMON';

	if (a_cmn && (false == b_cmn))
		return -1;
	if (b_cmn && (false == a_cmn))
		return 1;

	// Move auxiliary folders to bottom:
	if (g_sort_aux_bottom)
	{
		let a_aux = c_AuxFolder(a);
		let b_aux = c_AuxFolder(b);

		if (a_aux && (false == b_aux))
			return 1;
		if (b_aux && (false == a_aux))
			return -1;
	}

	// Compare folders names:
	if (a.name < b.name)
		return -1;
	if (a.name > b.name)
		return 1;

	return 0;
}

function g_CloseFolder(i_elFolder)
{
	if (false == i_elFolder.classList.contains('opened'))
		return;

	if (i_elFolder.m_elFolders && i_elFolder.m_elFolders.length)
	{
		for (let i = 0; i < i_elFolder.m_elFolders.length; i++)
		{
			let elFolder = i_elFolder.m_elFolders[i];
			elFolder.parentNode.removeChild(elFolder);
		}

		for (let i = 0; i < i_elFolder.m_elGroups.length; i++)
		{
			let elGroup = i_elFolder.m_elGroups[i];

			// For now parent folder remove all child foders from any parent
			//for (let f = 0; f < elGroup.m_elFolders.length; f++)
			//	elGroup.removeChild(elGroup.m_elFolders[f]);

			elGroup.parentNode.removeChild(elGroup);
		}
	}

	i_elFolder.m_elFolders = [];
	i_elFolder.m_elGroups = [];
	i_elFolder.classList.remove('opened');
}

function g_FolderOnClick(i_evt, i_double)
{
	i_evt.stopPropagation();
	var elFolder = i_evt.currentTarget;
	if (elFolder.classList.contains('dummy'))
		return;

	if (elFolder.classList.contains('current'))
	{
		if (elFolder.classList.contains('opened'))
			g_CloseFolder(elFolder);
		else
			g_OpenFolder(elFolder);
		return;
	}

	g_nav_clicked = true;
	window.location.hash = elFolder.m_path;
}

function g_FolderOnDblClick(i_evt)
{
	// console.log('Double Clicked');
	// g_FolderOnClick( i_evt, true);
	// return;
	i_evt.stopPropagation();
	var elFolder = i_evt.currentTarget;

	if (elFolder.classList.contains('opened'))
		g_CloseFolder(elFolder);
	else
		g_OpenFolder(elFolder);
}

function g_NavigShowInfo(i_toggle)
{
	for (var i = 0; i < g_navig_infos.all.length; i++)
	{
		var name = 'navig_show_' + g_navig_infos.all[i];
		if (localStorage[name] == null)
		{
			if (g_navig_infos.default.indexOf(g_navig_infos.all[i]) != -1)
			{
				localStorage[name] = 'true';
				g_navig_infos.current[name] = true;
			}
			else
			{
				localStorage[name] = 'false';
				g_navig_infos.current[name] = false;
			}
		}

		if (i_toggle && (i_toggle.id == name))
		{
			if (localStorage[name] == 'true')
			{
				localStorage[name] = 'false';
				g_navig_infos.current[name] = false;
			}
			else
			{
				localStorage[name] = 'true';
				g_navig_infos.current[name] = true;
			}
		}

		if (localStorage[name] == 'true')
		{
			$(name).classList.add('selected');
			u_el.navig.classList.add(name);
			g_navig_infos.current[name] = true;
		}
		else
		{
			$(name).classList.remove('selected');
			u_el.navig.classList.remove(name);
			g_navig_infos.current[name] = false;
		}
	}
}
