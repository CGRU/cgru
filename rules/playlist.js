p_elCurFolder = null;
p_elCurLink = null;
p_noFile = true;

function p_Init()
{
	p_elCurFolder = u_el.playlist;
	p_elCurFolder.m_path = '';
	if( localStorage.playlist_opened == "true" ) p_PlayListOpen();
	else p_PlayListClose();
	if( localStorage.playlist_opened_folders == null ) localStorage.playlist_opened_folders = '';
}

function p_PlayListClose()
{
	u_el['sidepanel_playlist'].classList.remove('opened');
	u_el['playlist'].innerHTML = '';
	localStorage.playlist_opened = false;
}
function p_PlayListOpen()
{
	u_el['sidepanel_playlist'].classList.add('opened');
	localStorage.playlist_opened = true;
	p_PlayListLoad();
}

function p_PlayListOnClick()
{
	if( u_el['sidepanel'].classList.contains('opened'))
	{
		if( u_el['sidepanel_playlist'].classList.contains('opened'))
			p_PlayListClose();
		else
			p_PlayListOpen();
	}
	else
	{
		u_SidePanelOpen();
		p_PlayListOpen();
	}
}

function p_PlayListNewOnClick()
{
	new cgru_Dialog( window, window, 'p_PlayListNewFolder', null, 'str', localStorage.user_name, 'playlist',
		'New Playlist Folder', 'Enter Folder Name');
}
function p_PlayListAddOnClick()
{
	var name = g_elCurFolder.m_path.split('/');
	name = name[name.length-1];
	new cgru_Dialog( window, window, 'p_PlayListAddLink', null, 'str', name, 'playlist',
		'New Playlist Link', 'Enter Link Name');
}

function p_PlayListNewFolder( i_param, i_value)
{
	var obj = {};
	obj.label = i_value;
	obj.id = p_elCurFolder.m_path + '/' + i_value;
	obj.playlist = [];
	var el = p_elCurFolder;
	p_PlayListAction( obj, true);
}
function p_PlayListAddLink( i_param, i_value)
{
	var obj = {};
	obj.label = i_value;
	obj.path = g_elCurFolder.m_path;
	obj.id = p_elCurFolder.m_path + '/' + i_value;
	p_PlayListAction( obj, true);
}

function p_PlayListFolderOnClick( i_evt)
{
	i_evt.stopPropagation();
	var el = i_evt.currentTarget;
	if( el.classList.contains('opened'))
	{
		el.classList.remove('opened');
		p_elCurFolder = el.parentNode;
	}
	else
	{
		el.classList.add('opened');
		p_elCurFolder = el;
	}
}
function p_PlayListLinkOnClick( i_evt)
{
	i_evt.stopPropagation();
	if( p_elCurLink )
		p_elCurLink.classList.remove('current');
	p_elCurLink = i_evt.currentTarget;
	p_elCurLink.classList.add('current');
	g_GO( p_elCurLink.m_path );
}

function p_PlayListDelOnClick()
{
	if( p_elCurFolder == u_el.playlist ) return;
	p_PlayListAction({"id":p_elCurFolder.m_path}, false );
}

function p_PlayListAction( i_obj, i_add)
{
	var obj = {};
	if( i_add == true )
	{
		if( p_noFile )
		{
			obj.object = {"id":"","playlist":[i_obj]};
			obj.add = true;
		}
		else
		{	
			obj.objects = [i_obj];
			obj.pusharray = "playlist";
			obj.id = p_elCurFolder.m_path;
		}
	}
	else
	{
		obj.objects = [i_obj];
		obj.delobj = true;
	}
	obj.file = 'playlist.json';
	n_Request({"editobj":obj});
	p_PlayListLoad();
}

function p_PlayListLoad()
{
	u_el.playlist.innerHTML = '';
	p_elCurFolder = u_el.playlist;
	var obj = c_Parse( n_Request({"readobj":"playlist.json"}));
	if( obj == null ) return;
	if( obj.error == null )
		p_noFile = false;
	p_PlayListRead( obj.playlist);
}
function p_PlayListRead( i_playlist)
{
	if( i_playlist == null ) return;
	if( i_playlist.length == null ) return;
	for( var i = 0; i < i_playlist.length; i++)
	{
		if( i_playlist[i].playlist )
		{
			p_elCurFolder = p_PlayListCreateFolder( i_playlist[i].label);
			if( i_playlist[i].playlist.length )
				p_PlayListRead( i_playlist[i].playlist);
			p_elCurFolder = p_elCurFolder.parentNode;
		}
		else
			p_PlayListCreateLink( i_playlist[i]);
	}
}
function p_PlayListCreateFolder( i_label)
{
	var el = document.createElement('div');
	p_elCurFolder.appendChild( el);
	el.classList.add('folder');
el.classList.add('opened');
	el.textContent = i_label;
	el.onclick = p_PlayListFolderOnClick;
	el.m_path = p_elCurFolder.m_path + '/' + i_label;
	el.m_id = el.m_path;
	return el;
}
function p_PlayListCreateLink( i_obj)
{
	var el = document.createElement('div');
	p_elCurFolder.appendChild( el);
	el.classList.add('link');
	el.textContent = i_obj.label;
	el.onclick = p_PlayListLinkOnClick;
	el.m_path = i_obj.path;
	el.m_id = i_obj.id;
}

