nw_initialized = false;
nw_elements = ['subscribe','subscribe_btn','unsubscribe_btn','subscribe_label','subscribe_path','sidepanel_news','news','channels'];
nw_el = {};

function nw_Init()
{
	if( g_auth_user == null ) return;

	for( var i = 0; i < nw_elements.length; i++) nw_el[nw_elements[i]] = document.getElementById( nw_elements[i]);

	nw_el.subscribe.style.display = 'block';
	nw_el.sidepanel_news.style.display = 'block';
	nw_initialized = true;

	if( localStorage.newnw_opened == "true" ) nw_NewsOpen();
	else nw_NewsClose();

	nw_Finish();
	nw_UpdateChannels();
}

function nw_UpdateChannels()
{
	nw_el.channels.innerHTML = '';
	for( var i = 0; i < g_auth_user.channels.length; i++ )
	{
		nw_path = g_auth_user.channels[i].id;
		var el = document.createElement('div');
		nw_el.channels.appendChild( el);
		el.title = 'Subscribed by '+g_auth_user.channels[i].user+' at\n'+c_DT_StrFromS( g_auth_user.channels[i].time);

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.textContent = '-';
		elBtn.m_path = nw_path;
		elBtn.ondblclick = function(e){ nw_Unsubscribe( e.currentTarget.m_path);};

		var elLink = document.createElement('a');
		el.appendChild( elLink);
		elLink.href = '#'+nw_path;
		elLink.textContent = nw_path;
	}
}

function nw_Process()
{
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
				nw_el.subscribe_path.style.display = 'none';
				nw_el.unsubscribe_btn.style.display = 'block';
			}
			else
			{
				nw_el.subscribe_path.style.display = 'block';
				nw_el.unsubscribe_btn.style.display = 'none';
				nw_el.subscribe_path.innerHTML = 'at <a href="#'+nw_path+'">'+nw_path+'</a>';
			}
			break;
		}
	}

	if( subscribed )
	{
		nw_el.subscribe_label.style.display = 'block';
		nw_el.subscribe_btn.style.display = 'none';
	}
	else
		nw_Finish();
}

function nw_Finish()
{
	if( false == nw_initialized ) return;

	nw_el.subscribe_label.style.display = 'none';
	nw_el.subscribe_btn.style.display = 'block';
	nw_el.unsubscribe_btn.style.display = 'none';
	nw_el.subscribe_path.style.display = 'none';
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
	if( u_el['sidepanel'].classList.contains('opened'))
	{
		if( nw_el.sidepanel_news.classList.contains('opened'))
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
	nw_el.sidepanel_news.classList.remove('opened');
	nw_el.news.innerHTML = '';
	localStorage.newnw_opened = false;
	g_auth_user.news = null;
}
function nw_NewsOpen()
{
	nw_el.sidepanel_news.classList.add('opened');
	localStorage.newnw_opened = true;
	nw_NewsLoad();
}

function nw_MakeNewsDialog()
{
	if( g_auth_user == null ) return;
	new cgru_Dialog( window, window, 'nw_MakeNews', null, 'str', '', 'news',
		'Create News', 'Enter News Title');
}

function nw_MakeNews( i_value, i_path)
{
//window.console.log(i_value);
	if( g_auth_user == null ) return;

	if( i_path == null ) i_path = g_CurPath();

	var news = {};
	news.time = c_DT_CurSeconds();
	news.path = i_path;
	news.title = i_value;
	news.id = g_auth_user.id+'_'+news.time+'_'+news.path;

	var msg = c_Parse( n_Request({"makenews":news}));
	if( msg.error )
	{
		c_Error( msg.error);
		return;
	}
	else
		nw_NewsLoad();

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

function nw_NewsLoad()
{
	if( g_auth_user == null ) return;

	nw_el.news.innerHTML = '';

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
		nw_el.news.appendChild( el);
		el.title = c_DT_StrFromS( news.time);

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.textContent = '-';
		elBtn.m_id = news.id;
		elBtn.ondblclick = function(e){ nw_RemoveNews( e.currentTarget.m_id);};

		var elLabel = document.createElement('div');
		el.appendChild( elLabel);
		elLabel.classList.add('newnw_label');
		elLabel.innerHTML = news.user+': '+news.title;

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
	obj.objects = [{"id":i_id}];
	obj.delobj = true;
	obj.file = 'users/'+g_auth_user.id+'.json';
	n_Request({"editobj":obj});
	nw_NewsLoad();
}

