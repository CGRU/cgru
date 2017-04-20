p_elCurFolder = null;
p_elCurItem = null;
p_file = 'playlist.json';
p_fileExist = false;

p_elLinks = [];

function p_Init()
{
	var ctrls = $('sidepanel_playlist').getElementsByClassName('playlist_ctrl');
	for( var i = 0; i < ctrls.length; i++)
		if( c_CanEditPlaylist())
			ctrls[i].style.display = 'block';
		else
			ctrls[i].style.display = 'none';

	if( RULES_TOP.playlist ) p_file = RULES_TOP.playlist;
	p_elCurFolder = $('playlist');

	var obj = {};
	obj.id = '';
	p_elCurFolder.m_obj = obj;

	if( localStorage.playlist_opened == null ) localStorage.playlist_opened = 'false';
	if( localStorage.playlist_opened == 'true') p_Open();
	else p_Close();
	if( localStorage.playlist_opened_folders == null ) localStorage.playlist_opened_folders = '';
}

function p_Close()
{
	$('sidepanel_playlist').classList.remove('opened');
	$('playlist').innerHTML = '';
	localStorage.playlist_opened = false;
}
function p_Open()
{
	$('sidepanel_playlist').classList.add('opened');
	localStorage.playlist_opened = true;
	p_Load();
}

function p_OnClick()
{
	if( $('sidepanel').classList.contains('opened'))
	{
		if( $('sidepanel_playlist').classList.contains('opened'))
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
	new cgru_Dialog({"handle":'p_NewFolder',"value":g_auth_user.id,
		"name":'playlist',"title":'New Playlist Folder',"info":'Enter Folder Name'});
}
function p_RenameOnClick()
{
	if( p_elCurItem == null ) return;
	new cgru_Dialog({"handle":'p_Rename',"value":p_elCurItem.m_obj.label,
		"name":'playlist',"title":'Rename Playlist Item',"info":'Enter New Name'});
}

function p_NewFolder( i_value)
{
	var obj = {};
	obj.label = i_value;
	obj.id = p_elCurFolder.m_obj.id + '/' + i_value;
	obj.user = g_auth_user.id;
	obj.time = c_DT_CurSeconds();
	obj.playlist = [];
	p_Action([obj], 'add');
}

function p_AddLinkAbc()
{
	p_AddLink( null, true);
}
function p_AddLinkAfter()
{
	var id_before = null;
	if( p_elCurItem && p_elCurItem.nextSibling) id_before = p_elCurItem.nextSibling.m_obj.id;
	p_AddLink( id_before);
}
function p_AddLinkBefore()
{
	var id_before = null;
	if( p_elCurItem ) id_before = p_elCurItem.m_obj.id;
	p_AddLink( id_before);
}
function p_AddLink( i_id_before, i_abc)
{
	if( g_CurPath() == '/' )
	{
		c_Error('Can`t add root folder to playlist.');
		return;
	}

	var objs = [];

	if( fv_cur_item )
	{
		// Files view selection:
		var obj = {};
		obj.label = c_PathBase( g_CurPath());
		obj.item = fv_cur_item.m_path;
		obj.id = p_elCurFolder.m_obj.id + '/' + obj.label;
		obj.id += '_' + c_PathBase( obj.item);
		obj.path = g_GetLocationArgs({"fv_Goto":fv_cur_item.m_path});
		objs.push( obj);
	}
	else if(( ASSETS.scene && ASSETS.scene.path == g_CurPath() ) || ( ASSETS.scenes && ASSETS.scenes.path == g_CurPath()))
	{
		var elShots = scenes_GetSelectedShots();
		for( var i = 0; i < elShots.length; i++)
		{
			var obj = {};
			obj.path = elShots[i].m_path;
			obj.label = c_PathBase( obj.path);
			obj.id = p_elCurFolder.m_obj.id + '/' + obj.label;
			objs.push( obj);
		}
	}

	if( objs.length == 0 )
	{
		var obj = {};
		obj.label = c_PathBase( g_CurPath());
		obj.path = document.location.hash;
		obj.id = p_elCurFolder.m_obj.id + '/' + obj.label;
		objs.push( obj);
	}

	for( var i = 0; i < objs.length; i++)
	{
		if( objs[i].path.indexOf('#') == 0 )
			objs[i].path = objs[i].path.substr(1);

		objs[i].user = g_auth_user.id;
		objs[i].time = c_DT_CurSeconds();
	}

	// Search for an index by alphabetically
	if( i_abc && p_elCurFolder && p_elCurFolder.m_elArray && p_elCurFolder.m_elArray.length )
		for( var i = 0; i < p_elCurFolder.m_elArray.length; i++)
			if( objs[0].id < p_elCurFolder.m_elArray[i].m_obj.id )
			{
				i_id_before = p_elCurFolder.m_elArray[i].m_obj.id;
				break;
			}

	p_Action( objs, 'add', i_id_before);
}
function p_Rename( i_value)
{
	var obj = {};
	obj.label = i_value;
	obj.id = p_elCurItem.m_obj.id;
	p_Action([obj], 'rename');
}

function p_FolderOnClick( i_evt)
{
	i_evt.stopPropagation();
	var el = i_evt.currentTarget;
	p_elCurFolder = el;
	if( p_elCurItem == el)
		p_FolderOpenClose( el);
	else
	{
		p_SetCurItem( el);
		if( false == el.classList.contains('opened'))
			p_FolderOpenClose( el);
	}
}
/*function p_FolderOnDblClick( i_evt)
{
	i_evt.stopPropagation();
	p_FolderOpenClose( i_evt.currentTarget);
}*/
function p_FolderOpenClose( i_el)
{
//if( el == p_elCurItem )
//{
	if( i_el.classList.contains('opened'))
	{
		i_el.classList.remove('opened');
//		p_elCurFolder = i_el.parentNode;
		p_elCurFolder = i_el.m_elParent;
		var folders = localStorage.playlist_opened_folders.split(' ');
		folders.splice( folders.indexOf( i_el.m_obj.id), 1);
		localStorage.playlist_opened_folders = folders.join(' ');
	}
	else
	{
		i_el.classList.add('opened');
		p_elCurFolder = i_el;
		var folders = localStorage.playlist_opened_folders.split(' ');
		folders.push( i_el.m_obj.id);
		localStorage.playlist_opened_folders = folders.join(' ');
	}
//}
	p_SetCurItem( p_elCurFolder);
}
function p_LinkOnClick( i_evt)
{
	i_evt.stopPropagation();
	p_SetCurItem( i_evt.currentTarget);
//	p_elCurFolder = p_elCurItem.parentNode;
	p_elCurFolder = p_elCurItem.m_elParent;
//	g_GO( p_elCurItem.m_path );
//g_elCurFolder.scrollIntoView(false);
}
function p_DelOnClick()
{
	if( p_elCurItem == null ) return;
	p_Action([{"id":p_elCurItem.m_obj.id}], 'del' );
}
function p_MoveUp()
{
	if( p_elCurItem && ( p_elCurItem != p_elCurItem.m_elParent.m_elFirst))
		p_Action([p_elCurItem.m_obj], 'add', p_elCurItem.previousSibling.m_obj.id);
}
function p_MoveTop()
{
	if( p_elCurItem && ( p_elCurItem != p_elCurItem.m_elParent.m_elFirst))
		p_Action([p_elCurItem.m_obj], 'add', p_elCurItem.m_elParent.m_elFirst.m_obj.id);
}
function p_MoveBottom()
{
	if( p_elCurItem && ( p_elCurItem != p_elCurItem.m_elParent.m_elLast))
		p_Action([p_elCurItem.m_obj], 'add');
}
function p_MoveDown()
{
	if( p_elCurItem && ( p_elCurItem != p_elCurItem.m_elParent.m_elLast))
		if( p_elCurItem.nextSibling.nextSibling && ( p_elCurItem.nextSibling.nextSibling != p_elCurItem.m_elParent.m_elFirst))
			p_Action([p_elCurItem.m_obj], 'add', p_elCurItem.nextSibling.nextSibling.m_obj.id);
		else
			p_Action([p_elCurItem.m_obj], 'add');
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

function p_Action( i_objects, i_action, i_id_before)
{
	var obj = {};

	for( var i = 0; i < i_objects.length; i++)
		if( i_objects[i].id )
			i_objects[i].id = i_objects[i].id.replace(/ /g,'_');

	if( i_action == 'add' )
	{
		if( p_fileExist )
		{	
			obj.objects = i_objects;
			obj.pusharray = 'playlist';
			obj.id_before = i_id_before;
			obj.id = p_elCurFolder.m_obj.id;

			// If we are moving a folder:
			if(i_objects[0].id == p_elCurFolder.m_obj.id )
				obj.id = p_elCurFolder.m_elParent.m_obj.id;
		}
		else
		{
			obj.object = {"id":"","playlist":i_objects};
			obj.add = true;
		}
	}
	else if( i_action == 'del')
	{
		obj.objects = i_objects;
		obj.delarray = 'playlist';
	}
	else if( i_action == 'rename')
	{
		obj.objects = i_objects;
		obj.replace = true;
	}
	else
	{
		c_Error('Playlist: Unknown action = '+i_action+'<br> Object: '+JSON.stringify(i_objects));
		return;
	}
	obj.file = p_file;
	n_Request({"send":{"editobj":obj},"func":p_EditFinished});
}
function p_EditFinished( i_data)
{
	if(( i_data == null ) || ( i_data.error ))
		c_Error( data.error);
	else
		c_Info('Playlist changes saved.');
	p_Load();
}

function p_RefreshOnClick( i_evt) { p_Load();}
function p_Load()
{
	$('playlist').innerHTML = 'Loading...';
	n_Request({"send":{"getfile":p_file},"func":p_Received,"info":'playlist'});
	p_elLinks = [];
}
function p_Received( obj)
{
	if( obj == null ) return;
	if( obj.error == null )
		p_fileExist = true;

	var params = {};
	params.wasopened = localStorage.playlist_opened_folders.split(' ');
	params.opened = [];
	params.curfolderid = p_elCurFolder ? p_elCurFolder.m_obj.id : null;
	params.curitemid = p_elCurItem ? p_elCurItem.m_obj.id : null;

	p_elCurFolder = $('playlist');
	p_elCurItem = null;

	$('playlist').innerHTML = '';

	p_Read( obj.playlist, params, $('playlist'));

	localStorage.playlist_opened_folders = params.opened.join(' ');

	p_HighlightCurPath();
}
function p_Read( i_playlist, i_params, i_elParent)
{
	if( i_playlist == null ) return;
	if( i_playlist.length == null ) return;

//	i_playlist.sort( function(a,b){if( a['label']<b['label'])return -1;return 1});

	for( var i = 0; i < i_playlist.length; i++)
	{
		if( i_playlist[i] == null )
			continue;

		var el = null;
		if( i_playlist[i].playlist )
		{
			var el = p_CreateFolder( i_playlist[i], i_elParent);
			if( el.m_obj.id == i_params.curfolderid )
			{
				p_elCurFolder = el;
				p_SetCurItem( el);
			}
			if( i_params.wasopened.indexOf( el.m_obj.id ) != -1 )
			{
				el.classList.add('opened');
				i_params.opened.push( el.m_obj.id);
			}
			if( i_playlist[i].playlist.length )
				p_Read( i_playlist[i].playlist, i_params, el);
		}
		else
			el = p_CreateLink( i_playlist[i], i_elParent);

		if( el && ( el.m_obj.id == i_params.curitemid ))
			p_SetCurItem( el);
	}
}
function p_CreateFolder( i_obj, i_elParent)
{
	var el = p_CreateElement( i_obj, i_elParent);
	el.classList.add('folder');
	el.textContent = i_obj.label;
	el.onclick = p_FolderOnClick;
//	el.ondblclick = p_FolderOnDblClick;
	return el;
}
function p_CreateLink( i_obj, i_elParent)
{
	var el = p_CreateElement( i_obj, i_elParent);
	el.classList.add('location');
	el.m_path = i_obj.path;
	el.onclick = p_LinkOnClick;
	p_elLinks.push( el);
	return el;
}
function p_CreateElement( i_obj, i_elParent, i_type)
{
	var el = document.createElement('div');
	el.classList.add('item');
	i_elParent.appendChild( el);
	el.m_obj = i_obj;

	el.m_elParent = i_elParent;
	if( i_elParent.m_elFirst == null ) i_elParent.m_elFirst = el;
	if( i_elParent.m_elArray == null ) i_elParent.m_elArray = [];
	i_elParent.m_elArray.push( el);
	i_elParent.m_elLast = el;

	i_elParent.title = i_elParent.m_title + '\nItems Count: ' + i_elParent.m_elArray.length;

	var title = '';
	if( i_obj.user ) title = c_GetUserTitle( i_obj.user);
	if( i_obj.time ) title += '\n' + c_DT_StrFromSec( i_obj.time);
	if( i_obj.path )
	{
		var path = i_obj.path.split('?')[0];
		title += '\n' + path;

		var elLink = document.createElement('a');
		el.appendChild( elLink);
		elLink.textContent = i_obj.label;
		elLink.classList.add('link');
		elLink.href = '#' + i_obj.path;

		if( i_obj.item )
		{
			title += '\n @ ' + i_obj.item.replace( path, '');
			var elAnchor = document.createElement('a');
			el.appendChild( elAnchor);
			elAnchor.textContent = c_PathBase( i_obj.item);
			elAnchor.classList.add('anchor');
			elAnchor.href = RULES.root + i_obj.item;
			elAnchor.target = '_blank';
		}
	}

	el.m_title = title;
	if( title != '' ) el.title = title;

	return el;
}

function p_GetCurrentShots()
{
	shots = [];

	if( p_elCurFolder == null )
	{
		c_Error('Current folder not found.');
		return shots;
	}
	if( p_elCurFolder.m_elArray == null )
	{
		c_Error('Current folder not has no childs.');
		return shots;
	}

	for( var i = 0; i < p_elCurFolder.m_elArray.length; i++)
		if( p_elCurFolder.m_elArray[i].m_path )
			shots.push( p_elCurFolder.m_elArray[i].m_path);

	return shots;
}

function p_NavigatePost()
{
	p_HighlightCurPath();
}

function p_HighlightCurPath()
{
	var path = g_CurPath();
	for( var i = 0; i < p_elLinks.length; i++)
		if( p_elLinks[i].m_path == path )
			p_elLinks[i].classList.add('cur_path');
		else
			p_elLinks[i].classList.remove('cur_path');
}

function p_MakeCut()
{
	var args = {};

	if( p_elCurFolder == null )
	{
		c_Error('Current folder not found.');
		return;
	}
	if( p_elCurFolder.m_elArray == null )
	{
		c_Error('Current folder not has no childs.');
		return;
	}

	args.shots = [];
	for( var i = 0; i < p_elCurFolder.m_elArray.length; i++)
		if( p_elCurFolder.m_elArray[i].m_path )
			args.shots.push( p_elCurFolder.m_elArray[i].m_path);

	if( args.shots.length < 2 )
	{
		c_Error('Current folder has less then 2 shots.');
		return;
	}

	args.cut_name = p_elCurFolder.m_obj.label;
	if( p_elCurItem && p_elCurItem.m_path )
		args.output = c_PathDir( p_elCurItem.m_path);
	else
		args.output = c_PathDir( args.shots[0]);
	args.output += '/' + RULES.cut.output;

	d_MakeCut( args);
}

function p_Put()
{
	var args = {};

	if( p_elCurFolder == null )
	{
		c_Error('Current folder not found.');
		return;
	}
	if( p_elCurFolder.m_elArray == null )
	{
		c_Error('Current folder not has no childs.');
		return;
	}

	args.shots = [];
	for( var i = 0; i < p_elCurFolder.m_elArray.length; i++)
		if( p_elCurFolder.m_elArray[i].m_path )
			args.shots.push( p_elCurFolder.m_elArray[i].m_path);

	if( args.shots.length < 1 )
	{
		c_Error('Current folder has less then one shot.');
		return;
	}

	fu_PutMultiDialog( args);
}

