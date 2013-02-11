ASSETS = {};
ASSET = null;

a_elThumbnails = null;
a_elFolders = null;
a_elFilter = null;

function a_Finish()
{
	ASSETS = {};
	ASSET = null;

	u_el.assets.innerHTML = '';
	u_el.asset.innerHTML = '';

	a_elThumbnails = null;
	a_elFolders = null;
	a_elFilter = null;
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

function a_ShowBody()
{
	u_el.asset.innerHTML = '';
	if( ASSET == null ) return;

	thumbnails = [];

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

	if( walk.paths.length )
		walk.walks = n_WalkDir( walk.paths, 0);

	if( ASSET.result )
	{
		var elResult = document.createElement('div');
		u_el.asset.appendChild( elResult);
//		elResult.classList.add('sequences');

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

			if( folders.length )
			{
				thumbnails.push( path);
				u_ShowFolder( elResult, path, {"folders":folders})
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
			var folders = walk.walks[walk.dailies[i]].folders;
			if(( files && files.length ) || ( folders && folders.length ))
			{
				elPath = document.createElement('div');
				elDailies.appendChild( elPath);
				elPath.textContent = ASSET.dailies.path[i];
				u_ShowFolder( elDailies, path, walk.walks[walk.dailies[i]]);
				if( thumbnails.length == 0 )
					thumbnails.push( path);
				founded = true;
			}
		}

		if( false == founded )
			elDailies.textContent = JSON.stringify( ASSET.dailies.path );
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
				u_ShowSequence( elSource, paths[i]+'/'+fname);
				founded = true;
			}
		}
	}
	if( false == founded )
		elSource.textContent = JSON.stringify( ASSET.source.path);
}

function a_SourceWalkFind( i_walk, o_list, i_path)
{
//window.console.log( JSON.stringify( i_walk).replace(/,/g,', '));
	if( i_walk.folders == null ) return;

	for( var f = 0; f < i_walk.folders.length; f++)
	{
		var fobj = i_walk.folders[f];
		var path = i_path;
		if( path ) path += '/' + fobj.name;
		else path = fobj.name;
		if( fobj.files && fobj.files.length)
			o_list.push( path);
		a_SourceWalkFind( fobj, o_list, path);
	}
}

function a_ShowThumbnails()
{
	a_elThumbnails = [];

	a_elFilter = document.createElement('div');
	u_el.asset.appendChild( a_elFilter);
	a_elFilter.style.padding = '4px';
	a_elFilter.style.position = 'relative';

	var elLabel = document.createElement('div');
	a_elFilter.appendChild( elLabel);
	a_elFilter.m_elLabel = elLabel;
	elLabel.textContent = 'Filter';
	elLabel.style.textAlign = 'center';
	elLabel.style.cursor = 'pointer';
	elLabel.style.width = '50%';
	elLabel.onclick = function(e){
		var el = e.currentTarget.parentNode;
		if( el.m_filtering )
		{
			var args = {};
			args.status = el.m_elStatus.textContent;
			g_SetLocationArgs({"a_TFilter":args});
			return;
		}
		el.m_filtering = true;
		el.m_elBody.style.display='block';
		el.m_elLabel.classList.add('button');
	};

	var elBody = document.createElement('div');
	a_elFilter.appendChild( elBody);
	a_elFilter.m_elBody = elBody;
	elBody.style.display = 'none';

	var elStatusLabel = document.createElement('div');
	elBody.appendChild( elStatusLabel);
	elStatusLabel.textContent = 'Status:';
	elStatusLabel.style.cssFloat = 'left';

	var elStatus = document.createElement('div');
	elBody.appendChild( elStatus);
	a_elFilter.m_elStatus = elStatus;
	elStatus.style.height = '20px';
	elStatus.contentEditable = 'true';
	elStatus.classList.add('editing');

	var elBtnShowAll = document.createElement('div');
	elBody.appendChild( elBtnShowAll);
	elBtnShowAll.textContent = 'Show All';
//	elBtnShowAll.onclick = function(e){g_ClearLocationArgs();a_ShowAllThumbnails();};
	elBtnShowAll.onclick = function(e){g_SetLocationArgs({"a_TFilter":null});};
	elBtnShowAll.classList.add('button');
	elBtnShowAll.style.position = 'absolute';
	elBtnShowAll.style.top = '4px';
	elBtnShowAll.style.right = '4px';
	elBtnShowAll.style.width = '40%';

	if( ASSET.thumbnails === 0 )
	{
		var folders = g_elCurFolder.m_dir.folders;
		for( var f = 0; f < folders.length; f++ )
		{
			if( folders[f].name.indexOf('.') == 0 ) continue;
			if( folders[f].name.indexOf('_') == 0 ) continue;

			var elFolder = document.createElement('div');
			a_elThumbnails.push( elFolder);
			elFolder.m_status = folders[f].status;
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
			u_StatusSetElLabel( elStatus, folders[f].status, true);

//			if( folders[f].status )
//				if( folders[f].status.color )
					u_StatusSetColor( folders[f].status, elFolder);
		}
//window.console.log( JSON.stringify( folders));
	}

	if( ASSET.thumbnails > 0 )
	{
		a_elFolders = [];

		var walk = n_WalkDir([ASSET.path], ASSET.thumbnails, RULES.rufolder,['rules','status'],['status'])[0];

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
				a_elThumbnails.push( elShot);
				elShot.m_status = fobj.status;
				elScene.appendChild( elShot);
				elScene.m_elThumbnails.push( elShot);
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

//				if( fobj.status )
					u_StatusSetColor( fobj.status, elShot);
			}
		}
	}
}

function a_TFilter( i_args)
{
//c_Info( JSON.stringify(i_args));
	if( a_elFilter )
	{
		a_elFilter.m_elBody.style.display = 'block';
		a_elFilter.m_elLabel.classList.add('button');
		if( i_args )
			if( i_args.status )
				a_elFilter.m_elStatus.textContent = i_args.status;
	}

	if( a_elThumbnails == null )
	{
		c_Error('Asset does not have any thumbnails.');
		return;
	}

	var anns = null;
	if( i_args && i_args.status )
	{
		var anns_or = i_args.status.split(',');
		anns = [];
		for( var o = 0; o < anns_or.length; o++)
			anns.push( anns_or[o].split(' '));
	}

	for( var i = 0; i < a_elThumbnails.length; i++)
	{
		var founded = (i_args == null);

		if( a_elThumbnails[i].m_status && anns )
			if( a_elThumbnails[i].m_status.annotation )
				for( var o = 0; o < anns.length; o++)
				{
					var founded_and = true;
					for( var a = 0; a < anns[o].length; a++)
					{
						if( a_elThumbnails[i].m_status.annotation.indexOf( anns[o][a]) == -1 )
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

		if( founded )
		{
			a_elThumbnails[i].style.display = 'block';
			a_elThumbnails[i].m_hidden = false;
		}
		else
		{
			a_elThumbnails[i].style.display = 'none';
			a_elThumbnails[i].m_hidden = true;
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
	for( var i = 0; i < a_elThumbnails.length; i++)
		a_elThumbnails[i].style.display = 'block';

	if( a_elFolders )
		for( var i = 0; i < a_elFolders.length; i++)
			a_elFolders[i].style.display = 'block';
}

