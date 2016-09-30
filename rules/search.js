
function s_SearchOnClick()
{
	// First time search operations:
	if( $('search').m_constcted != true )
	{
		$('search').m_constcted = true;
		$('search_artists').m_elArtists = [];

		// Constuct atrists just once, as they are not depending on location:
		var roles = c_GetRolesArtists( g_users);

		for( var r = 0; r < roles.length; r++)
		{
			var elRole = document.createElement('div');
			$('search_artists').appendChild( elRole);
			elRole.classList.add('role');

			var elLabel = document.createElement('div');
			elRole.appendChild( elLabel);
			elLabel.classList.add('label');
			elLabel.textContent = roles[r].role + ':';
			elLabel.title = 'Click to (un)select all role artists.';
			elLabel.m_elArtists = [];
			elLabel.onclick = function(e)
			{
				var el = e.currentTarget;
				for( var a = 0; a < el.m_elArtists.length; a++) c_ElToggleSelected(el.m_elArtists[a]);
				if( ASSET && ASSET.filter ) s_ProcessGUI();
			}

			var role_has_one_artist = false;
			var role_has_enabled = false;
			for( var a = 0; a < roles[r].artists.length; a++)
			{
				var artist = roles[r].artists[a];

				el = document.createElement('div');
				elRole.appendChild( el);
				$('search_artists').m_elArtists.push( el);
				el.style.cssFloat = 'left';
				el.textContent = c_GetUserTitle( artist.id);
				el.m_user = artist.id;
				el.classList.add('tag');
				el.classList.add('artist');

				if( artist.disabled )
					el.classList.add('disabled');
				else
					role_has_enabled = true;

				if( c_IsNotAnArtist( artist))
					el.classList.add('notartist');
				else
					role_has_one_artist = true;

				var avatar = c_GetAvatar( artist.id);
				if( avatar)
				{
					el.classList.add('with_icon');
					el.style.backgroundImage = 'url(' + avatar + ')';
				}

				el.onclick = function(e){ c_ElToggleSelected(e); if( ASSET && ASSET.filter ) s_ProcessGUI();};

				elLabel.m_elArtists.push(el);
			}

			if( role_has_one_artist != true ) elRole.classList.add('notartist');
			if( role_has_enabled != true ) elRole.classList.add('disabled');
		}

		var el = $('search_artists_notassigned');
		$('search_artists').m_elArtists.push( el);
		el.m_user = '_null_';
		el.classList.add('tag');
		el.classList.add('artist');
		el.onclick = function(e){ c_ElToggleSelected(e); if( ASSET && ASSET.filter ) s_ProcessGUI();};
	}

	$('search_result').textContent = '';
	$('search_result_div').style.display = 'none';
	$('search_result_none').style.display = 'none';

	if( $('search_btn').m_opened )
	{
		$('search_btn').m_opened = false;
		$('search_btn').textContent = 'Search';
		$('search').style.display = 'none';
		$('search_btn_process').style.display = 'none';
		g_ClearLocationArgs();
		if( ASSET && window[ASSET.filter] ) window[ASSET.filter]();
		$('search').m_path = null;
	}
	else
	{
		$('search_btn').m_opened = true;
		$('search_btn').textContent = 'Close';
		$('search').style.display = 'block';
		$('search_btn_process').style.display = 'block';

		// Flags:
		if( $('search_flags').m_elFlags )
			for( var i = 0; i < $('search_flags').m_elFlags.length; i++ )
				$('search_flags').removeChild( $('search_flags').m_elFlags[i]);
		$('search_flags').m_elFlags = [];
		for( var flag in RULES.flags )
		{
			el = document.createElement('div');
			$('search_flags').appendChild( el);
			el.style.cssFloat = 'left';
			el.textContent = c_GetFlagTitle( flag);
			el.m_flag = flag;
			el.classList.add('flag');
			el.onclick = function(e){ c_ElToggleSelected(e); if( ASSET && ASSET.filter ) s_ProcessGUI();};
			$('search_flags').m_elFlags.push( el);
		}
		var el = $('search_noflags');
		$('search_flags').m_elFlags.push( el);
		el.m_flag = '_null_';
		el.onclick = function(e){ c_ElToggleSelected(e); if( ASSET && ASSET.filter ) s_ProcessGUI();};

		// Tags:
		if( $('search_tags').m_elTags )
			for( var i = 0; i < $('search_tags').m_elTags.length; i++ )
				$('search_tags').removeChild( $('search_tags').m_elTags[i]);
		$('search_tags').m_elTags = [];
		for( var tag in RULES.tags )
		{
			el = document.createElement('div');
			$('search_tags').appendChild( el);
			el.style.cssFloat = 'left';
			el.textContent = c_GetTagTitle( tag);
			el.m_tag = tag;
			el.classList.add('tag');
			el.onclick = function(e){ c_ElToggleSelected(e); if( ASSET && ASSET.filter ) s_ProcessGUI();};
			$('search_tags').m_elTags.push( el);
		}
		var el = $('search_notags');
		$('search_tags').m_elTags.push( el);
		el.m_tag = '_null_';
		el.onclick = function(e){ c_ElToggleSelected(e); if( ASSET && ASSET.filter ) s_ProcessGUI();};

		if( ASSET && ( ASSET.thumbnails != null))
			$('search_comment_div').style.display = 'none';
		else
			$('search_comment_div').style.display = 'block';
	}
}

function s_ShowDisabledArtists( i_el)
{
	i_el.classList.toggle('pushed');
	$('search_artists_div').classList.toggle('show_disabled');
}
function s_ShowNotArtists( i_el)
{
	i_el.classList.toggle('pushed');
	$('search_artists_div').classList.toggle('show_notartist');
}

function s_ProcessGUI()
{
	if( $('search').m_path && ( $('search').m_path != g_CurPath() ))
	{
		g_GO($('search').m_path);
		g_PathChanged();
	}
	$('search').m_path = g_CurPath();

	var args = {};
	if( c_Strip($('search_annotation').textContent).length )
		args.ann = c_Strip($('search_annotation').textContent);

	// Artists:
	if( $('search_artists_notassigned').m_selected )
	{
		if( args.artists == null ) args.artists = [];

		for( var i = 0; i < $('search_artists').m_elArtists.length; i++)
			c_ElSetSelected($('search_artists').m_elArtists[i], false);
		c_ElSetSelected($('search_artists_notassigned'), true);

		args.artists.push($('search_artists_notassigned').m_user);
	}
	else for( var i = 0; i < $('search_artists').m_elArtists.length; i++)
		if( $('search_artists').m_elArtists[i].m_selected )
		{
			if( args.artists == null ) args.artists = [];
			args.artists.push( $('search_artists').m_elArtists[i].m_user);
		}

	// Flags:
	if( $('search_noflags').m_selected )
	{
		if( args.flags == null ) args.flags = [];
		for( var i = 0; i < $('search_flags').m_elFlags.length; i++)
			c_ElSetSelected($('search_flags').m_elFlags[i], false);
		c_ElSetSelected($('search_noflags'), true);

		args.flags.push($('search_noflags').m_flag);
	}
	else for( var i = 0; i < $('search_flags').m_elFlags.length; i++)
		if( $('search_flags').m_elFlags[i].m_selected )
		{
			if( args.flags == null ) args.flags = [];
			args.flags.push( $('search_flags').m_elFlags[i].m_flag);
		}

	// Tags:
	if( $('search_notags').m_selected )
	{
		if( args.tags == null ) args.tags = [];
		for( var i = 0; i < $('search_tags').m_elTags.length; i++)
			c_ElSetSelected($('search_tags').m_elTags[i], false);
		c_ElSetSelected($('search_notags'), true);

		args.tags.push($('search_notags').m_tag);
	}
	else for( var i = 0; i < $('search_tags').m_elTags.length; i++)
		if( $('search_tags').m_elTags[i].m_selected )
		{
			if( args.tags == null ) args.tags = [];
			args.tags.push( $('search_tags').m_elTags[i].m_tag);
		}

	var parm = ['percent','finish','statmod','bodymod'];
	for( var i = 0; i < parm.length; i++ )
	{
		var min = c_GetElInteger( $('search_'+parm[i]+'min'));
		var max = c_GetElInteger( $('search_'+parm[i]+'max'));
		if(( min != null ) || ( max != null ))
			args[parm[i]] = [min,max];
	}

	if( c_Strip($('search_body').textContent).length )
		args.body = c_Strip($('search_body').textContent);
	if( c_Strip($('search_comment').textContent).length )
		args.comment = c_Strip($('search_comment').textContent);

	g_SetLocationArgs({"s_Search":args});
}

function s_Search( i_args)
{
	$('search_result').textContent = '';
	$('search_result_div').style.display = 'none';
	$('search_result_none').style.display = 'none';

	if( $('search_btn').m_opened !== true ) s_SearchOnClick();

	if( i_args == null ) i_args = {};

	var anns = null;
	if( i_args.ann )
		$('search_annotation').textContent = i_args.ann;
	if( i_args.artists )
		for( i = 0; i < $('search_artists').m_elArtists.length; i++ )
			c_ElSetSelected( $('search_artists').m_elArtists[i], i_args.artists.indexOf( $('search_artists').m_elArtists[i].m_user ) != -1 )
	if( i_args.flags ) 
		for( i = 0; i < $('search_flags').m_elFlags.length; i++ )
			c_ElSetSelected( $('search_flags').m_elFlags[i], i_args.flags.indexOf( $('search_flags').m_elFlags[i].m_flag ) != -1 )
	if( i_args.tags ) 
		for( i = 0; i < $('search_tags').m_elTags.length; i++ )
			c_ElSetSelected( $('search_tags').m_elTags[i], i_args.tags.indexOf( $('search_tags').m_elTags[i].m_tag ) != -1 )

	var parm = ['percent','finish','statmod','bodymod'];
	for( var i = 0; i < parm.length; i++ )
		if( i_args[parm[i]] )
		{
			if(( i_args[parm[i]][0] != null ) && ( i_args[parm[i]][1] != null ) && ( i_args[parm[i]][0] > i_args[parm[i]][1] ))
			{
				i_args[parm[i]][0]+= i_args[parm[i]][1];
				i_args[parm[i]][1] = i_args[parm[i]][0] - i_args[parm[i]][1];
				i_args[parm[i]][0] = i_args[parm[i]][0] - i_args[parm[i]][1];
			}
			$('search_'+parm[i]+'min').textContent = i_args[parm[i]][0];
			$('search_'+parm[i]+'max').textContent = i_args[parm[i]][1];
		}

	if( i_args.body )
		$('search_body').textContent = i_args.body;
	if( i_args.comment )
		$('search_comment').textContent = i_args.comment;

	if( ASSET && ASSET.filter )
	{
		if( window[ASSET.filter])
			window[ASSET.filter]( i_args);
		return;
	}

	$('search_path').textContent = g_CurPath();
	$('search_path').href = '#'+g_CurPath();

	var args = {};
	for( arg in i_args )
	{
		if( arg == 'comment' ) { args.comment = i_args['comment']; continue; }
		if( arg == 'body' ) { args.body = i_args['body']; continue; }
		if( args.status == null ) args.status = {};
		args.status[arg] = i_args[arg];
	}
	args.path = RULES.root + g_CurPath();
	args.rufolder = RULES.rufolder;
	args.depth = 4;

	n_Request({"send":{"search":args},"func":s_ResultReceived});
	$('search').classList.add('waiting');
}

function s_ResultReceived( i_data)
{
	$('search').classList.remove('waiting');

	var res = i_data;

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

	$('search_results_count').textContent = res.result.length;

	$('search_result_div').style.display = 'block';
	if( res.result.length == 0 )
	{
		$('search_result').style.display = 'none';
		$('search_result_none').style.display = 'block';
		return;
	}
	$('search_result').style.display = 'block';

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
		elLink.textContent = path;
	}
}

