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

var cm_elStat = null;

var cm_filter_tags = []

function cm_Init()
{
	let array = c_Parse(localStorage.comments_filter_tags);
	if (null == array)
		return;

	for (let item of array)
		cm_filter_tags.push(item);
}

function View_comments_Open()
{
	cm_Load();
}

function cm_Finish()
{
	cm_array = [];

	if (cm_elStat)
	{
		cm_elStat.textContent = '';
		cm_elStat = null;
	}
}

function cm_Load()
{
	$('comments').textContent = '';
	if (ASSET && ASSET.comments_reversed)
	{
		let el = $('comments_btn_add');
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

	let obj_array = [];
	for (let key in i_data.comments)
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

	let i = 0;
	for (let i = 0; i < obj_array.length; i++)
		cm_array.push(new Comment(obj_array[i]));

	cm_DisplayStat();
	cm_Filter();

	g_POST('comments');
}

function cm_DisplayStat()
{
	if (cm_elStat)
		cm_elStat.textContent = '';
	else
	{
		if (cm_array.length == 0)
			return;

		cm_elStat = document.createElement('div');
		$('comments').insertBefore(cm_elStat, $('comments').firstChild);
		cm_elStat.classList.add('comments_stat');
	}

	let tags_counts = {};

	for (let cm of cm_array)
	{
		if (cm.obj.deleted)
			continue;

		let tags = cm.obj.tags;
		if ((tags == null) || (tags.length == 0))
			continue;

		for (let tag of tags)
		{
			if (tags_counts[tag])
				tags_counts[tag] += 1;
			else
				tags_counts[tag] = 1;
		}
	}

	for (let tag in tags_counts)
	{
		let el = document.createElement('div');
		el.classList.add('tag');
		el.textContent = c_GetTagTitle(tag) + ':' + tags_counts[tag];
		el.m_tag = tag;
		el.onclick = cm_TagClicked;
		el.title = 'This is comments tags count.'
			+ '\n ' + tags_counts[tag] + ' comment(s) has \"' + c_GetTagTitle(tag) + '\" tag.'
			+ '\n Click to filter comments by tag.'
			+ '\n Hold CTRL or SHIFT to select several tags.';
		if (cm_filter_tags.indexOf(tag) != -1)
			el.classList.add('selected');

		cm_elStat.appendChild(el);
	}

	// Display 'empty' tag filters.
	// This can be if some filter was selected in one location,
	// but there is no such comments in another location.
	for (let tag of cm_filter_tags)
	{
		if (tags_counts[tag])
			continue;

		let el = document.createElement('div');
		el.classList.add('tag');
		el.textContent = c_GetTagTitle(tag) + ':0';
		el.m_tag = tag;
		el.onclick = cm_TagClicked;
		el.classList.add('selected');
		el.classList.add('empty');

		cm_elStat.appendChild(el);
	}
}

function cm_TagClicked(i_evt)
{
	let tag = i_evt.currentTarget.m_tag;

	cm_filter_tags = [];
	for (let el of cm_elStat.childNodes)
	{
		if (el.m_tag == tag)
			el.classList.toggle('selected');
		else if ((i_evt.shiftKey == false) && (i_evt.ctrlKey == false))
			el.classList.remove('selected');

		if (el.classList.contains('selected'))
			cm_filter_tags.push(el.m_tag);
	}

	cm_Filter();

	localStorage.comments_filter_tags = JSON.stringify(cm_filter_tags);
}

function cm_Filter()
{
	for (let cm of cm_array)
		cm.filter();
}

function cm_NewOnClick(i_text)
{
	let comment = new Comment();
	if (i_text)
		comment.elText.innerHTML = i_text;
	comment.edit();
}

function cm_ColorOnclick(i_clr, i_data)
{
	i_data.comment.setColor(i_clr);
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

	let cm = null;

	for (let i = 0; i < cm_array.length; i++)
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
	// Translate OLD user vars:
	if (i_obj)
	{
		if (i_obj.user_name)  {i_obj.cuser = i_obj.user_name;  delete i_obj.user_name;}
		if (i_obj.muser_name) {i_obj.muser = i_obj.muser_name; delete i_obj.muser_name;}
	}


	// window.console.log( JSON.stringify( i_obj));
	this.el = document.createElement('div');
	if (ASSET && ASSET.comments_reversed)
		$('comments').appendChild(this.el);
	else
	{
		if (cm_array.length)
			$('comments').insertBefore(this.el, cm_array[cm_array.length-1].el);
		else
			$('comments').appendChild(this.el);
	}
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

	let elTypeTagsDiv = document.createElement('div');
	this.elPanel.appendChild(elTypeTagsDiv);
	elTypeTagsDiv.classList.add('type_tags');

	this.elType = document.createElement('a');
	elTypeTagsDiv.appendChild(this.elType);
//	this.elType.classList.add('tag');
	this.elType.classList.add('type');

	this.elTags = document.createElement('div');
	this.elTags.classList.add('tags');
	elTypeTagsDiv.appendChild(this.elTags);

	this.elAvatar = document.createElement('img');
	this.elAvatar.classList.add('avatar');
	this.elPanel.appendChild(this.elAvatar);

	this.elUser = document.createElement('div');
	this.elUser.classList.add('user');
	this.elPanel.appendChild(this.elUser);

	this.elReport = document.createElement('div');
	this.elPanel.appendChild(this.elReport);
	this.elReport.classList.add('report');

	this.elDuration = document.createElement('div');
	this.elDuration.classList.add('duration');
	this.elReport.appendChild(this.elDuration);

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

	ec_EditingFinish({'el':this.elText});

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
			this.obj.cuser = g_auth_user.id;
			/*
			if (g_auth_user.tag && g_auth_user.tag.length)
			{
				this.obj.tags = [g_auth_user.tag];
			}
			*/
		}
	}

	let user = null;
	let avatar = null;
	let signature = null;

	// Get user object:
	if (this.obj.cuser && g_users[this.obj.cuser])
		user = g_users[this.obj.cuser];
	else if (this.obj.guest)
		user = this.obj.guest;
	if (user == null)
		user = {};

	if (this.obj.cuser)
		this.elUser.textContent = c_GetUserTitle(this.obj.cuser, this.obj.guest);

	// Signature:
	if (user.signature)
		this.elSignature.textContent = user.signature;

	// console.log( g_auth_user.id + ' ' + this.obj.cuser );
	if (g_auth_user)
	{
		// Edit button only for admins or a comment owner:
		if (g_admin || (this.obj && (this.obj.cuser == g_auth_user.id)))
			this.elEdit.style.display = 'block';
		else
			this.elEdit.style.display = 'none';

		// If this is a new comment or and own old:
		if ((this.obj == null) || (this.obj.cuser == g_auth_user.id))
			this.el.classList.add('own');
	}
	else
		this.elEdit.style.display = 'none';

	avatar = c_GetAvatar(this.obj.cuser, this.obj.guest);
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
		for (let i = 0; i < this.obj.tags.length; i++)
		{
			let el = document.createElement('div');
			this.elTags.appendChild(el);
			el.classList.add('tag');
			el.textContent = c_GetTagTitle(this.obj.tags[i]);
		}
	}

	this.elDate.textContent = c_DT_StrFromMSec(this.obj.ctime);
	if (this.obj.duration && this.obj.duration > 0)
		this.elDuration.textContent = this.obj.duration;

	let info = '';

	// Email is shown for admins only:
	if (g_admin && this.obj && this.obj.guest && this.obj.guest.email)
		info += 'Guest email: ' + c_EmailDecode(this.obj.guest.email);

	if (this.obj.mtime)
	{
		let date = c_DT_StrFromMSec(this.obj.mtime);
		if (info.length)
			info += '<br>';
		info += 'Modified: ' + c_GetUserTitle(this.obj.muser) + ' ' + date;
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
	for (let type in RULES.comments)
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

Comment.prototype.filter = function() {
	if (this.obj.deleted)
	{
		this.el.style.display = 'none';
		return;
	}

	this.el.style.display = 'block';

	if (cm_filter_tags.length == 0)
		return;

	this.el.style.display = 'none';

	if ((this.obj.tags == null) || (this.obj.tags.length == 0))
		return;

	for (let tag of cm_filter_tags)
		if (this.obj.tags.indexOf(tag) == -1)
			return;

	this.el.style.display = 'block';
}

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
	for (let type in RULES.comments)
	{
		let el = document.createElement('div');
		this.elEditTypesDiv.appendChild(el);
		el.classList.add('tag');
		el.textContent = RULES.comments[type].title;
		el.m_type = type;
		el.m_comment = this;
		el.onclick = function(e) { e.currentTarget.m_comment.setType(e.currentTarget.m_type); };
		st_SetElColor({"color": RULES.comments[type].color}, el);
	}

	this.elEditTags = document.createElement('div');
	this.elForEdit.appendChild(this.elEditTags);
	this.elEditTags.classList.add('list');
	this.elEditTags.classList.add('tags');

	let el = document.createElement('div');
	this.elEditTags.appendChild(el);
	el.textContent = 'Tags:';
	el.classList.add('label');

	this.elEditTags.m_elTags = [];
	for (let tag in RULES.tags)
	{
		let el = document.createElement('div');
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

	this.elReportEdit = document.createElement('div');
	this.elForEdit.appendChild(this.elReportEdit);
	this.elReportEdit.classList.add('report');

	let elDurationDiv = document.createElement('div');
	this.elReportEdit.appendChild(elDurationDiv);
	elDurationDiv.classList.add('edit_duration');

	let elDurationLabel = document.createElement('div');
	elDurationDiv.appendChild(elDurationLabel);
	elDurationLabel.textContent = 'Duration:';

	this.elEditDuration = document.createElement('div');
	elDurationDiv.appendChild(this.elEditDuration);
	this.elEditDuration.classList.add('editing');
	this.elEditDuration.contentEditable = 'true';
	if (this.obj.duration)
		this.elEditDuration.textContent = this.obj.duration;

	for (let i = 0; i < cm_durations.length; i++)
	{
		let el = document.createElement('div');
		elDurationDiv.appendChild(el);
		el.classList.add('sample');
		el.textContent = cm_durations[i];
		el.m_elDrtn = this.elEditDuration;
		el.onclick = function(e) { e.currentTarget.m_elDrtn.textContent = e.currentTarget.textContent; }
	}

	this.elColor = document.createElement('div');
	this.elForEdit.appendChild(this.elColor);
	u_DrawColorBars({"el": this.elColor, "onclick": cm_ColorOnclick, "data":{"comment":this}});
	// Store that some (or empty) color was chosen.
	// If empty color was clicked, we should reset color by sending an empty array ("color"=[]).
	this.color_changed = false;

	if (g_auth_user == null)
		u_GuestAttrsDraw(this.elForEdit);

	this.elText.style.backgroundColor = '#DDDDDD';
	this.elText.style.color = '#000000';

	ec_EditingStart({'el':this.elText,'form':'comment'});
};

Comment.prototype.setColor = function(i_clr) {
	this.color = i_clr;
	this.color_changed = true;
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
/*
	if (g_auth_user == null)
	{
		this.obj.guest = u_GuestAttrsGet(this.elForEdit);
		this.obj.cuser = this.obj.guest.id;
	}

	this.obj.text = c_LinksProcess(this.elText.innerHTML);
	this.obj.color = this.color;
	this.obj.type = this.type;
	if (this.obj.deleted != true)
		this.processUploads();

	this.obj.tags = [];
	for (let i = 0; i < this.elEditTags.m_elTags.length; i++)
	{
		let el = this.elEditTags.m_elTags[i];
		if (el.classList.contains('selected'))
			this.obj.tags.push(el.m_tag);
	}

	this.obj.duration = -1;
	let duration = parseFloat(this.elEditDuration.textContent);
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
		this.obj.muser = g_auth_user.id;
	}

	let key = this.obj.ctime + '_' + this.obj.cuser;

	this.obj.key = key;
	this.init();

	cm_DisplayStat();

	let file = c_GetRuFilePath(cm_file);
	n_GetFileFlushCache(file);

	let comments = {};
	comments[key] = this.obj;
	let edit = {};
	edit.object = {"comments": comments};
	edit.add = true;
	edit.file = file;

	n_Request({"send": {"editobj": edit}, "func": this.saveFinished, "this": this});
*/
	let obj = {};
	obj.paths = [g_CurPath()];

	if (g_auth_user == null)
	{
		obj.guest = u_GuestAttrsGet(this.elForEdit);
		//this.obj.cuser = this.obj.guest.id;
	}

	obj.text = c_LinksProcess(this.elText.innerHTML);
	obj.ctype = this.type;
	if (this.color_changed) // Some (or empty) color was clicked during edit
	{
		if (this.color)
			obj.color = this.color;
		else
			obj.color = []; // An empty array resets color
	}
	if (this.obj.deleted)
		obj.deleted = true;
	else
	{
		this.processUploads();
		obj.uploads = this.obj.uploads;
	}

	let tags = [];
	for (let i = 0; i < this.elEditTags.m_elTags.length; i++)
	{
		let el = this.elEditTags.m_elTags[i];
		if (el.classList.contains('selected'))
			tags.push(el.m_tag);
	}
	if (tags.length)
		obj.tags = tags;

	let duration = parseFloat(this.elEditDuration.textContent);
	if (false == isNaN(duration))
		obj.duration = duration;

	if (this.obj.key)
		obj.key = this.obj.key;

	if (nw_disabled)
		obj.nonews = true;

	//console.log(JSON.stringify(obj));
	n_Request({'send':{'setcomment':obj},'func':cm_SaveFinished, "comment": this,'info':'setComment','wait':false});
};

function cm_SaveFinished(i_data, i_args)
{
	//console.log(JSON.stringify(i_data));

	if (c_NullOrErrorMsg(i_data))
		return;

	let comment = i_args.comment;
	comment.saveFinished(i_data);

	// Get news if subscribed:
	if (i_data.users_subscribed && (i_data.users_subscribed.indexOf(g_auth_user.id) != -1))
	{
		nw_NewsLoad();
	}
}

Comment.prototype.saveFinished = function(i_data)
{
	if (i_data.comments == null)
	{
		c_Error('No comments created.');
		return;
	}

    let obj = i_data.comments[g_CurPath()];
	if (obj == null)
	{
		c_Error('No new comment created.');
		return;
	}

	this.obj = obj;

	if (this._new)
	{
		this._new = false;
		cm_array.push(this);
	}

	this.init();

	cm_DisplayStat();
/*
	let news_user = i_args.this.obj.cuser;
	if (i_args.this.obj.muser)
		news_user = i_args.this.obj.muser;

	let news_title = 'comment';
	if (i_args.this.obj.type == 'report')
		news_title = 'report';

	nw_MakeNews({
		"title": news_title,
		"link": i_args.this.getLink(),
		"user": news_user,
		"guest": i_args.this.obj.guest
	});
*/
	this.updateStatus();

	this.sendEmails();
};

Comment.prototype.sendEmails = function() {
	let emails = [];
	if (RULES.status && RULES.status.body && RULES.status.body.guest && RULES.status.body.guest.email)
		emails.push(RULES.status.body.guest.email);
	for (let i = 0; i < cm_array.length; i++)
	{
		let cm = cm_array[i].obj;
		if (cm.guest && cm.guest.email && cm.guest.email.length && (emails.indexOf(cm.guest.email) == -1))
			emails.push(cm.guest.email);
	}

	for (let i = 0; i < emails.length; i++)
	{
		let email = c_EmailDecode(emails[i]);
		if (false == c_EmailValidate(email))
			continue;
		let subject = 'RULES Comment: ' + g_CurPath();
		let href = this.getLink(true);
		let body = '<a href="' + href + '" target="_blank">' + href + '</a>';
		body += '<br><br>';
		body += this.obj.text;
		body += '<br><br>';
		let user = c_GetUserTitle(this.obj.cuser, this.obj.guest);
		body += user;
		if (user != this.obj.cuser)
			body += ' [' + this.obj.cuser + ']';

		n_SendMail(email, subject, body);
	}
};

Comment.prototype.updateStatus = function() {
	let reports = [];

	if (RULES.status == null)
		RULES.status = {};

	if (RULES.status.tags == null)
		RULES.status.tags = [];

	if (RULES.status.artists == null)
		RULES.status.artists = [];

	let reps_tags = [];
	let reps_arts = [];

	// Collect reports for status:
	for (let i = 0; i < cm_array.length; i++)
	{
		let obj = cm_array[i].obj;

		if (obj.deleted)
			continue;
		if (obj.type !== 'report')
			continue;
		if (obj.duration == null)
			continue;

		let rep = {};
		rep.duration = obj.duration;
		if (rep.duration < 0)
			rep.duration = 0;
		rep.tags = obj.tags;
		rep.artist = obj.cuser;
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
		if (this.obj.cuser && this.obj.cuser.length)
			if (RULES.status.artists.indexOf(this.obj.cuser) == -1)
				RULES.status.artists.push(this.obj.cuser);
	}
*/
	RULES.status.reports = reports;
	st_Save();
	st_Show(RULES.status);
};

Comment.prototype.processUploads = function() {
	let upfiles = [];
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

	let uploads = [];
	let u = -1;
	for (let f = 0; f < upfiles.length; f++)
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
	let el = document.createElement('div');
	i_el.appendChild(el);
	el.classList.add('file');
/*
	let elThumb = document.createElement('img');
	el.appendChild(elThumb);
	elThumb.classList.add('thumbnail');
	elThumb.src = RULES.root + c_GetThumbFileName(i_path + '/' + i_file.name);
*/
	let elSize = document.createElement('div');
	el.appendChild(elSize);
	elSize.classList.add('size');
	elSize.textContent = c_Bytes2KMG(i_file.size);

	let elLink = document.createElement('a');
	el.appendChild(elLink);
	elLink.classList.add('link');
	elLink.textContent = i_file.name;
	elLink.target = '_blank';
	elLink.href = RULES.root + i_path + '/' + i_file.name;
};

Comment.prototype.getLink = function(i_absolute) {
	return g_GetLocationArgs({"cm_Goto": this.obj.key}, i_absolute);
};

