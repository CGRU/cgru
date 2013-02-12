s_subscribes = [];
s_events = [];

s_initialized = false;
s_elements = ['subscribe','subscribe_btn','unsubscribe_btn','subscribe_label','subscribe_path','channels'];
s_el = {};

function s_Init( i_user)
{
	for( var i = 0; i < s_elements.length; i++)
		s_el[s_elements[i]] = document.getElementById( s_elements[i]);

	s_subscribes = i_user.subscribe;
	s_events = i_user.events;
	s_el.subscribe.style.display = 'block';
	u_el.sidepanel_news.style.display = 'block';
	s_initialized = true;

	if( localStorage.news_opened == "true" ) s_NewsOpen();
	else s_NewsClose();

	s_Finish();
	s_UpdateChannels();
}

function s_UpdateChannels()
{
	s_el.channels.innerHTML = '';
	for( var i = 0; i < s_subscribes.length; i++ )
	{
		s_path = s_subscribes[i].id;
		var el = document.createElement('div');
		s_el.channels.appendChild( el);
		el.title = 'Subscribed by '+s_subscribes[i].user+' at\n'+c_DT_StrFromS( s_subscribes[i].time);

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

//window.console.log(s_subscribes);
	var subscribed = false;
	var path = g_CurPath();
	for( var i = 0; i < s_subscribes.length; i++ )
	{
		s_path = s_subscribes[i].id;
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

	if( s_events.length == 0 ) return;
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

	var subscribe = {};
	subscribe.id = i_path;
	subscribe.time = c_DT_CurSeconds();
	subscribe.user = g_auth_user;

	var obj = {};
	obj.objects = [subscribe];
	obj.pusharray = 'subscribe';
	obj.id = g_auth_user;
	obj.file = 'users/' + g_auth_user + '.json';

	n_Request({"editobj":obj});

	s_subscribes.push( subscribe);
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
	obj.id = g_auth_user;
	obj.file = 'users/' + g_auth_user + '.json';

	n_Request({"editobj":obj});

	for( i = 0; i < s_subscribes.length; i++)
		if( s_subscribes[i].id == i_path )
		{
			s_subscribes.splice( i, 1);
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
		if( u_el['sidepanel_news'].classList.contains('opened'))
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
	u_el['sidepanel_news'].classList.remove('opened');
//	u_el['playlist'].innerHTML = '';
	localStorage.news_opened = false;
}
function s_NewsOpen()
{
	u_el['sidepanel_news'].classList.add('opened');
	localStorage.news_opened = true;
//	p_Load();
}

function s_MakeNewsDialog()
{
	new cgru_Dialog( window, window, 's_MakeNews', null, 'str', '', 'news',
		'Create News', 'Enter News Title');
}

function s_MakeNews( i_param, i_value)
{
//window.console.log(i_value);
	var news = {};
	news.time = c_DT_CurSeconds();
	news.path = g_CurPath();
	news.title = i_value;
	n_Request({"news":news});
}

