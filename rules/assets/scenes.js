sc_elShots = null;
sc_elScenes = null;
sc_elCurShot = null;

sc_elImgThumbs = [];

sc_thumb_params = {};
sc_thumb_params.force_update = {"width":'30%',"lwidth":'150px','type':"bool",'default':false,"tooltip":'Update or skip existing thumbnails'};
sc_thumb_params.skip_movies = {"width":'30%',"lwidth":'150px','type':"bool",'default':true,"tooltip":'Do not create thumbnails from movie files'};

sc_thumb_params_values = {};

sc_scenes = false;

if( ASSETS.scene && ( ASSETS.scene.path == g_CurPath()))
{
	a_SetLabel('Scene');
	sc_scenes = false;

	sc_Init();
}

if( ASSETS.scenes && ( ASSETS.scenes.path == g_CurPath()))
{
	a_SetLabel('Scenes');
	sc_scenes = true;

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

	gui_Create( $('scenes_make_thumbnails'), sc_thumb_params);
	gui_CreateChoises({"wnd":$('scenes_make_thumbnails'),"name":'colorspace',"value":RULES.thumbnail.colorspace,"label":'Colorspace:',"keys":RULES.dailies.colorspaces});

	if( sc_scenes )
	{
		scenes_Show();
	}
	else
	{
		scene_Show();
	}
}

function sc_Post()
{
	if( g_arguments )
		if( g_arguments.s_Search )
			sc_FilterShots( g_arguments.s_Search);
}

function scene_Show()
{
	var folders = g_elCurFolder.m_dir.folders;
	for( var f = 0; f < folders.length; f++ )
	{
		if( c_AuxFolder( folders[f])) continue;

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
		elShot.appendChild( elShot.m_elStatus);
		elShot.m_elStatus.classList.add('status');

		elSt.elProgress = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elProgress);
		elSt.elProgress.classList.add('progress');
		elSt.elProgressBar = document.createElement('div');
		elSt.elProgress.appendChild( elSt.elProgressBar);
		elSt.elProgressBar.classList.add('progressbar');

		var elEditBtn = document.createElement('div');
		elShot.m_elStatus.appendChild( elEditBtn);
		elEditBtn.classList.add('button');
		elEditBtn.classList.add('edit');
		elEditBtn.title = 'Edit status.\nSelect several shots to edit.';
		if( g_auth_user == null )
			elEditBtn.style.display = 'none';

		elSt.elPercentage = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elPercentage);
		elSt.elPercentage.classList.add('percent');

		elSt.elAnnotation = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elAnnotation);
		elSt.elAnnotation.classList.add('annotation');

		elSt.elArtists = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elArtists);
		elSt.elArtists.classList.add('artists');

		elSt.elFlags = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elFlags);
		elSt.elFlags.classList.add('flags');

		elSt.elTags = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elTags);
		elSt.elTags.classList.add('tags');

		function st_CreateSceneShot( i_status)
		{
			for( var el in elSt ) i_status[el] = elSt[el];
			i_status.elColor = elShot;
			i_status.elParent = elShot;
			i_status.elShow = elShot.m_elStatus;
		}

		if( folders[f].status && folders[f].status.flags )
			for( var fl = 0; fl < folders[f].status.flags.length; fl++)
				elShot.classList.add( folders[f].status.flags[fl]);

		var st_obj = new Status( folders[f].status, {"path":path,"createGUI": st_CreateSceneShot});
		elShot.m_status = st_obj;
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
	if( i_data.indexOf('No such file ' + RULES.root) != -1 ) return;

	// Replace <br> with spaces through some pattern:
	i_args.elShot.m_elBody.innerHTML = i_data.replace(/\<\s*br\s*\/?\s*\>/g,'@@BR@@');
	i_args.elShot.m_elBody.innerHTML = i_args.elShot.m_elBody.textContent.replace(/@@BR@@/g,' ');
}

function scenes_Show()
{
	n_WalkDir({"paths":[ASSET.path],"wfunc":scenes_Received,"info":'walk scenes',"depth":1,"rufiles":['rules','status'],"lookahead":['status'],"local":true});
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
		if( c_AuxFolder( fobj)) continue;

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
//window.console.log(JSON.stringify(fobj));
		st_SetElLabel( fobj.status, elStatus);
		st_SetElColor( fobj.status, elScene);

		if( walk.folders[sc].folders == null ) continue;

		walk.folders[sc].folders.sort( c_CompareFiles );
		for( var s = 0; s < walk.folders[sc].folders.length; s++)
		{
			var fobj = walk.folders[sc].folders[s];
			if( c_AuxFolder( fobj)) continue;

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

			elShot.m_elStatus = document.createElement('div');
			elShot.appendChild( elShot.m_elStatus);
			elShot.m_elStatus.classList.add('status');

			elSt.elAnnotation = document.createElement('div');
			elShot.m_elStatus.appendChild( elSt.elAnnotation);
			elSt.elAnnotation.classList.add('annotation');

			elSt.elPercentage = document.createElement('div');
			elShot.m_elStatus.appendChild( elSt.elPercentage);
			elSt.elPercentage.classList.add('percent');

			var elName = document.createElement('a');
			elShot.m_elStatus.appendChild( elName);
			elName.classList.add('name');
			elName.textContent = fobj.name;
			elName.href = '#'+elShot.m_path;

			elSt.elProgress = document.createElement('div');
			elShot.m_elStatus.appendChild( elSt.elProgress);
			elSt.elProgress.classList.add('progress');
			elSt.elProgressBar = document.createElement('div');
			elSt.elProgress.appendChild( elSt.elProgressBar);
			elSt.elProgressBar.classList.add('progressbar');

			elSt.elFlags = document.createElement('div');
			elShot.m_elStatus.appendChild( elSt.elFlags);
			elSt.elFlags.classList.add('flags');

			elSt.elTags = document.createElement('div');
			elShot.m_elStatus.appendChild( elSt.elTags);
			elSt.elTags.classList.add('tags');

			elSt.elArtists = document.createElement('div');
			elShot.m_elStatus.appendChild( elSt.elArtists);
			elSt.elArtists.classList.add('artists');

/*			elSt.elFinish = document.createElement('div');
			elShot.m_elStatus.appendChild( elSt.elFinish);
			elSt.elFinish.classList.add('finish');*/

			function st_CreateSceneShot( i_status)
			{
				for( var el in elSt ) i_status[el] = elSt[el];
				i_status.elParent = elShot;
				i_status.elColor = elShot;
				i_status.elShow = elShot.m_elStatus;
			}

			if( fobj.status && fobj.status.flags )
				for( var f = 0; f < fobj.status.flags.length; f++)
					elShot.classList.add( fobj.status.flags[f]);

			var st_obj = new Status( fobj.status, {"path":elShot.m_path,"createGUI": st_CreateSceneShot});
			elShot.m_status = st_obj;

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

function sc_FilterShots( i_args)
{
	if( sc_elShots == null ) return;

//console.log( JSON.stringify(i_args));
//	if( i_args == null ) i_args = {};
	if( i_args == null )
	{
		sc_ShowAllShots();
		return;
	}

	var anns = null;
	if( i_args.ann )
	{
		var anns_or = i_args.ann.split(',');
		anns = [];
		for( var o = 0; o < anns_or.length; o++)
			anns.push( anns_or[o].split(' '));
	}

	for( var th = 0; th < sc_elShots.length; th++)
	{
		var found = ( i_args == null );

		var el = sc_elShots[th];
		var st_obj = {};
		if( el.m_status && el.m_status.obj )
			st_obj = el.m_status.obj;

		if( anns )
		{
			if( st_obj.annotation )
				for( var o = 0; o < anns.length; o++)
				{
					var found_and = true;
					for( var a = 0; a < anns[o].length; a++)
					{
						if( st_obj.annotation.indexOf( anns[o][a]) == -1 )
						{
							found_and = false;
							break;
						}
					}
					if( found_and )
					{
						found = true;
						break;
					}
				}
		}
		else found = true;

		if( i_args.flags && found )
		{
			found = false;
			if( st_obj.flags && st_obj.flags.length )
			{
				for( i = 0; i < i_args.flags.length; i++ )
					if( st_obj.flags.indexOf( i_args.flags[i]) != -1 )
						{ found = true; break; }
			}
			else if( i_args.flags.indexOf('_null_') != -1)
				found = true;
		}

		if( i_args.tags && found )
		{
			found = false;
			if( st_obj.tags && st_obj.tags.length )
			{
				for( i = 0; i < i_args.tags.length; i++ )
					if( st_obj.tags.indexOf( i_args.tags[i]) != -1 )
						{ found = true; break; }
			}
			else if( i_args.tags.indexOf('_null_') != -1)
				found = true;
		}

		if( i_args.artists && found )
		{
			found = false;
			if( st_obj.artists && st_obj.artists.length )
			{
				for( i = 0; i < i_args.artists.length; i++ )
					if( st_obj.artists.indexOf( i_args.artists[i]) != -1 )
						{ found = true; break; }
			}
			else if( i_args.artists.indexOf('_null_') != -1)
				found = true;
		}

		if( i_args.percent && found )
		{
			found = false;
			if(( st_obj.progress != null ) &&
				(( i_args.percent[0] == null ) || ( st_obj.progress >= i_args.percent[0] )) &&
				(( i_args.percent[1] == null ) || ( st_obj.progress <= i_args.percent[1] )))
				found = true;
		}

		if( i_args.finish && found )
		{
			found = false;
			if( st_obj.finish )
			{
				var days = c_DT_DaysLeft( st_obj.finish);
				if( (( i_args.finish[0] == null ) ||  days >= i_args.finish[0] ) &&
					(( i_args.finish[1] == null ) ||  days <= i_args.finish[1] ))
					found = true;
			}
		}

		if( found )
		{
			el.style.display = 'block';
			el.m_filtered = false;
		}
		else
		{
			el.style.display = 'none';
			el.m_filtered = true;
		}
	}

	if( sc_elScenes )
		for( var f = 0; f < sc_elScenes.length; f++)
		{
			var oneShown = false;
			for( var t = 0; t < sc_elScenes[f].m_elThumbnails.length; t++)
			{
				if( sc_elScenes[f].m_elThumbnails[t].m_filtered != true )
				{
					oneShown = true;
					break;
				}
			}
			if( oneShown )
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

	if( ASSET.type == 'scenes')
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
//console.log(cmd);

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":scenes_makeThumbnail,"elThumb":el,"info":'shot thumbnail',"local":true,"wait":false,"parse":true});
}

function scenes_Put()
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

	fu_PutMultiDialog( args);
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

