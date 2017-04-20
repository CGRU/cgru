cm_file = 'comments.json';
cm_durations = ['.25','.5','1','1.5','2','3','4','5','6','7','8','9','10','12','14','16','18','20','24','32','40','80'];
cm_array = [];

function View_comments_Open() { cm_Load(); }

function cm_Finish()
{
	cm_array = [];
}

function cm_Load()
{
	$('comments').textContent = '';
	if( ASSET && ASSET.comments_reversed )
	{
		var el = $('comments_btn_add');
		$('comments_show').removeChild( el);
		$('comments_show').appendChild( el);
	}

	cm_array = [];

	if( false == c_RuFileExists( cm_file)) return;

	$('comments').textContent = 'Loading...';

	n_GetFile({"path":c_GetRuFilePath( cm_file),"func":cm_Received,"cache_time":RULES.cache_time,"info":'comments',"parse":true,"local":true});

}

function cm_Received( i_data)
{
	$('comments').textContent = '';
	if( i_data == null ) return;
	if( i_data.comments == null )
	{
		c_Error('Invalid comments data received.');
		c_Log(JSON.stringify(i_data));
		return;
	}

	var obj_array = [];
	for( key in i_data.comments )
	{
		i_data.comments[key].key = key;
		obj_array.push( i_data.comments[key]);
	}

	obj_array.sort( function(a,b){if(a.key<b.key)return -1;if(a.key>b.key)return 1;return 0;});

	var i = 0;
	for( var i = 0; i < obj_array.length; i++)
		cm_array.push( new Comment( obj_array[i]));

	g_POST('comments');
}

function cm_NewOnClick( i_text)
{
	var comment = new Comment();
	if( i_text)
		comment.elText.innerHTML = i_text;
	comment.edit();
}

function Comment( i_obj) 
{
//window.console.log( JSON.stringify( i_obj));
	this.el = document.createElement('div');
	if( ASSET && ASSET.comments_reversed )
		$('comments').appendChild( this.el);
	else
		$('comments').insertBefore( this.el, $('comments').firstChild);
	this.el.classList.add('comment');
	this.el.m_comment = this;

	this.elPanel = document.createElement('div');
	this.el.appendChild( this.elPanel);
	this.elPanel.classList.add('panel');

	this.elEdit = document.createElement('div');
	this.elPanel.appendChild( this.elEdit);
	this.elEdit.classList.add('button');
	this.elEdit.classList.add('edit');
	this.elEdit.title = 'Edit comment';
	this.elEdit.onclick = function(e){ e.currentTarget.m_comment.edit();};
	this.elEdit.m_comment = this;

	this.elEditBtnsDiv = document.createElement('div');
	this.elPanel.appendChild( this.elEditBtnsDiv);
	this.elEditBtnsDiv.classList.add('edit_btns_div');

	this.elCancel = document.createElement('div');
	this.elEditBtnsDiv.appendChild( this.elCancel);
	this.elCancel.classList.add('button');
	this.elCancel.textContent = 'Cancel';
	this.elCancel.title = 'Cancel comment editing.';
	this.elCancel.m_comment = this;
	this.elCancel.onclick = function(e){ e.currentTarget.m_comment.editCancel();};

	this.elSave = document.createElement('div');
	this.elEditBtnsDiv.appendChild( this.elSave);
	this.elSave.classList.add('button');
	this.elSave.innerHTML = '<b>Save</b> <small>(CRTL+ENTER)</small>';
	this.elSave.title = 'Save comment.\n(CRTL+ENTER)';
	this.elSave.m_comment = this;
	this.elSave.onclick = function(e){ e.currentTarget.m_comment.save();};

	this.elRemMU = document.createElement('div');
	this.elEditBtnsDiv.appendChild( this.elRemMU);
	this.elRemMU.classList.add('button');
	this.elRemMU.textContent = 'Remove all markup';
	this.elRemMU.title = 'Double click to remove all markup from comment.';
	this.elRemMU.m_comment = this;
	this.elRemMU.ondblclick = function(e){ c_elMarkupRemove( e.currentTarget.m_comment.elText)};

	this.elDel = document.createElement('div');
	this.elEditBtnsDiv.appendChild( this.elDel);
	this.elDel.classList.add('button');
	this.elDel.textContent = 'Delete';
	this.elDel.title = 'Double click to delete comment.';
	this.elDel.ondblclick = function(e){ e.currentTarget.m_comment.destroy();};
	this.elDel.m_comment = this;

	this.elType = document.createElement('a');
	this.elPanel.appendChild( this.elType);
	this.elType.classList.add('tag');
	this.elType.classList.add('type');
	this.elType.style.cssFloat = 'left';

	this.elAvatar = document.createElement('img');
	this.elAvatar.classList.add('avatar');
	this.elPanel.appendChild( this.elAvatar);

	this.elUser = document.createElement('div');
	this.elUser.classList.add('user');
	this.elPanel.appendChild( this.elUser);

	this.elReport = document.createElement('div');
	this.elPanel.appendChild( this.elReport);
//	this.elReport.style.display = 'none';
	this.elReport.classList.add('report');

	this.elDuration = document.createElement('div');
	this.elDuration.classList.add('duration');
	this.elReport.appendChild( this.elDuration);

	this.elTags = document.createElement('div');
	this.elTags.classList.add('tags');
	this.elReport.appendChild( this.elTags);

	this.elDate = document.createElement('div');
	this.elDate.classList.add('date');
	this.elPanel.appendChild( this.elDate);

	this.elInfo = document.createElement('div');
	this.elInfo.classList.add('info');
	this.elPanel.appendChild( this.elInfo);

	this.elText = document.createElement('div');
	this.el.appendChild( this.elText);
	this.elText.classList.add('text');
	this.elText.m_obj = this;
	this.elText.onkeydown = function(e){ e.currentTarget.m_obj.textOnKeyDown(e);};

	this.elForEdit = document.createElement('div');
	this.el.appendChild( this.elForEdit);
	this.elForEdit.classList.add('edit');

	this.elUploads = document.createElement('div');
	this.el.appendChild( this.elUploads);
	this.elUploads.classList.add('uploads');
	this.elUploads.style.display = 'none';

	this.elSignature = document.createElement('div');
	this.el.appendChild( this.elSignature);
	this.elSignature.classList.add('signature');

	this.obj = i_obj;
	this.init();
}

Comment.prototype.init = function() 
{
	this.elTags.textContent = '';
	this.elForEdit.innerHTML = '';
	this.editing = false;
	this.el.classList.remove('edit');
	this.elEditBtnsDiv.style.display = 'none';

	this.elText.contentEditable = 'false';
	this.elText.classList.remove('editing');
	if( localStorage.text_color && ( localStorage.text_color != ''))
		this.elText.style.color = localStorage.text_color;
	else
		this.elText.style.color = u_textColor;
	if( localStorage.back_comments && ( localStorage.back_comments != ''))
		this.elText.style.background = localStorage.back_comments;
	else if( localStorage.background && ( localStorage.background != '' ))
		this.elText.style.background = localStorage.background;
	else
		this.elText.style.backgroundColor = u_backgroundColor;


	if( this.obj == null )
	{
		this.obj = {};
		this.obj.ctime = (new Date()).getTime();
		this._new = true;
		if( g_auth_user )
		{
			this.obj.user_name = g_auth_user.id;
			if( g_auth_user.tag && g_auth_user.tag.length )
			{
				this.obj.tags = [g_auth_user.tag];
			}
		}
	}

	var user = null;
	var avatar = null;
	var signature = null;

	// Get user object:
	if( this.obj.user_name && g_users[this.obj.user_name])
		user = g_users[this.obj.user_name];
	else if( this.obj.guest )
		user = this.obj.guest;
	if( user == null )
		user = {};

	if( this.obj.user_name )
		this.elUser.textContent = c_GetUserTitle( this.obj.user_name, this.obj.guest);

	// Signature:
	if( user.signature )
		this.elSignature.textContent = user.signature;

//console.log( g_auth_user.id + ' ' + this.obj.user_name );
	if( g_auth_user )
	{
		// Edit button only for admins or a comment owner:
		if( g_admin || ( this.obj && ( this.obj.user_name == g_auth_user.id )))
			this.elEdit.style.display = 'block';
		else
			this.elEdit.style.display = 'none';

		// If this is a new comment or and own old:
		if(( this.obj == null ) || ( this.obj.user_name == g_auth_user.id ))
			this.el.classList.add('own');
	}
	else
		this.elEdit.style.display = 'none';

	avatar = c_GetAvatar( this.obj.user_name, this.obj.guest);
	if( avatar != null )
	{
		this.elAvatar.src = avatar;
		this.elAvatar.style.display = 'block';
	}
	else
		this.elAvatar.style.display = 'none';

	this.setElType( this.obj.type);
	if( this.obj.key )
		this.elType.href = this.getLink();

	this.type = this.obj.type;

	if( this.obj.tags && this.obj.tags.length )
	{
		for( var i = 0; i < this.obj.tags.length; i++ )
		{
			var tag = this.obj.tags[i];

			var el = document.createElement('div');
			this.elTags.appendChild( el);
			el.classList.add('tag');

			if( RULES.tags[tag] && RULES.tags[tag].title )
				el.textContent = RULES.tags[tag].title;
			else
				el.textContent = tag;
		}
	}

	this.elDate.textContent = c_DT_StrFromMSec( this.obj.ctime);
	if( this.obj.duration && this.obj.duration > 0 )
		this.elDuration.textContent = this.obj.duration;

	var info = '';

	// Email is shown for admins only:
	if( g_admin && this.obj && this.obj.guest && this.obj.guest.email )
		info += 'Guest email: ' + c_EmailDecode( this.obj.guest.email);

	if( this.obj.mtime )
	{
		var date = c_DT_StrFromMSec( this.obj.mtime);
		if( info.length ) info += '<br>';
		info += 'Modified: ' + c_GetUserTitle( this.obj.muser_name)+' '+date;
	}

	this.elInfo.innerHTML = info;

	if( this.obj.text )
		this.elText.innerHTML = this.obj.text;

	if( this.obj.uploads && ( this.uploads_created != true ))
	{
//console.log( JSON.stringify( this.obj.uploads));
		this.uploads_created = true;
		this.elUploads.style.display = 'block';
		for( var i = 0; i < this.obj.uploads.length; i++)
		{
			var up = this.obj.uploads[i];

			var el = document.createElement('div');
			this.elUploads.appendChild( el);
			el.classList.add('path');

			c_CreateOpenButton( el, up.path);

			var elLink = document.createElement('a');
			el.appendChild( elLink);
			elLink.href = '#' + up.path;
			var dir = up.path.replace( g_CurPath(), '');
			if( dir[0] == '/' ) dir = dir.substr( 1);
			elLink.textContent = dir;

			for( var f = 0; f < up.files.length; f++)
				this.showFile( up.path, up.files[f]);
		}
	}

	this.color = this.obj.color;

	st_SetElColor({"color":this.color}, this.el, null, false);

	if( this.obj.deleted )
		this.el.style.display = 'none';
}

Comment.prototype.setElType = function( i_type)
{
	for( var type in RULES.comments )
		this.el.classList.remove( type);

	if( i_type )
	{
		this.el.classList.add( i_type);

		if( RULES.comments[i_type] )
		{
			this.elType.textContent = RULES.comments[i_type].title;
			st_SetElColor({"color":RULES.comments[i_type].color}, this.el);
		}
		else
		{
			this.elType.textContent = i_type;
			this.el.style.color = 'inherit';
			st_SetElColor( null, this.el);
		}
	}
	else
	{
		this.elType.textContent = 'Comment';
		st_SetElColor( null, this.el);
	}
}

Comment.prototype.edit = function()
{
	if( this._new != true )
	{
		if( g_auth_user == null )
		{
			c_Error('Guests can`t edit comments.');
			return;
		}
/*		if( g_admin == false )
		{
			c_Error('You can`t edit comments.');
			return;
		}*/
		this.elDel.style.display = 'block';
	}
	else
		this.elDel.style.display = 'none';

	this.editing = true;
	this.el.classList.add('edit');
	this.elEdit.style.display = 'none';
	this.elEditBtnsDiv.style.display = 'block';

	this.elEditPanel = u_EditPanelCreate( this.elForEdit);

	this.elEditTypesDiv = document.createElement('div');
	this.elForEdit.appendChild( this.elEditTypesDiv);
	this.elEditTypesDiv.classList.add('types');
//	this.elEditTypesDiv.style.clear = 'both';
	for( var type in RULES.comments )
	{
		var el = document.createElement('div');
		this.elEditTypesDiv.appendChild( el);
		el.classList.add('tag');
		el.textContent = RULES.comments[type].title;
		el.m_type = type;
		el.m_comment = this;
		el.onclick = function(e){ e.currentTarget.m_comment.setType( e.currentTarget.m_type);};
		st_SetElColor({"color":RULES.comments[type].color}, el);
	}

	this.elReportEdit = document.createElement('div');
	this.elForEdit.appendChild( this.elReportEdit);
	this.elReportEdit.classList.add('report');

	this.elEditTags = document.createElement('div');
	this.elReportEdit.appendChild( this.elEditTags);
	this.elEditTags.classList.add('list');
	this.elEditTags.classList.add('tags');

	var el = document.createElement('div');
	this.elEditTags.appendChild( el);
	el.textContent = 'Tags:';
	el.classList.add('label');

	this.elEditTags.m_elTags = [];
	for( var tag in RULES.tags)
	{
		var el = document.createElement('div');
		this.elEditTags.appendChild( el);
		el.classList.add('tag');
		el.m_tag = tag;
		el.onclick = c_ElToggleSelected;

		if( RULES.tags[tag].title ) el.textContent = RULES.tags[tag].title;
		else el.textContent = tag;

		if( this.obj.tags )
			if( this.obj.tags.indexOf( tag) != -1 )
				c_ElSetSelected( el, true);

		this.elEditTags.m_elTags.push( el);
	}

	var elDurationDiv = document.createElement('div');
	this.elReportEdit.appendChild( elDurationDiv);
	elDurationDiv.classList.add('edit_duration');

	var elDurationLabel = document.createElement('div');
	elDurationDiv.appendChild( elDurationLabel);
	elDurationLabel.textContent = 'Duration:';

	this.elEditDuration = document.createElement('div');
	elDurationDiv.appendChild( this.elEditDuration);
	this.elEditDuration.classList.add('editing');
	this.elEditDuration.contentEditable = 'true';
	if( this.obj.duration ) this.elEditDuration.textContent = this.obj.duration;

	for( var i = 0; i < cm_durations.length; i++)
	{
		var el = document.createElement('div');
		elDurationDiv.appendChild( el);
		el.classList.add('sample')
		el.textContent = cm_durations[i];
		el.m_elDrtn = this.elEditDuration;
		el.onclick = function(e){ e.currentTarget.m_elDrtn.textContent = e.currentTarget.textContent;}
	}

	this.elColor = document.createElement('div');
	this.elForEdit.appendChild( this.elColor);
	u_DrawColorBars({"el":this.elColor,"onclick":cm_ColorOnclick,"data":this});

	if( g_auth_user == null )
		u_GuestAttrsDraw( this.elForEdit);

	this.elText.classList.add('editing');
	this.elText.style.backgroundColor = '#DDDDDD';
	this.elText.style.color = '#000000';
	this.elText.contentEditable = 'true';
	this.elText.focus();
}

function cm_ColorOnclick( i_clr, i_data) { i_data.setColor( i_clr); }
Comment.prototype.setColor = function( i_clr)
{
	this.color = i_clr;
	this.setElType( this.type);
	st_SetElColor({"color":this.color}, this.el, null, false);
}

Comment.prototype.setType = function( i_type)
{
	this.type = i_type;
	this.setElType( i_type);
	st_SetElColor({"color":this.color}, this.el, null, false);
}

Comment.prototype.editCancel = function()
{
	if( this._new )
		$('comments').removeChild( this.el);
	else
		this.init();
}

Comment.prototype.textOnKeyDown = function( i_e)
{
	if( this.editing )
	{
		if(( i_e.keyCode == 13 ) && i_e.ctrlKey ) // CTRL + ENTER
		{
			this.save();
			this.elText.blur();
		}
	}
}

Comment.prototype.destroy = function()
{
	this.obj.deleted = true;
	this.save();
}

Comment.prototype.save = function()
{
	if( g_auth_user == null )
	{
		this.obj.guest = u_GuestAttrsGet( this.elForEdit);
		this.obj.user_name = this.obj.guest.id;
	}

	this.obj.text = c_LinksProcess( this.elText.innerHTML);
	this.obj.color = this.color;
	this.obj.type = this.type;
	if( this.obj.deleted != true )
		this.processUploads();

	if( this.obj.type == 'report' )
	{
		this.obj.tags = [];
		for( var i = 0; i < this.elEditTags.m_elTags.length; i++)
		{
			var el = this.elEditTags.m_elTags[i];
			if( el.classList.contains('selected'))
			this.obj.tags.push( el.m_tag);
		}
	}
	else
		delete this.obj.tags;

	this.obj.duration = -1;
	var duration = parseFloat( this.elEditDuration.textContent);
	if( false == isNaN( duration ))
		this.obj.duration = duration;

	if( this._new )
	{
		this._new = false;
		cm_array.push( this);
	}
	else
	{
		this.obj.mtime = (new Date()).getTime();
		this.obj.muser_name = g_auth_user.id;
	}

	var key = this.obj.ctime + '_' + this.obj.user_name;

	this.obj.key = key;
	this.init();

	var file = c_GetRuFilePath( cm_file);
	n_GetFileFlushCache( file);

	var comments = {};
	comments[key] = this.obj;
	var edit = {};
	edit.object = {"comments":comments};
	edit.add = true;
	edit.file = file;

	n_Request({"send":{"editobj":edit},"func":this.saveFinished,"this":this});
}
Comment.prototype.saveFinished = function( i_data, i_args)
{
	if( c_NullOrErrorMsg( i_data)) return;

	var news_user = i_args.this.obj.user_name;
	if( i_args.this.obj.muser_name ) news_user = i_args.this.obj.muser_name;

	var news_title = 'comment';
	if( i_args.this.obj.type == 'report' ) news_title = 'report';

	nw_MakeNews({"title":news_title,"link":i_args.this.getLink(),"user":news_user,"guest":i_args.this.obj.guest});

	i_args.this.updateStatus();

	i_args.this.sendEmails();
}

Comment.prototype.sendEmails = function()
{
	var emails = [];
	if( RULES.status && RULES.status.body && RULES.status.body.guest && RULES.status.body.guest.email )
		emails.push( RULES.status.body.guest.email);
	for( var i = 0; i < cm_array.length; i++)
	{
		var cm = cm_array[i].obj;
		if( cm.guest && cm.guest.email && cm.guest.email.length && ( emails.indexOf( cm.guest.email) == -1 ))
			emails.push( cm.guest.email);
	}

	for( var i = 0; i < emails.length; i++)
	{
		var email = c_EmailDecode( emails[i]);
		if( false == c_EmailValidate( email)) continue;
		var subject = 'RULES Comment: '+g_CurPath();
		var href = this.getLink( true);
		var body = '<a href="'+href+'" target="_blank">'+href+'</a>';
		body += '<br><br>';
		body += this.obj.text;
		body += '<br><br>';
		var user = c_GetUserTitle( this.obj.user_name, this.obj.guest);
		body += user;
		if( user != this.obj.user_name ) body += ' ['+this.obj.user_name+']';

		n_SendMail( email, subject, body);
	}
}

Comment.prototype.updateStatus = function()
{
	var reports = [];

	if( RULES.status == null )
		RULES.status = {};

	if( RULES.status.tags == null )
		RULES.status.tags = [];

	if( RULES.status.artists == null )
		RULES.status.artists = [];

	var reps_tags = [];
	var reps_arts = [];

	for( var i = 0; i < cm_array.length; i++ )
	{
		var obj = cm_array[i].obj;

		if( obj.deleted ) continue;
		if( obj.type !== 'report') continue;
		if( obj.duration == null ) continue;

//console.log( JSON.stringify( obj));
		var rep = {};
		rep.duration = obj.duration;
		if( rep.duration < 0 ) rep.duration = 0;
		rep.tags = obj.tags;
		rep.artist = obj.user_name;
		rep.time = obj.time;

		for( var t = 0; t < obj.tags.length; t++ )
			if( reps_tags.indexOf( obj.tags[t] ) == -1 )
				reps_tags.push( obj.tags[t] )

		reps_arts.push( obj.user_name);

		reports.push( rep);
	}

	for( var t = 0; t < reps_tags.length; t++ )
		if( RULES.status.tags.indexOf( reps_tags[t] ) == -1 )
			RULES.status.tags.push( reps_tags[t] )

	for( var t = 0; t < reps_arts.length; t++ )
		if( RULES.status.artists.indexOf( reps_arts[t] ) == -1 )
			RULES.status.artists.push( reps_arts[t] )

	RULES.status.reports = reports;
	st_Save();
	st_Show( RULES.status);
}

Comment.prototype.processUploads = function()
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
	this.obj.uploads = uploads;
}

Comment.prototype.showFile = function( i_path, i_file)
{
	var el = document.createElement('div');
	this.el.appendChild( el);
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

Comment.prototype.getLink = function( i_absolute)
{
	return g_GetLocationArgs({"cm_Goto":this.obj.key}, i_absolute);
}

function cm_Goto( i_key)
{
	c_Log('cm_Goto: ' + i_key);
//	console.log('cm_Goto: ' + i_key);

	if( i_key == null ) return;

	// This function is async, but this works.
	// As post function will be called once more
	//  after comments will be received.
	if( localStorage['view_comments'] !== 'true' )
		u_OpenCloseView( 'comments', true, true);

	var cm = null;

	for( var i = 0; i < cm_array.length; i++)
		if( cm_array[i].obj.key == i_key )
			cm = cm_array[i];
		else
			cm_array[i].el.classList.remove('goto');

	if( cm )
	{
		cm.el.scrollIntoView();
		cm.el.classList.add('goto');
		c_Info('Comment highlighted.');
	}
	else if( cm_array.length )
		c_Error('Comment with key=' + i_key + ' not found.');
}

