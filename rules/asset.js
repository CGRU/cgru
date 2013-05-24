ASSETS = {};
ASSET = null;

function View_asset_Open() { a_Show(); }
function View_asset_Close() { $('asset').textContent = ''; }

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

	a_SetLabel('Asset');
	$('asset_div').style.display = 'none';
	u_el.assets.innerHTML = '';
	u_el.asset.innerHTML = '';
}

function a_SetLabel( i_label)
{
	$('asset_label').innerHTML = i_label;
}

function a_Show()
{
	$('asset_div').style.display = 'none';

	if( ASSET == null ) return;

	if(( ASSET.show_in_subdirectories !== true ) && ( ASSET.path != g_CurPath()))
		return;

	if( ASSET.show_files !== true )
		$('files_div').style.display = 'none';
	$('asset_div').style.display = 'block';

	window.document.title = ASSET.name + ' ' + window.document.title;

	if( ASSET.scripts )
	for( var i = 0; i < ASSET.scripts.length; i++ )
	{
		var script = document.createElement('script');
//		script.type = 'text/javascript';
		//ga.async = true;
		//ga.async = false;
	    script.src = 'rules/assets/' + ASSET.scripts[i];
	    var scripts = document.getElementsByTagName('script')[0];
		scripts.parentNode.insertBefore( script, scripts);
	}
}

function a_Create( i_type, i_name, i_path, i_absolute)
{
	var asset = {};
	asset.name = i_name;
	asset.path = i_path;
	asset.type = i_type;
	asset.absolute = i_absolute;

	c_RulesMergeObjs( asset, RULES.assets[i_type]);

	ASSETS[i_type] = asset;
	if( ASSET )
	{
		if( asset.path.length > ASSET.path.length )
			ASSET = asset;
	}
	else
		ASSET = asset;

	if( i_absolute )
		c_Log('Asset specified: ' + i_type + '=' + i_name);
	else
		c_Log('Asset founded: ' + i_type + '=' + i_name);
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
//			if( ASSETS[atype] ) continue;
			if( attr != atype ) continue;
			a_Create( atype, RULES[attr], i_path, true);
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

					var exists = false; // Verify whether an asset with the same path exists
					for( var asset in ASSETS)
					{
//console.log( ASSETS[asset].path + ' == ' + apath);
						if( ASSETS[asset].path == apath)
						{
							exists = true;
							break;
						}
					}
					if( exists ) break;

					a_Create( asset_type, aname, apath, false);
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

