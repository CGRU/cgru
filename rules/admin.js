ad_initialized = false;
ad_permissions = null;
ad_wnd = null;
ad_wnd_curgroup = null;

function ad_Init()
{
	if( g_auth_user == null ) return;
	if( false == g_admin ) return;

	$('admin_window').style.display = 'block';
	$('sidepanel_permissions').style.display = 'block';

	if( localStorage.permissions_opened == "true" ) ad_PermissionsOpen();
	else ad_PermissionsClose();

	ad_initialized = true;
}

function ad_PermissionsProcess()
{
	if( false == ad_initialized ) return;
	if( localStorage.permissions_opened == "true" )
		ad_PermissionsLoad();
}

function ad_PermissionsFinish()
{
	if( false == ad_initialized ) return;
	$('permissions').innerHTML = '';
}

function ad_PermissionsOnClick()
{
	if( u_el['sidepanel'].classList.contains('opened'))
	{
		if( $('sidepanel_permissions').classList.contains('opened'))
			ad_PermissionsClose();
		else
			ad_PermissionsOpen();
	}
	else
	{
		u_SidePanelOpen();
		ad_PermissionsOpen();
	}
}

function ad_PermissionsClose()
{
	$('sidepanel_permissions').classList.remove('opened');
	$('permissions').innerHTML = '';
	localStorage.permissions_opened = "false";
}
function ad_PermissionsOpen()
{
	$('sidepanel_permissions').classList.add('opened');
	localStorage.permissions_opened = "true";
	ad_PermissionsLoad();
}

function ad_PermissionsLoad()
{
	$('permissions').innerHTML = '';

	var path = g_CurPath();
	if( path == null ) return;
	ad_permissions = {};
	ad_permissions.path = RULES.root + path;
	ad_permissions = c_Parse( n_Request({"permissionsget": ad_permissions}));
	if( ad_permissions == null ) return;
	if( ad_permissions.error )
	{
		c_Error( ad_permissions.error );
		return;
	}
	if(( ad_permissions.users == null ) || ( ad_permissions.groups == null ))
	{
		c_Error('Error loading permissions.');
		return;
	}

	ad_permissions.path = RULES.root + path;

	if( ad_permissions.groups.length )
	{
		var el = document.createElement('div');
		$('permissions').appendChild( el);
		el.textContent = 'Groups:';
	}
	for( var i = 0; i < ad_permissions.groups.length; i++)
	{
		var group = ad_permissions.groups[i];
		var el = document.createElement('div');
		$('permissions').appendChild( el);

		if( group != 'admins' )
		{
			var elBtn = document.createElement('div');
			el.appendChild( elBtn);
			elBtn.classList.add('button');
			elBtn.textContent = '-';
			elBtn.m_group_id = group;
			elBtn.ondblclick = function(e){ad_PermissionsRemove('groups', e.currentTarget.m_group_id)};
		}

		var elName = document.createElement('div');
		el.appendChild( elName);
		elName.textContent = group;
	}

	if( ad_permissions.users.length )
	{
		var el = document.createElement('div');
		$('permissions').appendChild( el);
		el.textContent = 'Users:';
	}
	for( var i = 0; i < ad_permissions.users.length; i++)
	{
		var user = ad_permissions.users[i];
		var el = document.createElement('div');
		$('permissions').appendChild( el);

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.textContent = '-';
		elBtn.m_user_id = user;
		elBtn.ondblclick = function(e){ad_PermissionsRemove('users', e.currentTarget.m_user_id)};

		var elName = document.createElement('div');
		el.appendChild( elName);
		elName.textContent = user;
	}
}

function ad_PermissionsGrpAddOnClick()
{
	new cgru_Dialog( window, window, 'ad_PermissionsAdd', 'groups', 'str', 'admins', 'permissions', 'Add Group', 'Enter Group ID');
}
function ad_PermissionsUsrAddOnClick()
{
	new cgru_Dialog( window, window, 'ad_PermissionsAdd', 'users', 'str', '', 'permssions', 'Add User', 'Enter User ID');
}
function ad_PermissionsAdd( i_type, i_id)
{
	i_id = c_Strip( i_id);
	if( i_id.length < 1 )
	{
		c_Error('Empty name.');
		return;
	}
	if( ad_permissions[i_type].indexOf( i_id) != -1 )
	{
		c_Error( i_id+' is already in '+i_type);
		return;
	}
	ad_permissions[i_type].push( i_id);
	var res = c_Parse( n_Request({"permissionsset":ad_permissions}));
	if( res.error ) c_Error( res.error );
	ad_PermissionsLoad();
}

function ad_PermissionsRemove( i_type, i_id)
{
	var index = ad_permissions[i_type].indexOf( i_id);
	if( index == -1 )
	{
		c_Error( i_id+' is not in '+i_type);
		return;
	}
	ad_permissions[i_type].splice( index, 1);
	var res = c_Parse( n_Request({"permissionsset":ad_permissions}));
	if( res.error ) c_Error( res.error );
	ad_PermissionsLoad();
}

function ad_PermissionsClearOnClick()
{
	var res = c_Parse( n_Request({"permissionsclear":{"path":RULES.root+g_CurPath()}}));
	if( res.error )
	{
		c_Error( res.error );
		return;
	}
	ad_PermissionsLoad();
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

	var elBtnCreateUsr = document.createElement('div');
	elBtnsDiv.appendChild( elBtnCreateUsr);
	elBtnCreateUsr.classList.add('button');
	elBtnCreateUsr.textContent = 'Create User';
	elBtnCreateUsr.onclick = ad_CreateUserOnClick;

	var elBtnDeleteUsr = document.createElement('div');
	elBtnsDiv.appendChild( elBtnDeleteUsr);
	elBtnDeleteUsr.classList.add('button');
	elBtnDeleteUsr.textContent = 'Delete User';
	elBtnDeleteUsr.onclick = ad_DeleteUserOnClick;

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
		if( grp == ad_wnd_curgroup )
			el.classList.add('selected');
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

	ad_wnd_curgroup = el.m_group;
	for( var i = 0; i < ad_wnd.elUsrRows.length; i++)
	{
		var elU = ad_wnd.elUsrRows[i];
		if( elU.m_user.groups.indexOf( el.m_group) != -1 )
			elU.classList.add('selected');
		else
			elU.classList.remove('selected');
//console.log( elU.m_user.id+' '+elU.m_user.groups+' '+ el.m_group);
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
		if( i_user.groups.indexOf( ad_wnd_curgroup ) != -1 )
			el.classList.add('selected');
	}
	else el.style.backgroundColor = 'rgba(0,0,0,.2)';

	var elGroup = document.createElement('div');
	el.appendChild( elGroup);
	elGroup.style.width = '20px';
	elGroup.textContent = 'G';
	elGroup.title = 'Double click edit group';
	elGroup.style.cursor = 'pointer';
	elGroup.m_user = i_user;
	if( i_row ) elGroup.ondblclick = function(e){ad_WndUserGroupOnCkick( e.currentTarget.m_user);};

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
	if( i_user.channels && i_user.channels.length )
	{
		var channels = '';
		for( var i = 0; i < i_user.channels.length; i++)
		 channels += i_user.channels[i].id+'\n';
		elChannels.title = channels;
	}

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
	if( g_groups[i_group] != null )
	{
		c_Error('Group "'+i_group+'" already exists.');
		return;
	}
	g_groups[i_group] = {};
	ad_WriteGroups();
}

function ad_DeleteGrpOnClick() { new cgru_Dialog( window, window, 'ad_DeleteGroup', null, 'str', '', 'users', 'Delete Group', 'Enter Group Name');}
function ad_DeleteGroup( i_not_used, i_group)
{
	if( g_groups[i_group] == null )
	{
		c_Error('Group "'+i_group+'" does not exist.');
		return;
	}
	delete g_groups[i_group];
	ad_WriteGroups();
}

function ad_WndUserGroupOnCkick( i_user)
{
	var grp = ad_wnd_curgroup;
	if( grp == null )
	{
		c_Error('No group selected');
		return;
	}
	for( var i = 0; i < ad_wnd.elGrpBnts.length; i++)
		if( ad_wnd.elGrpBnts[i].classList.contains('selected'))
			grp = ad_wnd.elGrpBnts[i].m_group;

	if( i_user.groups.indexOf( grp ) == -1  )
	{
		if( g_groups[grp].indexOf( i_user.id ) != -1 )
		{
			c_Error('User '+i_user.id+' already in group '+grp);
			return;
		}
		g_groups[grp].push( i_user.id );
	}
	else
	{
		if( g_groups[grp].indexOf( i_user.id ) == -1 )
		{
			c_Error('User '+i_user.id+' is not in group '+grp);
			return;
		}
		g_groups[grp].splice( g_groups[grp].indexOf( i_user.id ), 1);
	}

	ad_WriteGroups();
}

function ad_WriteGroups()
{
	var res = c_Parse( n_Request({"writegroups":g_groups}));
	if( res == null ) return;
	if( res.error ) { c_Error( res.error ); return; }
	ad_WndRefresh();
}
/*
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
*/
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

