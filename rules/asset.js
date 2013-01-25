ASSETS = {};
ASSET = null;

function a_Finish()
{
	ASSETS = {};
	ASSET = null;
}

function a_Process()
{
	a_AutoSeek();

//	if( ASSET ) c_RulesMergeObjs( ASSET, RULES.assets[ASSET.type]);

	a_ShowHeaders();

	a_ShowBody();

	var path = cgru_PM('/'+RULES.root+g_elCurFolder.m_path);
	c_Info( path);
	u_el.open.setAttribute('cmdexec', JSON.stringify([RULES.open.replace(/@PATH@/g, path)]));
}

function a_ShowSequence( i_element, i_path, i_title)
{
	var link = RULES.root + i_path;

	if( i_title == null)
	{
		i_title = i_path.split('/');
		i_title = i_title[i_title.length-1];
	}

	var elFolder = document.createElement('div');
	elFolder.classList.add('folder');
	i_element.appendChild( elFolder);

	var elLinkA = document.createElement('a');
	elFolder.appendChild( elLinkA);
	elLinkA.setAttribute('href', link);
	elLinkA.setAttribute('target', '_blank');
	elLinkA.textContent = i_title;

	var cmds = RULES.cmdexec[ASSET.result.cmdexec];
	for( var c = 0; c < cmds.length; c++)
	{
		var elCmd = document.createElement('div');
		elFolder.appendChild( elCmd);
		elCmd.classList.add('cmdexec');
		elCmd.textContent = cmds[c].name;
		var cmd = cmds[c].cmd;
		cmd = cmd.replace('@PATH@', cgru_PM('/'+link));
		cmd = cmd.replace('@FPS@', RULES.fps);
		elCmd.setAttribute('cmdexec', JSON.stringify([cmd]));
	}

	if( ASSET.dailies )
	{
		var elMakeDailies = document.createElement('div');
		elFolder.appendChild( elMakeDailies);
		elMakeDailies.classList.add('button');
		elMakeDailies.textContent = 'Make Dailies';
		elMakeDailies.m_path = i_path;
		elMakeDailies.onclick = function(e){
			d_Make( e.currentTarget.m_path, ASSET.path+'/'+ASSET.dailies.path[0])};
	}
}

function a_ShowBody()
{
	u_el.asset.innerHTML = '';
	if( ASSET == null ) return;

	thumbnails = [];

	window.document.title = ASSET.name + ' ' + window.document.title;

	if( ASSET.source )
	{
		var elSource = document.createElement('div');
		u_el.asset.appendChild( elSource);
		elSource.classList.add('sequences');
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
	walk.walks = n_WalkDir( walk.paths, 0);

	if( ASSET.result )
	{
		var elResult = document.createElement('div');
		u_el.asset.appendChild( elResult);
		elResult.classList.add('sequences');

		var founded = false;
		for( var i = 0; i < walk.result.length; i++)
		{
			var folders = walk.walks[walk.result[i]].folders;
			var path = walk.paths[walk.result[i]];
			if( folders == null ) continue;
			if( folders.length )
			{
				var elPath = document.createElement('div');
				elResult.appendChild( elPath);
				elPath.textContent = ASSET.result.path[i];
			}
			else
				continue;

			thumbnails.push( path);

			for( var f = 0; f < folders.length; f++)
			{
				var fname = path + '/' + folders[f].name;
				a_ShowSequence( elResult, fname);
				founded = true;
			}
		}

		if( false == founded )
			elResult.textContent = JSON.stringify( ASSET.result.path );
	}

	if( ASSET.dailies )
	{
		var elDailies = document.createElement('div');
		u_el.asset.appendChild( elDailies);

		var founded = false;
		for( var i = 0; i < walk.dailies.length; i++)
		{
			var path = walk.paths[walk.dailies[i]];
			var files = walk.walks[walk.dailies[i]].files;
			if( files == null ) continue;
			if( files.length )
			{
				elPath = document.createElement('div');
				elDailies.appendChild( elPath);
				elPath.textContent = ASSET.dailies.path[i];
			}
			else
				continue;

			for( var f = 0; f < files.length; f++)
			{
				var file = files[f];
				if( file.indexOf('.mov') == -1 ) continue;

				var elLinkDiv = document.createElement('div');
				elDailies.appendChild( elLinkDiv);

				var elLinkA = document.createElement('a');
				elLinkDiv.appendChild( elLinkA);
				elLinkA.setAttribute('href', RULES.root + path + '/' + file);
				elLinkA.setAttribute('target', '_blank');
				elLinkA.textContent = file;

				founded = true;
			}
		}

		if( false == founded )
			elDailies.textContent = JSON.stringify( ASSET.dailies.path );
	}

	if( ASSET.statuses )
	{
		var folders = g_elCurFolder.m_dir.folders;
		for( var f = 0; f < folders.length; f++ )
		{
			if( folders[f].name.indexOf('.') == 0 ) continue;
			if( folders[f].name.indexOf('_') == 0 ) continue;

			var elFolder = document.createElement('div');
			u_el.asset.appendChild( elFolder);
			elFolder.style.padding = '4px';

			var elImg = document.createElement('img');
			elFolder.appendChild( elImg);
			elImg.classList.add('thumbnail');
			elImg.m_path = g_elCurFolder.m_path + '/' + folders[f].name;
			elImg.onclick = function(e){g_GO(e.currentTarget.m_path)};
			elImg.src = RULES.root + g_elCurFolder.m_path + '/' + folders[f].name + '/' + RULES.rufolder + '/' + RULES.thumbnail.filename;

			var elName = document.createElement('div');
			elFolder.appendChild( elName);
			elName.classList.add('button');
			elName.m_path = g_elCurFolder.m_path + '/' + folders[f].name;
			elName.onclick = function(e){g_GO(e.currentTarget.m_path)};
			elName.textContent = folders[f].name;

			var elStatus = document.createElement('div');
			elFolder.appendChild( elStatus);
			u_StatusSetElLabel( elStatus, folders[f].status);

			if( folders[f].status )
				if( folders[f].status.color )
					u_StatusSetColor( folders[f].status, elFolder);
		}
//window.console.log( JSON.stringify( folders));
	}

	if( ASSET.thumbnails && ( ASSET.thumbnails > 0 ))
	{
		var walk = n_WalkDir([ASSET.path], ASSET.thumbnails, RULES.rufolder,['rules','status'],['status'])[0];
		for( var sc = 0; sc < walk.folders.length; sc++)
		{
			var fobj = walk.folders[sc];
			if( fobj.name.indexOf('.') == 0 ) continue;
			if( fobj.name.indexOf('_') == 0 ) continue;

			var elScene = document.createElement('div');
			u_el.asset.appendChild( elScene);
			elScene.classList.add('scene');
			elScene.m_path = ASSET.path + '/' + fobj.name;
			elScene.onclick = function(e){g_GO(e.currentTarget.m_path)};

			var elName = document.createElement('div');
			elScene.appendChild( elName);
			elName.classList.add('name');
			elName.textContent = fobj.name;

			var elStatus = document.createElement('div');
			elScene.appendChild( elStatus);
			elStatus.classList.add('status');
//window.console.log(JSON.stringify(fobj));
			u_StatusSetElLabel( elStatus, fobj.status);
			if( fobj.status )
				u_StatusSetColor( fobj.status, elScene);

			for( var s = 0; s < walk.folders[sc].folders.length; s++)
			{
				var fobj = walk.folders[sc].folders[s];
				if( fobj.name.indexOf('.') == 0 ) continue;
				if( fobj.name.indexOf('_') == 0 ) continue;

				var elShot = document.createElement('div');
				elScene.appendChild( elShot);
				elShot.classList.add('shot');
				elShot.m_path = elScene.m_path + '/' + fobj.name;
				elShot.onclick = function(e){e.stopPropagation();g_GO(e.currentTarget.m_path)};

				var elImg = document.createElement('img');
				elShot.appendChild( elImg);
				elImg.src = RULES.root + elShot.m_path +'/'+ RULES.rufolder +'/'+ RULES.thumbnail.filename;

				var elName = document.createElement('div');
				elShot.appendChild( elName);
				elName.classList.add('name');
				elName.textContent = fobj.name;

				var elStatus = document.createElement('div');
				elShot.appendChild( elStatus);
				elStatus.classList.add('status');
				u_StatusSetElLabel( elStatus, fobj.status);
//window.console.log(JSON.stringify(fobj));
				if( fobj.status )
					u_StatusSetColor( fobj.status, elShot);
			}
		}
	}

	if( thumbnails.length )
		c_MakeThumbnail( thumbnails, ASSET.path);
}

function a_Create( i_type, i_name, i_path)
{
	var asset = {};
	asset.name = i_name;
	asset.path = i_path;
	asset.type = i_type;

	c_RulesMergeObjs( asset, RULES.assets[i_type]);

	ASSETS[i_type] = asset;
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
			c_Log('Asset: ' + atype + '=' + asset.name);
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
				{
					var a_name = ASSETS[a_type].name;
					if( a_name == null ) continue;
					seekpath = seekpath.replace('['+a_type+']', a_name);
				}

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

	for( var a_type in ASSETS)
	{
		var asset = ASSETS[a_type];
//		if( RULES.assets[a_type].showontop === false ) continue;
		var a_name = asset.name;

		elHeader = document.createElement('div');
		u_el.assets.appendChild( elHeader);
		elHeader.classList.add('asset');
		elHeader.m_path = asset.path;
		elHeader.onclick = function(e){g_GO(e.currentTarget.m_path)};

		elType = document.createElement('div');
		elHeader.appendChild( elType);
		elType.classList.add('type');
		elType.textContent = a_type + ':';

		elName = document.createElement('div');
		elHeader.appendChild( elName);
		elName.classList.add('name');
		elName.textContent = a_name;
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
			for( var f = 0; f < flist.length; f++)
			{
				var fname = flist[f];
				a_ShowSequence( elSource, path + fname, fname);
				founded = true;
			}
		}
	}
	if( false == founded )
		elSource.textContent = JSON.stringify( ASSET.source.path);
}

function a_SourceWalkFind( i_walk, o_list, i_path)
{
	if( i_path == null )
		i_path = '';

//window.console.log( JSON.stringify( i_walk).replace(/,/g,', '));
	if( i_walk.folders )
		for( var f = 0; f < i_walk.folders.length; f++)
		{
			var fobj = i_walk.folders[f];
			path = i_path + '/' + fobj.name;
			if( fobj.files && fobj.files.length)
				o_list.push( path);
			if( fobj.walkdir )
				a_SourceWalkFind( fobj, o_list, path);
		}
}

