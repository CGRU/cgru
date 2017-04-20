bm_initialized = false;

bm_projects = [];
bm_elements = [];

function bm_Init()
{
	// Init localStorage:
	if( localStorage.bookmarks_opened == null )
		localStorage.bookmarks_opened = 'false';

	if( localStorage.bookmarks_projects_opened == null )
		localStorage.bookmarks_projects_opened = '';

	if( localStorage.bookmarks_thumbnails_show == null )
		localStorage.bookmarks_thumbnails_show = 'false';


	// Bookmarks are not available for guests:
	if( g_auth_user == null )
		return;

	// Bookmarks are available only for artists:
	if( c_IsNotAnArtist())
		return;

	$('sidepanel_bookmarks').style.display = 'block';
	bm_initialized = true;
}

function bm_InitConfigured()
{
	if( false == bm_initialized )
		return;

	if( localStorage.bookmarks_opened == 'true')
		bm_Open( false);
	else
		bm_Show();

	if( RULES.bookmarks.refresh == null ) return;
	if( RULES.bookmarks.refresh < 1 ) return;
	setInterval( bm_Load, RULES.bookmarks.refresh * 1000);
}

function bm_OnClick()
{
	if( $('sidepanel').classList.contains('opened'))
	{
		if( $('sidepanel_bookmarks').classList.contains('opened'))
			bm_Close();
		else
			bm_Open();
	}
	else
	{
		u_SidePanelOpen();
		bm_Open();
	}
}

function bm_Close()
{
	$('sidepanel_bookmarks').classList.remove('opened');
	$('bookmarks').innerHTML = '';
	localStorage.bookmarks_opened = 'false';
}
function bm_Open( i_load)
{
	if( false == bm_initialized )
		return;

	$('sidepanel_bookmarks').classList.add('opened');
	localStorage.bookmarks_opened = 'true';

	if( i_load !== false )
		bm_Load({"info":'open'});
	else
		bm_Show();
}

function bm_Load( i_args)
{
	if( false == bm_initialized )
		return;

	if( i_args == null )
		i_args = {};

	if( i_args.func == null )
		i_args.func = bm_Show;

	if( i_args.info == null )
		i_args.info = 'load';

	var filename = 'users/'+g_auth_user.id+'.json';
	n_Request({'send':{'getobjects':{'file':filename,'objects':['bookmarks']}},'func':bm_Received,'args':i_args,'info':'bookmarks ' + i_args.info});
}

function bm_Received( i_user, i_args)
{
	if( false == bm_initialized )
		return;

//console.log('nw_NewsReceived()');
	if( i_user == null ) return;
	if( i_user.error )
	{
		c_Error( i_user.error);
		return;
	}

	if( i_user.bookmarks == null )
		i_user.bookmarks = [];

	g_auth_user.bookmarks = [];
	for( var i = 0; i < i_user.bookmarks.length; i++)
		if( i_user.bookmarks[i] )
			g_auth_user.bookmarks.push( i_user.bookmarks[i]);

//	i_args.args.func( i_args.args);
	bm_Show();
}

function bm_Compare(a,b)
{
	if( a == null ) return  1;
	if( b == null ) return -1;
	if( a.path > b.path ) return  1;
	if( a.path < b.path ) return -1;
	return 0;
}
function bm_Show()
{
	if( false == bm_initialized )
		return;

//console.log(JSON.stringify(g_auth_user.bookmarks));
	$('bookmarks').innerHTML = '';
	$('bookmarks_label').textContent = 'Bookmarks';
	bm_projects = [];
	bm_elements = [];

	if(( g_auth_user.bookmarks == null ) || ( g_auth_user.bookmarks.length == 0 ))
		return;

	$('bookmarks_label').textContent = 'Bookmarks - ' + g_auth_user.bookmarks.length;

	g_auth_user.bookmarks.sort( bm_Compare);

	// Collect projects:
	var project = null;
	for( var i = 0; i < g_auth_user.bookmarks.length; i++ )
	{
		var bm = g_auth_user.bookmarks[i];

		if( bm == null )
			continue;

		var names = bm.path.split('/');

		if(( project == null ) || ( project.name != names[1] ))
		{
			project = {};
			project.name = names[1];
			project.bms = [];
			project.elBMs = [];

			bm_projects.push( project);
		}

		project.bms.push( bm);
	}

	// Construct elements:
	opened = localStorage.bookmarks_projects_opened.split('|');
	for( var p = 0; p < bm_projects.length; p++)
	{
		var project = bm_projects[p];

		// Project element:
		project.el = document.createElement('div');
		$('bookmarks').appendChild( project.el);
		project.el.classList.add('project');
		if( opened.indexOf( project.name) != -1 )
			project.el.classList.add('opened');
		else
			project.el.classList.add('closed');

		// Project label:
		var el = document.createElement('div');
		project.elLabel = el;
		project.el.appendChild( el);
		el.classList.add('label');
		el.onclick = bm_ProjectClicked;
		el.m_project = project;
		el.textContent = project.name + ' - ' + project.bms.length;

		// Project bookmarks:
		for( var b = 0; b < project.bms.length; b++)
		{
			var el = bm_CreateElements( project.bms[b]);
			bm_elements.push( el);
			project.elBMs.push( el);
			project.el.appendChild( el);
		}
	}

	bm_HighlightCurrent();
	bm_ThumbnailsShowHide();
}

function bm_CreateElements( i_bm)
{
	var name = i_bm.path.split('/');
	var cuts = 4;
	if( cuts > name.length )
		cuts = 0;
	if( cuts )
		name = name.splice( cuts);
	name = name.join('/');

	var el = document.createElement('div');
	el.classList.add('bookmark');

	if( i_bm.status && i_bm.status.progress && ( i_bm.status.progress > 0 ))
	{
		var elBar = document.createElement('div');
		el.appendChild( elBar);
		elBar.classList.add('bar');
		st_SetElProgress( i_bm.status, elBar);
	}

	var elDel = document.createElement('div');
	el.appendChild( elDel);
	elDel.classList.add('button');
	elDel.classList.add('delete');
	elDel.m_path = i_bm.path;
	elDel.ondblclick = function(e){ bm_Delete([e.currentTarget.m_path]);};
	elDel.title = 'Double click to delete.';

	var elPath = document.createElement('a');
	el.appendChild( elPath);
	elPath.textContent = name;
	elPath.href = '#' + i_bm.path;

	var tooltip = '';
	if( i_bm.cuser ) tooltip += 'Created by: '  + c_GetUserTitle(  i_bm.cuser) + '\n';
	if( i_bm.ctime ) tooltip += 'Created at: '  + c_DT_StrFromSec( i_bm.ctime) + '\n';
	if( i_bm.muser ) tooltip += 'Modified by: ' + c_GetUserTitle(  i_bm.muser) + '\n';
	if( i_bm.mtime ) tooltip += 'Modified at: ' + c_DT_StrFromSec( i_bm.mtime) + '\n';
	el.title = tooltip;

	if( bm_ObsoleteStatus( i_bm.status ))
		el.classList.add('obsolete');

	el.m_bookmark = i_bm;

	return el;
}

function bm_ProjectClicked( i_evt)
{
	var el = i_evt.currentTarget.m_project.el;
	el.classList.toggle('opened');
	el.classList.toggle('closed');

	var list = ''
	for( var p = 0; p < bm_projects.length; p++)
	{
		if( bm_projects[p].el.classList.contains('closed'))
			continue;

		if( list.length )
			list += '|';

		list += bm_projects[p].name;
	}

	localStorage.bookmarks_projects_opened = list;
}

function bm_NavigatePost()
{
	if( $('sidepanel').classList.contains('opened') != true )
		return;

	bm_HighlightCurrent();
}

function bm_ObsoleteStatus( i_status)
{
	if( i_status == null )
		return true;

	if( i_status.artists == null )
		return true;

	if( i_status.artists.indexOf( g_auth_user.id ) == -1 )
		return true;

	if( i_status.progress )
	{
		if( i_status.progress >= 100 )
			return true;
	}

	if( i_status.flags )
	{
		if( i_status.flags.indexOf('omit') != -1 )
			return true;
	}

	return false;
}

function bm_StatusesChanged( i_args)
{
	bm_Load({'info':'statuses'});
}

function bm_HighlightCurrent()
{
	var path = g_CurPath();

	for( var i = 0; i < bm_elements.length; i++)
	{
		if( path == bm_elements[i].m_bookmark.path )
		{
			bm_elements[i].classList.add('cur_path');
			if( g_CurPathDummy() || bm_ObsoleteStatus( RULES.status))
			{
				bm_elements[i].classList.add('obsolete');
			}
			//bm_elements[i].scrollIntoView();
		}
		else
		{
			bm_elements[i].classList.remove('cur_path');
		}
	}
}

function bm_Delete( i_paths)
{
	if( i_paths.length == 0 )
	{
		c_Error('No bookmarks to delete.');
		bm_Load({"info":'not deleted'});
		return;
	}

	var obj = {};
	obj.objects = [];
	for( var i = 0; i < i_paths.length; i++ )
		obj.objects.push({"path":i_paths[i]});
	obj.delarray = 'bookmarks';

	obj.file = 'users/'+g_auth_user.id+'.json';
	n_Request({"send":{"editobj":obj},"func":bm_DeleteFinished});
}
function bm_DeleteFinished( i_data)
{
	if(( i_data == null ) || ( i_data.error ))
	{
		c_Error( i_data.error );
		return;
	}

	c_Info('Bookmarks deleted');
	bm_Load({"info":'deleted'});
}

function bm_DeleteObsoleteOnClick()
{
	var paths = [];
	for( var i = 0; i < bm_elements.length; i++)
		if( bm_elements[i].classList.contains('obsolete'))
			paths.push( bm_elements[i].m_bookmark.path);

	if( paths.length == 0 )
	{
		c_Info('No obsolete bookmarks founded.');
		return;
	}

	bm_Delete( paths);
}

function bm_ThumbnailsOnClick()
{
	if( localStorage.bookmarks_thumbnails_show == 'true' )
		localStorage.bookmarks_thumbnails_show = 'false';
	else
		localStorage.bookmarks_thumbnails_show = 'true';
	bm_ThumbnailsShowHide();
}
function bm_ThumbnailsShowHide()
{
	if( localStorage.bookmarks_thumbnails_show == 'true' )
		bm_ThumbnailsShow();
	else
		bm_ThumbnailsHide();
}
function bm_ThumbnailsShow()
{
	c_ElSetSelected( $('bookmakrs_thumbs_btn'), true);

	for( var i = 0; i < bm_elements.length; i++)
	{
		var el = bm_elements[i];

		if( el.m_elTh == null )
		{
			el.m_elTh = document.createElement('a');
			el.appendChild( el.m_elTh);
			el.m_elTh.href = '#' + el.m_bookmark.path;

			var img = document.createElement('img');
			el.m_elTh.appendChild( img);
			img.src = RULES.root + el.m_bookmark.path + '/' + RULES.rufolder + '/thumbnail.jpg';
			img.style.display = 'none';
			img.onload = function( i_el) { i_el.currentTarget.style.display = 'block';}
		}

		el.m_elTh.style.display = 'block';
	}
}
function bm_ThumbnailsHide()
{
	c_ElSetSelected( $('bookmakrs_thumbs_btn'), false);

	for( var i = 0; i < bm_elements.length; i++)
	{
		var el = bm_elements[i];
		if( el.m_elTh )
			el.m_elTh.style.display = 'none';
	}
}

