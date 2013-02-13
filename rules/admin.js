ad_elements = ['sidepanel_users','users'];
ad_el = {};
ad_initialized = false;

function ad_Init()
{
	if( g_auth_user == null ) return;

	document.getElementById('set_password').style.display = 'block';

	if( g_auth_user.role != 'admin' ) return;

	document.getElementById('admin_window').style.display = 'block';

	for( var i = 0; i < ad_elements.length; i++)
		ad_el[ad_elements[i]] = document.getElementById( ad_elements[i]);

	ad_el.sidepanel_users.style.display = 'block';

	if( localStorage.users_opened == "true" ) ad_UsersOpen();
	else ad_UsersClose();

	ad_initialized = true;
}

function ad_UsersProcess()
{
	if( false == ad_initialized ) return;
	if( localStorage.users_opened == "true" )
		ad_UsersLoad();
}

function ad_UsersFinish()
{
	if( false == ad_initialized ) return;
	ad_el.users.innerHTML = '';
}

function ad_UsersOnClick()
{
	if( u_el['sidepanel'].classList.contains('opened'))
	{
		if( ad_el.sidepanel_users.classList.contains('opened'))
			ad_UsersClose();
		else
			ad_UsersOpen();
	}
	else
	{
		u_SidePanelOpen();
		ad_UsersOpen();
	}
}

function ad_UsersClose()
{
	ad_el.sidepanel_users.classList.remove('opened');
	ad_el.users.innerHTML = '';
	localStorage.users_opened = "false";
}
function ad_UsersOpen()
{
	ad_el.sidepanel_users.classList.add('opened');
	localStorage.users_opened = "true";
	ad_UsersLoad();
}

function ad_UsersLoad()
{
	ad_el.users.innerHTML = '';

	var path = g_CurPath();
	if( path == null ) return;
	var res = c_Parse( n_Request({"usersget":{"path":RULES.root+path}}));
	if( res == null ) return;
	if( res.error )
	{
		c_Error( res.error );
		return;
	}
	if( res.users == null )
	{
		c_Error('Error getting users.');
		return;
	}

	for( var i = 0; i < res.users.length; i++)
	{
		var el = document.createElement('div');
		ad_el.users.appendChild( el);

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.textContent = '-';
		elBtn.m_user_id = res.users[i];
		elBtn.ondblclick = function(e){ad_UsersDel(e.currentTarget.m_user_id)};

		var elName = document.createElement('div');
		el.appendChild( elName);
		elName.textContent = res.users[i];
	}
}

function ad_UsersAddOnClick()
{
	new cgru_Dialog( window, window, 'ad_UsersAdd', null, 'str', g_auth_user.id, 'users', 'Add User', 'Enter User ID');
}
function ad_UsersAdd( i_not_used, i_user_id)
{
	var res = c_Parse( n_Request({"usersadd":{"path":RULES.root+g_CurPath(),"id":i_user_id}}));
	if( res.error )
	{
		c_Error( res.error );
		return;
	}
	ad_UsersLoad();
}

function ad_UsersDel( i_user_id)
{
	var res = c_Parse( n_Request({"usersdel":{"path":RULES.root+g_CurPath(),"id":i_user_id}}));
	if( res.error )
	{
		c_Error( res.error );
		return;
	}
	ad_UsersLoad();
}

function ad_UsersClearOnClick()
{
	var res = c_Parse( n_Request({"usersclear":{"path":RULES.root+g_CurPath()}}));
	if( res.error )
	{
		c_Error( res.error );
		return;
	}
	ad_UsersLoad();
}

function ad_OpenWindow()
{
	var wnd = new cgru_Window('Administrate');

	var res = c_Parse( n_Request({"getallusers":true}));
	if( res == null )
	{
		wnd.elContent.innerHTML = 'Error getting users.';
		return;
	}
	if( res.error )
	{
		wnd.elContent.innerHTML = 'Error getting users:<br>'+res.error;
		return;
	}

	wnd.elContent.classList.add('administrate');
	wnd.elContent.classList.add('admin_users');

	var labels = {};
	labels.id = 'Name';
	labels.role = 'Role';
	labels.channels = {}; labels.channels.length = 'Cnls';
	labels.news = {}; labels.news.length = 'News';

	var row = 0;
	ad_WndAddUser( wnd.elContent, labels, row++);
	for( var user in res.users )
		ad_WndAddUser( wnd.elContent, res.users[user], row++);
}

function ad_WndAddUser( i_el, i_user, i_row)
{
		var el = document.createElement('div');
		i_el.appendChild(el);
		if( i_row )
		{
			el.classList.add('user');
			if( i_row % 2) el.style.backgroundColor = '#CCC';
			else el.style.backgroundColor = '#BBB';
		}
		else el.style.backgroundColor = '#999999';

		var elDel = document.createElement('div');
		el.appendChild( elDel);
		elDel.style.width = '20px';
//		if( i_row )
		{
//			elDel.classList.add('button');
			elDel.textContent = '-';
		}
//		else elDel.textContent = 'DEL';
		elDel.title = 'Double to delete user';

		var elName = document.createElement('div');
		el.appendChild( elName);
		elName.style.width = '100px';
		elName.textContent = i_user.id;

		var elRole = document.createElement('div');
		el.appendChild( elRole);
		elRole.style.width = '100px';
		elRole.textContent = i_user.role;
		elRole.m_user_id = i_user.id;
		if( i_row ) elRole.ondblclick = function(e){ad_ChangeRoleOnCkick(e.currentTarget.m_user_id);};

		var elPasswd = document.createElement('div');
		el.appendChild( elPasswd);
		elPasswd.style.width = '50px';
		if( i_row )
		{
			elPasswd.textContent = '***';
//			elPasswd.classList.add('button');
		}
		else elPasswd.textContent = 'Pass';
		elPasswd.title = 'Double click to edit password';

		var elChannels = document.createElement('div');
		el.appendChild( elChannels);
		elChannels.textContent = i_user.channels.length;
		elChannels.style.width = '50px';

		var elNews = document.createElement('div');
		el.appendChild( elNews);
		elNews.textContent = i_user.news.length;
		elNews.style.width = '50px';

		var elCTime = document.createElement('div');
		el.appendChild( elCTime);
		if( i_row ) elCTime.textContent = c_DT_StrFromS( i_user.ctime);
		else elCTime.textContent = 'Creation Time';
		elCTime.style.width = '200px';

		var elRTime = document.createElement('div');
		el.appendChild( elRTime);
		if( i_row ) elRTime.textContent = c_DT_StrFromS( i_user.rtime);
		else elRTime.textContent = 'Refresh Time';
		elRTime.style.width = '200px';
}

function ad_ChangeRoleOnCkick( i_user_id)
{
	new cgru_Dialog( window, window, 'ad_ChangeRole', i_user_id, 'str', '', 'users', 'Change Role', 'Enter New Role');
}
function ad_ChangeRole( i_user_id, i_role)
{
	var obj = {};

	obj.add = true;
	obj.object = {"role":i_role};
	obj.file = 'users/' + i_user_id + '.json';

	var res = n_Request({"editobj":obj});
	if( res && res.error )
	{
		c_Error( res.error );
		return;
	}
}

