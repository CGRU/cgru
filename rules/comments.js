cm_file = 'comments.json';
cm_durations = ['.1','.2','.3','.5','1','2','3','4','5','6','7','8','9','10','11','12','14','16','20','24','32','40','48','66','99'];
cm_all = {};

function View_comments_Open() { cm_Load(); }

function cm_Load()
{
	$('comments').textContent = '';
	cm_all = {};

	if( false == c_RuFileExists( cm_file)) return;

	var obj = c_Parse( n_Request({"readobj":c_GetRuFilePath( cm_file)}));
	if( obj == null ) return;
	if( obj.comments == null ) return;

	cm_all = obj.comments;
	for( key in cm_all )
		cm_Add( cm_all[key], key);
}

function cm_Add( i_obj, i_key)
{
//window.console.log( JSON.stringify( i_obj));
	var el = document.createElement('div');
	$('comments').insertBefore( el, $('comments').firstChild);
	el.classList.add('comment');

	var elPanel = document.createElement('div');
	el.appendChild( elPanel);
	elPanel.classList.add('panel');
	el.m_elPanel = elPanel;

	var elEdit = document.createElement('div');
	elPanel.appendChild( elEdit);
	elEdit.classList.add('button');
	elEdit.textContent = 'Edit';
	elEdit.onclick = function(e){cm_Edit(e.currentTarget.m_el);}
	elEdit.m_el = el;
	el.m_elEdit = elEdit;

	var elCancel = document.createElement('div');
	elPanel.appendChild( elCancel);
	elCancel.classList.add('button');
	elCancel.textContent = 'Cancel';
	elCancel.onclick = function(e){cm_Cancel(e.currentTarget.m_el)};
	elCancel.m_el = el;
	el.m_elCancel = elCancel;

	var elSave = document.createElement('div');
	elPanel.appendChild( elSave);
	elSave.classList.add('button');
	elSave.textContent = 'Save';
	elSave.style.display = 'none';
	elSave.onclick = function(e){cm_Save(e.currentTarget.m_el)};
	elSave.m_el = el;
	el.m_elSave = elSave;

	var elDel = document.createElement('div');
	elPanel.appendChild( elDel);
	elDel.classList.add('button');
	elDel.textContent = 'Delete';
	elDel.onclick = function(e){cm_Delete(e.currentTarget.m_el)};
	elDel.m_el = el;
	el.m_elDel = elDel;

	var elType = document.createElement('a');
	elPanel.appendChild( elType);
	elType.classList.add('tag');
	elType.classList.add('type');
	elType.style.cssFloat = 'left';
	el.m_elType = elType;

	var elAvatar = document.createElement('img');
	elAvatar.classList.add('avatar');
	elPanel.appendChild( elAvatar);
	el.m_elAvatar = elAvatar;

	var elUser = document.createElement('div');
	elUser.classList.add('user');
	elPanel.appendChild( elUser);
	el.m_elUser = elUser;

	var elDuration = document.createElement('div');
	elDuration.classList.add('duration');
	elPanel.appendChild( elDuration);
	el.m_elDuration = elDuration;

	var elDate = document.createElement('div');
	elDate.classList.add('date');
	elPanel.appendChild( elDate);
	el.m_elDate = elDate;

	var elModified = document.createElement('div');
	elModified.classList.add('modified');
	elPanel.appendChild( elModified);
	el.m_elModified = elModified;

	var elText = document.createElement('div');
	el.appendChild( elText);
	elText.classList.add('text');
	el.m_elText = elText;

	var elTypesDiv = document.createElement('div');
	el.appendChild( elTypesDiv);
	elTypesDiv.classList.add('types');
	el.m_elTypesDiv = elTypesDiv;
	el.m_elTypes = [];
	for( var type in RULES.comments )
	{
		eltp = document.createElement('div');
		elTypesDiv.appendChild( eltp);
		eltp.classList.add('tag');
		eltp.textContent = RULES.comments[type].title;
		eltp.m_type = type;
		eltp.m_el = el;
		eltp.onclick = function(e){ cm_TypeOnClick( e.currentTarget.m_el, e.currentTarget.m_type);};
		st_SetElColor({"color":RULES.comments[type].color}, eltp);
	}

	var elForEdit = document.createElement('div');
	el.appendChild( elForEdit);
	el.m_elForEdit = elForEdit;

	var elUploads = document.createElement('div');
	el.appendChild( elUploads);
	elUploads.classList.add('uploads');
	elUploads.style.display = 'none';
	el.m_elUploads = elUploads;

	el.m_obj = i_obj;
	cm_Init( el, i_key);
	return el;
}

function cm_Init( i_el, i_key)
{
	i_el.m_elForEdit.innerHTML = '';
	if( i_el.m_elEditPanel ) i_el.m_elPanel.removeChild( i_el.m_elEditPanel);

	i_el.m_elDel.style.display = 'none';
	i_el.m_elCancel.style.display = 'none';
	i_el.m_elSave.style.display = 'none';
	i_el.m_elTypesDiv.style.display = 'none';
	if( g_admin )
		i_el.m_elEdit.style.display = 'block';
	else
		i_el.m_elEdit.style.display = 'none';

	i_el.m_elText.contentEditable = 'false';
	i_el.m_elText.classList.remove('editing');
	i_el.m_elText.style.color = localStorage.text_color;

	if( localStorage.back_comments && ( localStorage.back_comments != ''))
		i_el.m_elText.style.background = localStorage.back_comments;
	else if( localStorage.background && ( localStorage.background != '' ))
		i_el.m_elText.style.background = localStorage.background;

	var avatar = null;
	if( i_el.m_obj == null )
	{
		i_el.m_obj = {};
		i_el.m_obj.ctime = (new Date()).getTime();
		i_el.m_new = true;
		if( g_auth_user )
			i_el.m_obj.user_name = g_auth_user.id;
	}

	if( i_el.m_obj.user_name )
	{
		i_el.m_elUser.textContent = c_GetUserTitle( i_el.m_obj.user_name, i_el.m_obj.guest);
		avatar = c_GetAvatar( i_el.m_obj.user_name, i_el.m_obj.guest);
	}

	if( avatar != null )
	{
		i_el.m_elAvatar.src = avatar;
		i_el.m_elAvatar.style.display = 'block';
	}
	else
		i_el.m_elAvatar.style.display = 'none';

	cm_SetElType( i_el.m_obj.type, i_el.m_elType, i_el);
	if( i_key )
	{
		i_el.m_elType.href = g_GetLocationArgs({"cm_Goto":i_key});
		i_el.id = i_key;
	}
	i_el.m_type = i_el.m_obj.type;

	i_el.m_elDate.textContent = c_DT_StrFromMSec( i_el.m_obj.ctime);
	if( i_el.m_obj.duration ) i_el.m_elDuration.textContent = i_el.m_obj.duration;
	else i_el.m_elDuration.textContent = ' ';

	if( i_el.m_obj.mtime )
	{
		var date = c_DT_StrFromMSec( i_el.m_obj.mtime);
		i_el.m_elModified.textContent = 'Modified: ' + c_GetUserTitle(i_el.m_obj.muser_name)+' '+date;
	}

	if( i_el.m_obj.text )
		i_el.m_elText.innerHTML = i_el.m_obj.text;

	if( i_el.m_obj.uploads && ( i_el.m_uploads_created != true ))
	{
//console.log( JSON.stringify(i_el.m_obj.uploads));
		i_el.m_uploads_created = true;
		i_el.m_elUploads.style.display = 'block';
		for( var i = 0; i < i_el.m_obj.uploads.length; i++)
		{
			var up = i_el.m_obj.uploads[i];

			var el = document.createElement('div');
			i_el.m_elUploads.appendChild( el);
			el.classList.add('path');

			c_CreateOpenButton( el, up.path);

			var elLink = document.createElement('a');
			el.appendChild( elLink);
			elLink.href = '#' + up.path;
			var dir = up.path.replace( g_CurPath(), '');
			if( dir[0] == '/' ) dir = dir.substr( 1);
			elLink.textContent = dir;

			for( var f = 0; f < up.files.length; f++)
				cm_ShowFile( el, up.path, up.files[f]);
		}
	}

	i_el.m_color = i_el.m_obj.color;

	st_SetElColor({"color":i_el.m_color}, i_el, null, false);

	if( i_el.m_obj.deleted )
		i_el.style.display = 'none';
}

function cm_SetElType( i_type, i_elType, i_elColor)
{
	if( i_elColor == null ) i_elColor = i_elType;
	if( i_type )
	{
		if( RULES.comments[i_type] )
		{
			i_elType.textContent = RULES.comments[i_type].title;
			st_SetElColor({"color":RULES.comments[i_type].color}, i_elColor);
		}
		else
		{
			i_elType.textContent = i_type;
			i_elColor.style.color = 'inherit';
			st_SetElColor( null, i_elColor);
		}
	}
	else
	{
		i_elType.textContent = 'Comment';
		st_SetElColor( null, i_elColor);
	}
}

function cm_Edit( i_el)
{
	if( i_el.m_new != true )
	{
		if( g_auth_user == null )
		{
			c_Error('Guests can`t edit comments.');
			return;
		}
		if( g_admin == false )
		{
			c_Error('You can`t edit comments.');
			return;
		}
		i_el.m_elDel.style.display = 'block';
	}
	i_el.m_elCancel.style.display = 'block';
	i_el.m_elSave.style.display = 'block';
	i_el.m_elEdit.style.display = 'none';
	i_el.m_elTypesDiv.style.display = 'block';

	i_el.m_elEditPanel = u_EditPanelCreate( i_el.m_elPanel);

	var elDCtrl = document.createElement('div');
	i_el.m_elForEdit.appendChild( elDCtrl);
	elDCtrl.classList.add('edit_duration');
	i_el.m_elDCtrl = elDCtrl;

	var elDLabel = document.createElement('div');
	elDCtrl.appendChild( elDLabel);
	elDLabel.textContent = 'Duration:';

	var elDrtn = document.createElement('div');
	elDCtrl.appendChild( elDrtn);
	elDrtn.classList.add('editing');
	elDrtn.contentEditable = 'true';
	i_el.m_elDrtn = elDrtn;
	if( i_el.m_obj.duration ) i_el.m_elDrtn.textContent = i_el.m_obj.duration;

	for( var i = 0; i < cm_durations.length; i++)
	{
		var elDr = document.createElement('div');
		elDCtrl.appendChild( elDr);
		elDr.classList.add('sample')
		elDr.textContent = cm_durations[i];
		elDr.m_elDrtn = elDrtn;
		elDr.onclick = function(e){e.currentTarget.m_elDrtn.textContent = e.currentTarget.textContent;}
	}

	var elColor = document.createElement('div');
	i_el.m_elForEdit.appendChild( elColor);
	i_el.m_elColor = elColor;
	u_DrawColorBars( elColor, cm_ColorOnclick);

	if( g_auth_user == null )
		u_GuestAttrsDraw( i_el.m_elForEdit);

	i_el.m_elText.classList.add('editing');
	i_el.m_elText.contentEditable = 'true';
	i_el.m_elText.style.color = '#000000';
	i_el.m_elText.style.background = '#DDDDDD';
	i_el.m_elText.focus();
}

function cm_TypeOnClick( i_el, i_type)
{
	i_el.m_type = i_type;
//	i_el.m_elType.textContent = RULES.comments[i_type].title;
	cm_SetElType( i_type, i_el.m_elType, i_el);
	st_SetElColor({"color":i_el.m_color}, i_el, null, false);
}

function cm_ColorOnclick( e)
{
	var clrEl = e.currentTarget;
	el = clrEl.parentNode.parentNode.parentNode;
	el.m_color = clrEl.m_color;
	cm_SetElType( el.m_type, el.m_elType, el);
	st_SetElColor({"color":el.m_color}, el, null, false);
}

function cm_Cancel( i_el)
{
	if( i_el.m_new )
		$('comments').removeChild( i_el);
	else
		cm_Init( i_el);
}

function cm_NewOnClick()
{
	cm_Edit( cm_Add());
}

function cm_Delete( i_el)
{
	i_el.m_obj.deleted = true;
	cm_Save( i_el);
}

function cm_Save( i_el)
{
	if( g_auth_user == null )
	{
		i_el.m_obj.guest = u_GuestAttrsGet( i_el.m_elForEdit);
		i_el.m_obj.user_name = i_el.m_obj.guest.id;
	}

	i_el.m_obj.text = i_el.m_elText.innerHTML;
	i_el.m_obj.color = i_el.m_color;
	i_el.m_obj.type = i_el.m_type;
	if( i_el.m_obj.deleted != true )
		cm_ProcessUploads( i_el.m_obj);

	var duration = parseFloat( i_el.m_elDrtn.textContent);
	if( false == isNaN( duration ))
		i_el.m_obj.duration = duration;

	if( i_el.m_new )
		i_el.m_new = false;
	else
	{
		i_el.m_obj.mtime = (new Date()).getTime();
		i_el.m_obj.muser_name = g_auth_user.id;
	}

	var key = i_el.m_obj.ctime+'_'+i_el.m_obj.user_name;

	cm_Init( i_el, key);

	var comments = {};
	comments[key] = i_el.m_obj;
	var edit = {};
	edit.object = {"comments":comments};
	edit.add = true;
	edit.file = c_GetRuFilePath( cm_file);

//window.console.log( JSON.stringify( edit));
	res = c_Parse( n_Request({"editobj":edit}));
	if( c_NullOrErrorMsg( res)) return;
	nw_MakeNews('<i>comments</i>', g_CurPath(), i_el.m_obj.user_name, i_el.m_obj.guest);

	cm_all[key] = i_el.m_obj;
	var emails = [];
	if( RULES.status && RULES.status.body && RULES.status.body.guest && RULES.status.body.guest.email )
		emails.push( RULES.status.body.guest.email);
	for( key in cm_all )
	{
		var cm = cm_all[key];
		if( cm.guest && cm.guest.email && cm.guest.email.length && ( emails.indexOf( cm.guest.email) == -1 ))
			emails.push( cm.guest.email);
	}
	if( emails.length )
		cm_EmailGuests( cm_all[key], key, emails);
}

function cm_EmailGuests( i_cm, i_key, i_emails)
{
	for( var i = 0; i < i_emails.length; i++)
	{
		var email = c_EmailDecode( i_emails[i]);
		if( false == c_EmailValidate( email)) continue;
		var subject = 'RULES Comment: '+g_CurPath();
		var href = g_GetLocationArgs({"cm_Goto":i_key}, true);
		var body = '<a href="'+href+'" target="_blank">'+href+'</a>';
		body += '<br><br>';
		body += i_cm.text;
		body += '<br><br>';
		var user = c_GetUserTitle( i_cm.user_name, i_cm.guest);
		body += user;
		if( user != i_cm.user_name ) body += ' ['+i_cm.user_name+']';

		n_SendMail( email, subject, body);
	}
}

function cm_Goto( i_name )
{
	if( localStorage['view_comments'] !== 'true' )
		u_OpenCloseView( 'comments', true, true);
	var el = $(i_name);
	if( el == null )
	{
		c_Error('Comment with key='+i_name+' not founded.');
		return;
	}
	el.scrollIntoView();
}

function cm_ProcessUploads( i_obj)
{
	var upfiles = [];
	for( var i = 0; i < up_elFiles.length; i++)
	{
		var el = up_elFiles[i];
		if( el.m_selected != true ) continue;
		if( el.m_uploading == true ) continue;

		up_Start( el);

		var path = c_PathDir( el.m_path);
		var file = {};
		file.name = el.m_upfile.name;
		file.size = el.m_upfile.size;
		upfiles.push({"path":path,"file":file});
	}

	if( upfiles.length == 0 ) return;

	upfiles.sort( function(a,b){if(a.path<b.path)return -1;if(a.path>b.path)return 1;return 0;});

	var uploads = [];
	var u = -1;
	for( var f = 0; f < upfiles.length; f++)
	{
		if(( u == -1 ) || ( upfiles[f].path != uploads[u].path ))
		{
			uploads.push({});
			u++;
			uploads[u].path = upfiles[f].path;
			uploads[u].files = [];
		}
		uploads[u].files.push( upfiles[f].file);
	}

//console.log( JSON.stringify( uploads));
	i_obj.uploads = uploads;
}

function cm_ShowFile( i_el, i_path, i_file)
{
	var el = document.createElement('div');
	i_el.appendChild( el);
	el.classList.add('file');

	var elThumb = document.createElement('img');
	el.appendChild( elThumb);
	elThumb.classList.add('thumbnail');
	elThumb.src = RULES.root + c_GetThumbFileName( i_path + '/' + i_file.name);

	var elLink = document.createElement('a');
	el.appendChild( elLink);
	elLink.classList.add('link');
	elLink.textContent = 'link';
	elLink.target = '_blank';
	elLink.href = RULES.root + i_path + '/' + i_file.name;

	var elSize = document.createElement('div');
	el.appendChild( elSize);
	elSize.classList.add('size');
	elSize.textContent = c_Bytes2KMG( i_file.size);

	var elGoto = document.createElement('a');
	el.appendChild( elGoto);
	elGoto.classList.add('goto');
	elGoto.textContent = i_file.name;
	elGoto.href = '#' + i_path + '?' + JSON.stringify({"fv_Goto":i_path+'/'+i_file.name});
}

