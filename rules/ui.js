u_elements = ['asset','assets','content','info','open','log','navig','rules','status_annotation','status_artists','status_tags','status_percentage','status_progress','status_progressbar','cycle','files','content_info','content_status','thumbnail','sidepanel','status_finish'];
u_el = {};

u_show_description_brief = 0;

function u_Init()
{
	for( var i = 0; i < u_elements.length; i++) u_el[u_elements[i]] = document.getElementById( u_elements[i]);

	if( u_el.sidepanel )
	{
		if( localStorage.sidepanel_opened_width == null ) localStorage.sidepanel_opened_width = 200;
		if( localStorage.sidepanel_closed_width == null ) localStorage.sidepanel_closed_width = 20;
		if( localStorage.sidepanel_opened == 'true' ) u_SidePanelOpen();
		else u_SidePanelClose();
	}

	if( localStorage.header_opened == 'true' ) u_OpenCloseHeader();
	if( localStorage.footer_opened == 'true' ) u_OpenCloseFooter();

	if( p_PLAYER != true)
	{
		if( localStorage.show_hidden == null ) localStorage.show_hidden = 'OFF';
		$('show_hidden').textContent = localStorage.show_hidden;
		$('search_artists').m_elArtists = [];
	}
}
function u_OpenCloseHeader(){u_OpenCloseHeaderFooter(document.getElementById('headeropenbtn'),'header',-200,0);}
function u_OpenCloseFooter(){u_OpenCloseHeaderFooter(document.getElementById('footeropenbtn'),'footer',38,238);}

function u_ShowHiddenToggle()
{
	if( localStorage.show_hidden == 'ON') localStorage.show_hidden = 'OFF';
	else localStorage.show_hidden = 'ON';
	$('show_hidden').textContent = localStorage.show_hidden;
}

function u_Process()
{
	if( g_elCurFolder.m_dir.rufiles && ( g_elCurFolder.m_dir.rufiles.indexOf( RULES.thumbnail.filename ) != -1 ))
	{
		u_el.thumbnail.setAttribute('src', RULES.root+g_elCurFolder.m_path+'/'+RULES.rufolder+'/'+RULES.thumbnail.filename);
		u_el.thumbnail.style.display = 'inline';
	}
	else
	{
		u_el.thumbnail.setAttribute('src', null );
		u_el.thumbnail.style.display = 'none';
	}

	u_el.rules.innerHTML = 'ASSET='+JSON.stringify( ASSET)+'<br><br>ASSETS='+JSON.stringify( ASSETS)+'<br><br>RULES='+JSON.stringify( RULES);
//	u_el.rules.innerHTML = 'ASSET='+JSON.stringify( ASSET)+'<br><br>RULES='+JSON.stringify( RULES);

	if( ASSET && ( ASSET.path == g_elCurFolder.m_path ))
		u_el.files.parentNode.style.display = 'none';
	else
		u_FilesOnClick();

	u_StatusApply();
	nw_Process();
	cm_Process();
}

function u_InitUsers()
{
	$('search_artists_div').style.display = 'block';
	var elArtists = $('search_artists');
	for( var user in g_users )
	{
		el = document.createElement('div');
		elArtists.appendChild( el);
		el.style.cssFloat = 'left';
		el.textContent = c_GetUserTitle( user);
		el.m_user = user;
		el.classList.add('tag');
		el.onclick = function(e){ c_ElToggleSelected(e); if( a_elThumbnails ) u_SearchSearch();};
		elArtists.m_elArtists.push( el);
	}
}

function u_StatusApply( i_status)
{
	if( i_status != null )
		RULES.status = c_CloneObj( i_status);
	else
		i_status = RULES.status;

	st_SetElAnnotation( i_status, u_el.status_annotation);
	st_SetElColor( i_status, u_el.content_info);
	st_SetElProgress( i_status, u_el.status_progressbar, u_el.status_progress, u_el.status_percentage);
	st_SetElArtists( i_status, u_el.status_artists);
	st_SetElTags( i_status, u_el.status_tags);
	st_SetElFinish( i_status, u_el.status_finish);
	u_ShowDescription();
}
function u_ShowDescription( i_elToggleBrief)
{
	if( i_elToggleBrief )
	{
		u_show_description_brief = 1 - u_show_description_brief;
		if( u_show_description_brief ) i_elToggleBrief.textContent = '-';
		else i_elToggleBrief.textContent = '+';
	}
	if( RULES.status && RULES.status.description && c_Strip(RULES.status.description).length )
	{
		var text = c_Strip( RULES.status.description);
		var maxlen = 48;
		if( u_show_description_brief )
		{
			text = text.replace(/<br>/g, ' ');
			if( text.length > maxlen )
				text = text.substr(0,maxlen)+'...';
		}
		$('description').innerHTML = text;
	}
	else $('description').innerHTML = '';
}

function u_StatusEditOnClick()
{
	st_CreateEditUI( u_el.content_info, g_CurPath(), RULES.status, u_StatusApply, u_el.content_status);
}

function u_FilesOnClick( i_el)
{
	if( u_el.files.m_opened ) return;
	u_el.files.parentNode.style.display = 'block';
	u_el.files.m_opened = true;
	$('files_btn').style.display = 'none';
	u_ShowFolder( u_el.files, g_elCurFolder.m_path, g_elCurFolder.m_dir);
}

function u_Finish()
{
	$('files_btn').style.display = 'block';
	u_el.files.m_opened = false;

	st_DestroyEditUI();
	u_el.files.textContent = '';

	u_StatusApply( null);
//	u_el.status_annotation.textContent = '';
//	st_StatusSetColor( null, u_el.content_info);

	u_el.thumbnail.style.display = 'none';

	nw_Finish();
	cm_Finish();
}

function u_OpenCloseHeaderFooter( i_elBtn, i_id, i_closed, i_opened)
{
	if( i_elBtn.classList.contains('opened'))
	{
		i_elBtn.classList.remove('opened');
		if( i_id == 'header')
		{
			localStorage.header_opened = 'false';
			i_elBtn.innerHTML = '&darr;';
			document.getElementById( i_id).style.top = i_closed+'px';
		}
		else
		{
			localStorage.footer_opened = 'false';
			i_elBtn.innerHTML = '&uarr;';
			document.getElementById('footer').style.height = i_closed+'px';
			document.getElementById('log').style.display= 'none';
		}
	}
	else
	{
		i_elBtn.classList.add('opened');
		if( i_id == 'header')
		{
			localStorage.header_opened = 'true';
			i_elBtn.innerHTML = '&uarr;';
			document.getElementById( i_id).style.top = i_opened+'px';
		}
		else
		{
			localStorage.footer_opened = 'true';
			i_elBtn.innerHTML = '&darr;';
			document.getElementById('footer').style.height = i_opened+'px';
			document.getElementById('log').style.display= 'block';
		}
	}
}

function u_SidePanelHideOnClick() { u_SidePanelClose()}
function u_SidePanelClose()
{
	u_el['sidepanel'].classList.remove('opened');
	localStorage.sidepanel_opened = false;
	u_el.content.style.right = '0';
	u_el.sidepanel.style.width = localStorage.sidepanel_closed_width + 'px';
}
function u_SidePanelOpen()
{
	u_el['sidepanel'].classList.add('opened');
	localStorage.sidepanel_opened = true;
	u_el.content.style.right = localStorage.sidepanel_opened_width + 'px';
	u_el.sidepanel.style.width = localStorage.sidepanel_opened_width + 'px';
}

function u_RulesOnClick()
{
	if( u_el.rules.m_opened )
	{
		u_el.rules.m_opened = false;
		u_el.rules.style.display = 'none';
	}
	else
	{
		u_el.rules.m_opened = true;
		u_el.rules.style.display = 'block';
	}
}

function u_DrawColorBars( i_el, i_onclick, i_height)
{
	if( i_height == null )
		i_height = '20px';
	else
		i_height = Math.round( i_height/3 )+'px';
	i_el.classList.add('colorbars');
	var ccol = 35;
	var crow = 3;
	var cstep = 5;
	var cnum = crow * ccol;
	for( var cr = 0; cr < crow; cr++)
	{
		elRaw = document.createElement('div');
		i_el.appendChild( elRaw);
		for( var cc = 0; cc < ccol; cc++)
		{
			el = document.createElement('div');
			elRaw.appendChild( el);
			el.style.width = 100/ccol + '%';
			el.style.height = i_height;
			el.onclick = i_onclick;

			var r = 0, g = 0, b = 0;
			r = ( ( cc % cstep ) + 1 ) / ( cstep + 1 );

			if     (cc < cstep  ) { r = cc/(cstep-1); g = r; b = r; }
			else if(cc < cstep*2) { r = r; }
			else if(cc < cstep*3) { g = r; r = 0; }
			else if(cc < cstep*4) { b = r; r = 0; }
			else if(cc < cstep*5) { g = r; }
			else if(cc < cstep*6) { b = r; }
			else if(cc < cstep*7) { g = r; b = r; r = 0; }

			if( cr > 0 )
			{
				var avg = (r+g+b)/2.5;
				var sat = 2, add = .1;
				if( cr > 1 ) { sat = 1.2; add = .2};
				r += add+(avg-r)/sat;
				g += add+(avg-g)/sat;
				b += add+(avg-b)/sat;
			}

			r = Math.round( 255*r);
			g = Math.round( 255*g);
			b = Math.round( 255*b);
			if( r > 255 ) r = 255;
			if( g > 255 ) g = 255;
			if( b > 255 ) b = 255;

			if( cr && (cc < cstep))
				el.m_color = null;
			else
			{
				el.style.background = 'rgb('+r+','+g+','+b+')';
				el.m_color = [r,g,b];
			}
//window.console.log('rgb('+r+','+g+','+b+')');
		}
	}
}

function u_ShowFolder( i_element, i_path, i_walk)
{
	i_element.classList.add('show_folder');

	var elPanel = document.createElement('div');
	i_element.appendChild( elPanel);
	elPanel.classList.add('panel');

	var elTitle = document.createElement('div');
	elPanel.appendChild( elTitle);
	elTitle.classList.add('title');
	var title = '';
	if( ASSET && ASSET.name )
		title = ASSET.name;
	elTitle.textContent = title;

	var elPath = document.createElement('div');
	elPanel.appendChild( elPath);
	var path = i_path;
	if( ASSET && ASSET.path )
		path = path.replace( ASSET.path, '');
	elPath.classList.add('path');
	elPath.textContent = path;

	c_CreateOpenButton( elPath, path);

	if( i_walk.folders)
	{
		i_walk.folders.sort( c_CompareFolders );
		for( var i = 0; i < i_walk.folders.length; i++)
			if( false == u_SkipFile( i_walk.folders[i].name))
				u_ShowSequence( i_element, i_path + '/' + i_walk.folders[i].name)
	}

	if( i_walk.files)
	{
		i_walk.files.sort();
		for( var i = 0; i < i_walk.files.length; i++)
			if( false == u_SkipFile( i_walk.files[i]))
				u_ShowFile( i_element, i_path + '/' + i_walk.files[i])
	}
}

function u_ShowSequence( i_element, i_path, i_name)
{
	if( i_name == null)
	{
		i_name = i_path.split('/');
		i_name = i_name[i_name.length-1];
	}

	var elFolder = document.createElement('div');
	elFolder.classList.add('folder');
	i_element.appendChild( elFolder);

	var elLinkA = document.createElement('a');
	elFolder.appendChild( elLinkA);
	elLinkA.setAttribute('href', 'player.html#'+i_path);
	elLinkA.setAttribute('target', '_blank');
	elLinkA.textContent = i_name;

	var cmds = c_CloneObj( RULES.cmdexec.play_sequence);
	cmds.push({"name":"Open","cmd":RULES.cmdexec.open_folder});
	for( var c = 0; c < cmds.length; c++)
	{
		var elCmd = document.createElement('div');
		elFolder.appendChild( elCmd);
		elCmd.classList.add('cmdexec');
		elCmd.textContent = cmds[c].name;
		var cmd = cmds[c].cmd;
		cmd = cmd.replace('@PATH@', cgru_PM('/'+RULES.root + i_path));
		cmd = cmd.replace('@FPS@', RULES.fps);
		elCmd.setAttribute('cmdexec', JSON.stringify([cmd]));
	}

	if( ASSET && ( ASSET.dailies ))
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

function u_ShowFile( i_element, i_path)
{
	var name = i_path.substr( i_path.lastIndexOf('/')+1);
	var type = name.substr( name.lastIndexOf('.')+1);

	var elFile = document.createElement('div');
	i_element.appendChild( elFile);
	elFile.classList.add('folder');

	var elLinkA = document.createElement('a');
	elFile.appendChild( elLinkA);
	elLinkA.href = RULES.root + i_path;
	elLinkA.target = '_blank';
	elLinkA.textContent = name;

	if( RULES.movtypes.indexOf(type) != -1)
	{
		var cmds = RULES.cmdexec.play_movie;
		for( var c = 0; c < cmds.length; c++)
		{
			var elCmd = document.createElement('div');
			elFile.appendChild( elCmd);
			elCmd.classList.add('cmdexec');
			elCmd.textContent = cmds[c].name;
			var cmd = cmds[c].cmd;
			cmd = cmd.replace('@PATH@', cgru_PM('/'+RULES.root + i_path));
			cmd = cmd.replace('@FPS@', RULES.fps);
			elCmd.setAttribute('cmdexec', JSON.stringify([cmd]));
		}

		var elExplode = document.createElement('div');
		elFile.appendChild( elExplode);
		elExplode.classList.add('button');
		elExplode.textContent = 'Explode';
		elExplode.m_path = i_path;
		elExplode.onclick = function(e){ d_Explode( e.currentTarget.m_path)};
	}
}

function u_SkipFile( i_filename)
{
	if( i_filename.indexOf('/') != -1 )
		i_filename = i_filename.substr( i_filename.lastIndexOf('/')+1);
	for( var i = 0; i < RULES.skipfiles.length; i++ )
		if( i_filename.indexOf( RULES.skipfiles[i]) == 0 )
			return true;
	return false;
}

function u_SearchOnClick()
{
	$('search_result').textContent = '';
	$('search_result_div').style.display = 'none';

	if( $('search_btn').m_opened )
	{
		$('search_btn').m_opened = false;
		$('search_btn').textContent = 'Search';
		$('search').style.display = 'none';
		g_ClearLocationArgs();
		a_ShowAllThumbnails();
		$('search').m_path = null;
	}
	else
	{
		$('search_btn').m_opened = true;
		$('search_btn').textContent = 'Close Search';
		$('search').style.display = 'block';

		if( $('search_tags').m_elTags )
			for( var i = 0; i < $('search_tags').m_elTags.length; i++ )
				$('search_tags').removeChild( $('search_tags').m_elTags[i]);
		$('search_tags').m_elTags = [];
		for( var tag in RULES.tags )
		{
			el = document.createElement('div');
			$('search_tags').appendChild( el);
			el.style.cssFloat = 'left';
			st_SetElTag( el, tag);
			el.m_tag = tag;
			el.classList.add('tag');
			el.onclick = function(e){ c_ElToggleSelected(e); if( a_elThumbnails ) u_SearchSearch();};
			$('search_tags').m_elTags.push( el);
		}

		if( ASSET && ( ASSET.thumbnails != null))
			$('search_comment_div').style.display = 'none';
		else
			$('search_comment_div').style.display = 'block';
	}
}

function u_SearchSearch()
{
//console.log('g_CurPath(): ' + g_CurPath());
//console.log('search path: ' + $('search').m_path);
	if( $('search').m_path && ( $('search').m_path != g_CurPath() ))
	{
console.log('g_GO: ' + $('search').m_path);
		g_GO($('search').m_path);
		g_PathChanged();
	}
	$('search').m_path = g_CurPath();
//console.log(g_CurPath());

	var args = {};
	if( c_Strip($('search_annotation').textContent).length )
		args.ann = c_Strip($('search_annotation').textContent);

	for( var i = 0; i < $('search_artists').m_elArtists.length; i++)
		if( $('search_artists').m_elArtists[i].m_selected )
		{
			if( args.artists == null ) args.artists = [];
			args.artists.push( $('search_artists').m_elArtists[i].m_user);
		}

	for( var i = 0; i < $('search_tags').m_elTags.length; i++)
		if( $('search_tags').m_elTags[i].m_selected )
		{
			if( args.tags == null ) args.tags = [];
			args.tags.push( $('search_tags').m_elTags[i].m_tag);
		}

	var permin = c_GetElInteger( $('search_percentmin'));
	var permax = c_GetElInteger( $('search_percentmax'));
	if(( permin != null ) || ( permax != null ))
		args.percent = [permin,permax];

	var finmin = c_GetElInteger( $('search_finishmin'));
	var finmax = c_GetElInteger( $('search_finishmax'));
	if(( finmin != null ) || ( finmax != null ))
		args.finish = [finmin,finmax];

	if( c_Strip($('search_comment').textContent).length )
		args.comment = c_Strip($('search_comment').textContent);

	g_SetLocationArgs({"u_Search":args});
}

function u_Search( i_args)
{
	$('search_result').textContent = '';
	$('search_result_div').style.display = 'none';

	if( $('search_btn').m_opened !== true ) u_SearchOnClick();

	if( i_args == null ) i_args = {};

	var anns = null;
	if( i_args.ann )
		$('search_annotation').textContent = i_args.ann;
	if( i_args.artists )
		for( i = 0; i < $('search_artists').m_elArtists.length; i++ )
			c_ElSetSelected( $('search_artists').m_elArtists[i], i_args.artists.indexOf( $('search_artists').m_elArtists[i].m_user ) != -1 )
	if( i_args.tags ) 
		for( i = 0; i < $('search_tags').m_elTags.length; i++ )
			c_ElSetSelected( $('search_tags').m_elTags[i], i_args.tags.indexOf( $('search_tags').m_elTags[i].m_tag ) != -1 )
	if( i_args.percent )
	{
		if(( i_args.percent[0] != null ) && ( i_args.percent[1] != null ) && ( i_args.percent[0] > i_args.percent[1] ))
		{
			i_args.percent[0]+= i_args.percent[1];
			i_args.percent[1] = i_args.percent[0] - i_args.percent[1];
			i_args.percent[0] = i_args.percent[0] - i_args.percent[1];
		}
		$('search_percentmin').textContent = i_args.percent[0];
		$('search_percentmax').textContent = i_args.percent[1];
	}
	if( i_args.finish )
	{
		if(( i_args.finish[0] != null ) && ( i_args.finish[1] != null ) && ( i_args.finish[0] > i_args.finish[1] ))
		{
			i_args.finish[0]+= i_args.finish[1];
			i_args.finish[1] = i_args.finish[0] - i_args.finish[1];
			i_args.finish[0] = i_args.finish[0] - i_args.finish[1];
		}
		$('search_finishmin').textContent = i_args.finish[0];
		$('search_finishmax').textContent = i_args.finish[1];
	}
	if( i_args.comment )
		$('search_comment').textContent = i_args.comment;

	if( a_elThumbnails )
	{
		a_ThumbFilter( i_args);
		return;
	}

	$('search_path').textContent = g_CurPath();
	$('search_path').href = '#'+g_CurPath();

	var args = {};
	for( arg in i_args )
	{
		if( arg == 'comment' )
		{
			args.comment = i_args['comment'];
			continue;
		}
		if( args.status == null ) args.status = {};
		args.status[arg] = i_args[arg];
	}
	args.path = RULES.root + g_CurPath();
	args.rufolder = RULES.rufolder;
	args.depth = 4;

	var res = c_Parse( n_Request({"search":args}));

	if( res.error )
	{
		c_Error( res.error);
		return;
	}

	if( res.result == null )
	{
		c_Error('Search returned null result.');
		return;
	}

	$('search_result_div').style.display = 'block';
	res.result.sort();
	for( var i = 0; i < res.result.length; i++)
	{
		var path = res.result[i];
		path = path.replace( RULES.root, '');
		path = path.replace( /\/\//g, '/');
		var el = document.createElement('div');
		$('search_result').appendChild( el);
		var elLink = document.createElement('a');
		el.appendChild( elLink);
		elLink.href = '#'+path;
//		elLink.target = '_blank';
		elLink.textContent = path;
	}
}

