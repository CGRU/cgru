sc_elShots = null;
sc_elScenes = null;
sc_elCurShot = null;

sc_elImgThumbs = [];

sc_thumb_params = {};
sc_thumb_params.force_update = {"width":'30%',"lwidth":'150px','type':"bool",'default':false,"tooltip":'Update or skip existing thumbnails'};
sc_thumb_params.skip_movies = {"width":'30%',"lwidth":'150px','type':"bool",'default':true,"tooltip":'Do not create thumbnails from movie files'};

sc_thumb_params_values = {};

sc_area = false;
sc_show_aux = false;
sc_show_omit = false;

if (ASSETS.scene && (ASSETS.scene.path == g_CurPath()))
{
	a_SetLabel('Scene');
	sc_area = false;

	sc_Init();
}

if (ASSETS.area && (ASSETS.area.path == g_CurPath()))
{
	a_SetLabel('Area');
	sc_area = true;

	sc_Init();
}

function sc_Init()
{
	sc_elShots = [];
	sc_elScenes = [];

	n_GetFile({"path":'rules/assets/scenes.html',"func":sc_InitHTML,"info":'scenes.html',"parse":false});
}
function sc_InitHTML( i_data)
{
	$('asset').innerHTML = i_data;

	// Show top buttons:
	if (c_CanCreateShot())
	{
		let el = document.createElement('div');
		$('asset_top_left').appendChild(el);
		el.classList.add('button');
		el.textContent = 'NEW SCENE';
		el.title = 'Create new scene.';
		el.onclick = sc_Create_Scene;

		if (sc_area == false)
		{
			el = document.createElement('div');
			$('asset_top_left').appendChild(el);
			el.classList.add('button');
			el.textContent = 'NEW SHOT';
			el.title = 'Create new shot.';
			el.onclick = sc_Create_Shot;
		}
	}

	let elBtnShowAux = document.createElement('div');
	elBtnShowAux.textContent = 'Show Aux';
	elBtnShowAux.classList.add('button');
	$('asset_top_left').appendChild(elBtnShowAux);
	elBtnShowAux.onclick = function(e){
		let el = e.currentTarget;
		c_ElToggleSelected(el);
		sc_show_aux = el.m_selected;
	}


	let elBtnShowOmit = document.createElement('div');
	elBtnShowOmit.textContent = 'Show Omit';
	elBtnShowOmit.classList.add('button');
	$('asset_top_left').appendChild(elBtnShowOmit);
	elBtnShowOmit.onclick = function(e){
		let el = e.currentTarget;
		c_ElToggleSelected(el);
		sc_show_omit = el.m_selected;
	}


	gui_Create( $('scenes_make_thumbnails'), sc_thumb_params);
	gui_CreateChoices({"wnd":$('scenes_make_thumbnails'),"name":'colorspace',"value":RULES.thumbnail.colorspace,"label":'Colorspace:',"keys":RULES.dailies.colorspaces});

	if (sc_area)
	{
		$('scenes_load_btn').textContent = 'Load All Shots';
		$('scenes_load_btn').onclick = function()
		{
			sc_Show_Loaded();
			scenes_Show();
		}
	}
	else
	{
		$('scenes_load_btn').textContent = 'Show Scene Shots';
		$('scenes_load_btn').onclick = function()
		{
			sc_Show_Loaded();
			scene_Show();
		}
	}
}
function sc_AuxFolder(i_fobj)
{
	if (c_AuxFolder(i_fobj))
	{
		if (sc_show_aux)
		{
			if (i_fobj && i_fobj.status && i_fobj.status.flags && (i_fobj.status.flags.indexOf('aux') != -1))
				return false;
		}
		if (sc_show_omit)
		{
			if (i_fobj && i_fobj.status && i_fobj.status.flags && (i_fobj.status.flags.indexOf('omit') != -1))
				return false;
		}
		return true;
	}
	return false;
}
function sc_Create_Shot()
{
	let args = {};
	args.title = 'Create New Shot';
	args.template = RULES.assets.shot.template;
	args.destination = g_CurPath();
	args.name = ASSET.name + '_SHOT_0010';
	a_Copy(args);
}

function sc_Create_Scene()
{
	let args = {};
	args.title = 'Create New Scene';
	args.template = RULES.assets.scene.template;
	args.destination = g_CurPath();

	if (sc_area)
	{
		args.name = 'SCENE_01';
	}
	else
	{
		args.destination = c_PathDir(args.destination);

		// Try to increment latest number in name:
		// Find all numbers
		let numbers = ASSET.name.match(/\d+/g);
		if ((numbers != null) && numbers.length)
		{
			let number = numbers[numbers.length-1];
			let numplus = '' + (parseInt(number) + 1);
			// Apply padding
			while (numplus.length < number.length)
				numplus = '0' + numplus;
			// Lenght may be bigger on 'SHOT_990'
			if (numplus.length == number.length)
				args.name = ASSET.name.replace(number, numplus);
		}
	}

	a_Copy(args);
}

function sc_Show_Loaded()
{
	$('scenes_load_btn').style.display = 'none';
	$('scenes_show_loaded').style.display = 'block';
}

function sc_Post()
{
	if( g_arguments )
		if( g_arguments.s_Search )
			s_Found(sc_FilterShots( g_arguments.s_Search));
}

function scene_Show()
{
	var folders = g_elCurFolder.m_dir.folders;
	for( var f = 0; f < folders.length; f++ )
	{
		if (sc_AuxFolder(folders[f]))
		{
			continue;
		}

		var path = g_elCurFolder.m_path + '/' + folders[f].name;

		var elShot = document.createElement('div');
		sc_elShots.push( elShot);
		elShot.m_filtered = false;
		elShot.m_status = folders[f].status;
		elShot.m_path = path;
		$('scenes_div').appendChild( elShot);
		elShot.style.padding = '4px';
		elShot.classList.add('shot');

		// Thumbnail:
		var elImg = document.createElement('img');
		elShot.appendChild( elImg);
		elImg.classList.add('thumbnail');
		elImg.m_path = path;
		elImg.m_src = RULES.root + path + '/' + RULES.rufolder + '/' + RULES.thumbnail.filename;
		sc_elImgThumbs.push( elImg);
		elImg.src = elImg.m_src;

		// Right float div:
		var elDiv = document.createElement('div');
		elShot.appendChild( elDiv);
		elDiv.classList.add('name_body');
		// Name:
		var elName = document.createElement('a');
		elDiv.appendChild( elName);
		elName.href = '#' + path;
		elName.textContent = folders[f].name;
		// Body:
		if( g_auth_user )
		{
			var elBodyEditBtn = document.createElement('div');
			elDiv.appendChild( elBodyEditBtn);
			elBodyEditBtn.classList.add('button');
			elBodyEditBtn.classList.add('edit');
			elBodyEditBtn.title = 'Edit shot body.';
			elBodyEditBtn.m_elShot = elShot;
			elBodyEditBtn.onclick = sc_EditBody;
			elShot.m_elBodyEditBtn = elBodyEditBtn;

			var elBodyEditPanel = document.createElement('div');
			elDiv.appendChild( elBodyEditPanel);
			elBodyEditPanel.classList.add('edit_panel');
			elBodyEditPanel.style.display = 'none';
			elShot.m_elBodyEditPanel = elBodyEditPanel;

			var elBodyCancelBtn = document.createElement('div');
			elBodyEditPanel.appendChild( elBodyCancelBtn);
			elBodyCancelBtn.classList.add('button');
			elBodyCancelBtn.innerHTML = 'CANCEL <small>[ESC]</small>.';
			elBodyCancelBtn.title = 'Cancel shot body editing.';
			elBodyCancelBtn.m_elShot = elShot;
			elBodyCancelBtn.onclick = sc_EditBodyCancel;

			var elBodySaveBtn = document.createElement('div');
			elBodyEditPanel.appendChild( elBodySaveBtn);
			elBodySaveBtn.classList.add('button');
			elBodySaveBtn.innerHTML = '<b>SAVE</b> <small>[CTRL+ENTER]</small>.';
			elBodySaveBtn.title = 'Save shot body.';
			elBodySaveBtn.m_elShot = elShot;
			elBodySaveBtn.onclick = sc_EditBodySave;
		}

		elShot.m_elBody = document.createElement('div');
		elShot.m_elBody.onclick = function(i_e){i_e.stopPropagation();}
		elDiv.appendChild( elShot.m_elBody);
		elShot.m_elBody.classList.add('body');
		elShot.m_elBody.m_elShot = elShot;

		// Elements for status:
		var elSt = {};

		var elFramesNumDiv = document.createElement('div');
		elDiv.insertBefore( elFramesNumDiv, elName);
		elFramesNumDiv.classList.add('frames_num_div');
		elSt.elFramesNum = document.createElement('div');
		elFramesNumDiv.appendChild( elSt.elFramesNum);
		elSt.elFramesNum.classList.add('frames_num');
		var elFramesNumLabel = document.createElement('div');
		elFramesNumDiv.appendChild( elFramesNumLabel);
		elFramesNumLabel.textContent = 'F:';
		elFramesNumLabel.classList.add('frames_num_label');

		// Timecode:
		elSt.elTimeCode = document.createElement('div');
		elDiv.insertBefore( elSt.elTimeCode, elName);
		elSt.elTimeCode.classList.add('timecode');

		elSt.elFinish = document.createElement('div');
		elShot.appendChild( elSt.elFinish);

		elShot.m_elStatus = document.createElement('div');
		elShot.appendChild(elShot.m_elStatus);
		elShot.m_elStatus.classList.add('status');

		elSt.elShow = document.createElement('div');
		elShot.m_elStatus.appendChild(elSt.elShow);
		elSt.elShow.classList.add('status_show');

		elSt.elShowCompact = document.createElement('div');
		elSt.elShowCompact.classList.add('status_show_compact');
		elSt.elShow.appendChild(elSt.elShowCompact);

		elSt.elProgress = document.createElement('div');
		elSt.elShowCompact.appendChild(elSt.elProgress);
		elSt.elProgress.classList.add('progress');
		elSt.elProgressBar = document.createElement('div');
		elSt.elProgress.appendChild( elSt.elProgressBar);
		elSt.elProgressBar.classList.add('progressbar');

		let elEditBtn = document.createElement('div');
		elSt.elShowCompact.appendChild( elEditBtn);
		elEditBtn.classList.add('button');
		elEditBtn.classList.add('edit');
		elEditBtn.title = 'Edit status.\nSelect several shots to edit.';
		if( g_auth_user == null )
			elEditBtn.style.display = 'none';

		elSt.elPercentage = document.createElement('div');
		elSt.elShowCompact.appendChild(elSt.elPercentage);
		elSt.elPercentage.classList.add('percent');

		elSt.elAnnotation = document.createElement('div');
		elSt.elShowCompact.appendChild(elSt.elAnnotation);
		elSt.elAnnotation.classList.add('annotation');

		elSt.elFlags = document.createElement('div');
		elSt.elShowCompact.appendChild(elSt.elFlags);
		elSt.elFlags.classList.add('flags');

		elSt.elTasksBadges = document.createElement('div');
		elSt.elShowCompact.appendChild(elSt.elTasksBadges);
		elSt.elTasksBadges.classList.add('tasks_badges');

		elSt.elTags = document.createElement('div');
		elSt.elShowCompact.appendChild( elSt.elTags);
		elSt.elTags.classList.add('tags');

		elSt.elArtists = document.createElement('div');
		elSt.elShowCompact.appendChild(elSt.elArtists);
		elSt.elArtists.classList.add('artists');

		elSt.elTasks = document.createElement('div');
		elSt.elShow.appendChild(elSt.elTasks);
		elSt.elTasks.classList.add('tasks');
		elSt.elTasks.onclick = function(e){e.stopPropagation();return false;}

		function sc_CreateSceneShot(i_status)
		{
			for (let el in elSt)
				i_status[el] = elSt[el];

			i_status.elColor = elShot;
			i_status.elParent = elShot;
		}

		if (folders[f].status && folders[f].status.flags)
			for(let fl = 0; fl < folders[f].status.flags.length; fl++)
				elShot.classList.add(folders[f].status.flags[fl]);

		let st_obj = new Status( folders[f].status, {"path":path,"createGUI": sc_CreateSceneShot,'multi':true,'tasks_badges':true});
		elShot.m_status = st_obj;
        st_Statuses[path] = st_obj;
		elEditBtn.m_status = st_obj;
		elEditBtn.onclick = sc_EditStatus;

		elShot.onclick = sc_ShotClicked;
	}

	sc_DisplayStatistics();
	sc_Post();

	// Get scene shots bodies:
	for( var i = 0; i < sc_elShots.length; i++)
	{
		var path = sc_elShots[i].m_path;
		path = RULES.root + path + '/' + RULES.rufolder + '/' + u_body_filename;
		n_GetFile({"path":path,"func":sc_BodyReceived,"info":'scene_bodies',"elShot":sc_elShots[i],"parse":false});
	}
}
function sc_BodyReceived( i_data, i_args)
{
	if( i_data.indexOf('No such file: ' + RULES.root) != -1 ) return;

	// Replace <br> with spaces through some pattern:
	i_args.elShot.m_elBody.innerHTML = i_data.replace(/\<\s*br\s*\/?\s*\>/g,'@@BR@@');
	i_args.elShot.m_elBody.innerHTML = i_args.elShot.m_elBody.textContent.replace(/@@BR@@/g,' ');

	if (i_args.elShot.m_status && i_args.elShot.m_status.obj)
	{
		if (i_args.elShot.m_status.obj.body == null)
			i_args.elShot.m_status.obj.body = {};
		i_args.elShot.m_status.obj.body.data = i_data;
	}
}

function scenes_Show()
{
	n_WalkDir({"paths":[ASSET.path],"wfunc":scenes_Received,"info":'walk area',"depth":1,"rufiles":['rules','status'],"lookahead":['status'],"local":true});
	$('asset').classList.add('waiting');
	scenes_elWaiting = document.createElement('div');
	scenes_elWaiting.innerHTML = '<h3 style="text-align:center">Loading all project shots...</h3>';
	$('asset').appendChild( scenes_elWaiting);
}

function scenes_Received( i_data, i_args)
{
	$('asset').classList.remove('waiting');
	$('asset').removeChild( scenes_elWaiting);
	
	var walk = i_data[0];
	if( walk.folders == null ) return;
	walk.folders.sort( c_CompareFiles );

	for( var sc = 0; sc < walk.folders.length; sc++)
	{
		var fobj = walk.folders[sc];
		if (sc_AuxFolder(fobj))
			continue;

		var elScene = document.createElement('div');
		sc_elScenes.push( elScene);
		elScene.m_filtered = false;
		elScene.m_elThumbnails = [];
		elScene.m_elShots = [];
		$('scenes_div').appendChild( elScene);
		elScene.classList.add('scene');
		elScene.m_path = ASSET.path + '/' + fobj.name;
		elScene.onclick = function(e){scenes_SelectScene(e.currentTarget)};

		var elName = document.createElement('a');
		elScene.appendChild( elName);
		elName.classList.add('name');
		elName.textContent = fobj.name;
		elName.href = '#'+elScene.m_path;

		var elStatus = document.createElement('div');
		elScene.appendChild( elStatus);
		elStatus.classList.add('status');
		st_SetElLabel( fobj.status, elStatus);
		st_SetElColor( fobj.status, elScene);

		if( walk.folders[sc].folders == null ) continue;

		walk.folders[sc].folders.sort( c_CompareFiles );
		for( var s = 0; s < walk.folders[sc].folders.length; s++)
		{
			var fobj = walk.folders[sc].folders[s];
			if (sc_AuxFolder(fobj))
				continue;

			var elShot = document.createElement('div');
			sc_elShots.push( elShot);
			elScene.m_elShots.push( elShot);
			elShot.m_status = fobj.status;
			elShot.m_filtered = false;
			elScene.appendChild( elShot);
			elScene.m_elThumbnails.push( elShot);
			elShot.classList.add('shot');
			elShot.m_path = elScene.m_path + '/' + fobj.name;

			var elImg = document.createElement('img');
			elShot.appendChild( elImg);
			elImg.m_path = elShot.m_path;
			elImg.m_src = RULES.root + elShot.m_path +'/'+ RULES.rufolder +'/'+ RULES.thumbnail.filename;
			sc_elImgThumbs.push( elImg);
			elImg.src = elImg.m_src;

			var elSt = {};

			elSt.elShow = document.createElement('div');
			elShot.appendChild( elSt.elShow);
			elSt.elShow.classList.add('status','status_show','status_show_compact');

			elSt.elAnnotation = document.createElement('div');
			elSt.elShow.appendChild( elSt.elAnnotation);
			elSt.elAnnotation.classList.add('annotation');

			elSt.elPercentage = document.createElement('div');
			elSt.elShow.appendChild( elSt.elPercentage);
			elSt.elPercentage.classList.add('percent');

			var elName = document.createElement('a');
			elSt.elShow.appendChild( elName);
			elName.classList.add('name');
			elName.textContent = fobj.name;
			elName.href = '#'+elShot.m_path;

			elSt.elProgress = document.createElement('div');
			elSt.elShow.appendChild( elSt.elProgress);
			elSt.elProgress.classList.add('progress');
			elSt.elProgressBar = document.createElement('div');
			elSt.elProgress.appendChild( elSt.elProgressBar);
			elSt.elProgressBar.classList.add('progressbar');

			elSt.elFlags = document.createElement('div');
			elSt.elShow.appendChild( elSt.elFlags);
			elSt.elFlags.classList.add('flags');

			elSt.elTasksBadges = document.createElement('div');
			elSt.elShow.appendChild(elSt.elTasksBadges);
			elSt.elTasksBadges.classList.add('tasks_badges');

			elSt.elTags = document.createElement('div');
			elSt.elShow.appendChild( elSt.elTags);
			elSt.elTags.classList.add('tags');

			elSt.elArtists = document.createElement('div');
			elSt.elShow.appendChild( elSt.elArtists);
			elSt.elArtists.classList.add('artists');

/*			elSt.elFinish = document.createElement('div');
			elSt.elShow.appendChild( elSt.elFinish);
			elSt.elFinish.classList.add('finish');*/

			function sc_CreateAreaShot( i_status)
			{
				for( var el in elSt ) i_status[el] = elSt[el];
				i_status.elParent = elShot;
				i_status.elColor = elShot;
				i_status.elShow = elSt.elShow;
			}

			if( fobj.status && fobj.status.flags )
				for( var f = 0; f < fobj.status.flags.length; f++)
					elShot.classList.add( fobj.status.flags[f]);

			let st_obj = new Status(fobj.status, {"path":elShot.m_path,"createGUI": sc_CreateAreaShot,"display_short":true,'tasks_badges':true});
			elShot.m_status = st_obj;
            st_Statuses[elShot.m_path] = st_obj;

			elShot.ondblclick = sc_EditStatus;
			elShot.onclick = sc_ShotClicked;
		}
	}

	sc_DisplayStatistics();
	sc_Post();
}

function sc_EditStatus( e)
{
	e.stopPropagation();

	var status = e.currentTarget.m_status;
	var shots = scenes_GetSelectedShots();
	var statuses = [];
	for( var i = 0; i < shots.length; i++)
		if( shots[i].m_status != status )
			statuses.push( shots[i].m_status);

	status.edit({"statuses":statuses});

	return false;
}

function sc_EditTask(e)
{
	e.stopPropagation();

	let task = e.currentTarget.m_task;
	let shots = scenes_GetSelectedShots();
	let paths = [task.statusClass.path];
	for (let shot of shots)
	{
		let path = shot.m_status.path;
		if (false == paths.includes(path))
			paths.push(path);
	}

	task.edit({"paths":paths});

	return false;
}

function sc_AddTask(i_status)
{
	let shots = scenes_GetSelectedShots();
	let paths = [i_status.path];
	for (let shot of shots)
	{
		let path = shot.m_status.path;
		if (false == paths.includes(path))
			paths.push(path);
	}

	new Task(i_status, /* task = */null/* as it is a new task*/,{"paths":paths});
}

function sc_EditBody( i_e)
{
	i_e.stopPropagation();

	var el = i_e.currentTarget.m_elShot;

	el.m_elBody.m_text = el.m_elBody.textContent;
	el.m_elBody.classList.add('editing');
	el.m_elBody.contentEditable = 'true';
	el.m_elBodyEditBtn.style.display = 'none';
	el.m_elBodyEditPanel.style.display = 'block';

	el.m_elBody.onkeydown = function( i_e)
	{
		if(( i_e.keyCode == 13 ) && i_e.ctrlKey ) // CTRL + ENTER
		{
			sc_EditBodySave( i_e);
			i_e.currentTarget.blur();
		}
		if( i_e.keyCode == 27 ) // ESC
		{
			sc_EditBodyCancel( i_e);
			i_e.currentTarget.blur();
		}
	}

	return false;
}

function sc_EditBodyCancel( i_e)
{
	i_e.stopPropagation();

	var el = i_e.currentTarget.m_elShot;

	el.m_elBody.textContent = el.m_elBody.m_text;
	el.m_elBody.classList.remove('editing');
	el.m_elBody.contentEditable = 'false';
	el.m_elBodyEditBtn.style.display = 'block';
	el.m_elBodyEditPanel.style.display = 'none';
	el.m_elBody.onkeydown = null;

	return false;
}

function sc_EditBodySave( i_e)
{
	var el = i_e.currentTarget.m_elShot;
	var text = el.m_elBody.textContent;

	sc_EditBodyCancel( i_e);

	var shots = scenes_GetSelectedShots();
	if( shots.indexOf( el) == -1 )
		shots.push( el);

	var news = [];

	for( var i = 0; i < shots.length; i++)
	{
		if( shots[i].m_status.obj == null )
			shots[i].m_status.obj = {};

		n_Request({"send":{"save":{"file":c_GetRuFilePath( u_body_filename, shots[i].m_path),"data":text}},
		"func":sc_EditBodyFinished,"elShot":shots[i],"info":'body save'});

		news.push( nw_CreateNews({'title':'body','path':shots[i].m_path,'artists':shots[i].m_status.obj.artists}));

		st_BodyModified(shots[i].m_status.obj, shots[i].m_path);
	}

	nw_SendNews( news);

	return false;
}
function sc_EditBodyFinished( i_data, i_args)
{
	var shot = i_args.elShot;
	var path = c_GetRuFilePath( u_body_filename, shot.m_path);
	n_GetFile({"path":path,"func":sc_BodyReceived,"info":'scene_bodies',"elShot":shot,
		"cache_time":-1,"parse":false});
}

function sc_ShotClicked( i_evt)
{
	i_evt.stopPropagation();
	var el = i_evt.currentTarget;
	sc_SelectShot( el, el.m_selected !== true );
	if( i_evt.shiftKey && sc_elCurShot )
	{
		var i_p = sc_elShots.indexOf( sc_elCurShot );
		var i_c = sc_elShots.indexOf( el);
		if( i_p != i_c )
		{
			var select = false;
			if( el.m_selected ) select = true;
			var step = 1;
			if( i_c < i_p ) step = -1;
			while( i_p != i_c )
			{
				sc_SelectShot( sc_elShots[i_p], select);
				i_p += step;
			}
		}
	}
	sc_elCurShot = el;

//	c_Info( scenes_GetSelectedShots().length + ' shots selected.');
	sc_DisplayStatistics();
}
function sc_SelectShot( i_elShot, i_select)
{
	if( i_select )
	{
		i_elShot.m_selected = true;
		i_elShot.classList.add('selected');
	}
	else
	{
		i_elShot.m_selected = false;
		i_elShot.classList.remove('selected');
	}
}
function scenes_SelectAll( i_select)
{
	for( var i = 0; i < sc_elShots.length; i++)
		sc_SelectShot( sc_elShots[i], i_select);

	sc_DisplayStatistics();
}
function scenes_SelectInvert()
{
	for( var i = 0; i < sc_elShots.length; i++)
		sc_SelectShot( sc_elShots[i], sc_elShots[i].m_selected != true);

	sc_DisplayStatistics();
}
function scenes_SelectSameColor()
{
	var sel = scenes_GetSelectedShots()
	if( sel.length == 0 )
	{
		c_Error('No shots selected.');
		return;
	}

	var clr = null;
	if( sel[0].m_status && sel[0].m_status.obj && sel[0].m_status.obj.color )
		clr = sel[0].m_status.obj.color;

	scenes_SelectAll( false);

	for( var i = 0; i < sc_elShots.length; i++)
	{
		var c = null;
		var s = sc_elShots[i];
		if( s.m_status && s.m_status.obj && s.m_status.obj.color )
			c = s.m_status.obj.color;

		if( c && c.length && clr && clr.length )
		{
			if(( clr[0] == c[0] ) && ( clr[1] == c[1] ) && ( clr[2] == c[2] ))
				sc_SelectShot( s, true);
		}
		else if(( c == null ) && ( clr == null ))
			sc_SelectShot( s, true);
	}

	sc_DisplayStatistics();
}
function scenes_SelectPlaylist()
{
	scenes_SelectAll( false);
	var shots = p_GetCurrentShots();
	for( var i = 0; i < sc_elShots.length; i++)
		if( shots.indexOf( sc_elShots[i].m_path) != -1 )
			sc_SelectShot( sc_elShots[i], true);

	sc_DisplayStatistics();
}
function scenes_SelectScene( i_elScene)
{
	var shots = i_elScene.m_elShots;
	if( shots.length == 0 )
		return;

	var select = shots[0].m_selected != true;

	for( var i = 0; i < shots.length; i++)
	{
		sc_SelectShot( shots[i], select);
	}

	sc_DisplayStatistics();
}

function scenes_GetSelectedShots()
{
	var shots = [];
	for( var i = 0; i < sc_elShots.length; i++)
	{
		if( sc_elShots[i].m_selected != true )
			continue;

		if( sc_elShots[i].m_filtered )
			continue;

		shots.push( sc_elShots[i]);
	}
	return shots;
}

function sc_FilterShots(i_args)
{
	let o_res = {};
	o_res.artists = [];
	o_res.flags = [];
	o_res.tags = [];

	if (sc_elShots == null)
		return;

	if (i_args == null)
		i_args = {};

	// Prepare special flags and tags:
	let flags_and = false;
	let flags_tsk = false;
	if (i_args.flags)
	{
		let index = i_args.flags.indexOf('_AND_');
		if (index != -1)
		{
			flags_and = true;
			i_args.flags.splice(index, 1);
		}
		index = i_args.flags.indexOf('_TSK_');
		if (index != -1)
		{
			flags_tsk = true;
			i_args.flags.splice(index, 1);
		}
	}

	let tags_and  = false;
	let tags_tsk  = false;
	if (i_args.tags)
	{
		let index = i_args.tags.indexOf('_AND_');
		if (index != -1)
		{
			tags_and = true;
			i_args.tags.splice(index, 1);
		}
		index = i_args.tags.indexOf('_TSK_');
		if (index != -1)
		{
			tags_tsk = true;
			i_args.tags.splice(index, 1);
		}
	}

	let anns = null;
	if (i_args.ann )
	{
		let anns_or = i_args.ann.split('|');
		anns = [];
		for (let o = 0; o < anns_or.length; o++)
			anns.push( anns_or[o].split('+'));
	}

	let bodies = null;
	if (i_args.body)
	{
		let bodies_or = i_args.body.split('|');
		bodies = [];
		for (let o = 0; o < bodies_or.length; o++)
			bodies.push(bodies_or[o].split('+'));
	}

	for (let th = 0; th < sc_elShots.length; th++)
	{
		let found = (i_args == null);

		let el = sc_elShots[th];
		let st_obj = {};
		if (el.m_status && el.m_status.obj)
			st_obj = c_CloneObj(el.m_status.obj);
			//st_obj = Object.assign({}, el.m_status.obj);

		// Join status with tasks
		if (st_obj.tasks)
		{
			let keys = ['artists'];
			// Join flags or tags, never join and flags and tags.
			// If task search enabled for both tags and flags,
			// we should search tasks, not just join shot status with tasks status.
			if (false == (flags_tsk && tags_tsk))
			{
				if (flags_tsk)
					keys.push('flags');
				if (tags_tsk)
					keys.push('tags');
			}

			for (let t in st_obj.tasks)
			{
				let task = st_obj.tasks[t];
				if (task.deleted)
				{
					found = false;
					continue;
				}

				for (let key of keys)
					if (task[key])
					{
						if (null == st_obj[key])
							st_obj[key] = [];
						for (let val of task[key])
							if (st_obj[key].indexOf(val) == -1)
								st_obj[key].push(val);
					}

				// Calc maximum priority, but task should be completed:
				if ((task.priority != null) && (task.progress < 100) && (task.flags.indexOf('done') == -1))
				{
					if (st_obj.priority == null)
					{
						st_obj.priority = task.priority;
					}
					else
					{
						if (task.priority > st_obj.priority)
							st_obj.priority = task.priority;
					}
				}
			}
		}

		if (anns)
		{
			if (st_obj.annotation)
				for (let o = 0; o < anns.length; o++)
				{
					let found_and = true;
					for (let a = 0; a < anns[o].length; a++)
					{
						if (st_obj.annotation.toLowerCase().indexOf(anns[o][a].toLowerCase()) == -1)
						{
							found_and = false;
							break;
						}
					}
					if (found_and)
					{
						found = true;
						break;
					}
				}
		}
		else found = true;

		if (bodies && found)
		{
			found = false;
			if (st_obj.body && st_obj.body.data)
				for (let o = 0; o < bodies.length; o++)
				{
					let found_and = true;
					for (let b = 0; b < bodies[o].length; b++)
					{
						if (st_obj.body.data.toLowerCase().indexOf(bodies[o][b].toLowerCase()) == -1)
						{
							found_and = false;
							break;
						}
					}
					if (found_and)
					{
						found = true;
						break;
					}
				}
		}

		if (i_args.flags && i_args.flags.length && found)
		{
			found = false;
			if (st_obj.flags && st_obj.flags.length)
			{
				for (let f of i_args.flags)
				{
					// skip special flags
					if (f.charAt(0) == '_') continue;

					if (st_obj.flags.includes(f))
					{
						found = true;
						if (false == flags_and)
							break;
					}
					else
					{
						found = false;
						if (flags_and)
							break;
					}
				}
			}
			else if (i_args.flags.indexOf('_null_') != -1)
				found = true;
		}

		if (i_args.tags && i_args.tags.length && found)
		{
			found = false;
			if (st_obj.tags && st_obj.tags.length)
			{
				for (let t of i_args.tags)
				{
					// skip special tags
					if (t.charAt(0) == '_') continue;

					if (st_obj.tags.includes(t))
					{
						found = true;
						if (false == tags_and)
							break;
					}
					else
					{
						found = false;
						if (tags_and)
							break;
					}
				}
			}
			else if (i_args.tags.indexOf('_null_') != -1)
				found = true;
		}

		// Search tasks:
		if (flags_tsk && tags_tsk)
		{
			if (st_obj.tasks)
			{
				for (let t in st_obj.tasks)
				{
					let task = st_obj.tasks[t];
					if (task.deleted)
					{
						found = false;
						continue;
					}
					found = true;

					if (found && i_args.flags && i_args.flags.length)
					{
						found = false;
						let flags = [];
						// Join this task flags with status flags
						if (st_obj.flags && st_obj.flags.length)
							flags = flags.concat(st_obj.flags);
						if (task.flags && task.flags.length)
							flags = flags.concat(task.flags);

						for (let i = 0; i < i_args.flags.length; i++)
						{
							if (flags.includes(i_args.flags[i]))
							{
								found = true;
								if (false == flags_and)
									break;
							}
							else
							{
								found = false;
								if (flags_and)
									break;
							}
						}
					}

					if (found && i_args.tags && i_args.tags.length)
					{
						found = false;
						let tags = [];
						// Join this task tags with status tags
						if (st_obj.tags && st_obj.tags.length)
							tags = tags.concat(st_obj.tags);
						if (task.tags && task.tags.length)
							tags = tags.concat(task.tags);

						for (let i = 0; i < i_args.tags.length; i++)
						{
							if (tags.includes(i_args.tags[i]))
							{
								found = true;
								if (false == tags_and)
									break;
							}
							else
							{
								found = false;
								if (tags_and)
									break;
							}
						}
					}

					// We found one task matching flags and tags.
					// One matching task it is enough to show entire shot.
					if (found)
						break;
				}
			}
			else
				found = false;
		}

		if (i_args.artists && found)
		{
			found = false;
			if (st_obj.artists && st_obj.artists.length)
			{
				for (let i = 0; i < i_args.artists.length; i++)
					if (st_obj.artists.indexOf(i_args.artists[i]) != -1)
						{ found = true; break; }
			}
			else if (i_args.artists.indexOf('_null_') != -1)
				found = true;
		}

		if( i_args.priority && found )
		{
			found = false;
			if ((st_obj.priority != null) &&
				((i_args.priority[0] == null) || (st_obj.priority >= i_args.priority[0])) &&
				((i_args.priority[1] == null) || (st_obj.priority <= i_args.priority[1])))
				found = true;
		}

		if( i_args.percent && found )
		{
			found = false;
			if ((st_obj.progress != null) &&
				((i_args.percent[0] == null) || (st_obj.progress >= i_args.percent[0])) &&
				((i_args.percent[1] == null) || (st_obj.progress <= i_args.percent[1])))
				found = true;
		}

		if (i_args.finish && found)
		{
			found = false;
			if (st_obj.finish)
			{
				let days = c_DT_DaysLeft(st_obj.finish);
				if (((i_args.finish[0] == null) ||  days >= i_args.finish[0]) &&
					((i_args.finish[1] == null) ||  days <= i_args.finish[1]))
					found = true;
			}
		}

		if (found)
		{
			el.style.display = 'block';
			el.m_filtered = false;

			// We should return not filtered shots all artists,
			// flags and tags for search field to mute it.
			if (st_obj.artists)
				o_res.artists = o_res.artists.concat(st_obj.artists);

			if (st_obj.flags)
				o_res.flags = o_res.flags.concat(st_obj.flags);

			if (st_obj.tags)
				o_res.tags = o_res.tags.concat(st_obj.tags);

			if (flags_tsk && tags_tsk && st_obj.tasks)
				for (let t in st_obj.tasks)
				{
					let task = st_obj.tasks[t];
					if (task.flags)
						o_res.flags = o_res.flags.concat(task.flags);
					if (task.tags)
						o_res.tags = o_res.tags.concat(task.tags);
				}

			// Remove duplicates from arrays:
			o_res.artists = Array.from(new Set(o_res.artists));
			o_res.flags   = Array.from(new Set(o_res.flags));
			o_res.tags    = Array.from(new Set(o_res.tags));
		}
		else
		{
			el.style.display = 'none';
			el.m_filtered = true;
		}
	}

	// Hide scenes where are no shots to show
	if (sc_elScenes)
		for (let f = 0; f < sc_elScenes.length; f++)
		{
			let oneShown = false;
			for (var t = 0; t < sc_elScenes[f].m_elThumbnails.length; t++)
			{
				if (sc_elScenes[f].m_elThumbnails[t].m_filtered != true)
				{
					oneShown = true;
					break;
				}
			}
			if (oneShown)
			{
				sc_elScenes[f].style.display = 'block';
				sc_elScenes[f].m_filtered = false;
			}
			else
			{
				sc_elScenes[f].style.display = 'none';
				sc_elScenes[f].m_filtered = true;
			}
		}

	sc_DisplayStatistics();

	return o_res;
}

function sc_ShowAllShots()
{
	if( sc_elShots == null ) return;

	for( var i = 0; i < sc_elShots.length; i++)
	{
		sc_elShots[i].style.display = 'block';
		sc_elShots[i].m_filtered = false;
	}

	if( sc_elScenes )
		for( var i = 0; i < sc_elScenes.length; i++)
		{
			sc_elScenes[i].style.display = 'block';
			sc_elScenes[i].m_filtered = false;
		}

	sc_DisplayStatistics();
}

function sc_DisplayStatistics()
{
	var selShots = scenes_GetSelectedShots();
//	c_Info( selShots.length + ' shots selected.');

	// Gather shots:
	var filtered = 0;
	var shots = [];
	for( var i = 0; i < sc_elShots.length; i++)
	{
		if( sc_elShots[i].m_filtered == true )
		{
			filtered++;
			continue;
		}

		if( selShots.length && ( sc_elShots[i].m_selected != true ))
			continue;

		shots.push( sc_elShots[i]);
	}

	// Shots count, progress, frames count:
	//
	var statuses = [];
	var omits = 0;
	var progress = 0;
	var frames_count = 0;
	for( var i = 0; i < shots.length; i++)
	{
		var stat = shots[i].m_status.obj;
		if( stat == null) continue;

		if( stat && stat.flags && ( stat.flags.indexOf('omit') != -1 ))
		{
			omits++;
			continue;
		}

		statuses.push( stat);

		if( stat.progress && ( stat.progress > 0 ))
			progress += stat.progress;

		if( stat.frames_num )
			frames_count += stat.frames_num;
	}

	var info = 'Shots count: <big><b>' + (shots.length - omits) + '</big></b>';
	if( omits ) info += ' (+' + omits + ' omits)';
	if( filtered ) info += ' (+' + filtered + ' filtered)';
	if( shots.length )
		info += '<br>Average progress: ' + Math.floor(progress/(shots.length-omits)) + '%';

	info += '<br>Frames count: ' + frames_count + ' = ' + c_DT_DurFromSec( frames_count / RULES.fps) + ' at ' + RULES.fps + ' FPS';

	if (ASSET.type == 'area')
	{
		var scenes_count = 0;
		for( var i = 0; i < sc_elScenes.length; i++)
		{
			for( var s = 0; s < sc_elScenes[i].m_elShots.length; s++ )
			{
				if( shots.indexOf( sc_elScenes[i].m_elShots[s]) != -1 )
				{
					scenes_count++;
					break;
				}
			}
		}
		info += '<br>Scenes count: ' + scenes_count;
	}

	if( selShots.length )
		info = '<i><b>Selected</b></i> ' + info;

	$('scenes_info').innerHTML = info;

//return;
	// Statistics:
	//
	var args = {};
	args.statuses = statuses;
	args.elTasks = $('scenes_tasks');
	args.elTasksDiv = $('scenes_tasks_div');
	args.elReports = $('scenes_reports');
	args.elReportsDiv = $('scenes_reports_div');
	args.elDiffer = $('scenes_differ');
	args.elDifferDiv = $('scenes_differ_div');
	args.main_artists = true;
	args.draw_bars = true;

	stcs_Show( args);
}

function scenes_makeThumbnails()
{
	for( var i = 0; i < sc_elImgThumbs.length; i++)
		sc_elImgThumbs[i].updated = false;

	sc_thumb_params_values = gui_GetParams( $('scenes_make_thumbnails'), sc_thumb_params);
	for( key in $('scenes_make_thumbnails').m_choises )
		sc_thumb_params_values[key] = $('scenes_make_thumbnails').m_choises[key].value;

	scenes_makeThumbnail();
}

//limit = 10; limit_count = 0;
function scenes_makeThumbnail( i_data, i_args)
{
//limit_count ++; if( limit_count > limit ) return;
	if( i_data )
	{
		if( i_data.error )
		{
			c_Error( i_data.error);
			return;
		}
		if( i_data.cmdexec && i_data.cmdexec.length )
		{
			var img = i_data.cmdexec[0].thumbnail;
			if( img == null )
			{
				c_Error('No thumbnail output received');
				return;
			}

			i_args.elThumb.src = img + '#' + (new Date().getTime());
			i_args.elThumb.updated = true;
		}
		else
		{
			c_Error('Data has no command execution results.');
			return;
		}
	}

	var el = null;
	var num_updated = 0;
	for( var i = 0; i < sc_elImgThumbs.length; i++)
	{
		if( sc_elImgThumbs[i].updated )
		{
			num_updated++;
			continue;
		}
		el = sc_elImgThumbs[i];
		break;
	}

	if( el == null )
	{
		c_Info('Thumbnails generation finished.');
		return;
	}

	var thumb = RULES.root + el.m_path + '/' + RULES.rufolder + '/' + RULES.thumbnail.filename;
	var cmd = RULES.thumbnail.cmd_asset.replace(/@INPUT@/g, RULES.root + el.m_path).replace(/@OUTPUT@/g, thumb);

	if( sc_thumb_params_values.force_update ) cmd += ' --force';
	if( sc_thumb_params_values.skip_movies ) cmd += ' --nomovie';
	cmd += ' -c ' + sc_thumb_params_values.colorspace;

	c_Info('Generating thumbnail for ' + el.m_path + ' (' + num_updated + '/' + sc_elImgThumbs.length + ')');

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":scenes_makeThumbnail,"elThumb":el,"info":'shot thumbnail',"local":true,"wait":false,"parse":true});
}

function scenes_Put()
{
	let args = {};
	args.paths = [];
	let elShots = scenes_GetSelectedShots();
	if (elShots.length < 1)
		elShots = sc_elShots;
	for (let i = 0; i < elShots.length; i++)
		args.paths.push(elShots[i].m_path);

	if (args.paths.length < 1 )
	{
		c_Error('Select at least one shot.');
		return;
	}

	fu_PutMultiDialog(args);
}

function scenes_Convert()
{
	var args = {};
	args.paths = [];
	var paths = scenes_GetSelectedShots();
	for( var i = 0; i < paths.length; i++)
		args.paths.push( paths[i].m_path);

	if( args.paths.length < 1 )
	{
		c_Error('Select at least one shot.');
		return;
	}

	args.results = true;

	d_Convert( args);
}

function scenes_MakeCut()
{
	let args = {};
	args.shots = [];
	let elShots = scenes_GetSelectedShots();
	if (elShots.length < 1)
		elShots = sc_elShots;
	for (let i = 0; i < elShots.length; i++)
		args.shots.push(elShots[i].m_path);

	if (args.shots.length < 2)
	{
		c_Error('There should be at least two shots to make cut.');
		return;
	}

	args.cut_name = ASSET.name;
	args.output = ASSET.path + '/' + RULES.cut.output;

	d_MakeCut(args);
}

function scenes_ExportTable()
{
	var args = {};
	args.shots = [];

	var elShots = scenes_GetSelectedShots();
	if( elShots.length < 1 )
		elShots = sc_elShots;

	for( var i = 0; i < elShots.length; i++)
	{
		if( elShots[i].m_filtered )
			continue;

		var shot = {};
		shot.path = elShots[i].m_path;
		shot.status = elShots[i].m_status.obj;

		args.shots.push( shot);
	}

	if( args.shots.length < 1 )
	{
		c_Error('No shots to export.');
		return;
	}

	table_Export( args);
}

