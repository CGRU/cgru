ad_initialized = false;
ad_permissions = null;
ad_wnd = null;
ad_wnd_curgroup = null;
ad_wnd_sort_prop = 'id';
ad_wnd_sort_dir = 0;
ad_wnd_prof = null;

function ad_Init()
{
	if( g_auth_user == null ) return;

	$('profile_button').style.display = 'block';
	$('ad_logout').style.display = 'block';
	$('ad_login').style.display = 'none';

	if( false == g_admin ) return;

	$('admin_button').style.display = 'block';
	$('sidepanel_permissions').style.display = 'block';

	if( localStorage.permissions_opened == "true" ) ad_PermissionsOpen();
	else ad_PermissionsClose();

	ad_initialized = true;
}

function ad_Login()
{
	c_Info('Login');
	if( SERVER.AUTH_RULES )
	{
		localStorage.auth_digest = '';
		new cgru_Dialog( window, window, 'ad_LoginGetPassword', 'user_id', 'str', '', 'login', 'Login', 'Enter User ID');
	}
	else
		ad_loginProcess({"realm":'RULES'});
}
function ad_loginProcess( i_obj)
{
	var data = c_Parse( n_Request({"login":i_obj}));
	if( data == null ) return;
	if( data.error )
	{
		if( SERVER.AUTH_RULES )
		{
			localStorage.auth_user = '';
			localStorage.auth_digest = '';
		}
		c_Error( data.error);
		return;
	}
	g_GO('/');
	window.location.reload();
}
function ad_LoginGetPassword( i_notused, i_user_id)
{
	new cgru_Dialog( window, window, 'ad_LoginConstruct', i_user_id, 'str', '', 'login', 'Login', 'Enter Password');
}
function ad_LoginConstruct( i_user_id, i_password)
{
	var digest = ad_ConstructDigest( i_user_id, i_password);
	ad_loginProcess({"digest":digest});
}
function ad_ConstructDigest( i_user_id, i_password)
{
	var obj = {};
	obj.nc = 1;
	obj.uri = 'cgru';
	obj.qop = 'auth';
	obj.cnonce = Math.random().toString(36).substring(2);
	obj.nonce = SERVER.nonce;

	if( i_user_id == null )
	{
		if( ( localStorage.auth_user          == null ) ||
			( localStorage.auth_user.length   == 0    ) ||
			( localStorage.auth_digest        == null ) ||
			( localStorage.auth_digest.length == 0    ))
			return null;
		i_user_id = localStorage.auth_user;
	}
	else
	{
		localStorage.auth_user = i_user_id;
		localStorage.auth_digest = c_MD5( i_user_id+':RULES:'+i_password);
console.log('Digest: '+localStorage.auth_digest);
	}

	obj.response = c_MD5( localStorage.auth_digest+':'+SERVER.nonce+':'+obj.nc+':'+obj.cnonce+':'+obj.qop+':'+c_MD5('POST:'+obj.uri));
	obj.username = i_user_id;

	return obj;
}

function ad_Logout()
{
	c_Info('Logout');

	if( SERVER.AUTH_RULES )
	{
		localStorage.auth_user = '';
		localStorage.auth_digest = '';
		g_GO('/');
		window.location.reload();
		return;
	}
//*
 	var obj = {};
	obj.logout = {"realm":'RULES'};
	var data = n_Request( obj);
//c_Log( data);
//	*/
///*
	var xhr = new XMLHttpRequest;
	xhr.open('GET', '/', true, 'null', 'null');
	xhr.send('');
	xhr.onreadystatechange = function() {
		if (xhr.readyState == 4) { g_GO('/'); window.location.reload(); }
	}
//*/
//c_Log( data);
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
	if( $('sidepanel').classList.contains('opened'))
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
	ad_wnd.closeOnEsc = false;

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

function ad_WndDrawGroups()
{
	ad_wnd.elGroups.innerHTML = '';
	ad_wnd.elGrpBnts = [];
	for( var grp in g_groups )
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

function ad_WndDrawUsers()
{
	ad_wnd.elUsers.innerHTML = '';
	ad_wnd.elUsrRows = [];

	var labels = {};
	labels.id = 'ID';
	labels.title = 'Title';
	labels.role = 'Role';
	labels.email = 'Email';
	labels.channels = {}; labels.channels.length = 'Cnls';
	labels.news = {}; labels.news.length = 'News';

	var users = [];
	for( var user in g_users ) users.push( g_users[user]);
		users.sort( function( a, b ) {
			var val_a = a[ad_wnd_sort_prop];
			var val_b = b[ad_wnd_sort_prop];

			var arrays = ['news','channels'];
			if( arrays.indexOf( ad_wnd_sort_prop) != -1 )
			{
				val_a = val_a.length;
				val_b = val_b.length;
			}

			if( val_a == null ) val_a = '';
			if( val_b == null ) val_b = '';

			if(( val_a > val_b ) == ad_wnd_sort_dir ) return -1;
			if(( val_a < val_b ) == ad_wnd_sort_dir ) return  1;

			return 0;
		});
	
	var row = 0;
	ad_WndAddUser( ad_wnd.elUsers, labels, row++);
	for( var i = 0; i < users.length; i++ )
		ad_WndAddUser( ad_wnd.elUsers, users[i], row++);
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
		ad_wnd.elContent.innerHTML = '"'+i_type+'" are NULL.';
		return null;
	}
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
	g_groups = ad_GetAll('groups');
	if( g_groups == null ) return;

	g_users = ad_GetAll('users');
	if( g_users == null ) return;

	for( var u in g_users )
	{
//window.console.log(user);
		g_users[u].groups = [];
		for( var grp in g_groups )
			if( g_groups[grp].indexOf( g_users[u].id ) != -1 )
				g_users[u].groups.push( grp);
	}

	ad_WndDrawGroups();
	ad_WndDrawUsers();
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
	else
	{
		el.style.backgroundColor = 'rgba(0,0,0,.2)';
		el.style.cursor = 'pointer';
	}

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
	if( i_row == 0 ) elName.onclick = function(e) { ad_WndSortUsers('id'); };

	var elTitle = document.createElement('div');
	el.appendChild( elTitle);
	elTitle.style.width = '200px';
	elTitle.textContent = i_user.title;
	elTitle.m_user_id = i_user.id;
	elTitle.title = 'Double click edit title';
	if( i_row ) elTitle.ondblclick = function(e){ad_ChangeTitleOnCkick(e.currentTarget.m_user_id);};
	else elTitle.onclick = function(e) { ad_WndSortUsers('title'); }

	var elRole = document.createElement('div');
	el.appendChild( elRole);
	elRole.style.width = '100px';
	elRole.textContent = i_user.role;
	elRole.m_user_id = i_user.id;
	elRole.title = 'Double click edit role';
	if( i_row ) elRole.ondblclick = function(e){ad_ChangeRoleOnCkick(e.currentTarget.m_user_id);};
	else elRole.onclick = function(e) { ad_WndSortUsers('role'); }

	var elEmail = document.createElement('div');
	el.appendChild( elEmail);
	elEmail.style.width = '200px';
	elEmail.textContent = i_user.email;
	elEmail.m_user_id = i_user.id;
	elEmail.title = 'Double click edit email';
	if( i_row ) elEmail.ondblclick = function(e){ad_ChangeEmailOnCkick(e.currentTarget.m_user_id);};
	else elEmail.onclick = function(e) { ad_WndSortUsers('email'); };

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
	if( i_row == 0 ) elChannels.onclick = function(e) { ad_WndSortUsers('channels'); };

	var elNews = document.createElement('div');
	el.appendChild( elNews);
	elNews.textContent = i_user.news.length;
	elNews.style.width = '50px';
	if( i_row == 0 ) elNews.onclick = function(e) { ad_WndSortUsers('news'); };

	var elCTime = document.createElement('div');
	el.appendChild( elCTime);
	if( i_row ) elCTime.textContent = c_DT_StrFromSec( i_user.ctime).substr(4,11);
	else elCTime.textContent = 'Created';
	elCTime.style.width = '150px';
	if( i_row == 0 ) elCTime.onclick = function(e) { ad_WndSortUsers('ctime'); };

	var elRTime = document.createElement('div');
	el.appendChild( elRTime);
	if( i_row ) elRTime.textContent = c_DT_StrFromSec( i_user.rtime).substr(4);
	else elRTime.textContent = 'Entered';
	elRTime.style.width = '200px';
	if( i_row == 0 ) elRTime.onclick = function(e) { ad_WndSortUsers('rtime'); };
}

function ad_WndSortUsers( i_prop)
{
	if( ad_wnd_sort_prop == i_prop )
		ad_wnd_sort_dir = 1 - ad_wnd_sort_dir;
	else
		ad_wnd_sort_prop = i_prop;
	ad_WndDrawUsers();
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
function ad_ChangeTitleOnCkick( i_user_id) { new cgru_Dialog( window, window, 'ad_ChangeTitle', i_user_id, 'str', g_users[i_user_id].title, 'users', 'Change Title', 'Enter New Title');}
function ad_ChangeTitle( i_user_id, i_title) { ad_SaveUserProp( i_user_id, 'title', i_title); ad_WndRefresh();}

function ad_ChangeRoleOnCkick( i_user_id) { new cgru_Dialog( window, window, 'ad_ChangeRole', i_user_id, 'str', g_users[i_user_id].role, 'users', 'Change Role', 'Enter New Role'); }
function ad_ChangeRole( i_user_id, i_role) { ad_SaveUserProp( i_user_id, 'role', i_role); ad_WndRefresh();}

function ad_ChangeEmailOnCkick( i_user_id) { new cgru_Dialog( window, window, 'ad_ChangeEmail', i_user_id, 'str', g_users[i_user_id].email, 'users', 'Change Email', 'Enter New Address');}
function ad_ChangeEmail( i_user_id, i_email) { ad_SaveUserProp( i_user_id, 'email', i_email); ad_WndRefresh();}

function ad_SaveUserProp( i_user_id, i_prop, i_value)
{
	var obj = {};

	obj.add = true;
	obj.object = {};
	obj.object[i_prop] = i_value;
	obj.file = 'users/' + i_user_id + '.json';

	var res = c_Parse( n_Request({"editobj":obj}));
	if( res && res.error )
	{
		c_Error( res.error );
		return;
	}
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
	var pw = '';
	for( var i = 0; i < 10; i ++) pw += Math.random().toString(36).substring(2);
	pw = btoa( pw).substr( 0, 60);
	new cgru_Dialog( window, window, 'ad_SetPassword', i_user_id, 'str', pw, 'password',
		'Set Password', 'Enter New Password');
}
function ad_SetPassword( i_user_id, i_passwd)
{
	var email = g_users[i_user_id].email;
	if( email && email.length )
	{
		var subject = 'RULES Password';
		var body = window.location.protocol + '//' + window.location.host + window.location.pathname;
		body = '<a href="'+body+'" target="_blank">'+body+'</a>';
		body += ' password for ';
		if( g_users[i_user_id].title )
			body += g_users[i_user_id].title+' ['+i_user_id+']';
		else
			body += i_user_id;
		body += ' is set to:';
		body += '<br><br>';
		body += i_passwd;
		body += '<br><br>';
		body += 'Login name: '+i_user_id;

		n_SendMail( email, subject, body);
	}

	var digest = c_MD5( i_user_id + ':RULES:' + i_passwd);
	digest = i_user_id + ':RULES:' + digest;
	var result = c_Parse( n_Request({"htdigest":{"user":i_user_id,"digest":digest}}, true, true));
	if( result == null ) return;
	if( result.error )
	{
		c_Error( result.error);
		return;
	}

	if( result.status ) c_Info( result.status);
}


ad_prof_props = [];
ad_prof_props.push({"name":'id',     "label":'ID',     "edit":false});
ad_prof_props.push({"name":'title',  "label":'Title',  "edit":false});
ad_prof_props.push({"name":'role',   "label":'Role',   "edit":false});
ad_prof_props.push({"name":'avatar', "label":'Avatar', "edit":true});
ad_prof_props.push({"name":'email',  "label":'Email',  "edit":true});

function ad_ProfileOpen()
{
	if( g_auth_user == null )
	{
		c_Error('No authenticated user founded.');
		return;
	}

	var wnd = new cgru_Window('My Profile');
	wnd.elContent.classList.add('profile');
	wnd.closeOnEsc = false;
	ad_wnd_prof = wnd;
	ad_ProfileDraw();
}
function ad_ProfileDraw()
{
	var wnd = ad_wnd_prof;
	wnd.elContent.innerHTML = '';

	var avatar = c_GetAvatar();
	if( avatar )
	{
		var el = document.createElement('img');
		wnd.elContent.appendChild( el);
		el.classList.add('avatar');
		el.src = avatar;
		el.width = 100;
		el.height = 100;
	}

	for( var i = 0; i < ad_prof_props.length; i++ )
	{
		var prop = ad_prof_props[i];

		var el = document.createElement('div');
		wnd.elContent.appendChild( el);

		var elLabel = document.createElement('div');
		el.appendChild( elLabel);
		elLabel.textContent = prop.label;
		elLabel.classList.add('label');

		if( prop.edit )
		{
			var elEdit = document.createElement('div');
			el.appendChild( elEdit);
			elEdit.textContent = 'Edit';
			elEdit.classList.add('button');
			elEdit.m_prop = prop;
			elEdit.onclick = ad_ProfileEditPropOnClick;
		}

		var elProp = document.createElement('div');
		el.appendChild( elProp);
		elProp.textContent = g_auth_user[prop.name];
		elProp.classList.add('prop');
	}

	return;
var el = document.createElement('div');
wnd.elContent.appendChild( el);
el.textContent = JSON.stringify( g_auth_user);
}

function ad_ProfileEditPropOnClick( i_evt)
{
	var prop = i_evt.currentTarget.m_prop;
	new cgru_Dialog( window, window, 'ad_ProfileEditProp', prop.name, 'str', g_auth_user[prop.name],
		'users', 'Change '+prop.label, 'Enter New Value');
}
function ad_ProfileEditProp( i_prop, i_value)
{
	g_auth_user[i_prop] = i_value;
	ad_SaveUserProp( g_auth_user.id, i_prop, i_value)
	ad_ProfileDraw();
}

