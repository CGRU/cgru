ad_elements = ['sidepanel_users','users'];
ad_el = {};
ad_initialized = false;
ad_wnd = null;

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
	ad_wnd = new cgru_Window('Administrate');

	var users = ad_GetAllUsers();
	if( users == null ) return;

	ad_wnd.elContent.classList.add('administrate');
	ad_wnd.elContent.classList.add('admin_users');

	var elBtnsDiv = document.createElement('div');
	ad_wnd.elContent.appendChild( elBtnsDiv );

	var elBtnRefresh = document.createElement('div');
	elBtnsDiv.appendChild( elBtnRefresh);
	elBtnRefresh.classList.add('button');
	elBtnRefresh.textContent = 'Refresh';
	elBtnRefresh.onclick = ad_WndRefresh;

	var elBtnCreate = document.createElement('div');
	elBtnsDiv.appendChild( elBtnCreate);
	elBtnCreate.classList.add('button');
	elBtnCreate.textContent = 'Create';
	elBtnCreate.onclick = ad_CreateUserOnClick;

	var elBtnDelete = document.createElement('div');
	elBtnsDiv.appendChild( elBtnDelete);
	elBtnDelete.classList.add('button');
	elBtnDelete.textContent = 'Delete';
	elBtnDelete.onclick = ad_DeleteUserOnClick;

	ad_wnd.elUsers = document.createElement('div');
	ad_wnd.elContent.appendChild( ad_wnd.elUsers);

	ad_WndDrawUsers( users);
}

function ad_WndDrawUsers( i_users)
{
	ad_wnd.elUsers.innerHTML = '';

	var labels = {};
	labels.id = 'Name';
	labels.title = 'Title';
	labels.role = 'Role';
	labels.channels = {}; labels.channels.length = 'Cnls';
	labels.news = {}; labels.news.length = 'News';

	var row = 0;
	ad_WndAddUser( ad_wnd.elUsers, labels, row++);
	for( var user in i_users )
		ad_WndAddUser( ad_wnd.elUsers, i_users[user], row++);
}

function ad_GetAllUsers()
{
	var res = c_Parse( n_Request({"getallusers":true}));
	if( res == null )
	{
		ad_wnd.elContent.innerHTML = 'Error getting users.';
		return null;
	}
	if( res.error )
	{
		ad_wnd.elContent.innerHTML = 'Error getting users:<br>'+res.error;
		return null;
	}
	if( res.users == null )
	{
		ad_wnd.elContent.innerHTML = 'Users are NULL.';
		return null;
	}
	g_users = res.users;
	return res.users;
}

function ad_WndRefresh()
{
	if( ad_wnd == null ) return;
	var users = ad_GetAllUsers();
	if( users == null ) return;

	ad_WndDrawUsers( users);
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

	var elName = document.createElement('div');
	el.appendChild( elName);
	elName.style.width = '100px';
	elName.textContent = i_user.id;

	var elTitle = document.createElement('div');
	el.appendChild( elTitle);
	elTitle.style.width = '150px';
	elTitle.textContent = i_user.title;
	elTitle.m_user_id = i_user.id;
	elTitle.title = 'Double click edit title';
	if( i_row ) elTitle.ondblclick = function(e){ad_ChangeTitleOnCkick(e.currentTarget.m_user_id);};

	var elRole = document.createElement('div');
	el.appendChild( elRole);
	elRole.style.width = '100px';
	elRole.textContent = i_user.role;
	elRole.m_user_id = i_user.id;
	elRole.title = 'Double click edit role';
	if( i_row ) elRole.ondblclick = function(e){ad_ChangeRoleOnCkick(e.currentTarget.m_user_id);};

	var elPasswd = document.createElement('div');
	el.appendChild( elPasswd);
	elPasswd.style.width = '50px';
	if( i_row ) elPasswd.textContent = '***';
	else elPasswd.textContent = 'Pass';
	elPasswd.m_user_id = i_user.id;
	elPasswd.title = 'Double click to edit password';
	if( i_row ) elPasswd.ondblclick = function(e){ad_SetPasswordDialog( null, e.currentTarget.m_user_id);};

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
	if( i_row ) elCTime.textContent = c_DT_StrFromS( i_user.ctime).substr(4,11);
	else elCTime.textContent = 'Created';
	elCTime.style.width = '150px';

	var elRTime = document.createElement('div');
	el.appendChild( elRTime);
	if( i_row ) elRTime.textContent = c_DT_StrFromS( i_user.rtime).substr(4);
	else elRTime.textContent = 'Entered';
	elRTime.style.width = '200px';
}

function ad_ChangeRoleOnCkick( i_user_id)
{
	new cgru_Dialog( window, window, 'ad_ChangeRole', i_user_id, 'str', g_users[i_user_id].role, 'users', 'Change Role', 'Enter New Role');
}
function ad_ChangeRole( i_user_id, i_role)
{
	var obj = {};

	obj.add = true;
	obj.object = {"role":i_role};
	obj.file = 'users/' + i_user_id + '.json';

	var res = c_Parse( n_Request({"editobj":obj}));
	if( res && res.error )
	{
		c_Error( res.error );
		return;
	}
	ad_WndRefresh();
}

function ad_ChangeTitleOnCkick( i_user_id)
{
	new cgru_Dialog( window, window, 'ad_ChangeTitle', i_user_id, 'str', g_users[i_user_id].title, 'users', 'Change Title', 'Enter New Title');
}
function ad_ChangeTitle( i_user_id, i_title)
{
	var obj = {};

	obj.add = true;
	obj.object = {"title":i_title};
	obj.file = 'users/' + i_user_id + '.json';

	var res = c_Parse( n_Request({"editobj":obj}));
	if( res && res.error )
	{
		c_Error( res.error );
		return;
	}
	ad_WndRefresh();
}

function ad_CreateUserOnClick()
{
	new cgru_Dialog( window, window, 'ad_CreateUser', null, 'str', '', 'users',
		'Create New User', 'Enter User Login Name');
}
function ad_CreateUser( not_used, i_user_id)
{
	var user = {};
	user.id = i_user_id;
	user.channels = [];
	user.news = [];
	user.ctime = Math.round((new Date()).valueOf()/1000);
	user.role = 'user';

	var obj = {};
	obj.add = true;
	obj.object = user;
	obj.file = 'users/' + i_user_id + '.json';

	var res = c_Parse( n_Request({"editobj":obj}));
	if( res && res.error )
	{
		c_Error( res.error );
		return;
	}
	c_Info('User "'+i_user_id+'" created.');
	ad_WndRefresh();
}

function ad_DeleteUserOnClick()
{
	new cgru_Dialog( window, window, 'ad_DeleteUser', null, 'str', '', 'users',
		'Delete User', 'Enter User Login Name');
}
function ad_DeleteUser( not_used, i_user_id)
{
	var res = c_Parse( n_Request({"deleteuser":i_user_id}));
	if( res && res.error )
	{
		c_Error( res.error );
		return;
	}
	c_Info('User "'+i_user_id+'" deleted.');
	ad_WndRefresh();
}

function ad_SetPasswordOnclick()
{
	if( g_auth_user == null ) return;
	ad_SetPasswordDialog( null, g_auth_user.id )
}

function ad_SetPasswordDialog( not_used, i_user_id)
{
	new cgru_Dialog( window, window, 'ad_SetPassword', i_user_id, 'str', '', 'password',
		'Set Password', 'Enter New Password');
}

function ad_SetPassword( i_user_id, i_passwd)
{
	var result = c_Parse( n_Request({"htdigest":{"user":i_user_id,"p":i_passwd}}, true, true));

	if( result.error )
		c_Error( result.error);
	else if( result.status )
		c_Info( result.status);
}

