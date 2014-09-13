nw_recent_file = 'recent.json';

nw_initialized = false;

nw_recents = {};

function nw_Init()
{
	// Recent:
	if( localStorage.recent_opened == null ) localStorage.recent_opened = 'true';
	if( localStorage.recent_opened == 'true') nw_RecentOpen( false);

	// News are not available for guests:
	if( g_auth_user == null ) return;

	// News:
	nw_ShowCount();

	$('subscribe').style.display = 'block';
	$('sidepanel_news').style.display = 'block';
	nw_initialized = true;

	if( localStorage.news_disabled == null ) localStorage.news_disabled = 'false';
	if( localStorage.news_ignore_own == null ) localStorage.news_ignore_own = 'false';

	if( localStorage.news_opened == "true" ) nw_NewsOpen();
	else nw_NewsClose();

	nw_Finish();
	nw_UpdateChannels();
	nw_DisableNewsToggle( false);
	nw_IgnoreOwnToggle( false);
}

function nw_InitConfigured()
{
	if( RULES.news.refresh == null ) return;
	if( RULES.news.refresh < 1 ) return;
	setInterval( nw_NewsLoad, RULES.news.refresh * 1000);
}

function nw_DisableNewsToggle( i_toggle)
{
	if( i_toggle === true )
	{
		if( localStorage.news_disabled == 'true')
			localStorage.news_disabled = 'false';
		else
			localStorage.news_disabled = 'true';
	}

	if( localStorage.news_disabled == 'true')
	{
		$('news_disable').textContent = 'News Disabled';
		$('news_disable').classList.add('disabled');
		$('news_make').style.display = 'none';
	}
	else
	{
		$('news_disable').textContent = 'Disable News';
		$('news_disable').classList.remove('disabled');
		$('news_make').style.display = 'block';
	}
}

function nw_IgnoreOwnToggle( i_toggle)
{
	if( i_toggle === true )
	{
		if( localStorage.news_ignore_own == 'true')
			localStorage.news_ignore_own = 'false';
		else
			localStorage.news_ignore_own = 'true';
	}

	if( localStorage.news_ignore_own == 'true')
	{
		$('news_ignore_own').textContent = 'Ignoring Own';
		$('news_ignore_own').classList.add('ignore');
	}
	else
	{
		$('news_ignore_own').textContent = 'Listening Own';
		$('news_ignore_own').classList.remove('ignore');
	}
}

function nw_SortOrderToggle()
{
	if( localStorage.news_sort_order == 'ASC' )
		localStorage.news_sort_order = 'DESC';
	else
		localStorage.news_sort_order = 'ASC';

	nw_NewsLoad();
}

function nw_UpdateChannels()
{
	$('channels').innerHTML = '';
	for( var i = 0; i < g_auth_user.channels.length; i++ )
	{
		nw_path = g_auth_user.channels[i].id;
		var el = document.createElement('div');
		$('channels').appendChild( el);
		el.classList.add('channel');
		el.title = 'Subscribed by '+g_auth_user.channels[i].user+' at\n'+c_DT_StrFromSec( g_auth_user.channels[i].time);

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.classList.add('delete');
		elBtn.m_path = nw_path;
		elBtn.ondblclick = function(e){ nw_Unsubscribe( e.currentTarget.m_path);};
		elBtn.title = 'Double click to remove channel';

		var elLink = document.createElement('a');
		el.appendChild( elLink);
		elLink.href = '#'+nw_path;
		elLink.textContent = nw_path;
	}
}

function nw_Process()
{
	if( localStorage.recent_opened == 'true' )
		nw_RecentLoad();

	if( false == nw_initialized ) return;

//window.console.log(g_auth_user.channels);
	var subscribed = false;
	var path = g_CurPath();
	for( var i = 0; i < g_auth_user.channels.length; i++ )
	{
		var nw_path = g_auth_user.channels[i].id;
		if( path.indexOf( nw_path) == 0 )
		{
			subscribed = true;
			if( path == nw_path )
			{
				$('subscribe_path').style.display = 'none';
				$('unsubscribe_btn').style.display = 'block';
			}
			else
			{
				$('subscribe_path').style.display = 'block';
				$('unsubscribe_btn').style.display = 'none';
				$('subscribe_path').innerHTML = '<a href="#'+nw_path+'">'+nw_path+'</a>';
			}
			break;
		}
	}

	if( subscribed )
	{
		$('subscribe_label').style.display = 'block';
		$('subscribe_btn').style.display = 'none';
	}
	else
		nw_Finish( false);
}

function nw_Finish( i_finish_recent)
{
	if( i_finish_recent !== false )
		$('recent').innerHTML = '';

	if( false == nw_initialized ) return;

	$('subscribe_label').style.display = 'none';
	$('subscribe_btn').style.display = 'block';
	$('unsubscribe_btn').style.display = 'none';
	$('subscribe_path').style.display = 'none';
}

function nw_Subscribe( i_path)
{
	if( i_path == null )
		i_path = g_CurPath();

	var channel = {};
	channel.id = i_path;
	channel.time = c_DT_CurSeconds();
	channel.user = g_auth_user.id;

	var obj = {};
	obj.objects = [channel];
	obj.pusharray = 'channels';
	obj.id = g_auth_user.id;
	obj.file = 'users/' + g_auth_user.id + '.json';

	n_Request({"send":{"editobj":obj},"func":nw_SubscribeFinished,"channel":channel});
}
function nw_SubscribeFinished( i_data, i_args)
{
	if(( i_data == null ) || ( i_data.error ))
	{
		c_Error( i_data.error );
		return;
	}

	g_auth_user.channels.push( i_args.channel);
	nw_Process();
	nw_UpdateChannels();

	c_Info('Subscribed at ' + i_args.channel.id);
}

function nw_Unsubscribe( i_path)
{
	if( i_path == null )
		i_path = g_CurPath();


	var obj = {};

	obj.objects = [{"id":i_path}];
	obj.delobj = true;
	obj.id = g_auth_user.id;
	obj.file = 'users/' + g_auth_user.id + '.json';

	n_Request({"send":{"editobj":obj},"func":nw_UnsubscribeFinished,"news_path":i_path});
}
function nw_UnsubscribeFinished( i_data, i_args)
{
	if(( i_data == null ) || ( i_data.error ))
	{
		c_Error( i_data.error );
		return;
	}

	var path = i_args.news_path;

	for( i = 0; i < g_auth_user.channels.length; i++)
		if( g_auth_user.channels[i].id == path )
		{
			g_auth_user.channels.splice( i, 1);
			break;
		}

	nw_Process();
	nw_UpdateChannels();

	c_Info('Unsubscribed from ' + path);
}

function nw_NewsOnClick()
{
	if( $('sidepanel').classList.contains('opened'))
	{
		if( $('sidepanel_news').classList.contains('opened'))
			nw_NewsClose();
		else
			nw_NewsOpen();
	}
	else
	{
		u_SidePanelOpen();
		nw_NewsOpen();
	}
}

function nw_NewsClose()
{
	$('sidepanel_news').classList.remove('opened');
	$('news').innerHTML = '';
	localStorage.news_opened = false;
	delete g_auth_user.news;
}
function nw_NewsOpen()
{
	$('sidepanel_news').classList.add('opened');
	localStorage.news_opened = true;
	nw_NewsLoad();
}

function nw_MakeNewsDialog()
{
	if( g_auth_user == null ) return;
	new cgru_Dialog({"handle":'nw_MakeNewsDialogApply',
		"name":'news',"title":'Create News',"info":'Enter News Title'});
}
function nw_MakeNewsDialogApply( i_title) { nw_MakeNews({"title":i_title}); }
//function nw_MakeNews( i_title, i_path, i_user_id, i_guest )
function nw_MakeNews( i_news, i_args )
{
	if( localStorage.news_disabled == 'true') return;

	var news = i_news;

	if( news.user_id == null )
	{
		if( g_auth_user )
			news.user = g_auth_user.id;
		else
			c_Error('Can`t make news with no user.');
	}

	if( news.path == null ) news.path = g_CurPath();

	news.time = c_DT_CurSeconds();
	news.id = news.user + '_' + news.time + '_' + news.path;

	if( localStorage.news_ignore_own == 'true' )
		news.ignore_own = true;

	// If news path is the current we get artists from status, if them not set in input arguments:
	if(( news.artists == null ) && ( news.path == g_CurPath()) && RULES.status && RULES.status.artists )
		news.artists = RULES.status.artists;

	var email_subject = c_GetUserTitle( news.user) + ' - ' + news.title;
	var email_body = '<a href="';
	email_body += document.location.protocol + '//' + document.location.host + document.location.pathname + '#' + news.path;
	email_body += '">' + news.path + '</a>';

	var request = {};
	request.news = news;
	request.email_from_title = c_EmailFromTitle();
	request.email_subject = email_subject;
	request.email_body = email_body;
	request.root = RULES.root;
	request.rufolder = RULES.rufolder;
	request.limit = RULES.news.limit;
	request.recent_max = RULES.news.recent;
	request.recent_file = nw_recent_file;
	n_Request({"send":{"makenews":request},"func":nw_MakeNewsFinished,"args":i_args});
}
function nw_MakeNewsFinished( i_data, i_args)
{
	if( i_data.error )
	{
		c_Error( i_data.error);
		return;
	}

	if( i_args.args && ( i_args.args.load === false )) return;

	nw_NewsLoad();
	nw_RecentLoad(/*file exists check=*/ false);
		
	if( i_data.users.length == 0 )
	{
		c_Log('No subscribed users found.');
		return;
	}

	var info = 'Subscibed users:';
	for( var i = 0; i < i_data.users.length; i++)
		info += ' '+i_data.users[i];
	c_Log( info);
}

function nw_ShowCount()
{
	if( g_auth_user && g_auth_user.news && g_auth_user.news.length )
	{
		$('news_count').textContent = g_auth_user.news.length;
		$('news_count').style.display = 'block';
	}
	else
	{
		$('news_count').style.display = 'none';
		$('news_count').textContent = '';
	}
}

function nw_NewsLoad()
{
	if( g_auth_user == null ) return;

	if( g_auth_user.news != null )
	{
		nw_NewsShow( g_auth_user.news);
		return;
	}

	var filename = 'users/'+g_auth_user.id+'.json';
	n_Request({"send":{"readobj":filename},"func":nw_NewsReceived,"info":"news","wait":false,"parse":true});
	$('news').innerHTML = 'Loading...';
}

function nw_NewsReceived( i_user)
{
	if( i_user == null ) return;
	if( i_user.error )
	{
		c_Error( i_user.error);
		return;
	}
	nw_NewsShow( i_user.news);
}

function nw_NewsShow( i_news)
{
	g_auth_user.news = i_news;

	nw_ShowCount();

	$('news').innerHTML = '';
	$('news').m_elArray = [];

	var projects = [];

	g_auth_user.news.sort( function(a,b){
		var attr = 'time';
		if(a[attr] > b[attr]) return -1;
		if(a[attr] < b[attr]) return 1;
		return 0;
	});

	if( localStorage.news_sort_order == 'ASC' )
		g_auth_user.news.reverse();

	for( var i = 0; i < g_auth_user.news.length; i++ )
	{
		var news = g_auth_user.news[i];

		var el = document.createElement('div');
		$('news').appendChild( el);
		$('news').m_elArray.push( el);
		el.classList.add('news');
		el.m_news = news;

		el.title = c_DT_StrFromSec( news.time);
		if( news.artists && ( news.artists.indexOf( g_auth_user.id) != -1 ))
			el.classList.add('assigned');

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.classList.add('delete');
		elBtn.m_id = news.id;
		elBtn.ondblclick = function(e){ nw_DeleteNews([e.currentTarget.m_id]);};
		elBtn.title = 'Double click to remove link';

		var avatar = c_GetAvatar( news.user, news.guest);
		if( avatar )
		{
			var elAvatar = document.createElement('img');
			el.appendChild( elAvatar);
			elAvatar.classList.add('avatar');
			elAvatar.src = avatar;
			elAvatar.title = c_GetUserTitle( news.user, news.guest) + '\nDouble click to delete all news from this user.';
			elAvatar.m_news = news;
			elAvatar.ondblclick = function(e){ nw_DeleteNewsUser( e.currentTarget.m_news);};
		}

		var elLabel = document.createElement('div');
		el.appendChild( elLabel);
		elLabel.classList.add('news_label');
		elLabel.innerHTML = c_GetUserTitle(news.user, news.guest)+': '+news.title;

		var elLink = document.createElement('a');
		el.appendChild( elLink);
		elLink.href = '#'+news.path;
		elLink.textContent = news.path;

		var prj = news.path.substr(1);
		prj = prj.substr( 0, prj.indexOf('/'));
		if( projects.indexOf(prj) == -1 )
			projects.push( prj);
	}

	delete g_auth_user.news;

	// News projects:
	$('news_projects').innerHTML = '';

	for( var i = 0; i < projects.length; i++ )
	{
		var el = document.createElement('div');
		$('news_projects').appendChild( el);
		el.classList.add('button');
		el.classList.add('nw_fb');
		el.textContent = projects[i];
		el.onclick=function(e){ nw_FilterBtn( e.currentTarget, e.currentTarget.textContent, true);};
	}

	nw_HighlightCurrent();
	nw_Filter();
}

function nw_NavigatePost()
{
	nw_Process();
	nw_HighlightCurrent();
}

function nw_HighlightCurrent()
{
	var path = g_CurPath();
	var elNews = $('news').m_elArray;
	if( elNews == null ) return;

	for( var i = 0; i < elNews.length; i++)
		if( path == elNews[i].m_news.path )
			elNews[i].classList.add('cur_path');
		else
			elNews[i].classList.remove('cur_path');
}

function nw_FilterBtn(i_btn, i_filter, i_project)
{
	i_btn.m_filter = i_filter;
	i_btn.m_project = i_project;

	// Get all news filter buttons and remove push:
	var btns = document.getElementsByClassName('nw_fb');
	for( var i = 0; i < btns.length; i++)
		btns[i].classList.remove('pushed');

	// Add push on clicked button except 'All':
	if( i_filter != '_all_') i_btn.classList.add('pushed');

	nw_Filter();
}

function nw_Filter()
{
	var filter = null;
	var project = null;

	// Get all news filter buttons and remove push:
	var btns = document.getElementsByClassName('nw_fb');
	for( var i = 0; i < btns.length; i++)
		if( btns[i].classList.contains('pushed'))
		{
			filter = btns[i].m_filter;
			project = btns[i].m_project;
		}

	var my  = ( filter == '_my_' );

	var elNews = $('news').m_elArray;
	for( var i = 0; i < elNews.length; i++)
	{
		if( ( filter === null ) ||
			( project && ( elNews[i].m_news.path.indexOf( filter ) == 1 )) ||
			( elNews[i].m_news.title == filter ) ||
			( my && ( elNews[i].classList.contains('assigned'))))
		{
			elNews[i].style.display = 'block';
		}
		else
		{
			elNews[i].style.display = 'none';
		}
	}
}

function nw_DeleteFiltered( i_visible)
{
	var elNews = $('news').m_elArray;
	var display = 'none';
	if( i_visible )
		display = 'block';

	var ids = [];
	for( var i = 0; i < elNews.length; i++)
		if( elNews[i].style.display == display )
			ids.push( elNews[i].m_news.id);

	if( i_visible )
		nw_FilterBtn($('news_filter_show_all'),'_all_');

	nw_DeleteNews( ids);
}

function nw_DeleteNewsUser( i_news)
{
	var elNews = $('news').m_elArray;
	var ids = []
	for( var i = 0; i < elNews.length; i++)
		if( elNews[i].m_news.user == i_news.user )
			ids.push( elNews[i].m_news.id);

	nw_DeleteNews( ids);
}

function nw_DeleteNews( i_ids)
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
		nw_NewsLoad();
		return;
	}

	var obj = {};
	obj.objects = [];
	for( var i = 0; i < i_ids.length; i++ )
		obj.objects.push({"id":i_ids[i]});
	obj.delobj = true;

	obj.file = 'users/'+g_auth_user.id+'.json';
	n_Request({"send":{"editobj":obj},"func":nw_DeleteNewsFinished});
}
function nw_DeleteNewsFinished( i_data)
{
	if(( i_data == null ) || ( i_data.error ))
	{
		c_Error( i_data.error );
		return;
	}

	c_Info('News deleted');
	nw_NewsLoad();
}


// Recent:

function nw_RecentOnClick()
{
	if( $('sidepanel').classList.contains('opened'))
	{
		if( $('sidepanel_recent').classList.contains('opened'))
			nw_RecentClose();
		else
			nw_RecentOpen();
	}
	else
	{
		u_SidePanelOpen();
		nw_RecentOpen();
	}
}

function nw_RecentClose()
{
	$('sidepanel_recent').classList.remove('opened');
	$('recent').innerHTML = '';
	localStorage.recent_opened = 'false';
}
function nw_RecentOpen( i_noload )
{
	$('sidepanel_recent').classList.add('opened');
	localStorage.recent_opened = 'true';
	if( i_noload !== false )
		nw_RecentLoad();
}
function nw_RecentLoad( i_check, i_cache)
{
	if(( i_check !== false ) && ( false == c_RuFileExists( nw_recent_file)))
		return;

	if(( i_cache !== false ) && nw_recents[g_CurPath()] && ( c_DT_CurSeconds() - nw_recents[g_CurPath()].time < RULES.cache_time ))
	{
		c_Log('Recent cached '+RULES.cache_time+'s: '+g_CurPath());
		nw_RecentReceived( nw_recents[g_CurPath()].array );
		return;
	}

	var file = c_GetRuFilePath( nw_recent_file);
	n_Request({"send":{"readobj":file},"local":true,"func":nw_RecentReceived,"info":"recent","wait":false,"parse":true});
	$('recent').textContent = 'Loading...';
}

function nw_RecentRefresh()
{
	nw_RecentLoad( true, false);
}

function nw_RecentReceived( i_data, i_args)
{
//console.log( i_args);
//console.log( i_data.length);
//return;
	if( i_args )
	{
		nw_recents[i_args.path] = {"array":i_data,"time":c_DT_CurSeconds()};
		if( i_args.path != g_CurPath()) return;
	}

	$('recent').innerHTML = '';
	if( i_data == null ) return;
	if( i_data.error ) return;

	for( var i = 0; i < i_data.length; i++)
	{
		var news = i_data[i];

		var el = document.createElement('div');
		$('recent').appendChild( el);

		var avatar = c_GetAvatar( news.user, news.guest);
		if( avatar )
		{
			var elAvatar = document.createElement('img');
			el.appendChild( elAvatar);
			elAvatar.classList.add('avatar');
			elAvatar.src = avatar;
		}

		var elUser = document.createElement('div');
		el.appendChild( elUser);
		elUser.classList.add('user');
		elUser.textContent = c_GetUserTitle( news.user, news.guest);

		var elTitle = document.createElement('div');
		el.appendChild( elTitle);
		elTitle.classList.add('title');
		elTitle.innerHTML = news.title;

		var elLink = document.createElement('a');
		el.appendChild( elLink);
		elLink.href = '#'+news.path;
		elLink.textContent = news.path;

		var elTime = document.createElement('div');
		el.appendChild( elTime);
		elTime.classList.add('time');
		elTime.innerHTML = c_DT_StrFromSec( news.time, /*no seconds = */ true);
	}
}

