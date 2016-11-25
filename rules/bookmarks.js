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


	// Bookmarks are not available for guests:
	if( g_auth_user == null ) return;

	$('sidepanel_bookmarks').style.display = 'block';
	bm_initialized = true;

	if( localStorage.bookmarks_opened == 'true')
		bm_Open( false);
	else
		bm_Close();
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
	$('sidepanel_bookmarks').classList.add('opened');
	localStorage.bookmarks_opened = 'true';

	if( i_load !== false )
		bm_Load({"info":'open'});
	else
		bm_Show();
}

function bm_Load( i_args)
{
	if( g_auth_user == null ) return;

	if( i_args == null )
		i_args = {};

	if( i_args.func == null )
		i_args.func = bm_Show;

	if( i_args.info == null )
		i_args.info = 'load';

	//$('bookmarks').innerHTML = 'Loading...';
	var filename = 'users/'+g_auth_user.id+'.json';
	n_Request({"send":{"getfile":filename},"func":bm_Received,"args":i_args,"info":"bookmarks " + i_args.info});
}

function bm_Received( i_user, i_args)
{
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

	i_args.args.func( i_args.args)
}

function bm_Show()
{
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
//	bm_Filter();
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

	if( bm_StatusOld( i_bm.status ))
	{
		var elDel = document.createElement('div');
		el.appendChild( elDel);
		elDel.classList.add('button');
		elDel.classList.add('delete');
		elDel.m_path = i_bm.path;
		elDel.ondblclick = function(e){ bm_Delete([e.currentTarget.m_path]);};
		elDel.title = 'Double click to delete.';
	}

	var elPath = document.createElement('a');
	el.appendChild( elPath);
	elPath.textContent = name;
	elPath.href = '#' + i_bm.path;

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

	bm_Process();
	bm_HighlightCurrent();
}

function bm_StatusOld( i_status)
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

function bm_Process()
{
	if( g_auth_user == null )
		return;

	var bm = null;
	var path = g_CurPath();

	if( g_auth_user.bookmarks )
		for( var b = 0; b < g_auth_user.bookmarks.length; b++)
			if( g_auth_user.bookmarks[b] && ( path == g_auth_user.bookmarks[b].path ))
			{
				bm = g_auth_user.bookmarks[b];
				break;
			}


	// No bookmark was founded with the current path:
	if( bm == null )
	{
		if( bm_StatusOld( RULES.status))
			return;

		// Create a new bookmark:
		bm = {};
		bm.path = path;
		bm.ctime = c_DT_CurSeconds();
	}

	bm.mtime = c_DT_CurSeconds();
	bm.status = RULES.status;

	var obj = {};
	obj.pusharray = 'bookmarks';
	obj.objects = [bm];
	obj.keyname = 'path';
	obj.file = 'users/' + g_auth_user.id + '.json';

	n_Request({"send":{"editobj":obj},"func":bm_Load,"info":"bookmarks add"});
}

function bm_HighlightCurrent()
{
	var path = g_CurPath();

	for( var i = 0; i < bm_elements.length; i++)
	{
		if( path == bm_elements[i].m_bookmark.path )
		{
			bm_elements[i].classList.add('cur_path');
			//bm_elements[i].scrollIntoView();
		}
		else
		{
			bm_elements[i].classList.remove('cur_path');
		}
	}
}

function bm_FilterBtn( i_btn, i_project)
{
	i_btn.m_filter = i_filter;
	i_btn.m_project = i_project;

	// Get all news filter buttons and remove push:
	var btns = document.getElementsByClassName('bm_prj');
	for( var i = 0; i < btns.length; i++)
		btns[i].classList.remove('pushed');

	// Add push on clicked button except 'All':
	if( i_filter == '_all_')
		bm_filter_project = null;
	else
		i_btn.classList.add('pushed');

	bm_Filter();
}

function bm_Filter()
{
	var filter = null;
	var project = null;

	// Get all news filter buttons and remove push:
	var btns = document.getElementsByClassName('bm_prj');
	for( var i = 0; i < btns.length; i++)
		if( btns[i].classList.contains('pushed'))
		{
			filter = btns[i].m_filter;
			project = btns[i].m_project;
		}

	for( var i = 0; i < bm_elements.length; i++)
	{
		if( project && ( bm_elements[i].m_news.path.indexOf( filter ) == 1 ))
		{
			elMBs[i].style.display = 'block';
		}
		else
		{
			bm_elements[i].style.display = 'none';
		}
	}

	if( project )
		bm_filter_project = filter;
}

function bm_Delete( i_paths)
{
	// Delete all bookmarks if paths are not specified:
	if( i_paths == null )
	{
		i_paths = [];
		// Make an array with all ids:
		for( var i = 0; i < bm_elements.length; i++)
			i_paths.push( bm_elements[i].m_news.id);
	}

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

function bm_Compare(a,b)
{
	if( a.path > b.path ) return  1;
	if( a.path < b.path ) return -1;
	return 0;
}
