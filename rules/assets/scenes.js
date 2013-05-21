sc_elShots = null;
sc_elScenes = null;
sc_elCurEditShot = null;
sc_elStatShotsCount = null;
sc_elStatScenesCount = null;

if( ASSETS.scene && ( ASSETS.scene.path == g_CurPath()))
{
	sc_Init();
	a_SetLabel('Scene');
	scene_Show();
	sc_Post();
}

if( ASSETS.scenes && ( ASSETS.scenes.path == g_CurPath()))
{
	sc_Init();
	a_SetLabel('Scenes');
	scenes_Show();
	sc_Post();
}

function sc_Init()
{
	sc_elShots = [];
	sc_elScenes = [];
//	ASSET.filter = sc_FilterShots;

	var elStatDiv = document.createElement('div');
	$('asset').appendChild( elStatDiv);
	elStatDiv.classList.add('statistics');

	var elStatShotsCountLabel = document.createElement('div');
	elStatDiv.appendChild( elStatShotsCountLabel);
	elStatShotsCountLabel.classList.add('label');
	elStatShotsCountLabel.textContent = 'Shots Count:';

	sc_elStatShotsCount = document.createElement('div');
	elStatDiv.appendChild( sc_elStatShotsCount);
	sc_elStatShotsCount.classList.add('value');

	var elStatShotsProgressLabel = document.createElement('div');
	elStatDiv.appendChild( elStatShotsProgressLabel);
	elStatShotsProgressLabel.classList.add('label');
	elStatShotsProgressLabel.textContent = 'Shots Progress:';

	sc_elStatShotsProgress = document.createElement('div');
	elStatDiv.appendChild( sc_elStatShotsProgress);
	sc_elStatShotsProgress.classList.add('value');

	if( ASSET.type == 'scenes' )
	{
		var elStatScenesCountLabel = document.createElement('div');
		elStatDiv.appendChild( elStatScenesCountLabel);
		elStatScenesCountLabel.classList.add('label');
		elStatScenesCountLabel.textContent = 'Scenes Count:';

		sc_elStatScenesCount = document.createElement('div');
		elStatDiv.appendChild( sc_elStatScenesCount);
		sc_elStatScenesCount.classList.add('value');
	}
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
		if( sc_ShotSkip( folders[f].name)) continue;

		var path = g_elCurFolder.m_path + '/' + folders[f].name;

		var elFolder = document.createElement('div');
		sc_elShots.push( elFolder);
		elFolder.m_hidden = false;
		elFolder.m_status = folders[f].status;
		elFolder.m_path = path;
		u_el.asset.appendChild( elFolder);
		elFolder.style.padding = '4px';
		elFolder.classList.add('shot');

		var elImg = document.createElement('img');
		elFolder.appendChild( elImg);
		elImg.classList.add('thumbnail');
		elImg.m_path = path;
		elImg.onclick = function(e){g_GO(e.currentTarget.m_path)};
		elImg.src = RULES.root + path + '/' + RULES.rufolder + '/' + RULES.thumbnail.filename;

		var elName = document.createElement('div');
		elFolder.appendChild( elName);
		elName.classList.add('button');
		elName.m_path = path;
		elName.onclick = function(e){g_GO(e.currentTarget.m_path)};
		elName.textContent = folders[f].name;

		elFolder.m_elFinish = document.createElement('div');
		elFolder.appendChild( elFolder.m_elFinish);

		elFolder.m_elStatus = document.createElement('div');
		elFolder.appendChild( elFolder.m_elStatus);
		elFolder.m_elStatus.classList.add('status');

		elFolder.m_elEdit = document.createElement('div');
		elFolder.m_elStatus.appendChild( elFolder.m_elEdit);
		elFolder.m_elEdit.classList.add('button');
		elFolder.m_elEdit.classList.add('btn_edit');
		elFolder.m_elEdit.textContent = 'Edit';
		elFolder.m_elEdit.m_elFolder = elFolder;
		elFolder.m_elEdit.onclick = function(e){
			var el = e.currentTarget.m_elFolder;
			sc_elCurEditShot = el;
			st_CreateEditUI( el, el.m_path, el.m_status, sc_ShotStatusApply, el.m_elStatus);
		};

		elFolder.m_elProgress = document.createElement('div');
		elFolder.m_elStatus.appendChild( elFolder.m_elProgress);
		elFolder.m_elProgress.classList.add('progress');
		elFolder.m_elProgressBar = document.createElement('div');
		elFolder.m_elProgress.appendChild( elFolder.m_elProgressBar);
		elFolder.m_elProgressBar.classList.add('progressbar');

		elFolder.m_elPercent = document.createElement('div');
		elFolder.m_elStatus.appendChild( elFolder.m_elPercent);
		elFolder.m_elPercent.classList.add('percent');

		elFolder.m_elAnn = document.createElement('div');
		elFolder.m_elStatus.appendChild( elFolder.m_elAnn);
		elFolder.m_elAnn.classList.add('annotation');

		elFolder.m_elArtists = document.createElement('div');
		elFolder.m_elStatus.appendChild( elFolder.m_elArtists);
		elFolder.m_elArtists.classList.add('artists');

		elFolder.m_elTags = document.createElement('div');
		elFolder.m_elStatus.appendChild( elFolder.m_elTags);
		elFolder.m_elTags.classList.add('tags');

		sc_elCurEditShot = elFolder;
		sc_ShotStatusApply( folders[f].status);
		sc_elCurEditShot = null;
	}
	sc_DisplayCounts();
}

function scenes_Show()
{
	var walk = n_WalkDir([ASSET.path], 1, RULES.rufolder,['rules','status'],['status'])[0];
	if( walk.folders == null ) return;
	walk.folders.sort( c_CompareFiles );

	for( var sc = 0; sc < walk.folders.length; sc++)
	{
		var fobj = walk.folders[sc];
		if( sc_ShotSkip( fobj.name)) continue;

		var elScene = document.createElement('div');
		sc_elScenes.push( elScene);
		elScene.m_hidden = false;
		elScene.m_elThumbnails = [];
		u_el.asset.appendChild( elScene);
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
			if( sc_ShotSkip( fobj.name)) continue;

			var elShot = document.createElement('div');
			sc_elShots.push( elShot);
			elShot.m_status = fobj.status;
			elShot.m_hidden = false;
			elScene.appendChild( elShot);
			elScene.m_elThumbnails.push( elShot);
			elShot.classList.add('shot');
			elShot.m_path = elScene.m_path + '/' + fobj.name;
//			elShot.onclick = function(e){e.stopPropagation();g_GO(e.currentTarget.m_path)};
			elShot.ondblclick = function(e){
				var el = e.currentTarget;
				sc_elCurEditShot = el;
				st_CreateEditUI( el, el.m_path, el.m_status, sc_ShotStatusApply);
				return false;
			};

			var elImg = document.createElement('img');
			elShot.appendChild( elImg);
			elImg.src = RULES.root + elShot.m_path +'/'+ RULES.rufolder +'/'+ RULES.thumbnail.filename;

			elShot.m_elStatus = document.createElement('div');
			elShot.appendChild( elShot.m_elStatus);
			elShot.m_elStatus.classList.add('status');

			elShot.m_elAnn = document.createElement('div');
			elShot.m_elStatus.appendChild( elShot.m_elAnn);
			elShot.m_elAnn.classList.add('annotation');

			elShot.m_elPercent = document.createElement('div');
			elShot.m_elStatus.appendChild( elShot.m_elPercent);
			elShot.m_elPercent.classList.add('percent');

//			var elName = document.createElement('div');
			var elName = document.createElement('a');
			elShot.m_elStatus.appendChild( elName);
			elName.classList.add('name');
			elName.textContent = fobj.name;
			elName.href = '#'+elShot.m_path;

			elShot.m_elProgress = document.createElement('div');
			elShot.m_elStatus.appendChild( elShot.m_elProgress);
			elShot.m_elProgress.classList.add('progress');
			elShot.m_elProgressBar = document.createElement('div');
			elShot.m_elProgress.appendChild( elShot.m_elProgressBar);
			elShot.m_elProgressBar.classList.add('progressbar');

			elShot.m_elTags = document.createElement('div');
			elShot.m_elStatus.appendChild( elShot.m_elTags);
			elShot.m_elTags.classList.add('tags');

			elShot.m_elArtists = document.createElement('div');
			elShot.m_elStatus.appendChild( elShot.m_elArtists);
			elShot.m_elArtists.classList.add('artists');

			elShot.m_elFinish = document.createElement('div');
			elShot.m_elStatus.appendChild( elShot.m_elFinish);
			elShot.m_elFinish.classList.add('finish');

			sc_elCurEditShot = elShot;
			sc_ShotStatusApply( fobj.status);
			sc_elCurEditShot = null;
		}
	}
	sc_DisplayCounts();
}

function sc_ShotStatusApply( i_status)
{
	if( i_status != null ) sc_elCurEditShot.m_status = c_CloneObj( i_status);
	st_SetElLabel( i_status, sc_elCurEditShot.m_elAnn, false);
	st_SetElArtists( i_status, sc_elCurEditShot.m_elArtists);
	st_SetElTags( i_status, sc_elCurEditShot.m_elTags);
	st_SetElProgress( i_status, sc_elCurEditShot.m_elProgressBar, sc_elCurEditShot.m_elProgress, sc_elCurEditShot.m_elPercent);
	st_SetElFinish( i_status, sc_elCurEditShot.m_elFinish, ASSET.type == 'scene' );
	st_SetElColor( i_status, sc_elCurEditShot);
}

function sc_ShotSkip( i_name)
{
	if( i_name.indexOf('.') == 0 ) return true;
	if( i_name.indexOf('_') == 0 ) return true;
	return false;
}

function sc_FilterShots( i_args)
{
	if( sc_elShots == null ) return;

//c_Info( JSON.stringify(i_args));
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
		var el = sc_elShots[th];
		var founded = ( i_args == null );

		if( el.m_status == null ) el.m_status = {};

		if( anns )
		{
			if( el.m_status.annotation )
				for( var o = 0; o < anns.length; o++)
				{
					var founded_and = true;
					for( var a = 0; a < anns[o].length; a++)
					{
						if( el.m_status.annotation.indexOf( anns[o][a]) == -1 )
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
			if( el.m_status.tags )
				for( i = 0; i < i_args.tags.length; i++ )
					if( el.m_status.tags.indexOf( i_args.tags[i]) != -1 )
						{ founded = true; break; }
		}

		if( i_args.artists && founded )
		{
			founded = false;
			if( el.m_status.artists )
				for( i = 0; i < i_args.artists.length; i++ )
					if( el.m_status.artists.indexOf( i_args.artists[i]) != -1 )
						{ founded = true; break; }
		}

		if( i_args.percent && founded )
		{
			founded = false;
			if( el.m_status.progress &&
				(( i_args.percent[0] == null ) || ( el.m_status.progress >= i_args.percent[0] )) &&
				(( i_args.percent[1] == null ) || ( el.m_status.progress <= i_args.percent[1] )))
				founded = true;
		}

		if( i_args.finish && founded )
		{
			founded = false;
			if( el.m_status.finish )
			{
				var days = c_DT_DaysLeft( el.m_status.finish);
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

	sc_DisplayCounts();
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

	sc_DisplayCounts();
}

function sc_DisplayCounts()
{
	var shots = 0;
	var progress = 0;
	for( var i = 0; i < sc_elShots.length; i++)
	{
		if( sc_elShots[i].m_hidden ) continue;
		shots++;
		var stat = sc_elShots[i].m_status;
		if( stat && stat.progress && ( stat.progress > 0 ))
			progress += stat.progress;
	}
	sc_elStatShotsCount.textContent = shots;
	if( shots )
		sc_elStatShotsProgress.textContent = Math.round(progress/shots) + '%';

	if( ASSET.type != 'scenes') return;

	var scenes = 0;
	for( var i = 0; i < sc_elScenes.length; i++)
	{
		if( sc_elScenes[i].m_hidden ) continue;
		scenes++;
	}
	sc_elStatScenesCount.textContent = scenes;
}

