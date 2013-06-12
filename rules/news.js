nw_recent_file = 'recent.json';

nw_initialized = false;

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
//console.log( RULES.newsrefresh);
	if( RULES.newsrefresh == null ) return;
	if( RULES.newsrefresh < 1 ) return;
	setInterval( nw_NewsLoad, RULES.newsrefresh * 1000);
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

function nw_UpdateChannels()
{
	$('channels').innerHTML = '';
	for( var i = 0; i < g_auth_user.channels.length; i++ )
	{
		nw_path = g_auth_user.channels[i].id;
		var el = document.createElement('div');
		$('channels').appendChild( el);
		el.title = 'Subscribed by '+g_auth_user.channels[i].user+' at\n'+c_DT_StrFromSec( g_auth_user.channels[i].time);

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.textContent = '-';
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
				$('subscribe_path').innerHTML = 'at <a href="#'+nw_path+'">'+nw_path+'</a>';
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
		nw_Finish();
}

function nw_Finish()
{
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

	n_Request({"editobj":obj});

	g_auth_user.channels.push( channel);
	nw_Process();
	nw_UpdateChannels();

	c_Info('Subscribed on ' + i_path);
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

	n_Request({"editobj":obj});

	for( i = 0; i < g_auth_user.channels.length; i++)
		if( g_auth_user.channels[i].id == i_path )
		{
			g_auth_user.channels.splice( i, 1);
			break;
		}

	nw_Process();
	nw_UpdateChannels();

	c_Info('Unsubscribed from ' + i_path);
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
	g_auth_user.news = null;
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
	new cgru_Dialog( window, window, 'nw_MakeNewsDialogApply', null, 'str', '', 'news',
		'Create News', 'Enter News Title');
}
function nw_MakeNewsDialogApply( i_not_used, i_title) { nw_MakeNews( i_title); }
function nw_MakeNews( i_title, i_path, i_user_id, i_guest )
{
	if( localStorage.news_disabled == 'true') return;
//window.console.log(i_title);
//	if( g_auth_user == null ) return;
	if( i_user_id == null )
	{
		if( g_auth_user )
			i_user_id = g_auth_user.id;
		else
			c_Error('Can`t make news with no user.');
	}

	if( i_path == null ) i_path = g_CurPath();

	var news = {};
	news.time = c_DT_CurSeconds();
	news.user = i_user_id;
	news.path = i_path;
	news.title = i_title;
	if( i_guest ) news.guest = i_guest;
	news.id = i_user_id+'_'+news.time+'_'+news.path;
	if( localStorage.news_ignore_own == 'true' )
		news.ignore_own = true;

	var request = {};
	request.news = news;
	request.root = RULES.root;
	request.rufolder = RULES.rufolder;
	request.recent_max = RULES.newsrecent;
	request.recent_file = nw_recent_file;
	var msg = c_Parse( n_Request({"makenews":request}));
	if( msg.error )
	{
		c_Error( msg.error);
		return;
	}

	nw_NewsLoad();
	nw_RecentLoad();
		
	if( msg.users.length == 0 )
	{
		c_Log('No subscribed users founded.');
		return;
	}
	var info = 'Subscibed users:';
	for( var i = 0; i < msg.users.length; i++)
		info += ' '+msg.users[i];
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

	if( g_auth_user.news == null )
	{
		var filename = 'users/'+g_auth_user.id+'.json';
		var user = c_Parse( n_Request({"readobj":filename}));
		$('news').innerHTML = '';
		if( user == null ) return;
		if( user.error )
		{
			c_Error( user.error);
			return;
		}
		g_auth_user.news = user.news;
	}

	nw_ShowCount();

	g_auth_user.news.sort( function(a,b){var attr='time';if(a[attr]>b[attr])return -1;if(a[attr]<b[attr])return 1;return 0;});
	for( var i = 0; i < g_auth_user.news.length; i++ )
	{
		var news = g_auth_user.news[i];

		var el = document.createElement('div');
		$('news').appendChild( el);
		el.title = c_DT_StrFromSec( news.time);

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.textContent = '-';
		elBtn.m_id = news.id;
		elBtn.ondblclick = function(e){ nw_RemoveNews( e.currentTarget.m_id);};
		elBtn.title = 'Double click to remove link';

		var avatar = c_GetAvatar( news.user, news.guest);
		if( avatar )
		{
			var elAvatar = document.createElement('img');
			el.appendChild( elAvatar);
			elAvatar.classList.add('avatar');
			elAvatar.src = avatar;
		}

		var elLabel = document.createElement('div');
		el.appendChild( elLabel);
		elLabel.classList.add('news_label');
		elLabel.innerHTML = c_GetUserTitle(news.user, news.guest)+': '+news.title;

		var elLink = document.createElement('a');
		el.appendChild( elLink);
		elLink.href = '#'+news.path;
		elLink.textContent = news.path;
	}

	g_auth_user.news = null;
}

function nw_RemoveNews( i_id)
{
	var obj = {};
	if( i_id == null )
	{
		obj.object = {"news":[]};
		obj.add = true;
	}
	else
	{
		obj.objects = [{"id":i_id}];
		obj.delobj = true;
	}
	obj.file = 'users/'+g_auth_user.id+'.json';
	n_Request({"editobj":obj});
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
function nw_RecentLoad()
{
	recent = c_Parse( n_GetRuFile( nw_recent_file, true));
	$('recent').innerHTML = '';
	if( recent.error ) return;

	for( var i = 0; i < recent.length; i++)
	{
		var news = recent[i];

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

