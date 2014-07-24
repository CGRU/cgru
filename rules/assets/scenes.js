sc_elShots = null;
sc_elScenes = null;
sc_elCurShot = null;

sc_elImgThumbs = [];

sc_thumb_params = {};
sc_thumb_params.force_update = {"width":'30%',"lwidth":'150px',"bool":false,"tooltip":'Update or skip existing thumbnails'};
sc_thumb_params.skip_movies = {"width":'30%',"lwidth":'150px',"bool":true,"tooltip":'Do not create thumbnails from movie files'};

sc_thumb_params_values = {};

sc_frames_total = 0;

if( ASSETS.scene && ( ASSETS.scene.path == g_CurPath()))
{
	sc_Init();
	a_SetLabel('Scene');
	scene_Show();
}

if( ASSETS.scenes && ( ASSETS.scenes.path == g_CurPath()))
{
	sc_Init();
	a_SetLabel('Scenes');
	scenes_Show();
}

function sc_Init()
{
	var data = n_Request({"send":{"getfile":'rules/assets/scenes.html'}});
	$('asset').innerHTML = data;

	gui_Create( $('scenes_make_thumbnails'), sc_thumb_params);
	gui_CreateChoises({"wnd":$('scenes_make_thumbnails'),"name":'colorspace',"value":RULES.dailies.colorspace,"label":'Colorspace:',"keys":RULES.dailies.colorspaces});

	sc_elShots = [];
	sc_elScenes = [];
}

function sc_Post()
{
	if( g_arguments )
		if( g_arguments.u_Search )
			sc_FilterShots( g_arguments.u_Search);
}

function scene_Show()
{
	var folders = g_elCurFolder.m_dir.folders;
	for( var f = 0; f < folders.length; f++ )
	{
		if( sc_SkipFolder( folders[f].name)) continue;

		var path = g_elCurFolder.m_path + '/' + folders[f].name;

		var elShot = document.createElement('div');
		sc_elShots.push( elShot);
		elShot.m_hidden = false;
		elShot.m_status = folders[f].status;
		elShot.m_path = path;
		$('scenes_div').appendChild( elShot);
		elShot.style.padding = '4px';
		elShot.classList.add('shot');

		var elLink = document.createElement('a');
		elShot.appendChild( elLink);
		elLink.href = '#' + path;
		var elImg = document.createElement('img');
		elLink.appendChild( elImg);
		elImg.classList.add('thumbnail');
		elImg.m_path = path;
		elImg.m_src = RULES.root + path + '/' + RULES.rufolder + '/' + RULES.thumbnail.filename;
		sc_elImgThumbs.push( elImg);
		elImg.src = elImg.m_src;

		var elName = document.createElement('a');
		elShot.appendChild( elName);
		elName.href = '#' + path;
		elName.textContent = folders[f].name;

		var elSt = {};

		elSt.elFinish = document.createElement('div');
		elShot.appendChild( elSt.elFinish);

		elShot.m_elStatus = document.createElement('div');
		elShot.appendChild( elShot.m_elStatus);
		elShot.m_elStatus.classList.add('status');

		var elEditBtn = document.createElement('div');
		elShot.m_elStatus.appendChild( elEditBtn);
		elEditBtn.classList.add('button');
		elEditBtn.classList.add('btn_edit');
		elEditBtn.textContent = 'Edit';

		elSt.elProgress = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elProgress);
		elSt.elProgress.classList.add('progress');
		elSt.elProgressBar = document.createElement('div');
		elSt.elProgress.appendChild( elSt.elProgressBar);
		elSt.elProgressBar.classList.add('progressbar');

		elSt.elPercentage = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elPercentage);
		elSt.elPercentage.classList.add('percent');

		elSt.elAnnotation = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elAnnotation);
		elSt.elAnnotation.classList.add('annotation');

		elSt.elArtists = document.createElement('div');
		elShot.m_elStatus.appendChild( elSt.elArtists);
		elSt.elArtists.classList.add('artists');

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

		var st_obj = new Status( folders[f].status, {"path":path,"createGUI": st_CreateSceneShot});
		elShot.m_status = st_obj;
		elEditBtn.m_status = st_obj;
		elEditBtn.onclick = sc_EditStatus;

		elShot.onclick = sc_ShotClicked;
	}
	sc_DisplayStatistics();
	sc_Post();
}

function scenes_Show()
{
	n_WalkDir({"paths":[ASSET.path],"wfunc":scenes_Received,"depth":1,"rufiles":['rules','status'],"lookahead":['status'],"local":true});
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
		if( sc_SkipFolder( fobj.name)) continue;

		var elScene = document.createElement('div');
		sc_elScenes.push( elScene);
		elScene.m_hidden = false;
		elScene.m_elThumbnails = [];
		$('scenes_div').appendChild( elScene);
		elScene.classList.add('scene');
		elScene.m_path = ASSET.path + '/' + fobj.name;
//		elScene.onclick = function(e){g_GO(e.currentTarget.m_path)};

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
			if( sc_SkipFolder( fobj.name)) continue;

			var elShot = document.createElement('div');
			sc_elShots.push( elShot);
			elShot.m_status = fobj.status;
			elShot.m_hidden = false;
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

function sc_ShotClicked( i_evt)
{
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
}
function scenes_SelectPlaylist()
{
	scenes_SelectAll( false);
	var shots = p_GetCurrentShots();
	for( var i = 0; i < sc_elShots.length; i++)
		if( shots.indexOf( sc_elShots[i].m_path) != -1 )
			sc_SelectShot( sc_elShots[i], true);
}
function scenes_GetSelectedShots()
{
	var shots = [];
	for( var i = 0; i < sc_elShots.length; i++)
		if( sc_elShots[i].m_selected === true )
			shots.push( sc_elShots[i]);
	return shots;
}
function sc_SkipFolder( i_name)
{
	var name = c_PathBase( i_name);
	for( var i = 0; i < RULES.assets.shot.skip.length; i++)
		if( name.toLowerCase().indexOf( RULES.assets.shot.skip[i]) === 0 )
			return true;
	return false;
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

	var scenes_count = 0;
	var shots_count = 0;

	for( var th = 0; th < sc_elShots.length; th++)
	{
		var founded = ( i_args == null );

		var el = sc_elShots[th];
		var st_obj = {};
		if( el.m_status && el.m_status.obj )
			st_obj = el.m_status.obj;

		if( anns )
		{
			if( st_obj.annotation )
				for( var o = 0; o < anns.length; o++)
				{
					var founded_and = true;
					for( var a = 0; a < anns[o].length; a++)
					{
						if( st_obj.annotation.indexOf( anns[o][a]) == -1 )
						{
							founded_and = false;
							break;
						}
					}
					if( founded_and )
					{
						founded = true;
						break;
					}
				}
		}
		else founded = true;

		if( i_args.tags && founded )
		{
			founded = false;
			if( st_obj.tags )
				for( i = 0; i < i_args.tags.length; i++ )
					if( st_obj.tags.indexOf( i_args.tags[i]) != -1 )
						{ founded = true; break; }
		}

		if( i_args.artists && founded )
		{
			founded = false;
			if( st_obj.artists )
				for( i = 0; i < i_args.artists.length; i++ )
					if( st_obj.artists.indexOf( i_args.artists[i]) != -1 )
						{ founded = true; break; }
		}

		if( i_args.percent && founded )
		{
			founded = false;
			if( st_obj.progress &&
				(( i_args.percent[0] == null ) || ( st_obj.progress >= i_args.percent[0] )) &&
				(( i_args.percent[1] == null ) || ( st_obj.progress <= i_args.percent[1] )))
				founded = true;
		}

		if( i_args.finish && founded )
		{
			founded = false;
			if( st_obj.finish )
			{
				var days = c_DT_DaysLeft( st_obj.finish);
				if( (( i_args.finish[0] == null ) ||  days >= i_args.finish[0] ) &&
					(( i_args.finish[1] == null ) ||  days <= i_args.finish[1] ))
					founded = true;
			}
		}

		if( founded )
		{
			el.style.display = 'block';
			el.m_hidden = false;
			shots_count++;
		}
		else
		{
			el.style.display = 'none';
			el.m_hidden = true;
		}
	}

	if( sc_elScenes )
		for( var f = 0; f < sc_elScenes.length; f++)
		{
			var oneShown = false;
			for( var t = 0; t < sc_elScenes[f].m_elThumbnails.length; t++)
			{
				if( sc_elScenes[f].m_elThumbnails[t].m_hidden != true )
				{
					oneShown = true;
					break;
				}
			}
			if( oneShown )
			{
				sc_elScenes[f].style.display = 'block';
				sc_elScenes[f].m_hidden = false;
				scenes_count++;
			}
			else
			{
				sc_elScenes[f].style.display = 'none';
				sc_elScenes[f].m_hidden = true;
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
		sc_elShots[i].m_hidden = false;
	}

	if( sc_elScenes )
		for( var i = 0; i < sc_elScenes.length; i++)
		{
			sc_elScenes[i].style.display = 'block';
			sc_elScenes[i].m_hidden = false;
		}

	sc_DisplayStatistics();
}

function sc_DisplayStatistics()
{
	// Shots count and progress:
	//
	var shots = 0;
	var progress = 0;
	for( var i = 0; i < sc_elShots.length; i++)
	{
		if( sc_elShots[i].m_hidden ) continue;
		shots++;
		var stat = sc_elShots[i].m_status.obj;
		if( stat && stat.progress && ( stat.progress > 0 ))
			progress += stat.progress;
	}

	var info = 'Shots Count: ' + shots;
	if( shots )
		info += ' Shots Progress: ' + Math.round(progress/shots) + '%';

	if( ASSET.type == 'scenes')
	{
		var scenes = 0;
		for( var i = 0; i < sc_elScenes.length; i++)
		{
			if( sc_elScenes[i].m_hidden ) continue;
			scenes++;
		}
		info = ' Scenes Count: ' + scenes + ' ' + info;
	}

	if( sc_frames_total ) info += ' Frames count: ' + sc_frames_total + ' = ' + c_DT_DurFromSec( sc_frames_total / RULES.fps);

	$('scenes_info').textContent = info;

	// Statistics:
	//
	var statuses = [];
	for( var i = 0; i < sc_elShots.length; i++)
		if( sc_elShots[i].m_hidden !== true )
			statuses.push( sc_elShots[i].m_status.obj);

	var args = {};
	args.statuses = statuses;
	args.elReports = $('scenes_reports');
	args.elReportsDiv = $('scenes_reports_div');
	args.elTasks = $('scenes_tasks');
	args.elTasksDiv = $('scenes_tasks_div');

	stsc_Show( args);
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
	args.shots = [];
	var shots = scenes_GetSelectedShots();
	for( var i = 0; i < shots.length; i++)
		args.shots.push( shots[i].m_path);

	if( args.shots.length < 1 )
	{
		c_Error('Select at least one shot.');
		return;
	}

	fu_PutMultiDialog( args);
}

