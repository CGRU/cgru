/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	comments.js - TODO: description
*/

"use strict";

var cm_file = 'comments.json';
var cm_durations = [
	'.25', '.5', '1',  '1.5', '2',  '3',  '4',  '5',  '6',  '7',  '8',
	'9',   '10', '12', '14',  '16', '18', '20', '24', '32', '40', '80'
];
var cm_array = [];

var cm_process_image = null;

function View_comments_Open()
{
	cm_Load();
}

function cm_Finish()
{
	cm_array = [];
}

function cm_Load()
{
	$('comments').textContent = '';
	if (ASSET && ASSET.comments_reversed)
	{
		var el = $('comments_btn_add');
		$('comments_show').removeChild(el);
		$('comments_show').appendChild(el);
	}

	cm_array = [];

	if (false == c_RuFileExists(cm_file))
		return;

	$('comments').textContent = 'Loading...';

	n_GetFile({
		"path": c_GetRuFilePath(cm_file),
		"func": cm_Received,
		"cache_time": RULES.cache_time,
		"info": 'comments',
		"parse": true,
		"local": true
	});
}

function cm_Received(i_data)
{
	$('comments').textContent = '';
	if (i_data == null)
		return;
	if (i_data.comments == null)
	{
		c_Error('Invalid comments data received.');
		c_Log(JSON.stringify(i_data));
		return;
	}

	var obj_array = [];
	for (var key in i_data.comments)
	{
		i_data.comments[key].key = key;
		obj_array.push(i_data.comments[key]);
	}

	obj_array.sort(function(a, b) {
		if (a.key < b.key)
			return -1;
		if (a.key > b.key)
			return 1;
		return 0;
	});

	var i = 0;
	for (var i = 0; i < obj_array.length; i++)
		cm_array.push(new Comment(obj_array[i]));

	g_POST('comments');
}

function cm_NewOnClick(i_text)
{
	var comment = new Comment();
	if (i_text)
		comment.elText.innerHTML = i_text;
	comment.edit();
}

function cm_ColorOnclick(i_clr, i_data)
{
	i_data.setColor(i_clr);
}

function cm_Goto(i_key)
{
	c_Log('cm_Goto: ' + i_key);
	//	console.log('cm_Goto: ' + i_key);

	if (i_key == null)
		return;

	// This function is async, but this works.
	// As post function will be called once more
	//  after comments will be received.
	if (localStorage['view_comments'] !== 'true')
		u_OpenCloseView('comments', true, true);

	var cm = null;

	for (var i = 0; i < cm_array.length; i++)
		if (cm_array[i].obj.key == i_key)
			cm = cm_array[i];
		else
			cm_array[i].el.classList.remove('goto');

	if (cm)
	{
		cm.el.scrollIntoView();
		cm.el.classList.add('goto');
		c_Info('Comment highlighted.');
	}
	else if (cm_array.length)
		c_Error('Comment with key=' + i_key + ' not found.');
}


function Comment(i_obj)
{
	// window.console.log( JSON.stringify( i_obj));
	this.el = document.createElement('div');
	if (ASSET && ASSET.comments_reversed)
		$('comments').appendChild(this.el);
	else
		$('comments').insertBefore(this.el, $('comments').firstChild);
	this.el.classList.add('comment');
	this.el.m_comment = this;

	this.elPanel = document.createElement('div');
	this.el.appendChild(this.elPanel);
	this.elPanel.classList.add('panel');

	this.elEdit = document.createElement('div');
	this.elPanel.appendChild(this.elEdit);
	this.elEdit.classList.add('button');
	this.elEdit.classList.add('edit');
	this.elEdit.title = 'Edit comment';
	this.elEdit.onclick = function(e) { e.currentTarget.m_comment.edit(); };
	this.elEdit.m_comment = this;

	this.elEditBtnsDiv = document.createElement('div');
	this.elPanel.appendChild(this.elEditBtnsDiv);
	this.elEditBtnsDiv.classList.add('edit_btns_div');

	this.elCancel = document.createElement('div');
	this.elEditBtnsDiv.appendChild(this.elCancel);
	this.elCancel.classList.add('button');
	this.elCancel.textContent = 'Cancel';
	this.elCancel.title = 'Cancel comment editing.';
	this.elCancel.m_comment = this;
	this.elCancel.onclick = function(e) { e.currentTarget.m_comment.editCancel(); };

	this.elSave = document.createElement('div');
	this.elEditBtnsDiv.appendChild(this.elSave);
	this.elSave.classList.add('button');
	this.elSave.innerHTML = '<b>Save</b> <small>(CTRL+ENTER)</small>';
	this.elSave.title = 'Save comment.\n(CTRL+ENTER)';
	this.elSave.m_comment = this;
	this.elSave.onclick = function(e) { e.currentTarget.m_comment.save(); };

	this.elRemMU = document.createElement('div');
	this.elEditBtnsDiv.appendChild(this.elRemMU);
	this.elRemMU.classList.add('button');
	this.elRemMU.textContent = 'Remove all markup';
	this.elRemMU.title = 'Double click to remove all markup from comment.';
	this.elRemMU.m_comment = this;
	this.elRemMU.ondblclick = function(e) { c_elMarkupRemove(e.currentTarget.m_comment.elText) };

	this.elDel = document.createElement('div');
	this.elEditBtnsDiv.appendChild(this.elDel);
	this.elDel.classList.add('button');
	this.elDel.textContent = 'Delete';
	this.elDel.title = 'Double click to delete comment.';
	this.elDel.ondblclick = function(e) { e.currentTarget.m_comment.destroy(); };
	this.elDel.m_comment = this;

	this.elType = document.createElement('a');
	this.elPanel.appendChild(this.elType);
	this.elType.classList.add('tag');
	this.elType.classList.add('type');
	this.elType.style.cssFloat = 'left';

	this.elAvatar = document.createElement('img');
	this.elAvatar.classList.add('avatar');
	this.elPanel.appendChild(this.elAvatar);

	this.elUser = document.createElement('div');
	this.elUser.classList.add('user');
	this.elPanel.appendChild(this.elUser);

	this.elReport = document.createElement('div');
	this.elPanel.appendChild(this.elReport);
	//	this.elReport.style.display = 'none';
	this.elReport.classList.add('report');

	this.elDuration = document.createElement('div');
	this.elDuration.classList.add('duration');
	this.elReport.appendChild(this.elDuration);

	this.elTags = document.createElement('div');
	this.elTags.classList.add('tags');
	this.elReport.appendChild(this.elTags);

	this.elDate = document.createElement('div');
	this.elDate.classList.add('date');
	this.elPanel.appendChild(this.elDate);

	this.elInfo = document.createElement('div');
	this.elInfo.classList.add('info');
	this.elPanel.appendChild(this.elInfo);

	this.elText = document.createElement('div');
	this.el.appendChild(this.elText);
	this.elText.classList.add('text');
	this.elText.m_obj = this;
	this.elText.onkeydown = function(e) { e.currentTarget.m_obj.textOnKeyDown(e); };

	this.elForEdit = document.createElement('div');
	this.el.appendChild(this.elForEdit);
	this.elForEdit.classList.add('edit');

	this.elUploads = document.createElement('div');
	this.el.appendChild(this.elUploads);
	this.elUploads.classList.add('uploads');
	this.elUploads.style.display = 'none';

	this.elSignature = document.createElement('div');
	this.el.appendChild(this.elSignature);
	this.elSignature.classList.add('signature');

	this.obj = i_obj;
	this.init();
}

Comment.prototype.init = function() {
	this.elTags.textContent = '';
	this.elForEdit.innerHTML = '';
	this.editing = false;
	this.el.classList.remove('edit');
	this.elEditBtnsDiv.style.display = 'none';

	this.elText.contentEditable = 'false';
	this.elText.removeEventListener('paste', cm_OnPaste);
	this.elText.removeEventListener('dragenter', cm_OnDragEnter);
	this.elText.removeEventListener('dragover',  cm_OnDragOver);
	this.elText.removeEventListener('dragleave', cm_OnDragLeave);
	this.elText.removeEventListener('drop', cm_OnDrop);
	this.elText.classList.remove('editing');
	if (localStorage.text_color && (localStorage.text_color != ''))
		this.elText.style.color = localStorage.text_color;
	else
		this.elText.style.color = u_textColor;
	if (localStorage.back_comments && (localStorage.back_comments != ''))
		this.elText.style.background = localStorage.back_comments;
	else if (localStorage.background && (localStorage.background != ''))
		this.elText.style.background = localStorage.background;
	else
		this.elText.style.background = u_background;


	if (this.obj == null)
	{
		this.obj = {};
		this.obj.ctime = (new Date()).getTime();
		this._new = true;
		if (g_auth_user)
		{
			this.obj.user_name = g_auth_user.id;
			if (g_auth_user.tag && g_auth_user.tag.length)
			{
				this.obj.tags = [g_auth_user.tag];
			}
		}
	}

	var user = null;
	var avatar = null;
	var signature = null;

	// Get user object:
	if (this.obj.user_name && g_users[this.obj.user_name])
		user = g_users[this.obj.user_name];
	else if (this.obj.guest)
		user = this.obj.guest;
	if (user == null)
		user = {};

	if (this.obj.user_name)
		this.elUser.textContent = c_GetUserTitle(this.obj.user_name, this.obj.guest);

	// Signature:
	if (user.signature)
		this.elSignature.textContent = user.signature;

	// console.log( g_auth_user.id + ' ' + this.obj.user_name );
	if (g_auth_user)
	{
		// Edit button only for admins or a comment owner:
		if (g_admin || (this.obj && (this.obj.user_name == g_auth_user.id)))
			this.elEdit.style.display = 'block';
		else
			this.elEdit.style.display = 'none';

		// If this is a new comment or and own old:
		if ((this.obj == null) || (this.obj.user_name == g_auth_user.id))
			this.el.classList.add('own');
	}
	else
		this.elEdit.style.display = 'none';

	avatar = c_GetAvatar(this.obj.user_name, this.obj.guest);
	if (avatar != null)
	{
		this.elAvatar.src = avatar;
		this.elAvatar.style.display = 'block';
	}
	else
		this.elAvatar.style.display = 'none';

	this.setElType(this.obj.type);
	if (this.obj.key)
		this.elType.href = this.getLink();

	this.type = this.obj.type;

	if (this.obj.tags && this.obj.tags.length)
	{
		for (var i = 0; i < this.obj.tags.length; i++)
		{
			var tag = this.obj.tags[i];

			var el = document.createElement('div');
			this.elTags.appendChild(el);
			el.classList.add('tag');

			if (RULES.tags[tag] && RULES.tags[tag].title)
				el.textContent = RULES.tags[tag].title;
			else
				el.textContent = tag;
		}
	}

	this.elDate.textContent = c_DT_StrFromMSec(this.obj.ctime);
	if (this.obj.duration && this.obj.duration > 0)
		this.elDuration.textContent = this.obj.duration;

	var info = '';

	// Email is shown for admins only:
	if (g_admin && this.obj && this.obj.guest && this.obj.guest.email)
		info += 'Guest email: ' + c_EmailDecode(this.obj.guest.email);

	if (this.obj.mtime)
	{
		var date = c_DT_StrFromMSec(this.obj.mtime);
		if (info.length)
			info += '<br>';
		info += 'Modified: ' + c_GetUserTitle(this.obj.muser_name) + ' ' + date;
	}

	this.elInfo.innerHTML = info;

	if (this.obj.text)
		this.elText.innerHTML = this.obj.text;

	if (this.obj.uploads && (this.uploads_created != true))
	{
		//console.log(JSON.stringify(this.obj.uploads));
		this.uploads_created = true;
		this.elUploads.style.display = 'block';
		for (let i = 0; i < this.obj.uploads.length; i++)
		{
			let up = this.obj.uploads[i];

			let el = document.createElement('div');
			this.elUploads.appendChild(el);
			el.classList.add('path');

			c_CreateOpenButton(el, up.path);

			let elLink = document.createElement('a');
			el.appendChild(elLink);
			elLink.href = '#' + up.path;
			let dir = up.path.replace(g_CurPath(), '');
			if (dir[0] == '/')
				dir = dir.substr(1);
			elLink.textContent = dir;
			elLink.textContent = up.path;

			for (let f = 0; f < up.files.length; f++)
				this.showFile(el, up.path, up.files[f]);
		}
	}

	this.color = this.obj.color;

	st_SetElColor({"color": this.color}, this.el, null, false);

	if (this.obj.deleted)
		this.el.style.display = 'none';
};

Comment.prototype.setElType = function(i_type) {
	for (var type in RULES.comments)
		this.el.classList.remove(type);

	if (i_type)
	{
		this.el.classList.add(i_type);

		if (RULES.comments[i_type])
		{
			this.elType.textContent = RULES.comments[i_type].title;
			st_SetElColor({"color": RULES.comments[i_type].color}, this.el);
		}
		else
		{
			this.elType.textContent = i_type;
			this.el.style.color = 'inherit';
			st_SetElColor(null, this.el);
		}
	}
	else
	{
		this.elType.textContent = 'Comment';
		st_SetElColor(null, this.el);
	}
};

Comment.prototype.edit = function() {
	if (this._new != true)
	{
		if (g_auth_user == null)
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

	this.elEditPanel = u_EditPanelCreate(this.elForEdit);

	this.elEditTypesDiv = document.createElement('div');
	this.elForEdit.appendChild(this.elEditTypesDiv);
	this.elEditTypesDiv.classList.add('types');
	//	this.elEditTypesDiv.style.clear = 'both';
	for (var type in RULES.comments)
	{
		var el = document.createElement('div');
		this.elEditTypesDiv.appendChild(el);
		el.classList.add('tag');
		el.textContent = RULES.comments[type].title;
		el.m_type = type;
		el.m_comment = this;
		el.onclick = function(e) { e.currentTarget.m_comment.setType(e.currentTarget.m_type); };
		st_SetElColor({"color": RULES.comments[type].color}, el);
	}

	this.elReportEdit = document.createElement('div');
	this.elForEdit.appendChild(this.elReportEdit);
	this.elReportEdit.classList.add('report');

	this.elEditTags = document.createElement('div');
	this.elReportEdit.appendChild(this.elEditTags);
	this.elEditTags.classList.add('list');
	this.elEditTags.classList.add('tags');

	var el = document.createElement('div');
	this.elEditTags.appendChild(el);
	el.textContent = 'Tags:';
	el.classList.add('label');

	this.elEditTags.m_elTags = [];
	for (var tag in RULES.tags)
	{
		var el = document.createElement('div');
		this.elEditTags.appendChild(el);
		el.classList.add('tag');
		el.m_tag = tag;
		el.onclick = c_ElToggleSelected;

		if (RULES.tags[tag].title)
			el.textContent = RULES.tags[tag].title;
		else
			el.textContent = tag;

		if (this.obj.tags)
			if (this.obj.tags.indexOf(tag) != -1)
				c_ElSetSelected(el, true);

		this.elEditTags.m_elTags.push(el);
	}

	var elDurationDiv = document.createElement('div');
	this.elReportEdit.appendChild(elDurationDiv);
	elDurationDiv.classList.add('edit_duration');

	var elDurationLabel = document.createElement('div');
	elDurationDiv.appendChild(elDurationLabel);
	elDurationLabel.textContent = 'Duration:';

	this.elEditDuration = document.createElement('div');
	elDurationDiv.appendChild(this.elEditDuration);
	this.elEditDuration.classList.add('editing');
	this.elEditDuration.contentEditable = 'true';
	if (this.obj.duration)
		this.elEditDuration.textContent = this.obj.duration;

	for (var i = 0; i < cm_durations.length; i++)
	{
		var el = document.createElement('div');
		elDurationDiv.appendChild(el);
		el.classList.add('sample');
		el.textContent = cm_durations[i];
		el.m_elDrtn = this.elEditDuration;
		el.onclick = function(e) { e.currentTarget.m_elDrtn.textContent = e.currentTarget.textContent; }
	}

	this.elColor = document.createElement('div');
	this.elForEdit.appendChild(this.elColor);
	u_DrawColorBars({"el": this.elColor, "onclick": cm_ColorOnclick, "data": this});

	if (g_auth_user == null)
		u_GuestAttrsDraw(this.elForEdit);

	this.elText.classList.add('editing');
	this.elText.style.backgroundColor = '#DDDDDD';
	this.elText.style.color = '#000000';

	this.elText.contentEditable = 'true';
	this.elText.addEventListener('paste', cm_OnPaste);
	this.elText.addEventListener('dragenter', cm_OnDragEnter);
	this.elText.addEventListener('dragover',  cm_OnDragOver);
	this.elText.addEventListener('dragleave', cm_OnDragLeave);
	this.elText.addEventListener('drop', cm_OnDrop);

	this.elText.focus();
};

Comment.prototype.setColor = function(i_clr) {
	this.color = i_clr;
	this.setElType(this.type);
	st_SetElColor({"color": this.color}, this.el, null, false);
};

Comment.prototype.setType = function(i_type) {
	this.type = i_type;
	this.setElType(i_type);
	st_SetElColor({"color": this.color}, this.el, null, false);
};

Comment.prototype.editCancel = function() {
	if (this._new)
		$('comments').removeChild(this.el);
	else
		this.init();
};

Comment.prototype.textOnKeyDown = function(i_e) {
	if (this.editing)
	{
		if ((i_e.keyCode == 13) && i_e.ctrlKey)  // CTRL + ENTER
		{
			this.save();
			this.elText.blur();
		}
	}
};

Comment.prototype.destroy = function() {
	this.obj.deleted = true;
	this.save();
};

Comment.prototype.save = function() {
	if (g_auth_user == null)
	{
		this.obj.guest = u_GuestAttrsGet(this.elForEdit);
		this.obj.user_name = this.obj.guest.id;
	}

	this.obj.text = c_LinksProcess(this.elText.innerHTML);
	this.obj.color = this.color;
	this.obj.type = this.type;
	if (this.obj.deleted != true)
		this.processUploads();

	if (this.obj.type == 'report')
	{
		this.obj.tags = [];
		for (var i = 0; i < this.elEditTags.m_elTags.length; i++)
		{
			var el = this.elEditTags.m_elTags[i];
			if (el.classList.contains('selected'))
				this.obj.tags.push(el.m_tag);
		}
	}
	else
		delete this.obj.tags;

	this.obj.duration = -1;
	var duration = parseFloat(this.elEditDuration.textContent);
	if (false == isNaN(duration))
		this.obj.duration = duration;

	if (this._new)
	{
		this._new = false;
		cm_array.push(this);
	}
	else
	{
		this.obj.mtime = (new Date()).getTime();
		this.obj.muser_name = g_auth_user.id;
	}

	var key = this.obj.ctime + '_' + this.obj.user_name;

	this.obj.key = key;
	this.init();

	var file = c_GetRuFilePath(cm_file);
	n_GetFileFlushCache(file);

	var comments = {};
	comments[key] = this.obj;
	var edit = {};
	edit.object = {"comments": comments};
	edit.add = true;
	edit.file = file;

	n_Request({"send": {"editobj": edit}, "func": this.saveFinished, "this": this});
};

Comment.prototype.saveFinished = function(i_data, i_args) {
	if (c_NullOrErrorMsg(i_data))
		return;

	var news_user = i_args.this.obj.user_name;
	if (i_args.this.obj.muser_name)
		news_user = i_args.this.obj.muser_name;

	var news_title = 'comment';
	if (i_args.this.obj.type == 'report')
		news_title = 'report';

	nw_MakeNews({
		"title": news_title,
		"link": i_args.this.getLink(),
		"user": news_user,
		"guest": i_args.this.obj.guest
	});

	i_args.this.updateStatus();

	i_args.this.sendEmails();
};

Comment.prototype.sendEmails = function() {
	var emails = [];
	if (RULES.status && RULES.status.body && RULES.status.body.guest && RULES.status.body.guest.email)
		emails.push(RULES.status.body.guest.email);
	for (var i = 0; i < cm_array.length; i++)
	{
		var cm = cm_array[i].obj;
		if (cm.guest && cm.guest.email && cm.guest.email.length && (emails.indexOf(cm.guest.email) == -1))
			emails.push(cm.guest.email);
	}

	for (var i = 0; i < emails.length; i++)
	{
		var email = c_EmailDecode(emails[i]);
		if (false == c_EmailValidate(email))
			continue;
		var subject = 'RULES Comment: ' + g_CurPath();
		var href = this.getLink(true);
		var body = '<a href="' + href + '" target="_blank">' + href + '</a>';
		body += '<br><br>';
		body += this.obj.text;
		body += '<br><br>';
		var user = c_GetUserTitle(this.obj.user_name, this.obj.guest);
		body += user;
		if (user != this.obj.user_name)
			body += ' [' + this.obj.user_name + ']';

		n_SendMail(email, subject, body);
	}
};

Comment.prototype.updateStatus = function() {
	var reports = [];

	if (RULES.status == null)
		RULES.status = {};

	if (RULES.status.tags == null)
		RULES.status.tags = [];

	if (RULES.status.artists == null)
		RULES.status.artists = [];

	var reps_tags = [];
	var reps_arts = [];

	// Collect reports for status:
	for (var i = 0; i < cm_array.length; i++)
	{
		var obj = cm_array[i].obj;

		if (obj.deleted)
			continue;
		if (obj.type !== 'report')
			continue;
		if (obj.duration == null)
			continue;

		var rep = {};
		rep.duration = obj.duration;
		if (rep.duration < 0)
			rep.duration = 0;
		rep.tags = obj.tags;
		rep.artist = obj.user_name;
		rep.time = obj.time;

		reports.push(rep);
	}
/*
	// If this is a report, we add artist and tags to current status
	if ((this.obj.type == 'report') && (this.obj.deleted !== true))
	{
		// Add tags:
		if (this.obj.tags && this.obj.tags.length)
			for (let tag of this.obj.tags)
				if (RULES.status.tags.indexOf(tag) == -1)
					RULES.status.tags.push(tag);

		// Add artist:
		if (this.obj.user_name && this.obj.user_name.length)
			if (RULES.status.artists.indexOf(this.obj.user_name) == -1)
				RULES.status.artists.push(this.obj.user_name);
	}
*/
	RULES.status.reports = reports;
	st_Save();
	st_Show(RULES.status);
};

Comment.prototype.processUploads = function() {
	var upfiles = [];
	for (let i = 0; i < up_elFiles.length; i++)
	{
		let el = up_elFiles[i];
		if (el.m_selected != true)
			continue;
		if (el.m_uploading == true)
			continue;

		up_Start(el);

		let path = c_PathDir(el.m_uppath);
		let file = {};
		file.name = el.m_upfile.name;
		file.size = el.m_upfile.size;
		upfiles.push({"path": path, "file": file});
	}

	if (upfiles.length == 0)
		return;

	upfiles.sort(function(a, b) {
		if (a.path < b.path)
			return -1;
		if (a.path > b.path)
			return 1;
		return 0;
	});

	var uploads = [];
	var u = -1;
	for (var f = 0; f < upfiles.length; f++)
	{
		if ((u == -1) || (upfiles[f].path != uploads[u].path))
		{
			uploads.push({});
			u++;
			uploads[u].path = upfiles[f].path;
			uploads[u].files = [];
		}
		uploads[u].files.push(upfiles[f].file);
	}

	this.obj.uploads = uploads;
};

Comment.prototype.showFile = function(i_el, i_path, i_file) {
	var el = document.createElement('div');
	i_el.appendChild(el);
	el.classList.add('file');
/*
	var elThumb = document.createElement('img');
	el.appendChild(elThumb);
	elThumb.classList.add('thumbnail');
	elThumb.src = RULES.root + c_GetThumbFileName(i_path + '/' + i_file.name);
*/
	var elSize = document.createElement('div');
	el.appendChild(elSize);
	elSize.classList.add('size');
	elSize.textContent = c_Bytes2KMG(i_file.size);

	var elLink = document.createElement('a');
	el.appendChild(elLink);
	elLink.classList.add('link');
	elLink.textContent = i_file.name;
	elLink.target = '_blank';
	elLink.href = RULES.root + i_path + '/' + i_file.name;
};

Comment.prototype.getLink = function(i_absolute) {
	return g_GetLocationArgs({"cm_Goto": this.obj.key}, i_absolute);
};


function cm_OnPaste(i_evt)
{
    i_evt.preventDefault();
	if (cm_process_image)
		return;

	// Process text
	let text = (i_evt.clipboardData || window.clipboardData).getData('text/plain');
	text.replace('\n','<br>\n');
	if (text && text.length)
		document.execCommand('insertHTML', false, text);

	cm_ProcessImageDataTransfer(i_evt.clipboardData);
}

function cm_OnDragEnter(i_evt){cm_DragSetStyle(i_evt, true);}
function cm_OnDragOver( i_evt){cm_DragSetStyle(i_evt, true);}
function cm_OnDragLeave(i_evt){cm_DragSetStyle(i_evt, false);}
function cm_DragSetStyle(i_evt, i_on)
{
//console.log(i_evt);
	// Preventing defaults means that element accept drop
	i_evt.preventDefault();
	i_evt.stopPropagation();

	// This can be not element but element text node
	let el = i_evt.target;
	if (! el.classList)
		return;

	// Child element can accept this event too,
	// so we should search text editing element.
	while ((false == el.classList.contains('editing')) && el.parentElement)
	{
		// We are already at the top of the comment,
		// we should exit in this case.
		el.classList.contains('comment');
			return;

		el = el.parentElement;
	}

	// Add or remove drag class style
	if (i_on)
		el.classList.add('drag');
	else
		el.classList.remove('drag');
}
function cm_OnDrop(i_evt)
{
	cm_DragSetStyle(i_evt, false);

	if (cm_process_image)
		return;

	cm_ProcessImageDataTransfer(i_evt.dataTransfer);
}

function cm_ProcessImageDataTransfer(i_data)
{
//console.log(i_data);
	if (null == i_data)
	{
		c_Error('Transfered data is null.');
		return;
	}
	if (null == i_data.items)
	{
		c_Error('Transfered data has null items.');
		return;
	}
	if (i_data.items.length == 0)
	{
		c_Error('Transfered data has zero items.');
		return;
	}

	let file = null;
	for (let i = 0; i < i_data.items.length; i++)
	{
		let item = i_data.items[i];
		if (item.type.indexOf('image') == -1)
			continue;

		if (file)
		{
			c_Error('You can paste only one image at once.');
			return;
		}

		file = item.getAsFile();
	}

	if (file)
		cm_ProcessImage(file);
}

function cm_ProcessImage(i_file)
{
	//console.log(i_file);
	let name = (new Date()).toISOString().replace(/[:.Z]/g,'-') +  g_auth_user.id + '-' + i_file.name;
	let path = c_GetRuFilePath(name);

	let elRoot = document.createElement('div');
	document.body.appendChild(elRoot);
	elRoot.classList.add('insert_image_root');

	let elImg = document.createElement('img');
	elRoot.appendChild(elImg);
	elImg.classList.add('insert_image_img');
	elImg.src = URL.createObjectURL(i_file);
	elImg.m_file = i_file;
	elImg.onload = cm_ProcessImageOnLoad;

	let elWnd = document.createElement('div');
	elRoot.appendChild(elWnd);
	elWnd.classList.add('insert_image_wnd');

	let elInfo = document.createElement('div');
	elWnd.appendChild(elInfo);
	elInfo.classList.add('insert_image_info');
	elInfo.innerHTML = i_file.name + ': ' + c_Bytes2KMG(i_file.size);
	elInfo.title = path;

	let elStatus = document.createElement('div');
	elWnd.appendChild(elStatus);
	elStatus.classList.add('insert_image_status');

	let btn_cancel = document.createElement('div');
	elWnd.appendChild(btn_cancel);
	btn_cancel.textContent = 'Cancel';
	btn_cancel.classList.add('button');
	btn_cancel.classList.add('insert_image_cancel');
	btn_cancel.onclick = cm_ProcessImageClose;

	let btn_upload = document.createElement('div');
	elWnd.appendChild(btn_upload);
	btn_upload.textContent = 'Upload Image';
	btn_upload.classList.add('button');
	btn_upload.classList.add('insert_image_upload');
	btn_upload.onclick = cm_ProcessImageUpload;

	let elProgress = document.createElement('div');
	elWnd.appendChild(elProgress);
	elWnd.style.background = u_background;
	elProgress.classList.add('insert_image_progress');

	cm_process_image = {};
	cm_process_image.el_root = elRoot;
	cm_process_image.file = i_file;
	cm_process_image.path = path;
	cm_process_image.btn_cancel = btn_cancel;
	cm_process_image.btn_upload = btn_upload;
	cm_process_image.el_info = elInfo;
	cm_process_image.el_status = elStatus;
	cm_process_image.el_progress = elProgress;
	cm_process_image.up_percent = -1;
	cm_process_image.uploading = false;
}

function cm_ProcessImageOnLoad()
{
	cm_process_image.width = this.naturalWidth;
	cm_process_image.height = this.naturalHeight;

	var info = cm_process_image.file.name + ':';
	info += ' ' + this.naturalWidth + 'x' + this.naturalHeight;
	info += ' ' + c_Bytes2KMG(cm_process_image.file.size);

	cm_process_image.el_info.innerHTML = info;
}

function cm_ProcessImageUpload()
{
	cm_process_image.uploading = true;
	cm_process_image.btn_cancel.style.display = 'none';
	cm_process_image.btn_upload.style.display = 'none';
	cm_process_image.up_time = (new Date() / 1000);

	var formData = new FormData();
	formData.append('upload_path', cm_process_image.path);
	formData.append('upload_file', cm_process_image.file);

	var xhr = new XMLHttpRequest();
	xhr.upload.addEventListener('progress', cm_ProcessImageUploadProgress, false);
	xhr.addEventListener('load',  cm_ProcessImageUpload_Load,  false);
	xhr.addEventListener('error', cm_ProcessImageUpload_Error, false);
	xhr.addEventListener('abort', cm_ProcessImageUpload_Abort, false);
	xhr.open('POST', n_server);
	xhr.send(formData);

	xhr.onreadystatechange = function() {
		if (xhr.readyState == 4)
		{
			if (xhr.status == 200)
			{
				c_Log('<b style="color:#404"><i>upload:</i></b> ' + xhr.responseText);
				cm_ProcessImageUploadFinished(c_Parse(xhr.responseText));
				return;
			}
		}
	};
}

function cm_ProcessImageUpload_Load()
{
	if (null == cm_process_image) return;
	cm_process_image.el_status.innerHTML = 'Uploading';
}
function cm_ProcessImageUpload_Error()
{
	if (null == cm_process_image) return;
	cm_process_image.el_status.innerHTML = 'Error';
}
function cm_ProcessImageUpload_Abort()
{
	if (null == cm_process_image) return;
	cm_process_image.el_status.innerHTML = 'Aborting';
}

function cm_ProcessImageUploadProgress(i_evt)
{
	if (null == cm_process_image) return;

	var dur = c_DT_DurFromNow(cm_process_image.up_time);
	var text = dur;

	if (i_evt.lengthComputable)
	{
		if (i_evt.total > 0)
		{
			let percent = Math.round(100 * i_evt.loaded / i_evt.total);
			if (cm_process_image.up_percent > percent)
			{
				c_Error('Upload: New progress lowered (' + cm_process_image.up_percent + ' > ' + percent + ') at ' + dur);
			}
			text += ' ' + percent + '%';
			cm_process_image.el_progress.style.width = percent + '%';
			cm_process_image.up_percent = percent;
		}
	}

	cm_process_image.el_status.innerHTML = text;
}

function cm_ProcessImageUploadFinished(i_args)
{
	if (i_args.files == null)
	{
		c_Error('Uploaded no files.');
		cm_process_image.el_status.innerHTML = 'Uploaded no files.';
		return;
	}
	if (i_args.files.length == 0)
	{
		c_Error('Uploaded zero files.');
		cm_process_image.el_status.innerHTML = 'Uploaded zero files.';
		return;
	}
	if (i_args.files.length > 1)
	{
		c_Error('Uploaded several files at once. Processing only the first one.');
	}

	cm_ProcessImageFileUploaded(i_args.files[0]);
}

function cm_ProcessImageFileUploaded(i_file)
{
	i_file.src = i_file.path;
	if (i_file.size < 200000)
	{
		cm_ProcessImageInsertHTML(i_file);
		return;
	}

	cm_process_image.el_status.innerHTML = 'Creating thumbnail...';

	i_file.thumbnail = i_file.path + '-thumbnail.jpg';

	var cmd = 'rules/bin/convert';
	cmd += ' -verbose';
	cmd += ' "' + i_file.path + '"';
	cmd += ' -quality 85%';
	cmd += ' -thumbnail 1280';
	cmd += ' "' + i_file.thumbnail + '"';

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}}, "func": cm_ProcessImageTumbnailed, "file": i_file, "info":'thumbnail'});
}

function cm_ProcessImageTumbnailed(i_data, i_args)
{
//console.log(JSON.stringify(i_args));
//console.log(JSON.stringify(i_data));
	var file = i_args.file;

	if ((null == i_data.cmdexec) || (i_data.cmdexec.length == 0))
	{
		c_Error('Error executing thumbnail creation.');
	}
	else if (i_data.cmdexec[0].search(/error/gi) != -1)
	{
		c_Error(i_data.cmdexec[0]);
	}
	else
	{
		file.src = file.thumbnail;
	}

	cm_ProcessImageInsertHTML(file);
}

function cm_ProcessImageInsertHTML(i_file)
{
	var info = i_file.name + ':';
	info += ' ' + cm_process_image.width + 'x' + cm_process_image.height;
	info += ' ' + c_Bytes2KMG(i_file.size);

	var html = '';
	html += '<div class="comment_inserted_image_div">'
	html += '<a target="_blank"';
	html += ' href="' + i_file.path + '"';
	html += '>';
	html += '<span class="comment_inserted_image_info">' + info + '</span>';
	html += '<br>';
	html += '<img class="comment_inserted_image"';
	html += ' src="' + i_file.src + '"';
	html += ' alt="' + i_file.name + '"';
	if (i_file.thumbnail || cm_process_image.width > 600)
		html += ' width=90%';
	html += '>';
	html += '</a>';
	html += '</div>'
	html += '<div></div>'

	document.execCommand('insertHTML', false, html);

	cm_ProcessImageClose();
}

function cm_ProcessImageClose()
{
	if (null == cm_process_image) return;

	let elParent = cm_process_image.el_root.parentElement;
	if (elParent)
		elParent.removeChild(cm_process_image.el_root);

	cm_process_image = null;
}
