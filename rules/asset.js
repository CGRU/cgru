ASSETS = {};
ASSET = null;

a_elThumbnails = null;
a_elFolders = null;
a_elCurEditStatus = null;
a_elFileLimits = [];

function View_asset_Open() { a_ShowBody(); }

function a_Process()
{
	a_AutoSeek();
//	if( ASSET ) c_RulesMergeObjs( ASSET, RULES.assets[ASSET.type]);
	a_ShowHeaders();
}

function a_Finish()
{
	ASSETS = {};
	ASSET = null;

	u_el.assets.innerHTML = '';
	u_el.asset.innerHTML = '';

	a_elThumbnails = null;
	a_elFolders = null;
}

function a_ShowBody()
{
	if( ASSET == null ) return;
	if( ASSET.path != g_CurPath()) return;

	u_el.asset.innerHTML = '';

	thumbnails = [];
	a_elFileLimits = [];

	window.document.title = ASSET.name + ' ' + window.document.title;

	if( ASSET.thumbnails != null )
	{
		a_ShowThumbnails();
		return;
	}

	if( ASSET.source )
	{
		var elSource = document.createElement('div');
		u_el.asset.appendChild( elSource);
		elSource.classList.add('source');
		elSource.classList.add('button');
		elSource.textContent = 'Scan Sources';
		elSource.onclick = a_OpenCloseSourceOnClick;
	}

	var walk = {};
	walk.paths = [];
	if( ASSET.result )
	{
		walk.result = [];
		for( var r = 0; r < ASSET.result.path.length; r++)
		{
			walk.result.push( walk.paths.length);
			walk.paths.push( ASSET.path + '/' + ASSET.result.path[r]);
		}
	}
	if( ASSET.dailies )
	{
		walk.dailies = [];
		for( var r = 0; r < ASSET.dailies.path.length; r++)
		{
			walk.dailies.push( walk.paths.length);
			walk.paths.push( ASSET.path + '/' + ASSET.dailies.path[r]);
		}
	}

	if( walk.paths.length )
		walk.walks = n_WalkDir( walk.paths, 0, RULES.rufolder);

	if( ASSET.result )
	{
		var elResult = document.createElement('div');
		u_el.asset.appendChild( elResult);
		elResult.classList.add('result');

		var founded = false;
		for( var i = 0; i < walk.result.length; i++)
		{
			var folders = walk.walks[walk.result[i]].folders;
			var path = walk.paths[walk.result[i]];
			if(( folders == null ) || ( folders.length == 0 )) continue;

			var elFileLimit = a_FileLimitAdd( elResult);

			thumbnails.push( path);
			elFileLimit.m_elFiles = u_ShowDirectory( elResult, path, walk.walks[walk.result[i]])
			founded = true;

			a_elFileLimits.push( elFileLimit);
		}

		if( false == founded )
			elResult.textContent = JSON.stringify( ASSET.result.path );
	}

	if( ASSET.dailies )
	{
		var elDailies = document.createElement('div');
		u_el.asset.appendChild( elDailies);
		elDailies.classList.add('resdailies');

		var founded = false;
		for( var i = 0; i < walk.dailies.length; i++)
		{
			var path = walk.paths[walk.dailies[i]];
			var files = walk.walks[walk.dailies[i]].files;
			var folders = walk.walks[walk.dailies[i]].folders;
			if(( files && files.length ) || ( folders && folders.length ))
			{
				var elFileLimit = a_FileLimitAdd( elDailies);
				elFileLimit.m_elFiles = u_ShowDirectory( elDailies, path, walk.walks[walk.dailies[i]]);
				if( thumbnails.length == 0 )
					thumbnails.push( path);
				founded = true;
				a_elFileLimits.push( elFileLimit);
			}
		}

		if( false == founded )
			elDailies.textContent = JSON.stringify( ASSET.dailies.path );
	}

	a_FileLimitsApplyAll();

	if( thumbnails.length )
		a_MakeThumbnail( thumbnails, ASSET.path);
}

function a_MakeThumbnail( i_sources, i_path)
{
	var input = null;
	for( var i = 0; i < i_sources.length; i++ )
	{
		if( input ) input += ',';
		else input = '';
			input += cgru_PM('/' + RULES.root + i_sources[i], true);
	}
	var output = cgru_PM('/' + RULES.root + i_path + '/'+RULES.rufolder+'/' + RULES.thumbnail.filename, true);
	var cmd = RULES.thumbnail.cmd_asset.replace(/@INPUT@/g, input).replace(/@OUTPUT@/g, output);
	n_Request({"cmdexec":{"cmds":[cmd]}}, false);
}

function a_Create( i_type, i_name, i_path)
{
	var asset = {};
	asset.name = i_name;
	asset.path = i_path;
	asset.type = i_type;

	c_RulesMergeObjs( asset, RULES.assets[i_type]);

	ASSETS[i_type] = asset;
	if( ASSET )
	{
		if( asset.path.length > ASSET.path.length )
			ASSET = asset;
	}
	else
		ASSET = asset;
}

function a_Append( i_path, i_rules)
{
//window.console.log('a_Append: '+ i_path);
//window.console.log( JSON.stringify( i_rules));
	for( var rules in i_rules)
	for( var attr in i_rules[rules])
	{
//window.console.log('attr='+attr);
		for( var atype in RULES.assets)
		{
			if( ASSETS[atype] ) continue;
			if( attr != atype ) continue;
			a_Create( atype, RULES[attr], i_path);
			c_Log('Asset: ' + atype + '=' + RULES[attr]);
		}
	}
}

function a_AutoSeek()
{
	var folders = g_elCurFolder.m_path.split('/');
	var path = '';
	for( var i = 0; i < folders.length; i++ )
	{
		if(( folders[i].length == 0 ) && ( i != 0 )) continue;

		var nextfolder = null;
		if( i < folders.length ) nextfolder = folders[i+1];
		if( nextfolder == '' ) continue;

		if( path == '/' )
			path += folders[i];
		else
			path += '/' + folders[i];
//window.console.log('path='+path);
		for( var asset_type in RULES.assets)
		{
			if( ASSETS[asset_type]) continue;
//window.console.log( asset_type);
			var seekpaths = RULES.assets[asset_type].seek;
			if( seekpaths == null ) continue;
			for( var l = 0; l < seekpaths.length; l++)
			{
				var subfolder = ( seekpaths[l].lastIndexOf('/') == (seekpaths[l].length-1))
//window.console.log('seekpath-'+subfolder+'='+seekpaths[l]);
				var seekpath = seekpaths[l];
				if( subfolder )
				{
					if( nextfolder == null ) break;
					seekpath = seekpaths[l].substr( 0, seekpaths[l].lastIndexOf('/'));
				}

				if( seekpath == '') seekpath = '/';

				for( var a_type in ASSETS)
					seekpath = seekpath.replace('['+a_type+']', ASSETS[a_type].path);

				var apath = path;
				var aname = folders[i];
				if( subfolder )
					aname = nextfolder;
//window.console.log('seekpath-'+subfolder+'='+seekpath);
//window.console.log('apath='+apath);

				if( apath == seekpath )
				{
					if( subfolder )
					{
						if( apath == '/' ) apath = '/' + nextfolder;
						else apath = apath + '/' + nextfolder;
					}
					a_Create( asset_type, aname, apath);
					c_Log('Asset founded: ' + asset_type + '=' + aname);
					break;
				}
			}
		}
	}
}

function a_ShowHeaders()
{
	u_el.assets.innerHTML = '';

	var assets = [];
	for( var a_type in ASSETS ) assets.push( ASSETS[a_type]);
	assets.sort(function(a,b){if(a.path.length>b.path.length)return 1;return -1;});

	for( var i = 0; i < assets.length; i++)
	{
		var asset = assets[i];
//		if( RULES.assets[a_type].showontop === false ) continue;
		var a_type = asset.type;
		var a_name = asset.name;
		var href = '#' + asset.path;

		elHeader = document.createElement('div');
		u_el.assets.appendChild( elHeader);
		elHeader.classList.add('asset');

		elType = document.createElement('a');
		elHeader.appendChild( elType);
		elType.classList.add('type');
		elType.textContent = a_type + ':';
		elType.href = href;

		elName = document.createElement('a');
		elHeader.appendChild( elName);
		elName.classList.add('name');
		elName.textContent = a_name;
		elName.href = href;
	}
}

function a_OpenCloseSourceOnClick( i_evt)
{
	var el = i_evt.currentTarget;
	var elSource = el;

	if( elSource.m_scanned ) return;
	elSource.m_scanned = true;
	elSource.textContent = '';
	elSource.classList.remove('button');

	var founded = false;
	var paths = [];
	for( var i = 0; i < ASSET.source.path.length; i++)
		paths.push( ASSET.path + '/' + ASSET.source.path[i]);
	var walkdir = n_WalkDir( paths, 5);
	for( var i = 0; i < walkdir.length; i++)
	{
		var flist = [];
		a_SourceWalkFind( walkdir[i], flist);
		if( flist.length )
		{
			var elPath = document.createElement('div');
			elSource.appendChild( elPath);
			elPath.textContent = ASSET.source.path[i];
//console.log('flist='+JSON.stringify(flist));
			for( var f = 0; f < flist.length; f++)
				u_ShowFolder( elSource, paths[i], flist[f], flist[f]);
			founded = true;
		}
	}
	if( false == founded )
		elSource.textContent = JSON.stringify( ASSET.source.path);
}

function a_SourceWalkFind( i_walk, o_list, i_path)
{
//window.console.log( JSON.stringify( i_walk).replace(/,/g,', '));
	if( i_walk.folders == null ) return;

	i_walk.folders.sort( c_CompareFiles );
	for( var f = 0; f < i_walk.folders.length; f++)
	{
		var fobj = i_walk.folders[f];
//console.log(JSON.stringify(fobj));
		var path = i_path;
		if( path ) path += '/' + fobj.name;
		else path = fobj.name;
		if( fobj.files && fobj.files.length)
		{
			fobj.name = path;
			o_list.push( fobj);
		}
		a_SourceWalkFind( fobj, o_list, path);
	}
}

function a_ShowThumbnails()
{
	a_elThumbnails = [];

	if( ASSET.thumbnails === 0 )
	{
		var folders = g_elCurFolder.m_dir.folders;
		for( var f = 0; f < folders.length; f++ )
		{
			if( folders[f].name.indexOf('.') == 0 ) continue;
			if( folders[f].name.indexOf('_') == 0 ) continue;

			var path = g_elCurFolder.m_path + '/' + folders[f].name;

			var elFolder = document.createElement('div');
			a_elThumbnails.push( elFolder);
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
				a_elCurEditStatus = el;
				st_CreateEditUI( el, el.m_path, el.m_status, a_ThumbStatusApply, el.m_elStatus);
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

			a_elCurEditStatus = elFolder;
			a_ThumbStatusApply( folders[f].status);
			a_elCurEditStatus = null;
		}
	}

	if( ASSET.thumbnails > 0 )
	{
		a_elFolders = [];

		var walk = n_WalkDir([ASSET.path], ASSET.thumbnails, RULES.rufolder,['rules','status'],['status'])[0];
		if( walk.folders == null ) return;
		walk.folders.sort( c_CompareFiles );

		for( var sc = 0; sc < walk.folders.length; sc++)
		{
			var fobj = walk.folders[sc];
			if( fobj.name.indexOf('.') == 0 ) continue;
			if( fobj.name.indexOf('_') == 0 ) continue;

			var elScene = document.createElement('div');
			a_elFolders.push( elScene);
			elScene.m_elThumbnails = [];
			u_el.asset.appendChild( elScene);
			elScene.classList.add('scene');
			elScene.m_path = ASSET.path + '/' + fobj.name;
//			elScene.onclick = function(e){g_GO(e.currentTarget.m_path)};

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

			walk.folders[sc].folders.sort( c_CompareFiles );
			for( var s = 0; s < walk.folders[sc].folders.length; s++)
			{
				var fobj = walk.folders[sc].folders[s];
				if( fobj.name.indexOf('.') == 0 ) continue;
				if( fobj.name.indexOf('_') == 0 ) continue;

				var elShot = document.createElement('div');
				a_elThumbnails.push( elShot);
				elShot.m_status = fobj.status;
				elScene.appendChild( elShot);
				elScene.m_elThumbnails.push( elShot);
				elShot.classList.add('shot');
				elShot.m_path = elScene.m_path + '/' + fobj.name;
//				elShot.onclick = function(e){e.stopPropagation();g_GO(e.currentTarget.m_path)};
				elShot.ondblclick = function(e){
					var el = e.currentTarget;
					a_elCurEditStatus = el;
					st_CreateEditUI( el, el.m_path, el.m_status, a_ThumbStatusApply);
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

//				var elName = document.createElement('div');
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

				a_elCurEditStatus = elShot;
				a_ThumbStatusApply( fobj.status);
				a_elCurEditStatus = null;
			}
		}
	}
}

function a_ThumbStatusApply( i_status)
{
	if( i_status != null ) a_elCurEditStatus.m_status = c_CloneObj( i_status);
	st_SetElLabel( i_status, a_elCurEditStatus.m_elAnn, false);
	st_SetElArtists( i_status, a_elCurEditStatus.m_elArtists);
	st_SetElTags( i_status, a_elCurEditStatus.m_elTags);
	st_SetElProgress( i_status, a_elCurEditStatus.m_elProgressBar, a_elCurEditStatus.m_elProgress, a_elCurEditStatus.m_elPercent);
	st_SetElFinish( i_status, a_elCurEditStatus.m_elFinish, ASSET.thumbnails === 0 );
	st_SetElColor( i_status, a_elCurEditStatus);
}

function a_ThumbFilter( i_args)
{
	if( a_elThumbnails == null ) return;

//c_Info( JSON.stringify(i_args));
	if( i_args == null ) i_args = {};

	var anns = null;
	if( i_args.ann )
	{
		var anns_or = i_args.ann.split(',');
		anns = [];
		for( var o = 0; o < anns_or.length; o++)
			anns.push( anns_or[o].split(' '));
	}

	for( var th = 0; th < a_elThumbnails.length; th++)
	{
		var el = a_elThumbnails[th];
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
		}
		else
		{
			el.style.display = 'none';
			el.m_hidden = true;
		}
	}

	if( a_elFolders )
		for( var f = 0; f < a_elFolders.length; f++)
		{
			var oneShown = false;
			for( var t = 0; t < a_elFolders[f].m_elThumbnails.length; t++)
			{
				if( a_elFolders[f].m_elThumbnails[t].m_hidden != true )
				{
					oneShown = true;
					break;
				}
			}
			if( oneShown )
				a_elFolders[f].style.display = 'block';
			else
				a_elFolders[f].style.display = 'none';
		}
}

function a_ShowAllThumbnails()
{
	if( a_elThumbnails == null ) return;

	for( var i = 0; i < a_elThumbnails.length; i++)
		a_elThumbnails[i].style.display = 'block';

	if( a_elFolders )
		for( var i = 0; i < a_elFolders.length; i++)
			a_elFolders[i].style.display = 'block';
}

function a_FileLimitAdd( i_el)
{
	var elPanel = document.createElement('div');
	i_el.appendChild( elPanel);

	if( localStorage.asset_filelimit == null ) localStorage.asset_filelimit = '0';

	var limits = [3,10,30,0];

	elPanel.m_elLimits = [];

	for( var i = 0; i < limits.length; i++)
	{
		var elLimit = document.createElement('div');
		elPanel.appendChild( elLimit);
		elPanel.m_elLimits.push( elLimit);
		elLimit.classList.add('filelimit');

		var text = limits[i];
		if( text == 0 ) text = 'all';
		elLimit.textContent = text;

		elLimit.m_limit = limits[i];
		elLimit.onclick = function(e){
			localStorage.asset_filelimit = ''+e.currentTarget.m_limit;
			a_FileLimitsApplyAll();
		}
	}

	return elPanel;
}

function a_FileLimitsApplyAll()
{
	for( var i = 0; i < a_elFileLimits.length; i++)
	{
		var limit = 0;
		for( var j = 0; j < a_elFileLimits[i].m_elLimits.length; j++)
		{
			var el = a_elFileLimits[i].m_elLimits[j];
			if( parseInt( localStorage.asset_filelimit ) == el.m_limit )
			{
				limit = el.m_limit;
				el.classList.remove('button');
				if( limit ) el.classList.add('selected');
			}
			else
			{
				el.classList.add('button');
				el.classList.remove('selected');
			}
		}

		var elFiles = a_elFileLimits[i].m_elFiles;
		for( var f = 0; f < elFiles.length; f++)
			if( limit && ( f < ( elFiles.length-limit )))
				elFiles[f].style.display = 'none';
			else
				elFiles[f].style.display = 'block';
	}
}

