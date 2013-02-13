s_initialized = false;
s_elements = ['subscribe','subscribe_btn','unsubscribe_btn','subscribe_label','subscribe_path','sidepanel_news','news','channels'];
s_el = {};

function s_Init()
{
	if( g_auth_user == null ) return;

	for( var i = 0; i < s_elements.length; i++)
		s_el[s_elements[i]] = document.getElementById( s_elements[i]);

	s_el.subscribe.style.display = 'block';
	s_el.sidepanel_news.style.display = 'block';
	s_initialized = true;

	if( localStorage.news_opened == "true" ) s_NewsOpen();
	else s_NewsClose();

	s_Finish();
	s_UpdateChannels();
}

function s_UpdateChannels()
{
	s_el.channels.innerHTML = '';
	for( var i = 0; i < g_auth_user.channels.length; i++ )
	{
		s_path = g_auth_user.channels[i].id;
		var el = document.createElement('div');
		s_el.channels.appendChild( el);
		el.title = 'Subscribed by '+g_auth_user.channels[i].user+' at\n'+c_DT_StrFromS( g_auth_user.channels[i].time);

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.textContent = '-';
		elBtn.m_path = s_path;
		elBtn.ondblclick = function(e){ s_Unsubscribe( e.currentTarget.m_path);};

		var elLink = document.createElement('a');
		el.appendChild( elLink);
		elLink.href = '#'+s_path;
		elLink.textContent = s_path;
	}
}

function s_Process()
{
	if( false == s_initialized ) return;

//window.console.log(g_auth_user.channels);
	var subscribed = false;
	var path = g_CurPath();
	for( var i = 0; i < g_auth_user.channels.length; i++ )
	{
		var s_path = g_auth_user.channels[i].id;
		if( path.indexOf( s_path) == 0 )
		{
			subscribed = true;
			if( path == s_path )
			{
				s_el.subscribe_path.style.display = 'none';
				s_el.unsubscribe_btn.style.display = 'block';
			}
			else
			{
				s_el.subscribe_path.style.display = 'block';
				s_el.subscribe_path.innerHTML = 'at <a href="#'+s_path+'">'+s_path+'</a>';
			}
			break;
		}
	}

	if( subscribed )
	{
		s_el.subscribe_label.style.display = 'block';
		s_el.subscribe_btn.style.display = 'none';
	}
	else
		s_Finish();
}

function s_Finish()
{
	if( false == s_initialized ) return;

	s_el.subscribe_label.style.display = 'none';
	s_el.subscribe_btn.style.display = 'block';
	s_el.unsubscribe_btn.style.display = 'none';
	s_el.subscribe_path.style.display = 'none';
}

function s_Subscribe( i_path)
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
	s_Process();
	s_UpdateChannels();

	c_Info('Subscribed on ' + i_path);
}

function s_Unsubscribe( i_path)
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

	s_Process();
	s_UpdateChannels();

	c_Info('Unsubscribed from ' + i_path);
}

function s_NewsOnClick()
{
	if( u_el['sidepanel'].classList.contains('opened'))
	{
		if( s_el.sidepanel_news.classList.contains('opened'))
			s_NewsClose();
		else
			s_NewsOpen();
	}
	else
	{
		u_SidePanelOpen();
		s_NewsOpen();
	}
}

function s_NewsClose()
{
	s_el.sidepanel_news.classList.remove('opened');
	s_el.news.innerHTML = '';
	localStorage.news_opened = false;
	g_auth_user.news = null;
}
function s_NewsOpen()
{
	s_el.sidepanel_news.classList.add('opened');
	localStorage.news_opened = true;
	s_NewsLoad();
}

function s_MakeNewsDialog()
{
	if( g_auth_user == null ) return;
	new cgru_Dialog( window, window, 's_MakeNews', null, 'str', '', 'news',
		'Create News', 'Enter News Title');
}

function s_MakeNews( i_param, i_value)
{
//window.console.log(i_value);
	if( g_auth_user == null ) return;
	var news = {};
	news.time = c_DT_CurSeconds();
	news.path = g_CurPath();
	news.title = i_value;
	news.id = g_auth_user.id+'_'+news.time+'_'+news.path;

	var msg = c_Parse( n_Request({"makenews":news}));
	if( msg.error )
	{
		c_Error( msg.error);
		return;
	}
	else
		s_NewsLoad();

	if( msg.users.length == 0 )
	{
		c_Error('No subscribed users founded.');
		return;
	}

	var info = 'Subscibed users:';
	for( var i = 0; i < msg.users.length; i++)
		info += ' '+msg.users[i];
	c_Info( info);
}

function s_NewsLoad()
{
	if( g_auth_user == null ) return;

	s_el.news.innerHTML = '';

	if( g_auth_user.news == null )
	{
		var filename = 'users/'+g_auth_user.id+'.json';
		var user = c_Parse( n_Request({"readobj":filename}));
		if( user == null ) return;
		if( user.error )
		{
			c_Error( user.error);
			return;
		}
		g_auth_user.news = user.news;
	}

	for( var i = 0; i < g_auth_user.news.length; i++ )
	{
		var news = g_auth_user.news[i];

		var el = document.createElement('div');
		s_el.news.appendChild( el);
		el.title = news.title + '\nby '+news.user+' at\n'+c_DT_StrFromS( news.time);

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.textContent = '-';
		elBtn.m_id = news.id;
		elBtn.ondblclick = function(e){ s_RemoveNews( e.currentTarget.m_id);};

		var elLabel = document.createElement('div');
		el.appendChild( elLabel);
		elLabel.classList.add('news_label');
		elLabel.textContent = news.user+': '+news.title;

		var elLink = document.createElement('a');
		el.appendChild( elLink);
		elLink.href = '#'+news.path;
		elLink.textContent = news.path;
	}

	g_auth_user.news = null;
}

function s_RemoveNews( i_id)
{
	var obj = {};
	obj.objects = [{"id":i_id}];
	obj.delobj = true;
	obj.file = 'users/'+g_auth_user.id+'.json';
	n_Request({"editobj":obj});
	s_NewsLoad();
}

