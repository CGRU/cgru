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

	if( ASSET )
		c_RulesMergeObjs( ASSET, RULES.assets[ASSET.type]);

	a_ShowHeaders();

	a_ShowBody();

	var path = cgru_PM('/'+RULES.root+g_elCurFolder.m_path);
	c_Info( path);
	u_el.open.setAttribute('cmdexec', JSON.stringify([RULES.open.replace(/@PATH@/g, path)]));
}

function a_WalkDir( i_walk, o_list, i_path)
{
	if( o_list.folders == null )
		o_list.folders = [];
	if( i_path == null )
		i_path = '';

//window.console.log( JSON.stringify( i_walk).replace(/,/g,', '));
	if( i_walk.folders )
		for( var folder in i_walk.folders)
		{
			path = i_path + '/' + folder;
			walk = i_walk.folders[folder];
			if( walk.files && walk.files.length)
				o_list.folders.push( path);
			a_WalkDir( walk, o_list, path);
		}
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
			d_Make( e.currentTarget.m_path, ASSET.path+'/'+ASSET.dailies[0])};
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

	if( ASSET.result )
	{
		var elResult = document.createElement('div');
		u_el.asset.appendChild( elResult);
		elResult.classList.add('sequences');

		var founded = false;
		for( var r = 0; r < ASSET.result.path.length; r++)
		{
			var path = ASSET.path + '/' + ASSET.result.path[r];
			var readdir = n_ListDir( path);
			if( readdir == null ) continue;
			var folders = readdir.folders;
			if( folders == null ) continue;
			if( folders.length )
			{
				var elPath = document.createElement('div');
				elResult.appendChild( elPath);
				elPath.textContent = ASSET.result.path[r];
			}
			else
				continue;

			thumbnails.push( path);

			for( var f = 0; f < folders.length; f++)
			{
				var folder = path + '/' + folders[f];
				a_ShowSequence( elResult, folder);
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
		for( var d = 0; d < ASSET.dailies.length; d++)
		{
			var path = ASSET.path + '/' + ASSET.dailies[d];
			var readdir = n_ListDir( path);
			if( readdir == null ) continue;
			var files = readdir.files;
			if( files == null ) continue;
			if( files.length )
			{
				elPath = document.createElement('div');
				elDailies.appendChild( elPath);
				elPath.textContent = ASSET.dailies[d];
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
			elDailies.textContent = JSON.stringify( ASSET.dailies );
	}

	if( ASSET.statuses )
	{
		var folders = g_elCurFolder.m_dir.folders;
		for( var f = 0; f < folders.length; f++ )
		{
			if( folders[f].name.indexOf('.') == 0 ) continue;

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

			if( folders[f].status )
			{
				if( folders[f].status.annotation )
					elStatus.innerHTML = folders[f].status.annotation;

				if( folders[f].status.color )
					u_StatusSetColor( folders[f].status.color, elFolder);
			}
		}
//window.console.log( JSON.stringify( folders));
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
	for( var i = 0; i < folders.length-1; i++ )
	{
		if(( folders[i].length == 0 ) && ( i != 0 )) continue;

		var folder = folders[i+1];
		if( folder == '' ) continue;

		if( path == '/' )
			path += folders[i];
		else
			path += '/' + folders[i];
//window.console.log('path='+path);
		for( var asset_type in RULES.assets)
		{
			if( ASSETS[asset_type]) continue;
//window.console.log( asset_type);
			var apaths = RULES.assets[asset_type].seek;
			if( apaths == null ) continue;
			for( var l = 0; l < apaths.length; l++)
			{
				var apath = apaths[l].substr( 0, apaths[l].lastIndexOf('/'));
				if( apath == '') apath = '/';
				for( var a_type in ASSETS)
				{
					var a_name = ASSETS[a_type].name;
					if( a_name == null ) continue;
					apath = apath.replace('['+a_type+']', a_name);
				}
//window.console.log('apath='+apath);
				if( path == apath )
				{
					if( apath == '/' ) apath = '/' + folder;
					else apath = apath + '/' + folder;
					a_Create( asset_type, folder, apath);
					c_Log('Asset founded: ' + asset_type + '=' + folder);
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
		var a_name = asset.name;

		elHeader = document.createElement('div');
		u_el.assets.appendChild( elHeader);
		elHeader.classList.add('asset');

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
	for( var r = 0; r < ASSET.source.path.length; r++)
	{
		var list = {};
		var path = ASSET.path + '/' + ASSET.source.path[r];
		a_WalkDir( n_WalkDir( path), list);
		if( list.folders.length )
		{
			var elPath = document.createElement('div');
			elSource.appendChild( elPath);
			elPath.textContent = ASSET.source.path[r];
			for( var f = 0; f < list.folders.length; f++)
			{
				a_ShowSequence( elSource, path + list.folders[f], list.folders[f]);
				founded = true;
			}
		}
	}
	if( false == founded )
		elSource.textContent = JSON.stringify( ASSET.source.path);
}

