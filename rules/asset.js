ASSETS = {};

function a_Process()
{
	a_AutoSeek();

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

function a_ShowSequence( i_element, i_asset, i_data, i_path, i_title)
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

	var cmds = RULES.cmdexec[i_data.result.cmdexec];
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

	if( i_data.dailies )
	{
		var elMakeDailies = document.createElement('div');
		elFolder.appendChild( elMakeDailies);
		elMakeDailies.classList.add('button');
		elMakeDailies.textContent = 'Make Dailies';
		elMakeDailies.m_path = i_path;
		elMakeDailies.onclick = function(e){
			d_Make( e.currentTarget.m_path, i_asset.path+'/'+i_data.dailies[0])};
	}
}

function a_ShowBody()
{
	u_el.asset.innerHTML = '';
	for( var a_type in ASSETS)
	{
		var asset = ASSETS[a_type];
		var a_name = asset.name;

		window.document.title = a_name + ' ' + window.document.title;


		var data = RULES.assets_data[a_type];
		if( data == null ) continue;

		var thumbnails = [];

		if( data.source )
		{
			var elSource = document.createElement('div');
			u_el.asset.appendChild( elSource);
			elSource.classList.add('sequences');
			elSource.classList.add('button');
			elSource.textContent = 'Scan Sources';
			elSource.onclick = a_OpenCloseSourceOnClick;

			elSource.m_asset = asset;
			elSource.m_data = data;
		}

		if( data.result )
		{
			var elResult = document.createElement('div');
			u_el.asset.appendChild( elResult);
			elResult.classList.add('sequences');

			var founded = false;
			for( var r = 0; r < data.result.path.length; r++)
			{
				var path = asset.path + '/' + data.result.path[r];
				var readdir = n_ReadDir( path);
				if( readdir == null ) continue;
				var folders = readdir.folders;
				if( folders == null ) continue;
				if( folders.length )
				{
					var elPath = document.createElement('div');
					elResult.appendChild( elPath);
					elPath.textContent = data.result.path[r];
				}
				else
					continue;

				thumbnails.push( path);

				for( var f = 0; f < folders.length; f++)
				{
					var folder = path + '/' + folders[f];
					a_ShowSequence( elResult, asset, data, folder);
					founded = true;
				}
			}

			if( false == founded )
				elResult.textContent = JSON.stringify( data.result.path );
		}

		if( data.dailies )
		{
			var elDailies = document.createElement('div');
			u_el.asset.appendChild( elDailies);

			var founded = false;
			for( var d = 0; d < data.dailies.length; d++)
			{
				var path = asset.path + '/' + data.dailies[d];
				var readdir = n_ReadDir( path);
				if( readdir == null ) continue;
				var files = readdir.files;
				if( files == null ) continue;
				if( files.length )
				{
					elPath = document.createElement('div');
					elDailies.appendChild( elPath);
					elPath.textContent = data.dailies[d];
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
				elDailies.textContent = JSON.stringify( data.dailies );
		}

		if( thumbnails.length )
			c_MakeThumbnail( thumbnails, asset.path);
	}

	u_el.rules.innerHTML = 'ASSETS='+JSON.stringify( ASSETS)+'<br><br>RULES='+JSON.stringify( RULES);
}

function a_Append( i_path, i_rules)
{
//window.console.log('a_Append: '+ i_path);
//window.console.log( JSON.stringify( i_rules));
	for( var rules in i_rules)
	for( var attr in i_rules[rules])
	{
//window.console.log('attr='+attr);
		for( var a = 0; a < RULES.assets.length; a++)
		{
			if( ASSETS[attr] ) continue;
			if( attr != RULES.assets[a] ) continue;
			asset = {};
			asset.type = attr;
			asset.name = RULES[attr];
			asset.path = i_path;
			ASSETS[attr] = asset;
			c_Log('Asset: ' + asset.type + '=' + asset.name);
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
		for( var j = 0; j < RULES.assets.length; j++)
		{
			var asset_type = RULES.assets[j];
			if( ASSETS[asset_type]) continue;
//window.console.log( asset_type);
			if( RULES.assets_seek[asset_type] == null ) continue;
			var apaths = RULES.assets_seek[asset_type];
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
					var asset_name = folder;
					var asset = {};
					asset.name = asset_name;
					asset.type = asset_type;
					if( path == '/' ) asset.path = '/' + folder;
					else asset.path = path + '/' + folder;
					ASSETS[asset_type] = asset;
					c_Log('Asset founded: ' + asset.type + '=' + asset.name);
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
	var asset = el.m_asset;
	var data = el.m_data;
	var elSource = el;

	if( elSource.m_scanned ) return;
	elSource.m_scanned = true;
	elSource.textContent = '';
	elSource.classList.remove('button');

	var founded = false;
	for( var r = 0; r < data.source.path.length; r++)
	{
		var list = {};
		var path = asset.path + '/' + data.source.path[r];
		a_WalkDir( n_WalkDir( path), list);
		if( list.folders.length )
		{
			var elPath = document.createElement('div');
			elSource.appendChild( elPath);
			elPath.textContent = data.source.path[r];
			for( var f = 0; f < list.folders.length; f++)
			{
				a_ShowSequence( elSource, asset, data, path + list.folders[f], list.folders[f]);
				founded = true;
			}
		}
	}
	if( false == founded )
		elSource.textContent = JSON.stringify( data.source.path);
}

