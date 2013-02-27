ad_elements = ['sidepanel_users','users'];
ad_el = {};
ad_initialized = false;
ad_wnd = null;

function ad_Init()
{
	if( g_auth_user == null ) return;

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
	ad_wnd.elContent.classList.add('administrate');

	var elBtnsDiv = document.createElement('div');
	ad_wnd.elContent.appendChild( elBtnsDiv );

	var elBtnRefresh = document.createElement('div');
	elBtnsDiv.appendChild( elBtnRefresh);
	elBtnRefresh.classList.add('button');
	elBtnRefresh.textContent = 'Refresh';
	elBtnRefresh.onclick = ad_WndRefresh;

	var elBtnCreateUsr = document.createElement('div');
	elBtnsDiv.appendChild( elBtnCreateUsr);
	elBtnCreateUsr.classList.add('button');
	elBtnCreateUsr.textContent = 'Create';
	elBtnCreateUsr.onclick = ad_CreateUserOnClick;

	var elBtnDeleteUsr = document.createElement('div');
	elBtnsDiv.appendChild( elBtnDeleteUsr);
	elBtnDeleteUsr.classList.add('button');
	elBtnDeleteUsr.textContent = 'Delete';
	elBtnDeleteUsr.onclick = ad_DeleteUserOnClick;

	var elBtnCreateGrp = document.createElement('div');
	elBtnsDiv.appendChild( elBtnCreateGrp);
	elBtnCreateGrp.classList.add('button');
	elBtnCreateGrp.textContent = 'Create Group';
	elBtnCreateGrp.onclick = ad_CreateGrpOnClick;

	var elBtnDeleteGrp = document.createElement('div');
	elBtnsDiv.appendChild( elBtnDeleteGrp);
	elBtnDeleteGrp.classList.add('button');
	elBtnDeleteGrp.textContent = 'Delete Group';
	elBtnDeleteGrp.onclick = ad_DeleteGrpOnClick;

	ad_wnd.elGroups = document.createElement('div');
	ad_wnd.elContent.appendChild( ad_wnd.elGroups);
	ad_wnd.elGroups.classList.add('admin_groups');

	ad_wnd.elUsers = document.createElement('div');
	ad_wnd.elContent.appendChild( ad_wnd.elUsers);
	ad_wnd.elUsers.classList.add('admin_users');

	ad_WndRefresh();
}

function ad_WndDrawGroups( i_groups)
{
	ad_wnd.elGroups.innerHTML = '';
	ad_wnd.elGrpBnts = [];
	for( var grp in i_groups )
	{
		var el = document.createElement('div');
		ad_wnd.elGroups.appendChild( el);
		ad_wnd.elGrpBnts.push( el);
		el.textContent = grp;
		el.m_group = grp;
		el.onclick = ad_WndGrpOnClick;
	}
}

function ad_WndDrawUsers( i_users)
{
	ad_wnd.elUsers.innerHTML = '';
	ad_wnd.elUsrRows = [];

	var labels = {};
	labels.id = 'Name';
	labels.title = 'Title';
	labels.role = 'Role';
//	labels.group = 'Group';
	labels.channels = {}; labels.channels.length = 'Cnls';
	labels.news = {}; labels.news.length = 'News';

	var row = 0;
	ad_WndAddUser( ad_wnd.elUsers, labels, row++);
	for( var user in i_users )
		ad_WndAddUser( ad_wnd.elUsers, i_users[user], row++);
}

function ad_GetAll( i_type)
{
	var request = {};
	request['getall'+i_type] = true;
	var res = c_Parse( n_Request( request));
	if( res == null )
	{
		ad_wnd.elContent.innerHTML = 'Error getting '+i_type+'.';
		return null;
	}
	if( res.error )
	{
		ad_wnd.elContent.innerHTML = 'Error getting '+i_type+':<br>'+res.error;
		return null;
	}
	if( res[i_type] == null )
	{
		ad_wnd.elContent.innerHTML = 'Users are NULL.';
		return null;
	}
	if( i_type == 'users' ) g_users = res[i_type];
	else if( i_type == 'groups') g_groups = res[i_type];
	return res[i_type];
}

function ad_WndGrpOnClick( i_evt)
{
	for( var i = 0; i < ad_wnd.elGrpBnts.length; i++)
		ad_wnd.elGrpBnts[i].classList.remove('selected');

	var el = i_evt.currentTarget;
	el.classList.add('selected');

	for( var i = 0; i < ad_wnd.elUsrRows.length; i++)
	{
		var elU = ad_wnd.elUsrRows[i];
		if( elU.m_user.groups.indexOf( el.m_group) != -1 )
			elU.classList.add('selected');
		else
			elU.classList.remove('selected');
console.log( elU.m_user.id+' '+elU.m_user.groups+' '+ el.m_group);
	}
}

function ad_WndRefresh()
{
	if( ad_wnd == null ) return;
	var groups = ad_GetAll('groups');
	if( groups == null ) return;

	var users = ad_GetAll('users');
	if( users == null ) return;

	for( var u in users )
	{
//window.console.log(user);
		users[u].groups = [];
		for( var grp in groups )
			if( groups[grp].indexOf( users[u].id ) != -1 )
				users[u].groups.push( grp);
	}

	ad_WndDrawGroups( groups);
	ad_WndDrawUsers( users);
}

function ad_WndAddUser( i_el, i_user, i_row)
{
	var el = document.createElement('div');
	i_el.appendChild(el);

	if( i_row )
	{
		ad_wnd.elUsrRows.push( el);
		el.m_user = i_user;
		el.classList.add('user');
		if( i_row % 2) el.style.backgroundColor = 'rgba(255,255,255,.1)';
		else el.style.backgroundColor = 'rgba(0,0,0,.1)';
	}
	else el.style.backgroundColor = 'rgba(0,0,0,.2)';

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
/*
	var elRole = document.createElement('div');
	el.appendChild( elRole);
	elRole.style.width = '100px';
	elRole.textContent = i_user.role;
	elRole.m_user_id = i_user.id;
	elRole.title = 'Double click edit role';
	if( i_row ) elRole.ondblclick = function(e){ad_ChangeRoleOnCkick(e.currentTarget.m_user_id);};
*/
/*
	var elGroup = document.createElement('div');
	el.appendChild( elGroup);
	elGroup.style.width = '100px';
	elGroup.textContent = i_user.group;
	elGroup.m_user_id = i_user.id;
	elGroup.title = 'Double click edit group';
	if( i_row ) elGroup.ondblclick = function(e){ad_ChangeGroupOnCkick(e.currentTarget.m_user_id);};
*/
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
	if( i_row ) elCTime.textContent = c_DT_StrFromSec( i_user.ctime).substr(4,11);
	else elCTime.textContent = 'Created';
	elCTime.style.width = '150px';

	var elRTime = document.createElement('div');
	el.appendChild( elRTime);
	if( i_row ) elRTime.textContent = c_DT_StrFromSec( i_user.rtime).substr(4);
	else elRTime.textContent = 'Entered';
	elRTime.style.width = '200px';
}

function ad_CreateGrpOnClick() { new cgru_Dialog( window, window, 'ad_CreateGroup', null, 'str', '', 'users', 'Create Group', 'Enter Group Name');}
function ad_CreateGroup( i_not_used, i_group)
{
	var res = c_Parse( n_Request({"creategroup":i_group}));
	if( res == null ) return;
	if( res.error ) { c_Error( res.error ); return; }
	c_Info('Group "'+i_group+'" created.');
	ad_WndRefresh();
}

function ad_DeleteGrpOnClick() { new cgru_Dialog( window, window, 'ad_DeleteGroup', null, 'str', '', 'users', 'Delete Group', 'Enter Group Name');}
function ad_DeleteGroup( i_not_used, i_group)
{
	var res = c_Parse( n_Request({"deletegroup":i_group}));
	if( res == null ) return;
	if( res.error ) { c_Error( res.error ); return; }
	c_Info('Group "'+i_group+'" deleted.');
	ad_WndRefresh();
}

function ad_ChangeRoleOnCkick( i_user_id) { new cgru_Dialog( window, window, 'ad_ChangeRole', i_user_id, 'str', g_users[i_user_id].role, 'users', 'Change Role', 'Enter New Role'); }
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

