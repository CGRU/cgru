s_paths = [];
s_events = [];

s_initialized = false;
s_elements = ['subscribe','subscribe_btn','subscribe_label','subscribe_path','subscribe_watched'];
s_el = {};

function s_Init( i_user)
{
	for( var i = 0; i < s_elements.length; i++)
		s_el[s_elements[i]] = document.getElementById( s_elements[i]);

	s_paths = i_user.subscribe;
	s_events = i_user.events;
	s_el.subscribe.style.display = 'block';
	s_initialized = true;

	s_Finish();
}

function s_Process()
{
	if( false == s_initialized ) return;

//window.console.log(s_paths);
	var subscribed = false;
	var path = g_CurPath();
	for( var i = 0; i < s_paths.length; i++ )
		if( path.indexOf( s_paths[i]) == 0 )
		{
			subscribed = true;
			subscribe_path = s_paths[i];
			if( path == s_paths[i] )
			{
				s_el.subscribe_path.style.display = 'none';
			}
			else
			{
				s_el.subscribe_path.style.display = 'block';
				s_el.subscribe_path.textContent = s_paths[i];
			}
			break;
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
	s_el.subscribe_path.style.display = 'none';
	s_el.subscribe_watched.style.display = 'none';
}

function s_Subscribe()
{
	var path = g_CurPath();

	var obj = {};
//	obj.path = g_elCurFolder.m_path;
//	obj.subscribe = true;
//	obj.user = g_auth_user;

	obj.objects = [path];
	obj.pusharray = 'subscribe';
	obj.id = g_auth_user;
	obj.file = 'users/' + g_auth_user + '.json';

	n_Request({"editobj":obj});

	s_paths.push( path);

	c_Info('Subscribed on ' + path);
}

