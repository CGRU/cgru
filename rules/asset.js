ASSETS = {};

function a_Process()
{
	a_AutoSeek();

	a_ShowHeaders();

	a_ShowBody();

	c_Info( cgru_PM( '/'+RULES.root+g_elCurFolder.m_path));
}

function a_ShowBody()
{
	g_el.asset.innerHTML = '';
	for( var a_type in ASSETS)
	{
		var asset = ASSETS[a_type];
		var a_name = asset.name;

		var data = RULES.assets_data[a_type];
		if( data == null ) continue;

		if( data.result )
		{
			var elResult = document.createElement('div');
			g_el.asset.appendChild( elResult);
			elResult.classList.add('result');

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
					elPath = document.createElement('div');
					elResult.appendChild( elPath);
					elPath.textContent = data.result.path[r];
				}
				else
					continue;

				for( var f = 0; f < folders.length; f++)
				{
					var link = RULES.root + path + '/' + folders[f];

					var elFolder = document.createElement('div');
					elFolder.classList.add('folder');
					elResult.appendChild( elFolder);

					var elLinkA = document.createElement('a');
					elFolder.appendChild( elLinkA);
					elLinkA.setAttribute('href', link);
					elLinkA.setAttribute('target', '_blank');
					elLinkA.textContent = folders[f];

					var elCmd = document.createElement('div');
					elFolder.appendChild( elCmd);
					elCmd.classList.add('menu');
					elCmd.textContent = 'CMD';
					elCmd.onmouseover = function(e){ return a_onMouseOver_ResultCmd(e);}
					elCmd.m_path = '/'+link;
					elCmd.m_exec = data.result.cmdexec;

					var elMakeDailies = document.createElement('div');
					elFolder.appendChild( elMakeDailies);
					elMakeDailies.classList.add('button');
					elMakeDailies.textContent = 'Make Dailies';

					founded = true;
				}
			}

			if( false == founded )
				elResult.textContent = JSON.stringify( data.result.path );
		}

		if( data.dailies )
		{
			var elDailies = document.createElement('div');
			g_el.asset.appendChild( elDailies);

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
	}

	g_el.data.innerHTML = 'RULES='+JSON.stringify( RULES)+'<br><br>ASSETS='+JSON.stringify( ASSETS);
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
	g_el.assets.innerHTML = '';

	for( var a_type in ASSETS)
	{
		var asset = ASSETS[a_type];
		var a_name = asset.name;

		elHeader = document.createElement('div');
		g_el.assets.appendChild( elHeader);
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

function a_onMouseOver_ResultCmd( i_evt)
{
	cgru_MenusCloseAll();
	var menu = new cgru_Menu( document, document.body, i_evt, null, 'asset');
	var path = cgru_PM( i_evt.currentTarget.m_path);
	var exec = i_evt.currentTarget.m_exec;
	for( var c = 0; c < exec.length; c++)
	{
		var cmd = exec[c].replace('@PATH@', path);
		menu.addItem( name, 'cmdexec', [cmd], cmd);
	}
	menu.show();
}

