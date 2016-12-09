ad_initialized = false;
ad_permissions = null;
ad_wnd = null;
ad_wnd_curgroup = null;
ad_wnd_curstate = null;
ad_wnd_sort_prop = 'role';
ad_wnd_sort_dir = 0;

ad_states = {};
ad_states.notart    = {"short":'NA',"label":'NotArt',"tooltip":'Not an artist.'};
ad_states.passwd    = {"short":'PS',"label":'Passwd',"tooltip":'Can change password.'};
ad_states.playlist  = {"short":'PL',"label":'Playlist',"tooltip":'Can edit playlist.'};
ad_states.assignart = {"short":'AA',"label":'AssignArt',"tooltip":'Can assign artists.'};
ad_states.edittasks = {"short":'TS',"label":'EditTasks',"tooltip":'Can edit tasks.'};
ad_states.editbody  = {"short":'BD',"label":'EditBody',"tooltip":'Can edit body.'};

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
		new cgru_Dialog({"handle":'ad_LoginGetPassword',"param":'user_id',
			"name":'login',"title":'Login',"info":'Enter User ID'});
	}
	else
		ad_LoginProcess({"realm":cgru_Config.realm});
}
function ad_LoginProcess( i_obj)
{
	n_Request({"send":{"login":i_obj},"func":ad_LoginReceived,"info":'login'});
}
function ad_LoginReceived( i_data)
{
	if( i_data == null ) return;
	if( i_data.error )
	{
		if( SERVER.AUTH_RULES )
		{
			localStorage.auth_user = '';
			localStorage.auth_digest = '';
		}
		c_Error( i_data.error);
		return;
	}
	g_GO('/');
	window.location.reload();
}
function ad_LoginGetPassword( i_user_id)
{
	new cgru_Dialog({"handle":'ad_LoginConstruct',"param":i_user_id,
		"name":'login',"title":'Login',"info":'Enter Password'});
}
function ad_LoginConstruct( i_password, i_user_id)
{
	var digest = ad_ConstructDigest( i_password, i_user_id);
	ad_LoginProcess({"digest":digest});
}
function ad_ConstructDigest( i_password, i_user_id)
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
		localStorage.auth_digest = c_MD5( i_user_id+':'+cgru_Config.realm+':'+i_password);
		//console.log('Digest: '+localStorage.auth_digest);
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
/*
 	var obj = {};
	obj.logout = {"realm":cgru_Config.realm};
	n_Request({"send":obj,"func":ad_LogoutReceived,"info":'logout'});
}
function ad_LogoutReceived( i_data)
{
console.log( JSON.stringify( i_data));
*/
///*
	var xhr = new XMLHttpRequest;
	xhr.open('GET', '/', true, 'null', 'null');
	xhr.send('');
	xhr.onreadystatechange = function()
	{
		if (xhr.readyState == 4) { g_GO('/'); window.location.reload(); }
	}
//*/
//c_Log( i_data);
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
	n_Request({"send":{"permissionsget": ad_permissions},"func":ad_PermissionsReceived});
}
function ad_PermissionsReceived( i_data)
{
	ad_permissions = i_data;

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

	ad_permissions.path = RULES.root + g_CurPath();

	if( ad_permissions.groups.length )
	{
		var el = document.createElement('div');
		$('permissions').appendChild( el);
		el.textContent = 'Groups:';
		el.classList.add('caption');
	}
	for( var i = 0; i < ad_permissions.groups.length; i++)
	{
		var group = ad_permissions.groups[i];
		var el = document.createElement('div');
		$('permissions').appendChild( el);
		el.classList.add('item');

		if( group != 'admins' )
		{
			var elBtn = document.createElement('div');
			el.appendChild( elBtn);
			elBtn.classList.add('button');
			elBtn.classList.add('delete');
			elBtn.m_group_id = group;
			elBtn.ondblclick = function(e){ad_PermissionsRemove('groups', e.currentTarget.m_group_id)};
		}

		var elName = document.createElement('div');
		el.appendChild( elName);
		elName.classList.add('name');
		elName.textContent = group;
	}

	if( ad_permissions.users.length )
	{
		var el = document.createElement('div');
		$('permissions').appendChild( el);
		el.textContent = 'Users:';
		el.classList.add('caption');
	}
	for( var i = 0; i < ad_permissions.users.length; i++)
	{
		var user = ad_permissions.users[i];
		var el = document.createElement('div');
		$('permissions').appendChild( el);
		el.classList.add('item');

		var elBtn = document.createElement('div');
		el.appendChild( elBtn);
		elBtn.classList.add('button');
		elBtn.classList.add('delete');
		elBtn.m_user_id = user;
		elBtn.ondblclick = function(e){ad_PermissionsRemove('users', e.currentTarget.m_user_id)};

		var elName = document.createElement('div');
		el.appendChild( elName);
		elName.classList.add('name');
		elName.textContent = user;
	}
}

function ad_PermissionsGrpAddOnClick()
{
	new cgru_Dialog({"handle":'ad_PermissionsAdd',"param":'groups',"value":'admins',
		"name":'permissions',"title":'Add Group',"info":'Enter Group ID'});
}
function ad_PermissionsUsrAddOnClick()
{
	new cgru_Dialog({"handle":'ad_PermissionsAdd',"param":'users',
		"name":'permssions',"title":'Add User',"info":'Enter User ID'});
}
function ad_PermissionsAdd( i_id, i_type)
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
	n_Request({"send":{"permissionsset":ad_permissions},"func":ad_ChangesFinished,"ad_func":ad_PermissionsLoad,"info":'perm add',"ad_msg":'Permissions '+i_type+' added'});
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
	n_Request({"send":{"permissionsset":ad_permissions},"func":ad_ChangesFinished,"ad_func":ad_PermissionsLoad,"info":'perm rem',"ad_msg":'Permissions '+i_type+' removed'});
}

function ad_PermissionsClearOnClick()
{
	n_Request({"send":{"permissionsclear":{"path":RULES.root+g_CurPath()}},"func":ad_ChangesFinished,"ad_func":ad_PermissionsLoad,"info":'perm clear',"ad_msg":'Permissions cleared'});
}
function ad_ChangesFinished( i_data, i_args)
{
	if(( i_data == null ) || ( i_data.error ))
		c_Error( i_data.error);
	else if( i_args.ad_msg )
		c_Info( i_args.ad_msg );

	if( i_args.ad_func )
		i_args.ad_func();
}
function ad_OpenWindow()
{
	u_OpenCloseHeader();
	ad_wnd = new cgru_Window({"name":'administrate',"title":'Administrate','padding':'3% 1%'});
	ad_wnd.elContent.classList.add('administrate');
	ad_wnd.closeOnEsc = false;

	var elBtnsDiv = document.createElement('div');
	ad_wnd.elContent.appendChild( elBtnsDiv );

	var el = document.createElement('div');
	elBtnsDiv.appendChild( el);
	el.classList.add('button');
	el.textContent = 'Refresh';
	el.onclick = ad_WndRefresh;
	el.style.cssFloat = 'right';

	var el = document.createElement('div');
	elBtnsDiv.appendChild( el);
	el.classList.add('button');
	el.textContent = 'Create Group';
	el.onclick = ad_CreateGrpOnClick;

	var el = document.createElement('div');
	elBtnsDiv.appendChild( el);
	el.classList.add('button');
	el.textContent = 'Delete Group';
	el.onclick = ad_DeleteGrpOnClick;

	var el = document.createElement('div');
	elBtnsDiv.appendChild( el);
	el.classList.add('button');
	el.textContent = 'Create User';
	el.onclick = ad_CreateUserOnClick;

	var el = document.createElement('div');
	elBtnsDiv.appendChild( el);
	el.classList.add('button');
	el.textContent = 'Enable User';
	el.onclick = ad_EnableUserOnClick;

	var el = document.createElement('div');
	elBtnsDiv.appendChild( el);
	el.classList.add('button');
	el.textContent = 'Disable User';
	el.onclick = ad_DisableUserOnClick;

	var el = document.createElement('div');
	elBtnsDiv.appendChild( el);
	el.classList.add('button');
	el.textContent = 'Delete User';
	el.onclick = ad_DeleteUserOnClick;

	ad_wnd.elInfo = document.createElement('div');
	ad_wnd.elContent.appendChild( ad_wnd.elInfo);

	ad_wnd.elGrpBtns = [];

	ad_wnd.elGroups = document.createElement('div');
	ad_wnd.elContent.appendChild( ad_wnd.elGroups);
	ad_wnd.elGroups.classList.add('admin_groups');

	ad_wnd.elStates = document.createElement('div');
	ad_wnd.elContent.appendChild( ad_wnd.elStates);
	ad_wnd.elStates.classList.add('admin_states');
	for( var st in ad_states )
	{
		var el = document.createElement('div');
		ad_wnd.elStates.appendChild( el);
		ad_wnd.elGrpBtns.push( el);
		el.textContent = ad_states[st].label;
		el.title = ad_states[st].tooltip;
		el.m_state = st;
		el.onclick = function(e){ad_WndStateSelect(e.currentTarget);}
		if( st == ad_wnd_curstate )
			el.classList.add('selected');
	}

	ad_wnd.elUsers = document.createElement('div');
	ad_wnd.elContent.appendChild( ad_wnd.elUsers);
	ad_wnd.elUsers.classList.add('admin_users');

	ad_WndRefresh();
}

function ad_WndStateSelect( i_el)
{
	var select = true;
	if( i_el.classList.contains('selected'))
		select = false;

	for( var i = 0; i < ad_wnd.elGrpBtns.length; i++)
		ad_wnd.elGrpBtns[i].classList.remove('selected');

	if( select )
	{
		i_el.classList.add('selected');
		ad_wnd_curstate = i_el.m_state;
	}
	else
		ad_wnd_curstate = null;

	ad_wnd_curgroup = null;

	for( var i = 0; i < ad_wnd.elUsrRows.length; i++)
	{
		var elU = ad_wnd.elUsrRows[i];
		if( elU.m_user.states && ( elU.m_user.states.indexOf( ad_wnd_curstate) != -1 ))
			elU.classList.add('selected');
		else
			elU.classList.remove('selected');
//console.log( elU.m_user.id+' '+elU.m_user.groups+' '+ el.m_group);
	}
}

function ad_WndDrawGroups()
{
	ad_wnd.elGroups.innerHTML = '';
	for( var grp in g_groups )
	{
		var el = document.createElement('div');
		ad_wnd.elGroups.appendChild( el);
		ad_wnd.elGrpBtns.push( el);
		el.textContent = grp + ' [' + g_groups[grp].length + ']';
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

	var users = [];
	for( var user in g_users ) users.push( g_users[user]);
		users.sort( function( a, b ) {
			var val_a = a[ad_wnd_sort_prop];
			var val_b = b[ad_wnd_sort_prop];
/*
			var arrays = ['news','channels'];
			if( arrays.indexOf( ad_wnd_sort_prop) != -1 )
			{
				if( val_a ) val_a = val_a.length;
				if( val_b ) val_b = val_b.length;
			}
*/
			if( val_a == null ) val_a = '';
			if( val_b == null ) val_b = '';

			if(( val_a > val_b ) == ad_wnd_sort_dir ) return -1;
			if(( val_a < val_b ) == ad_wnd_sort_dir ) return  1;

			if( ad_wnd_sort_prop != 'role' )
			{
				if(( a.role > b.role) == ad_wnd_sort_dir ) return -1;
				if(( a.role < b.role) == ad_wnd_sort_dir ) return  1;
			}

			if(( a.disabled && ( ! b.disabled )) == ad_wnd_sort_dir ) return -1;
			if(( b.disabled && ( ! a.disabled )) == ad_wnd_sort_dir ) return  1;

			if(( a.title > b.title ) == ad_wnd_sort_dir ) return -1;
			if(( a.title < b.title ) == ad_wnd_sort_dir ) return  1;

			return 0;
		});
	
	var elTable = document.createElement('table');
	ad_wnd.elUsers.appendChild( elTable);

	var elTr = document.createElement('tr');
	elTable.appendChild( elTr);
	elTr.style.backgroundColor = 'rgba(0,0,0,.2)';
	elTr.classList.add('notselectable');
	elTr.style.cursor = 'pointer';

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'G';
	el.title = 'Group button.\nDouble click to add/remove user from selected group.';

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'A';
	el.title = 'User avatar.\nDouble click to edit.';
	el.onclick = function(e) { ad_WndSortUsers('avatar'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'ID';
	el.title = 'User login and unique ID';
	el.onclick = function(e) { ad_WndSortUsers('id'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Title';
	el.title = 'User full name.\nDouble click to edit.';
	el.onclick = function(e) { ad_WndSortUsers('title'); }

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Role';
	el.title = 'Needed for sorting.\nDouble click to edit.';
	el.onclick = function(e) { ad_WndSortUsers('role'); }

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'DOS';
	el.title = 'Dossier record.';
	el.onclick = function(e) { ad_WndSortUsers('dossier'); }

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Tag';
	el.title = 'Default tag.\nDouble click to edit.';
	el.onclick = function(e) { ad_WndSortUsers('tag'); }

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Email';
	el.title = 'User email address.\nDouble click to edit.';
	el.onclick = function(e) { ad_WndSortUsers('email'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.title = 'User password.\nDouble click to set new.';
	el.textContent = 'Pass';

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Bmrs';
	el.title = 'Bookmarks count.\nDouble click clear bookmarks.';
	el.onclick = function(e) { ad_WndSortUsers('bookmarks'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Cnls';
	el.title = 'News subscribed channels.\nDouble click clear channels.';
	el.onclick = function(e) { ad_WndSortUsers('channels'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'News';
	el.title = 'News count.\nDouble click clear news.';
	el.onclick = function(e) { ad_WndSortUsers('news'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Lim';
	el.title = 'News count limit.\nDouble click to set limit.';
	el.onclick = function(e) { ad_WndSortUsers('news_limit'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Created';
	el.title = 'Time when the user wan created';
	el.onclick = function(e) { ad_WndSortUsers('ctime'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Entered';
	el.title = 'Last login(F5) time';
	el.onclick = function(e) { ad_WndSortUsers('rtime'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'Lastest News';
	el.title = 'Last news time';
	el.onclick = function(e) { ad_WndSortUsers('ntime'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'State';
	el.title = 'User states';
	el.onclick = function(e) { ad_WndSortUsers('states'); };

	var el = document.createElement('th');
	elTr.appendChild( el);
	el.textContent = 'S';
	el.title = 'State button.\nDouble click to add/remove user states.';

	for( var i = 0; i < users.length; i++ )
		ad_WndAddUser( elTable, users[i], i);
}

function ad_GetType( i_type, i_func)
{
	var request = {};
	request['getall'+i_type] = true;
	n_Request({"send":request,"func":ad_GetTypeReceived,"ad_get_type":i_type,"ad_func":i_func,"info":i_type});
}
function ad_GetTypeReceived( i_data, i_args)
{
	var type = i_args.ad_get_type;

	if( i_data == null )
	{
		ad_wnd.elContent.innerHTML = 'Error getting '+type+'.';
		return;
	}
	if( i_data.error )
	{
		ad_wnd.elContent.innerHTML = 'Error getting '+type+':<br>'+i_data.error;
		return;
	}
	if( i_data[type] == null )
	{
		ad_wnd.elContent.innerHTML = '"'+type+'" are NULL.';
		return;
	}

	if( type == 'users' )
		g_users = i_data[type];
	else if( type == 'groups' )
		g_groups = i_data[type];

	i_args.ad_func();
}

function ad_WndGrpOnClick( i_evt)
{
	var el = i_evt.currentTarget;
	var select = true;
	if( el.classList.contains('selected'))
		select = false;

	for( var i = 0; i < ad_wnd.elGrpBtns.length; i++)
		ad_wnd.elGrpBtns[i].classList.remove('selected');

	if( select )
	{
		el.classList.add('selected');
		ad_wnd_curgroup = el.m_group;
	}
	else
		ad_wnd_curgroup = null;

	ad_wnd_curstate = null;

	for( var i = 0; i < ad_wnd.elUsrRows.length; i++)
	{
		var elU = ad_wnd.elUsrRows[i];
		if( elU.m_user.groups.indexOf( ad_wnd_curgroup) != -1 )
			elU.classList.add('selected');
		else
			elU.classList.remove('selected');
//console.log( elU.m_user.id+' '+elU.m_user.groups+' '+ el.m_group);
	}
}

function ad_WndRefresh()
{
	ad_GetType('groups', ad_WndReceivedGroups);
}
function ad_WndReceivedGroups()
{
	ad_GetType('users', ad_WndReceivedUsers);
}
function ad_WndReceivedUsers()
{
	for( var u in g_users )
	{
//window.console.log(g_users[u]);
		g_users[u].groups = [];
		for( var grp in g_groups )
			if( g_groups[grp].indexOf( g_users[u].id ) != -1 )
				g_users[u].groups.push( grp);
	}

	if( ad_wnd == null ) return;

	var info = Object.keys(g_users).length + ' Users, ' + Object.keys(g_groups).length + ' Groups';
	var roles = c_GetRolesArtists( g_users);
	info += ', ' + roles.length + ' Roles: ';
	for( var r = 0; r < roles.length; r++)
	{
		if( r ) info += ', ';
		info += roles[r].artists.length + ' ' + roles[r].role;
	}
	info += '.';
	ad_wnd.elInfo.textContent = info;

	ad_WndDrawGroups();
	ad_WndDrawUsers();
}

function ad_WndAddUser( i_el, i_user, i_row)
{
	var elTr = document.createElement('tr');
	i_el.appendChild(elTr);
	ad_wnd.elUsrRows.push( elTr);
	elTr.m_user = i_user;
	elTr.classList.add('user');
	if( i_user.disabled )
		elTr.classList.add('disabled');

	if( i_row % 2 ) elTr.style.backgroundColor = 'rgba(0,0,0,.1)';
	else elTr.style.backgroundColor = 'rgba(255,255,255,.1)';

	if( i_user.groups.indexOf( ad_wnd_curgroup ) != -1 )
		elTr.classList.add('selected');

	if( i_user.states && ( i_user.states.indexOf( ad_wnd_curstate ) != -1 ))
		elTr.classList.add('selected');

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.textContent = 'G';
	el.style.cursor = 'pointer';
	el.m_user = i_user;
	el.ondblclick = function(e){ad_WndUserGroupOnCkick( e.currentTarget.m_user);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.classList.add('avatar');
	var avatar = c_GetAvatar(i_user.id);
	if( avatar )
		el.style.backgroundImage = 'url(' + avatar + ')';
	el.m_user_id = i_user.id;
	el.ondblclick = function(e){ad_ChangeAvatarOnCkick(e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.textContent = i_user.id;

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.textContent = i_user.title;
	el.m_user_id = i_user.id;
	el.ondblclick = function(e){ad_ChangeTitleOnCkick(e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.textContent = i_user.role;
	el.m_user_id = i_user.id;
	el.ondblclick = function(e){ad_ChangeRoleOnCkick(e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	var dossier = '';
	if( i_user.dossier ) dossier = i_user.dossier.split(' ')[0].split('\u00a0')[0];
	el.innerHTML = dossier;
	el.m_user_id = i_user.id;
	el.ondblclick = function(e){ad_ChangeDossierOnCkick(e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.textContent = i_user.tag;
	el.m_user_id = i_user.id;
	el.ondblclick = function(e){ad_ChangeTagOnCkick(e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.textContent = i_user.email;
	el.m_user_id = i_user.id;
	el.ondblclick = function(e){ad_ChangeEmailOnCkick(e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	if( i_user.disabled !== true )
	{
		el.textContent = '***';
		el.m_user_id = i_user.id;
		el.ondblclick = function(e){ad_SetPasswordDialog( e.currentTarget.m_user_id);};
	}

	var el = document.createElement('td');
	elTr.appendChild( el);
	if( i_user.bookmarks )
		el.textContent = i_user.bookmarks.length;
	el.m_user_id = i_user.id;
	if( i_user.disabled !== true )
		el.ondblclick = function(e){ad_UserBookmakrsClean( e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	if( i_user.channels )
		el.textContent = i_user.channels.length;
	if( i_user.channels && i_user.channels.length )
	{
		var channels = '';
		for( var i = 0; i < i_user.channels.length; i++)
		 channels += i_user.channels[i].id+'\n';
		el.title = channels;
	}
	el.m_user_id = i_user.id;
	if( i_user.disabled !== true )
		el.ondblclick = function(e){ad_UserChannelsClean( e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	if( i_user.news )
		el.textContent = i_user.news.length;
	el.m_user_id = i_user.id;
	if( i_user.disabled !== true )
		el.ondblclick = function(e){ad_UserNewsClean( e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	if( i_user.news_limit != null && ( i_user.news_limit > 0 ))
		el.textContent = i_user.news_limit;
	else
		el.innerHTML = '<i>' + RULES.news.limit + '</i>';
	el.m_user_id = i_user.id;
	el.ondblclick = function(e){ad_UserNewsLimitDialog( e.currentTarget.m_user_id);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.textContent = c_DT_StrFromSec( i_user.ctime).substr(4,11);

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.m_user = i_user;
	ad_UserShowEntries( el);
	el.ondblclick = function(e){ad_UserShowEntries( e.currentTarget, true);};

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.textContent = c_DT_StrFromSec( i_user.ntime).substr(4);

	var el = document.createElement('td');
	elTr.appendChild( el);
	var states = '';
	if( i_user.states ) for( var i = 0; i < i_user.states.length; i++)
		states += ' ' + ad_states[i_user.states[i]].short;
	el.textContent = states;

	var el = document.createElement('td');
	elTr.appendChild( el);
	el.textContent = 'S';
	el.style.cursor = 'pointer';
	el.m_user = i_user;
	el.ondblclick = function(e){ad_WndUserStateToggle( e.currentTarget.m_user);};
}
function ad_WndUserStateToggle( i_user)
{
	if( ad_wnd_curstate == null )
	{
		c_Error('No state selected');
		return;
	}

	if( i_user.states == null )
		i_user.states = [];

	if( i_user.states.indexOf( ad_wnd_curstate ) == -1  )
		i_user.states.push( ad_wnd_curstate);
	else
		i_user.states.splice( i_user.states.indexOf( ad_wnd_curstate ), 1);

	ad_SaveUser({"id":i_user.id,"states":i_user.states}, ad_WndRefresh);
}

function ad_WndSortUsers( i_prop)
{
//console.log('ad_WndSortUsers: ' + i_prop);
	if( ad_wnd_sort_prop == i_prop )
		ad_wnd_sort_dir = 1 - ad_wnd_sort_dir;
	else
		ad_wnd_sort_prop = i_prop;
	ad_WndDrawUsers();
}

function ad_CreateGrpOnClick()
{
	new cgru_Dialog({"handle":'ad_CreateGroup',
		"name":'users',"title":'Create Group',"info":'Enter Group Name'});
}
function ad_CreateGroup( i_group)
{
	if( g_groups[i_group] != null )
	{
		c_Error('Group "'+i_group+'" already exists.');
		return;
	}
	g_groups[i_group] = {};
	ad_WriteGroups();
}

function ad_DeleteGrpOnClick()
{
	new cgru_Dialog({"handle":'ad_DeleteGroup',
		"name":'users',"title":'Delete Group',"info":'Enter Group Name'});
}
function ad_DeleteGroup( i_group)
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
	n_Request({"send":{"writegroups":g_groups},"func":ad_ChangesFinished,"ad_func":ad_WndRefresh,"ad_msg":'Grounps written.',"info":'writegroups'});
}
function ad_ChangeAvatarOnCkick( i_user_id)
{
	new cgru_Dialog({"handle":'ad_ChangeAvatar',"param":i_user_id,"value":g_users[i_user_id].avatar,
		"name":'users',"title":'Change Avatar',"info":'Enter new avatar link for ' + c_GetUserTitle(i_user_id)});
}
function ad_ChangeAvatar( i_avatar, i_user_id)
{
	ad_SaveUser({"id":i_user_id,"avatar":i_avatar}, ad_WndRefresh);
}
function ad_ChangeTitleOnCkick( i_user_id)
{
	new cgru_Dialog({"handle":'ad_ChangeTitle',"param":i_user_id,"value":g_users[i_user_id].title,
		"name":'users',"title":'Change Title',"info":'Enter new title for ' + c_GetUserTitle(i_user_id)});
}
function ad_ChangeTitle( i_title, i_user_id)
{
	ad_SaveUser({"id":i_user_id,"title":i_title}, ad_WndRefresh);
}
function ad_ChangeRoleOnCkick( i_user_id)
{
	new cgru_Dialog({"handle":'ad_ChangeRole',"param":i_user_id,"value":g_users[i_user_id].role,
		"name":'users',"title":'Change Role',"info":'Enter new role for ' + c_GetUserTitle(i_user_id)});
}
function ad_ChangeRole( i_role, i_user_id)
{
	ad_SaveUser({"id":i_user_id,"role":i_role}, ad_WndRefresh);
}
function ad_ChangeDossierOnCkick( i_user_id)
{
	new cgru_Dialog({"handle":'ad_ChangeDossier',"param":i_user_id,"value":g_users[i_user_id].dossier,
		"name":'users',"title":'Edit Dossier',"type":'text',"info":c_GetUserTitle(i_user_id) + ' dossier:'});
}
function ad_ChangeDossier( i_dossier, i_user_id)
{
	ad_SaveUser({"id":i_user_id,"dossier":i_dossier}, ad_WndRefresh);
}
function ad_ChangeTagOnCkick( i_user_id)
{
	new cgru_Dialog({"handle":'ad_ChangeTag',"param":i_user_id,"value":g_users[i_user_id].tag,
		"name":'users',"title":'Change Role',"info":'Enter new tag for ' + c_GetUserTitle(i_user_id)});
}
function ad_ChangeTag( i_tag, i_user_id)
{
	if( i_tag.length && ( RULES.tags[i_tag] == null ))
	{
		c_Error('No such tag: "' + i_tag + '"');
		return;
	}
	ad_SaveUser({"id":i_user_id,"tag":i_tag}, ad_WndRefresh);
}
function ad_ChangeEmailOnCkick( i_user_id)
{
	new cgru_Dialog({"handle":'ad_ChangeEmail',"param":i_user_id,"value":g_users[i_user_id].email,
		"name":'users',"title":'Change Email',"info":'Enter new email for ' + c_GetUserTitle(i_user_id)});
}
function ad_ChangeEmail( i_email, i_user_id)
{
	ad_SaveUser({"id":i_user_id,"email":i_email}, ad_WndRefresh);
}
function ad_UserBookmakrsClean( i_user_id)
{
	ad_SaveUser({"id":i_user_id,"bookmarks":[]}, ad_WndRefresh);
}
function ad_UserChannelsClean( i_user_id)
{
	ad_SaveUser({"id":i_user_id,"channels":[]}, ad_WndRefresh);
}
function ad_UserNewsClean( i_user_id)
{
	ad_SaveUser({"id":i_user_id,"news":[]}, ad_WndRefresh);
}
function ad_UserNewsLimitDialog( i_user_id)
{
	new cgru_Dialog({"handle":'ad_UserNewsLimitSet',"param":i_user_id,"value":g_users[i_user_id].news_limit,
		"name":'users',"title":'Change News Limit',"info":'Enter news limit for ' + c_GetUserTitle(i_user_id)});
}
function ad_UserNewsLimitSet( i_limit, i_user_id)
{
	var limit = parseInt( i_limit);
	if( isNaN( limit))
	{
		c_Error('Invalid news limit: ' + i_limit);
		return;
	}
	ad_SaveUser({"id":i_user_id,"news_limit":limit}, ad_WndRefresh);
}
function ad_UserShowEntries( i_el, i_show_ips)
{
	var user = i_el.m_user;

	var text = c_DT_StrFromSec( user.rtime).substr(4);

	if( user.ips && user.ips.length && i_show_ips && ( i_el.m_ips_shown != true ))
	{
		for( var i = 0; i < user.ips.length; i++)
			text += '<br><br>' + user.ips[i].ip  + ' : ' + c_DT_StrFromSec(user.ips[i].time) + '<br>' + user.ips[i].url;

		i_el.m_ips_shown = true;
	}
	else
	{
		i_el.m_ips_shown = false;
	}

	i_el.innerHTML = text;
}
function ad_SaveUser( i_user, i_func)
{
	if( i_user == null ) i_user = g_auth_user;
	
	var obj = {};
	obj.add = true;
	obj.object = i_user;
	obj.file = 'users/' + i_user.id + '.json';

	n_Request({"send":{"editobj":obj},"func":ad_ChangesFinished,"ad_func":i_func,"ad_msg":'User '+i_user.id+' saved'});
}

function ad_CreateUserOnClick()
{
	new cgru_Dialog({"handle":'ad_CreateUser',
		"name":'users',"title":'Create New User',"info":'Enter User Login Name'});
}
function ad_CreateUser( i_user_id)
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

	n_Request({"send":{"editobj":obj},"func":ad_ChangesFinished,"ad_func":ad_WndRefresh,"ad_msg":'User "'+i_user_id+'" created.'});
}

function ad_EnableUserOnClick()
{
	new cgru_Dialog({"handle":'ad_EnableUser',
		"name":'users',"title":'Enable User',"info":'Enter user login name to enable:'});
}
function ad_EnableUser( i_user_id)
{
	if( g_users[i_user_id] == null )
	{
		c_Error('User "' + i_user_id + '" does not exist.');
		return;
	}

	ad_SaveUser({"id":i_user_id,"disabled":false}, ad_WndRefresh);
}

function ad_DisableUserOnClick()
{
	new cgru_Dialog({"handle":'ad_DisableUser',
		"name":'users',"title":'Disable User',"info":'Enter user login name to disable:'});
}
function ad_DisableUser( i_user_id)
{
	if( g_users[i_user_id] == null )
	{
		c_Error('User "' + i_user_id + '" does not exist.');
		return;
	}

	var uobj = g_users[i_user_id];
	uobj.disabled = true;
	delete uobj.news;
	delete uobj.channels;
	delete uobj.bookmarks;

	n_Request({"send":{"disableuser":{"uid":i_user_id,"uobj":uobj}},"func":ad_ChangesFinished,
			"ad_func":ad_WndRefresh,"ad_msg":'User "'+i_user_id+'" disabled.'});
}

function ad_DeleteUserOnClick()
{
	new cgru_Dialog({"handle":'ad_DeleteUser',
		"name":'users',"title":'Delete User',"info":'Enter user login name to <b>delete</b>:'});
}
function ad_DeleteUser( i_user_id)
{
	if( g_users[i_user_id] == null )
	{
		c_Error('User "' + i_user_id + '" does not exist.');
		return;
	}
	n_Request({"send":{"disableuser":{"uid":i_user_id}},"func":ad_ChangesFinished,
		"ad_func":ad_WndRefresh,"ad_msg":'User "'+i_user_id+'" deleted.'});
}

function ad_SetPasswordDialog( i_user_id)
{
	var pw = '';
	for( var i = 0; i < 10; i ++) pw += Math.random().toString(36).substring(2);
	pw = btoa( pw).substr( 0, 60);
	new cgru_Dialog({"handle":'ad_SetPassword',"param":i_user_id,"value":pw,
		"name":'password',"title":'Set Password',"info":'Enter new password for ' + c_GetUserTitle(i_user_id)});
}
function ad_SetPassword( i_passwd, i_user_id)
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

	var digest = c_MD5( i_user_id + ':'+cgru_Config.realm+':' + i_passwd);
	digest = i_user_id + ':'+cgru_Config.realm+':' + digest;

	var obj = {};
	obj.send = {"htdigest":{"user":i_user_id,"digest":digest}};
	obj.encode = SERVER.php_version >= "5.3";
	obj.func = ad_SetPasswordFinished;
	n_Request(obj);
}
function ad_SetPasswordFinished( i_data)
{
	if( i_data.user && g_auth_user && ( i_data.user == g_auth_user.id ))
	{
		// We have just chnged own password and should reload page:
		document.body.textContent = '';
		window.location.reload();
	}

	if( i_data == null )
	{
		c_Error('Unknown error.');
		return;
	}

	if( i_data.status )
		c_Info( i_data.status);

	if( i_data.error )
		c_Error( i_data.error);
}


ad_prof_props = [];
ad_prof_props.id         = {"disabled":true,"lwidth":'170px',"label":'ID'};
ad_prof_props.title      = {"disabled":true,"lwidth":'170px'};
ad_prof_props.role       = {"disabled":true,"lwidth":'170px'};
ad_prof_props.avatar     = {};
ad_prof_props.news_limit = {};
ad_prof_props.email      = {"width":'70%'};
ad_prof_props.email_news = {"width":'30%','type':"bool",'default':false};
ad_prof_props.signature  = {};

function ad_ProfileOpen()
{
	if( g_auth_user == null )
	{
		c_Error('No authenticated user found.');
		return;
	}

	u_OpenCloseHeader();

	var wnd = new cgru_Window({"name":'profile',"title":'My Profile'});
	wnd.elContent.classList.add('profile');

	var avatar = c_GetAvatar();
	if( avatar )
	{
		var el = document.createElement('img');
		wnd.elContent.appendChild( el);
		el.classList.add('avatar');
		el.src = avatar;
	}

	gui_Create( wnd.elContent, ad_prof_props, [g_auth_user]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);
	elBtns.style.clear = 'both';

	var el = document.createElement('div');
	elBtns.appendChild( el);
	el.textContent = 'Save';
	el.classList.add('button');
	el.onclick = function(e){ ad_ProfileSave( e.currentTarget.m_wnd);}
	el.m_wnd = wnd;

	var el = document.createElement('div');
	elBtns.appendChild( el);
	el.textContent = 'Cancel';
	el.classList.add('button');
	el.onclick = function(e){ e.currentTarget.m_wnd.destroy();}
	el.m_wnd = wnd;

//	if( g_auth_user.states.indexOf('passwd') != -1 )
	{
		var el = document.createElement('div');
		elBtns.appendChild( el);
		el.textContent = 'Set Password';
		el.classList.add('button');
		el.onclick = function(e){ ad_SetPasswordDialog( g_auth_user.id);}
		el.m_wnd = wnd;
	}
}

function ad_ProfileSave( i_wnd)
{
	var params = gui_GetParams( i_wnd.elContent, ad_prof_props);

	if( params.news_limit.length == 0 )
		params.news_limit = '-1';

	params.news_limit = parseInt( params.news_limit);
	if( isNaN(params.news_limit))
	{
		c_Error('Invalud news limit number.');
		return;
	}

	for( p in params ) g_auth_user[p] = params[p];
	ad_SaveUser();
	i_wnd.destroy();
}

