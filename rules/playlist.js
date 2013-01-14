p_elCurFolder = null;
p_elCurItem = null;
p_noFile = true;

function p_Init()
{
	p_elCurFolder = u_el.playlist;
	p_elCurFolder.m_path = '';
	p_elCurFolder.m_id = '';
	if( localStorage.playlist_opened == "true" ) p_Open();
	else p_Close();
	if( localStorage.playlist_opened_folders == null ) localStorage.playlist_opened_folders = '';
}

function p_Close()
{
	u_el['sidepanel_playlist'].classList.remove('opened');
	u_el['playlist'].innerHTML = '';
	localStorage.playlist_opened = false;
}
function p_Open()
{
	u_el['sidepanel_playlist'].classList.add('opened');
	localStorage.playlist_opened = true;
	p_Load();
}

function p_OnClick()
{
	if( u_el['sidepanel'].classList.contains('opened'))
	{
		if( u_el['sidepanel_playlist'].classList.contains('opened'))
			p_Close();
		else
			p_Open();
	}
	else
	{
		u_SidePanelOpen();
		p_Open();
	}
}

function p_NewOnClick()
{
	new cgru_Dialog( window, window, 'p_NewFolder', null, 'str', localStorage.user_name, 'playlist',
		'New Playlist Folder', 'Enter Folder Name');
}
function p_AddOnClick()
{
	var name = g_elCurFolder.m_path.split('/');
	name = name[name.length-1];
	new cgru_Dialog( window, window, 'p_AddLink', null, 'str', name, 'playlist',
		'New Playlist Link', 'Enter Link Name');
}
function p_RenameOnClick()
{
	if( p_elCurItem == null ) return;
	new cgru_Dialog( window, window, 'p_Rename', null, 'str', p_elCurItem.m_label, 'playlist',
		'Rename Playlist Item', 'Enter New Name');
}

function p_NewFolder( i_param, i_value)
{
	var obj = {};
	obj.label = i_value;
	obj.id = p_elCurFolder.m_path + '/' + i_value;
	obj.playlist = [];
	p_Action( obj, 'add');
}
function p_AddLink( i_param, i_value)
{
	var obj = {};
	obj.label = i_value;
	obj.path = g_elCurFolder.m_path;
	obj.id = p_elCurFolder.m_path + '/' + i_value;
	p_Action( obj, 'add');
}
function p_Rename( i_param, i_value)
{
	var obj = {};
	obj.label = i_value;
	obj.id = p_elCurItem.m_id;
	p_Action( obj, 'rename');
}

function p_FolderOnClick( i_evt)
{
	i_evt.stopPropagation();
	var el = i_evt.currentTarget;
//window.console.log('fclick: '+el.m_id)
	p_elCurFolder = el;
	p_SetCurItem( p_elCurFolder);
}
function p_FolderOnDblClick( i_evt)
{
	i_evt.stopPropagation();
	var el = i_evt.currentTarget;
window.console.log('dbl fclick: '+el.m_id)
//if( el == p_elCurItem )
//{
	if( el.classList.contains('opened'))
	{
		el.classList.remove('opened');
		p_elCurFolder = el.parentNode;
		var folders = localStorage.playlist_opened_folders.split(' ');
		folders.splice( folders.indexOf( el.m_id), 1);
		localStorage.playlist_opened_folders = folders.join(' ');
	}
	else
	{
		el.classList.add('opened');
		p_elCurFolder = el;
		var folders = localStorage.playlist_opened_folders.split(' ');
		folders.push( el.m_id);
		localStorage.playlist_opened_folders = folders.join(' ');
	}
//}
	p_SetCurItem( p_elCurFolder);
}
function p_LinkOnClick( i_evt)
{
	i_evt.stopPropagation();
	p_SetCurItem( i_evt.currentTarget);
	p_elCurFolder = p_elCurItem.parentNode;
	g_GO( p_elCurItem.m_path );
//g_elCurFolder.scrollIntoView(false);
}
function p_DelOnClick()
{
	if( p_elCurItem == null ) return;
	p_Action({"id":p_elCurItem.m_id}, 'del' );
}
function p_SetCurItem( i_el)
{
	if( i_el.classList.contains('current'))
		return;
	if( p_elCurItem )
		p_elCurItem.classList.remove('current');
	p_elCurItem = i_el;
	p_elCurItem.classList.add('current');
}

function p_Action( i_obj, i_action)
{
	var obj = {};
	if( i_obj.id )
		i_obj.id = i_obj.id.replace(/ /g,'_');

	if( i_action == 'add' )
	{
		if( p_noFile )
		{
			obj.object = {"id":"","playlist":[i_obj]};
			obj.add = true;
		}
		else
		{	
			obj.objects = [i_obj];
			obj.pusharray = 'playlist';
			obj.id = p_elCurFolder.m_id;
		}
	}
	else if( i_action == 'del')
	{
		obj.objects = [i_obj];
		obj.delobj = true;
	}
	else if( i_action == 'rename')
	{
		obj.objects = [i_obj];
		obj.replace = true;
	}
	else
	{
		c_Error('Playlist: Unknown action = '+i_action+'<br> Object: '+JSON.stringify(i_obj));
		return;
	}
	obj.file = 'playlist.json';
	n_Request({"editobj":obj});
	p_Load();
}

function p_RefreshOnClick( i_evt) { p_Load();}
function p_Load()
{
	var obj = c_Parse( n_Request({"readobj":"playlist.json"}));
	if( obj == null ) return;
	if( obj.error == null )
		p_noFile = false;

	var params = {};
	params.wasopened = localStorage.playlist_opened_folders.split(' ');
	params.opened = [];
	params.curfolderid = p_elCurFolder ? p_elCurFolder.m_id : null;
	params.curitemid = p_elCurItem ? p_elCurItem.m_id : null;

	p_elCurFolder = u_el.playlist;
	p_elCurItem = null;

	u_el.playlist.innerHTML = '';

	p_Read( obj.playlist, params, u_el.playlist);

	localStorage.playlist_opened_folders = params.opened.join(' ');
}
function p_Read( i_playlist, i_params, i_elParent)
{
	if( i_playlist == null ) return;
	if( i_playlist.length == null ) return;
	i_playlist.sort( p_CompareItems );
	for( var i = 0; i < i_playlist.length; i++)
	{
		var el = null;
		if( i_playlist[i].playlist )
		{
			var el = p_CreateFolder( i_playlist[i], i_elParent);
			if( el.m_id == i_params.curfolderid )
				p_elCurFolder = el;
			if( i_params.wasopened.indexOf( el.m_id ) != -1 )
			{
				el.classList.add('opened');
				i_params.opened.push( el.m_id);
			}
			if( i_playlist[i].playlist.length )
				p_Read( i_playlist[i].playlist, i_params, el);
		}
		else
			el = p_CreateLink( i_playlist[i], i_elParent);

		if( el && ( el.m_id == i_params.curitemid ))
			p_SetCurItem( el);
	}
}
function p_CompareItems(a,b)
{
	var attr = 'label';
	if( a[attr] < b[attr]) return -1;
	if( a[attr] > b[attr]) return 1;
	return 0;
}
function p_CreateFolder( i_obj, i_elParent)
{
	var el = p_CreateElement( i_obj, i_elParent);
	el.classList.add('folder');
	el.onclick = p_FolderOnClick;
	el.ondblclick = p_FolderOnDblClick;
	return el;
}
function p_CreateLink( i_obj, i_elParent)
{
	var el = p_CreateElement( i_obj, i_elParent);
	el.classList.add('link');
	el.onclick = p_LinkOnClick;
	return el;
}
function p_CreateElement( i_obj, i_elParent)
{
	var el = document.createElement('div');
	i_elParent.appendChild( el);
	el.textContent = i_obj.label;
	el.m_label = i_obj.label;
	el.m_path = i_obj.path;
	el.m_id = i_obj.id;
	return el;
}

