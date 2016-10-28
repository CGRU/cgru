bm_initialized = false;

function bm_Init()
{
	// Recent:
	if( localStorage.bookmarks_opened == null ) localStorage.bookmarks_opened = 'false';

	// Bookmarks are not available for guests:
	if( g_auth_user == null ) return;

	$('sidepanel_bookmarks').style.display = 'block';
	bm_initialized = true;

	if( localStorage.bookmarks_opened == 'true')
		bm_Open();
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
	localStorage.bookmarks_opened = false;
}
function bm_Open()
{
	$('sidepanel_bookmarks').classList.add('opened');
	localStorage.bookmarks_opened = true;
}

function bm_Load()
{
//console.log('nw_NewsLoad()');
	if( g_auth_user == null ) return;

	$('bookmarks').innerHTML = 'Loading...';
	var filename = 'users/'+g_auth_user.id+'.json';
	n_Request({"send":{"getfile":filename},"func":bm_Received,"info":"bookmarks"});
}

function bm_Received( i_user)
{
//console.log('nw_NewsReceived()');
	if( i_user == null ) return;
	if( i_user.error )
	{
		c_Error( i_user.error);
		return;
	}

	g_auth_user.bookmarks = i_user.bookmarks;

	bm_Show();
}

function bm_Show()
{
	$('bookmarks').innerHTML = '';
	$('bookmarks').m_elArray = [];

	var projects = [];

	if( g_auth_user.bookmarks == null )
		return;

	for( var i = 0; i < g_auth_user.bookmarks.length; i++ )
	{
		var bm = g_auth_user.news[i];

		var el = document.createElement('div');
		$('bookmarks').appendChild( el);
		$('bookmarks').m_elArray.push( el);
		el.classList.add('bookmark');
	}

	// News projects:
	$('bookmarks_projects').innerHTML = '';
	for( var i = 0; i < projects.length; i++ )
	{
		var el = document.createElement('div');
		$('bookmarks_projects').appendChild( el);
		el.classList.add('button');
		el.classList.add('bm_prj');
		el.textContent = projects[i];
		el.onclick = function(e){ bm_FilterBtn( e.currentTarget, projects[i]);};
	}

	bm_HighlightCurrent();
	bm_Filter();
}

function bm_NavigatePost()
{
	bm_Process();
	bm_HighlightCurrent();
}

function bm_HighlightCurrent()
{
	var path = g_CurPath();
	var elBMs = $('bookmarks').m_elArray;
	if( elBMs == null )
		return;

	for( var i = 0; i < elBMs.length; i++)
		if( path == elBMs[i].m_bookmark.path )
			elBMs[i].classList.add('cur_path');
		else
			elBMs[i].classList.remove('cur_path');
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

	var elBMs = $('news').m_elArray;
	for( var i = 0; i < elBMs.length; i++)
	{
		if( project && ( elBMs[i].m_news.path.indexOf( filter ) == 1 ))
		{
			elMBs[i].style.display = 'block';
		}
		else
		{
			elBMs[i].style.display = 'none';
		}
	}

	if( project )
		bm_filter_project = filter;
}

function bm_Delete( i_ids)
{
	// Delete all new if ids is not specified:
	if( i_ids == null )
	{
		i_ids = [];
		var elNews = $('news').m_elArray;
		// Make an array with all ids:
		for( var i = 0; i < elNews.length; i++)
			i_ids.push( elNews[i].m_news.id);
	}

	if( i_ids.length == 0 )
	{
		c_Error('No news to delete.');
		bm_Load();
		return;
	}

	var obj = {};
	obj.objects = [];
	for( var i = 0; i < i_ids.length; i++ )
		obj.objects.push({"id":i_ids[i]});
	obj.delobj = true;

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
	bm_Load();
}

