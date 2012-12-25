ASSETS = {};

function a_Process()
{
	a_AutoSeek();

	a_ShowHeaders();

	a_ShowBody();

	c_Info( g_elCurFolder.m_path);
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
			elResult.textContent = 'Result';

			for( var r = 0; r < data.result.length; r++)
			{
				var path = asset.path + '/' + data.result[r];
				var readdir = n_ReadDir( path);
				if( readdir == null ) continue;
				var folders = readdir.folders;
				if( folders == null ) continue;
				for( var f = 0; f < folders.length; f++)
				{
					var folder = folders[f];

					var elLinkDiv = document.createElement('div');
					elResult.appendChild( elLinkDiv);

					var elLinkA = document.createElement('a');
					elLinkDiv.appendChild( elLinkA);
					elLinkA.setAttribute('href', RULES.root + path + '/' + folder);
					elLinkA.setAttribute('target', '_blank');
					elLinkA.textContent = folder;
				}
			}
		}

		if( data.dailies )
		{
			var elDailies = document.createElement('div');
			g_el.asset.appendChild( elDailies);
			elDailies.textContent = 'Dailies';

			for( var d = 0; d < data.dailies.length; d++)
			{
				var path = asset.path + '/' + data.dailies[d];
				var readdir = n_ReadDir( path);
				if( readdir == null ) continue;
				var files = readdir.files;
				if( files == null ) continue;
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
				}
			}
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
			c_Info('Asset: ' + asset.type + '=' + asset.name);
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
					c_Info('Asset founded: ' + asset.type + '=' + asset.name);
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

