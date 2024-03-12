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
	status.js - TODO: description
*/

"use strict";

var st_Status = null;

var st_Statuses = [];

var st_MultiValue = '[...]';

var st_Hilighted = {};
var st_HilightedKeys = ['tag','flag','artist'];

function st_Init()
{
	st_HilightedStoreRead();
	tasks_Init();
}

function st_InitAuth()
{
	$('status_edit_btn').style.display = 'block';
	if (c_CanEditShot())
	{
		$('status_tasks_btn_add').style.display = 'block';
		$('status_tasks_btn_add_artist').style.display = 'block';
	}
}

function st_Finish()
{
	tasks_Finish();

	if (st_Status)
		st_Status.editCancel();

	st_Status = null;
	$('status').classList.remove('fading');
	$('status').classList.remove('updating');

	st_Statuses = [];
}

function st_OnClose()
{
	st_HilightedStoreWrite();
}

function st_HilightedStoreRead()
{
	for (let key of st_HilightedKeys)
	{
		let storageName = 'highlighted_' + key + 's';
		let storageValue = localStorage[storageName];
		st_Hilighted[key] = [];
		if (storageValue && storageValue.length)
			st_Hilighted[key] = storageValue.split(',');
	}
}

function st_HilightedStoreWrite()
{
	for (let key of st_HilightedKeys)
	{
		if (st_Hilighted[key] == null)
			continue;
		let storageName = 'highlighted_' + key + 's';
		let storageValue = st_Hilighted[key].join(',');
		localStorage[storageName] = storageValue;
	}
}

function st_BodyModified(i_st_obj, i_path)
{
	var st_obj = i_st_obj;
	if (st_obj == null)
		st_obj = RULES.status;
	if (st_obj == null)
		st_obj = {};

	if (st_obj.body == null)
	{
		st_obj.body = {};
		st_obj.body.cuser = g_auth_user.id;
		st_obj.body.ctime = c_DT_CurSeconds();
	}
	else
	{
		st_obj.body.muser = g_auth_user.id;
		st_obj.body.mtime = c_DT_CurSeconds();
	}

	st_Save({'body': st_obj.body}, i_path, /*func*/ null, /*args*/ null, /*navig folder update params*/ {});
}

function st_Show(i_status)
{
	if (ASSET && ASSET.subfolders_status_hide && (ASSET.path != g_CurPath()))
	{
		$('status').style.display = 'none';
		return;
	}

	$('status').style.display = 'block';

	/*	if( i_status != null )
			RULES.status = c_CloneObj( i_status);
		else
			i_status = RULES.status;*/
	if (st_Status)
	{
		st_Status.show(i_status);
	}
	else
	{
		st_Status = new Status(i_status);
	}
}

function st_UpdateCurrent(i_status)
{
	RULES.status = i_status;

	if (st_Status)
	{
		st_Status.update(i_status);
	}
	else
	{
		st_Status = new Status(i_status);
	}
}

function st_ReloadFile()
{
	if (st_Status)
		st_Status.reloadFile();
}

function Status(i_obj, i_args)
{
	if (i_args == null)
		i_args = {};

	this.multi = false;
	if (i_args.multi)
		this.multi = true;

	if (i_args.createGUI)
		i_args.createGUI(this);
	else
	{
		this.elParent = $('status');
		this.elShow = $('status_show');
		this.elEditBtn = $('status_edit_btn');
		this.elColor = $('status');
		this.elAnnotation = $('status_annotation');
		this.elAdinfo = $('status_adinfo');
		this.elProgress = $('status_progress');
		this.elProgressBar = $('status_progressbar');
		this.elPercentage = $('status_percentage');
		this.elArtists = $('status_artists');
		this.elFlags = $('status_flags');
		this.elTags = $('status_tags');
		this.elFramesNum = $('status_framesnum');
		this.elFinish = $('status_finish');
		this.elModified = $('status_modified');

		this.elTasks = $('status_tasks');

		this.elReportsDiv = $('status_reports_div');
		this.elReports = $('status_reports');
	}

	if (this.elEditBtn)
	{
		this.elEditBtn.m_status = this;
		this.elEditBtn.onclick = function(e) {
			e.stopPropagation();
			e.currentTarget.m_status.reloadFile(/*edit=*/true);
		};
		if (false == c_CanEditShot())
			this.elEditBtn.style.display = 'none';
	}

	this.path = i_args.path;
	if (this.path == null)
		this.path = g_CurPath();

	this.obj = i_obj;
	this.args = i_args;

	this.show();
}

Status.prototype.show = function(i_status, i_update = false)
{
	if (i_status)
	{
		if (i_update && this.obj)
		{
			for (let k in i_status)
				this.obj[k] = i_status[k];
		}
		else
			this.obj = i_status;
	}

	this.editCancel();

	if (this.elShow)
		this.elShow.style.display = 'block';
	if (this.elColor)
		st_SetElColor(this.obj, this.elColor);
	if (this.elAnnotation)
		st_SetElAnnotation(this.obj, this.elAnnotation);
	if (this.elProgress)
		st_SetElProgress(this.obj, this.elProgressBar, this.elProgress, this.elPercentage);
	if (this.elArtists)
		st_SetElArtists(this.obj, this.elArtists, this.args.display_short, true);
	if (this.elFlags)
		st_SetElFlags(this.obj, this.elFlags, this.args.display_short, true);
	if (this.elTags)
		st_SetElTags(this.obj, this.elTags, this.args.display_short, true);
	if (this.elFramesNum)
		st_SetElFramesNum(this.obj, this.elFramesNum);
	if (this.elFinish)
		st_SetElFinish(this.obj, this.elFinish);
	if (this.elTimeCode && this.obj && this.obj.timecode_start && this.obj.timecode_finish)
	{
		this.elTimeCode.textContent = 'TC:' + this.obj.timecode_start + '-' + this.obj.timecode_finish;
	}
	if (this.elModified)
	{
		var modified = '';
		if (this.obj)
		{
			if (this.obj.muser)
				modified += ' by ' + c_GetUserTitle(this.obj.muser);
			if (this.obj.mtime)
				modified += ' at ' + c_DT_StrFromSec(this.obj.mtime);
			if (modified != '')
				modified = 'Modified' + modified;
		}
		this.elModified.textContent = modified;
	}

	if (this.elAdinfo)
	{
		if (g_admin && this.obj && this.obj.adinfo)
		{
			this.elAdinfo.textContent = atob(this.obj.adinfo);
			this.elAdinfo.style.display = 'block';
		}
		else
		{
			this.elAdinfo.textContent = '';
			this.elAdinfo.style.display = 'none';
		}
	}

	// Global status will be cleaned in tasks_Finish() function.
	// So here we should clean multi statuses (scene shots view).
	if (this.multi)
	{
		if (this.tasks)
			for (let t in this.tasks)
				this.tasks[t].destroy();

		if (this.elTasks)
			this.elTasks.textContent = '';
	}

	if (this.args.tasks_badges)
	{
		let elBages = task_DrawBadges(this.obj, this.elTasksBadges/*, {'only_my':this.args.display_short}*/);
		if (this.multi)
		{
			if (elBages && elBages.length)
			{
				for (let el of elBages)
				{
					el.m_status = this;
					el.onclick = function(e){
						e.stopPropagation();
						e.currentTarget.m_status.elTasksBadges.style.display = 'none';
						e.currentTarget.m_status.args.tasks_badges = false;
						e.currentTarget.m_status.showTasks();
					};
				}
			}
			else
			{
				this.createAddTaskButton();
			}
		}
	}
	else
	{
		this.showTasks();
	}

	let args = {};
	args.statuses = [this.obj];
	args.elReports = this.elReports;
	args.elReportsDiv = this.elReportsDiv;
	args.noTasks = true;

	stcs_Show(args);

	this.elParent.classList.remove('fading');
	this.elParent.classList.remove('updating');
};

Status.prototype.showTasks = function()
{
	tasks_Finish();

	if (this.obj && this.obj.tasks)
	{
		for (let t in this.obj.tasks)
		{
			let task = new Task(this, this.obj.tasks[t]);
			if (this.multi)
				task.elBtnEdit.onclick = sc_EditTask;
		}
	}

	if (this.multi)
		this.createAddTaskButton();
}
Status.prototype.createAddTaskButton = function()
{
	let el = document.createElement('div');
	el.classList.add('button');
	el.textContent = 'Add Task';
	el.m_status = this;
	el.onclick = function(e){
		e.stopPropagation();
		sc_AddTask(e.currentTarget.m_status);
		return false;
	};

	if (this.args.tasks_badges)
		this.elTasksBadges.appendChild(el);
	else
		this.elTasks.appendChild(el);
}

Status.prototype.update = function(i_status)
{
	this.show(i_status);

	this.elParent.classList.remove('fading');
	this.elParent.classList.add('updating');

	setTimeout(st_UpdatingFinished, 1000);

	c_Info('Status updated.');
}

function st_UpdatingFinished()
{
	if (st_Status == null) return;
	st_Status.updatingFinished();
}

Status.prototype.updatingFinished = function()
{
	if (this.elParent.classList.contains('fading'))
	{
		this.elParent.classList.remove('fading');
		return;
	}

	this.elParent.classList.add('fading');
	this.elParent.classList.remove('updating');

	setTimeout(st_UpdatingFinished, 1000);
}

function st_SetElProgress(i_status, i_elProgressBar, i_elProgressHide, i_elPercentage)
{
	var clr = 'rgba(0,255,0,.4)';

	if (i_elPercentage)
	{
		i_elPercentage.classList.remove('done');
		i_elPercentage.classList.remove('started');
		i_elPercentage.classList.add('notstarted');
	}
	if (i_elProgressBar)
	{
		i_elProgressBar.classList.remove('done');
		i_elProgressBar.classList.remove('started');
		i_elProgressBar.classList.add('notstarted');
	}
	if (i_elProgressHide)
	{
		i_elProgressHide.classList.remove('done');
		i_elProgressHide.classList.remove('started');
		i_elProgressHide.classList.add('notstarted');
	}

	if (i_status && (i_status.progress != null) && (i_status.progress >= 0))
	{
		if (i_status.progress < 100)
		{
			clr = 255 - Math.round(2.55 * i_status.progress);
			clr = 'rgba(' + clr + ',255,0,.8)';
		}

		if (i_elProgressBar)
		{
			i_elProgressBar.style.width = i_status.progress + '%';
			if (i_status.progress > 0)
			{
				i_elProgressBar.classList.add('started');
				i_elProgressBar.classList.remove('notstarted');
			}
			if (i_status.progress >= 100)
				i_elProgressBar.classList.add('done');
		}
		if (i_elPercentage)
		{
			//i_elPercentage.style.display = 'block';
			i_elPercentage.textContent = i_status.progress + '%';
			if (i_status.progress > 0)
			{
				i_elPercentage.classList.add('started');
				i_elPercentage.classList.remove('notstarted');
			}
			if (i_status.progress >= 100)
				i_elPercentage.classList.add('done');
		}
		if (i_elProgressHide)
		{
			i_elProgressHide.style.display = 'block';
			i_elProgressHide.title = i_status.progress + '%';
			if (i_status.progress > 0)
			{
				i_elProgressHide.classList.add('started');
				i_elProgressHide.classList.remove('notstarted');
			}
			if (i_status.progress >= 100)
				i_elProgressHide.classList.add('done');
		}
	}
	else
	{
		if (i_elProgressBar)
			i_elProgressBar.style.width = '0';
		if (i_elPercentage)
		{
			i_elPercentage.textContent = '';
			i_elPercentage.style.display = 'none';
		}
		if (i_elProgressHide)
		{
			i_elProgressHide.style.display = 'none';
			i_elProgressHide.title = null;
		}
	}

	if (i_elProgressBar)
		i_elProgressBar.style.backgroundColor = clr;
}

function st_SetElLabel(i_status, i_el, i_full)
{
	if (i_full == null)
		i_full = false;
	st_SetElAnnotation(i_status, i_el, i_full);
}
function st_SetElFramesNum(i_status, i_el, i_full)
{
	if (i_full == null)
		i_full = true;

	var num = '';
	if (i_status && i_status.frames_num)
	{
		num = i_status.frames_num;
		i_el.title = ' ~ ' + Math.round(num / RULES.fps) + 'sec';
		if (i_full)
			i_el.parentNode.style.display = 'block';
		else
			num = 'F:' + num;
	}
	else if (i_full)
		i_el.parentNode.style.display = 'none';

	if (i_full)
	{
		i_el.parentNode.classList.remove('updated');
		i_el.parentNode.title = null;
	}

	i_el.textContent = num;
}
function st_SetElPrice(i_status, i_el)
{
	var price = '';
	if (i_status && i_status.price)
		price = i_status.price;

	i_el.textContent = price;
}
function st_SetElDuration(i_status, i_el)
{
	var duration = '';
	if (i_status && i_status.duration)
		duration = i_status.duration;

	i_el.textContent = duration;
}
function st_SetElAnnotation(i_status, i_el, i_full)
{
	st_SetElText(i_status, i_el, 'annotation', i_full);
}
function st_SetElText(i_status, i_el, i_field, i_full)
{
	if (i_full == null)
		i_full = true;
	if (i_status && i_status[i_field])
	{
		if (i_full)
			i_el.innerHTML = i_status[i_field];
		else
			i_el.innerHTML = i_status[i_field].replace(/^\s+/, '').split(' ')[0];
	}
	else
		i_el.innerHTML = '';
}
function st_SetElArtists(i_status, i_el, i_short, i_clickable, i_onclick)
{
	i_el.textContent = '';
	let elements = [];

	if ((i_status == null) || (i_status.artists == null))
		return elements;

	for (let i = 0; i < i_status.artists.length; i++)
	{
		let el = st_CreateElArtist(i_status.artists[i], i_short);
		elements.push(el);
		i_el.appendChild(el);
		el.m_name = i_status.artists[i];
		st_TagHilight(el, 'artist');

		if (i_clickable)
		{
			el.onclick = st_TagClicked;
			el.ondblclick = st_ArtistDblClicked;

			el.onmousedown = st_ArtistMouseDown;
		}
	}

	return elements;
}
function st_CreateElArtist(i_id, i_short)
{
	let el = document.createElement('div');
	el.classList.add('tag');
	el.classList.add('artist');
	el.textContent = c_GetUserTitle(i_id, null, i_short);

	if (g_users[i_id] && g_users[i_id].disabled)
		el.classList.add('disabled');

	if (i_short)
	{
		el.classList.add('short');
		if (g_auth_user && (g_auth_user.id == i_id))
			el.title = 'It`s me!';
		else
			el.title = c_GetUserTitle(i_id);
	}

	if (g_auth_user && (g_auth_user.id == i_id))
	{
		el.classList.add('me');

		if (i_short !== true)
		{
			el.title = 'It`s me!\n - may be i should do something here?';
		}
	}

	let avatar = c_GetAvatar(i_id);
	if (avatar)
	{
		el.classList.add('with_icon');
		el.style.backgroundImage = 'url(' + avatar + ')';
	}

	return el;
}
function st_SetElFlags(i_status, i_elFlags, i_short, i_clickable)
{
	let elements = [];

	if (i_elFlags.m_elFlags)
		for (let el of i_elFlags.m_elFlags)
			i_elFlags.removeChild(el);
	i_elFlags.m_elFlags = [];

	if (i_status && i_status.flags)
		for (let flag of i_status.flags)
		{
			let el = st_CreateElFlag(flag, i_short);
			elements.push(el);
			i_elFlags.appendChild(el);
			i_elFlags.m_elFlags.push(el);
			st_TagHilight(el, 'flag');

			if (i_clickable)
			{
				el.onclick = st_TagClicked;
				el.ondblclick = st_FlagDblClicked;
			}
		}

	return elements;
}
function st_CreateElFlag(i_flag, i_short, i_suffix)
{
	let el = document.createElement('div');
	el.classList.add('flag');
	let label = c_GetFlagTitle(i_flag);
	if (i_short)
		label = c_GetFlagShort(i_flag);
	if (i_suffix)
		label += i_suffix;
	el.textContent = label;
	el.title = c_GetFlagTip(i_flag);
	el.m_name = i_flag;

	let clr = null;
	if (RULES.flags[i_flag] && RULES.flags[i_flag].clr)
		clr = RULES.flags[i_flag].clr;
	if (clr)
		st_SetElColor({"color": clr}, el);

	return el;
}
function st_SetElTags(i_status, i_elTags, i_short, i_clickable)
{
	let elements = [];

	if (i_elTags.m_elTags)
		for (let i = 0; i < i_elTags.m_elTags.length; i++)
			i_elTags.removeChild(i_elTags.m_elTags[i]);
	i_elTags.m_elTags = [];

	if (i_status && i_status.tags)
		for (let i = 0; i < i_status.tags.length; i++)
		{
			let el = st_CreateElTag(i_status.tags[i], i_short);
			elements.push(el);
			i_elTags.appendChild(el);
			i_elTags.m_elTags.push(el);

			st_TagHilight(el, 'tag');

			if (i_clickable)
			{
				el.onclick = st_TagClicked;
				el.ondblclick = st_TagDblClicked;

				el.onmousedown = st_TagMouseDown;
			}
		}

	return elements;
}
function st_CreateElTag(i_tag, i_short, i_suffix)
{
	let el = document.createElement('div');
	el.classList.add('tag');
	let label = c_GetTagTitle(i_tag);
	if (i_short)
		label = c_GetTagShort(i_tag);
	if (i_suffix)
		label += i_suffix;
	el.textContent = label;
	el.title = c_GetTagTip(i_tag);
	el.m_name = i_tag;

	return el;
}
function st_TagHilight(i_el, i_key)
{
	if (st_Hilighted[i_key].indexOf(i_el.m_name) != -1)
	{
		i_el.classList.add('highlighted');
		return true;
	}

	return false;
}
function st_TagClicked(i_evt)
{
	i_evt.stopPropagation();
	return false;
}
function st_TagDblClicked(i_evt)
{
	st_TagClicked(i_evt);
	st_TagHilightToggle(i_evt.currentTarget, 'tag');
}
function st_TagMouseDown(i_evt)
{
	if (i_evt.button == 1)
		task_StatusTagClicked(i_evt.currentTarget.m_name);
}
function st_FlagDblClicked(i_evt)
{
	st_TagClicked(i_evt);
	st_TagHilightToggle(i_evt.currentTarget, 'flag');
}
function st_ArtistMouseDown(i_evt)
{
	if (i_evt.button == 1)
		task_StatusArtistClicked(i_evt.currentTarget.m_name);
}
function st_ArtistDblClicked(i_evt)
{
	st_TagClicked(i_evt);
	st_TagHilightToggle(i_evt.currentTarget, 'artist');
}
function st_TagHilightToggle(i_el, i_key)
{
	let name = i_el.m_name;

	if (i_el.classList.contains('highlighted'))
	{
		let i = 0;
		while (i < st_Hilighted[i_key].length)
			if (st_Hilighted[i_key][i] === name)
				st_Hilighted[i_key].splice(i, 1);
			else
				++i;
	}
	else
	{
		if (st_Hilighted[i_key].indexOf(name) == -1)
			st_Hilighted[i_key].push(name)
	}

	for (let el of document.getElementsByClassName(i_key))
		if (st_Hilighted[i_key].indexOf(el.m_name) != -1)
			el.classList.add('highlighted');
		else
			el.classList.remove('highlighted');
}

function st_SetElColor(i_status, i_elBack, i_elColor, i_setNone)
{
	if (i_elColor == null)
		i_elColor = i_elBack;
	if (i_setNone == null)
		i_setNone = true;

	var c = null;
	var a = 1;
	if (i_status && i_status.color)
	{
		c = i_status.color;
	}
	else if (i_status && i_status.flags && i_status.flags.length)
	{
		var flag = i_status.flags[i_status.flags.length - 1];
		if (RULES.flags[flag] && RULES.flags[flag].clr)
		{
			c = RULES.flags[flag].clr;
		}
	}

	if (c)
	{
		i_elBack.style.backgroundColor = 'rgba(' + c[0] + ',' + c[1] + ',' + c[2] + ',' + a + ')';
		st_SetElColorTextFromBack(c, i_elColor);
		return true;
	}
	else if (i_setNone)
	{
		if (localStorage.background && localStorage.background.length)
			i_elBack.style.background = localStorage.background;
		else
			i_elBack.style.background = u_background;

		if (localStorage.text_color && localStorage.text_color.length)
			i_elColor.style.color = localStorage.text_color;
		else
			i_elColor.style.color = u_textColor;
	}
	return false;
}
function st_SetElColorTextFromBack(i_clr, i_el)
{
	if (i_clr[0] + i_clr[1] + .3 * i_clr[2] > 300)
		i_el.style.color = '#000';
	else
		i_el.style.color = '#FFF';
}

function st_SetElStatus(i_el, i_status, i_show_all_tasks, i_user = null)
{
	if (null == i_el)
		return;

	if (null == i_user)
		i_user = g_auth_user;

	let elStatus = document.createElement('div');
	i_el.appendChild(elStatus);
	elStatus.classList.add('status');


	// Flags:
	if (i_status && i_status.flags && i_status.flags.length)
	{
		let elFlags = document.createElement('div');
		elStatus.appendChild(elFlags);
		elFlags.classList.add('flags');
		st_SetElFlags(i_status, elFlags);
	}


	let elTasks = document.createElement('div');
	elStatus.appendChild(elTasks);
	elTasks.classList.add('tasks');


	let tasks = {};
	let found = false;
	// If user has tasks, we should draw only tasks that user is assigned on.
	// But if user subscribed on location, he should see all tasks.
	if ((false == i_show_all_tasks) && i_status && i_status.tasks)
	{
		for (let t in i_status.tasks)
		{
			let task = i_status.tasks[t];
			if (task.deleted)
				continue;

			if (task.artists && (task.artists.indexOf(i_user.id) == -1))
				continue;

			tasks[task.name] = task;

			found = true;
		}
	}

	if (found)
	{
		task_DrawBadges({'tasks':tasks}, elTasks, {'hide_artists':true,'full_names':true});
	}
	else
	{
		// Draw all tasks if user received new, but was not assigned on any task.
		// This situation when supervisor subscribed on location,
		// and receiving news but does not assigned.
		task_DrawBadges(i_status, elTasks);
	}

	// Show progress bar:
	if (i_status && i_status.progress)
	{
		let elBar = document.createElement('div');
		i_el.appendChild(elBar);
		elBar.classList.add('bar');
		st_SetElProgress(i_status, elBar);
	}

	// Status color:
	st_SetElColor(i_status, i_el, i_el, false);
	/*
	if (i_status && i_status.color)
	{
		let c = i_status.color;
		i_el.style.backgroundColor = 'rgba('+c[0]+','+c[1]+','+c[2]+')';
		st_SetElColorTextFromBack(c, i_el);
	}
	*/
}

function st_SetElFinish(i_status, i_elFinish, i_full)
{
	if (i_full == null)
		i_full = true;
	var text = '';
	var shadow = '';
	var alpha = 0;
	if (i_status && i_status.finish)
	{
		var days = c_DT_DaysLeft(i_status.finish);
		var alpha = 1 / (1 + days);
		days = Math.round(days * 10) / 10;
		if (alpha < 0)
			alpha = 0;
		if (alpha > 1)
			alpha = 1;
		if (days <= 1)
			alpha = 1;
		if (i_full)
		{
			text += 'Finish at: ';
			text += c_DT_StrFromSec(i_status.finish).substr(0, 15);
			text += ' (' + days + ' days left)';
		}
		else
			text += days + 'days';
	}
	shadow = '0 0 2px rgba(255,0,0,' + alpha + '),';
	shadow += '0 0 4px rgba(255,0,0,' + alpha + ')';
	i_elFinish.style.textShadow = shadow;
	i_elFinish.style.color = 'rgb(' + Math.round(150 * alpha) + ',0,0)';
	i_elFinish.textContent = text;
}

Status.prototype.reloadFile = function(i_edit = false)
{
	n_GetFile({
		"path": c_GetRuFilePath('status.json', this.path),
		"func": st_StatusReceived,
		"status_object": this,
		"edit": i_edit,
		"info": 'status',
		"parse": true,
		"local": true,
		"cache_time":-1
	});
}

function st_StatusReceived(i_data, i_args)
{
	if (i_args.edit)
		i_args.status_object.updateOrEdit(i_data.status);
	else
		i_args.status_object.update(i_data.status);
}

Status.prototype.updateOrEdit = function(i_status)
{
	if (i_status && i_status.mtime && (i_status.mtime > this.obj.mtime))
		this.update(i_status);
	else
		this.edit();
}

Status.prototype.edit = function(i_args) {
	// console.log( JSON.stringify( i_args));
	// console.log(JSON.stringify(i_status));
	if (g_auth_user == null)
	{
		c_Error('Guests can`t edit status.');
		return;
	}

	if (this.obj == null)
		this.obj = {};

	// If editing element is exists, status is already in edit mode:
	if (this.elEdit)
		return;

	// Hide status show element, if any:
	if (this.elShow)
		this.elShow.style.display = 'none';

	// Create editing GUI:
	this.elParent.classList.add('status_editing');

	this.elEdit = document.createElement('div');
	this.elParent.appendChild(this.elEdit);
	this.elEdit.classList.add('status_edit');
	this.elEdit.onclick = function(e) { e.stopPropagation(); };
	this.elEdit.m_status = this;
	this.elEdit.onkeydown = function(e) { e.currentTarget.m_status.editOnKeyDown(e, i_args); };

	var elBtns = document.createElement('div');
	this.elEdit.appendChild(elBtns);
	elBtns.classList.add('buttons_div');

	var elBtnCancel = document.createElement('div');
	elBtns.appendChild(elBtnCancel);
	elBtnCancel.classList.add('button');
	elBtnCancel.textContent = 'Cancel';
	elBtnCancel.m_status = this;
	elBtnCancel.onclick = function(e) { e.currentTarget.m_status.show(); };

	var elBtnSave = document.createElement('div');
	elBtns.appendChild(elBtnSave);
	elBtnSave.classList.add('button');
	elBtnSave.textContent = 'Save';
	elBtnSave.m_status = this;
	elBtnSave.m_args = i_args;
	elBtnSave.onclick = function(e) { e.currentTarget.m_status.editProcess(i_args); };

	var elDiv = document.createElement('div');
	this.elEdit.appendChild(elDiv);
	elDiv.classList.add('status_edit_div');

	var elFinishDiv = document.createElement('div');
	elDiv.appendChild(elFinishDiv);
	elFinishDiv.classList.add('finish_div');
	var elFinishLabel = document.createElement('div');
	elFinishDiv.appendChild(elFinishLabel);
	elFinishLabel.classList.add('label');
	elFinishLabel.textContent = 'Fin:';
	elFinishLabel.onclick = function(e) { e.currentTarget.m_elEdit_finish.textContent = c_DT_FormStrNow(); };
	elFinishLabel.title = 'Click to set current date.';
	this.elEdit_finish = document.createElement('div');
	elFinishDiv.appendChild(this.elEdit_finish);
	this.elEdit_finish.classList.add('editing');
	this.elEdit_finish.contentEditable = 'true';
	elFinishLabel.m_elEdit_finish = this.elEdit_finish;

	var elProgressDiv = document.createElement('div');
	elDiv.appendChild(elProgressDiv);
	elProgressDiv.classList.add('progress_div');
	var elProgressLabel = document.createElement('div');
	elProgressDiv.appendChild(elProgressLabel);
	elProgressLabel.classList.add('label');
	elProgressLabel.textContent = '%';
	this.elEdit_progress = document.createElement('div');
	elProgressDiv.appendChild(this.elEdit_progress);
	this.elEdit_progress.contentEditable = 'true';
	this.elEdit_progress.classList.add('editing');
	this.elEdit_progress.onkeydown = function(e) {
		if (e.keyCode == 13)
			return false;
	};

	var elAnnDiv = document.createElement('div');
	elDiv.appendChild(elAnnDiv);
	elAnnDiv.classList.add('ann_div');
	var elAnnLabel = document.createElement('div');
	elAnnDiv.appendChild(elAnnLabel);
	elAnnLabel.classList.add('label');
	elAnnLabel.textContent = 'Annotation:';
	this.elEdit_annotation = document.createElement('div');
	elAnnDiv.appendChild(this.elEdit_annotation);
	this.elEdit_annotation.classList.add('editing');
	this.elEdit_annotation.contentEditable = 'true';

	// Objects needed to store half_selected state,
	// when several statuses selected.
	var artists = {};
	var flags = {};
	var tags = {};

	if (this.obj.artists)
		for (var a = 0; a < this.obj.artists.length; a++)
			artists[this.obj.artists[a]] = {};

	if (this.obj.flags)
		for (var a = 0; a < this.obj.flags.length; a++)
			flags[this.obj.flags[a]] = {};

	if (this.obj.tags)
		for (var a = 0; a < this.obj.tags.length; a++)
			tags[this.obj.tags[a]] = {};


	// If several statuses selected, we check/set "half_selected" state
	if (i_args && i_args.statuses)
		for (let s = 0; s < i_args.statuses.length; s++)
		{
			if (i_args.statuses[s].obj && i_args.statuses[s].obj.artists)
			{
				for (let id in artists)
					if (i_args.statuses[s].obj.artists.indexOf(id) == -1)
						artists[id].half_selected = true;

				for (let a = 0; a < i_args.statuses[s].obj.artists.length; a++)
				{
					let id = i_args.statuses[s].obj.artists[a];
					if (artists[id] == null)
						artists[id] = {"half_selected": true};
				}
			}
			else
				for (let id in artists)
					artists[id].half_selected = true;

			if (i_args.statuses[s].obj && i_args.statuses[s].obj.flags)
			{
				for (let id in flags)
					if (i_args.statuses[s].obj.flags.indexOf(id) == -1)
						flags[id].half_selected = true;

				for (let a = 0; a < i_args.statuses[s].obj.flags.length; a++)
				{

					let id = i_args.statuses[s].obj.flags[a];
					if (flags[id] == null)
						flags[id] = {"half_selected": true};
				}
			}
			else
				for (let id in flags)
					flags[id].half_selected = true;

			if (i_args.statuses[s].obj && i_args.statuses[s].obj.tags)
			{
				for (let id in tags)
					if (i_args.statuses[s].obj.tags.indexOf(id) == -1)
						tags[id].half_selected = true;

				for (let a = 0; a < i_args.statuses[s].obj.tags.length; a++)
				{
					let id = i_args.statuses[s].obj.tags[a];
					if (tags[id] == null)
						tags[id] = {"half_selected": true};
				}
			}
			else
				for (let id in tags)
					tags[id].half_selected = true;
		}

	if (c_CanAssignArtists())
		this.editAtrists = new EditList({
			"name": 'artists',
			"label": 'Artists:',
			"list": artists,
			"list_all": g_users,
			"elParent": this.elEdit
		});

	this.editFlags = new EditList(
		{"name": 'flags', "label": 'Flags:', "list": flags, "list_all": RULES.flags, "elParent": this.elEdit});
	this.editTags = new EditList(
		{"name": 'tags', "label": 'Tags:', "list": tags, "list_all": RULES.tags, "elParent": this.elEdit});

	this.elEdit_Color = document.createElement('div');
	this.elEdit.appendChild(this.elEdit_Color);
	this.elEdit_Color.classList.add('color');
	u_DrawColorBars({"el": this.elEdit_Color, "onclick": st_EditColorOnClick, "data": this});


	if (g_admin)
	{
		this.elEdit_adinfo = document.createElement('div');
		this.elEdit.appendChild(this.elEdit_adinfo);
		this.elEdit_adinfo.classList.add('adinfo');
		this.elEdit_adinfo.classList.add('editing');
		this.elEdit_adinfo.contentEditable = 'true';
	}


	this.elEdit_tasks = document.createElement('div');
	this.elEdit.appendChild(this.elEdit_tasks);
	this.elEdit_tasks.classList.add('edit_tasks');

	// Get values:
	var annotation = this.obj.annotation;
	var adinfo = this.obj.adinfo;
	var progress = this.obj.progress;
	var finish = this.obj.finish;

	if (i_args && i_args.statuses && i_args.statuses.length)
	{
		// Several statuses (shots) selected:
		annotation = this.getMultiVale('annotation', i_args.statuses);
		progress = this.getMultiVale('progress', i_args.statuses);
		finish = this.getMultiVale('finish', i_args.statuses);
	}

	// Set values:
	if (annotation != null)
	{
		this.elEdit_annotation.textContent = annotation;
	}
	if (g_admin && (adinfo != null))
	{
		this.elEdit_adinfo.textContent = atob(adinfo);
	}
	if (finish != null)
	{
		if (finish != st_MultiValue)
			finish = c_DT_FormStrFromSec(finish);
		this.elEdit_finish.textContent = finish;
	}
	if (progress != null)
	{
		this.elEdit_progress.textContent = progress;
	}

	this.elEdit_annotation.focus();
};
Status.prototype.editOnKeyDown = function(e, i_args) {
	if (e.keyCode == 27)
		this.show();  // ESC
	if (e.keyCode == 13)
		this.editProcess(i_args);  // ENTER
};
function st_EditColorOnClick(i_clr, i_data)
{
	i_data.elEdit_Color.m_color = i_clr;
	i_data.elEdit_Color.m_color_changed = true;
	st_SetElColor({"color": i_clr}, i_data.elColor);
}
Status.prototype.getMultiVale = function(i_key, i_statuses) {
	if (i_statuses.indexOf(this) == -1)
		i_statuses.push(this);

	var value = this.obj[i_key];
	if ((i_statuses.length == 1) && (i_statuses[0] == this))
		return value;

	if (value == "" || ((typeof value) == 'undefined'))
		value = null;

	for (let i = 0; i < i_statuses.length; i++)
	{
		let other = null;
		if ((i_statuses[i].obj) && (i_statuses[i].obj[i_key]))
			other = i_statuses[i].obj[i_key];

		if (other == "" || ((typeof other) == 'undefined'))
			other = null;

		if (value != other)
		{
			value = st_MultiValue;
			return value;
		}
	}
	return value;
};

Status.prototype.editCancel = function() {
	if (this.elEdit == null)
		return;

	if (this.elParent)
	{
		this.elParent.removeChild(this.elEdit);
		this.elParent.classList.remove('status_editing');
	}
	this.elEdit = null;
};

Status.prototype.editProcess = function(i_args) {
	if (this.obj == null)
		this.obj = {};

	let finish = null;
	let annotation = null;
	let adinfo = null;
	let progress = null;
	let artists = null;
	let flags = null;
	let tags = null;
	let tasks = null;

	// Get values from GUI:

	let finish_edit = this.elEdit_finish.textContent;
	if (finish_edit.length && (finish_edit != st_MultiValue))
	{
		finish_edit = c_DT_SecFromStr(finish_edit);
		if (finish_edit != 0)
			finish = finish_edit;
		else
			c_Error('Invalid date format: ' + this.elEdit_finish.textContent);
	}

	let progress_edit = this.elEdit_progress.textContent;
	if (progress_edit.length && (progress_edit != st_MultiValue))
	{
		progress_edit = c_Strip(progress_edit);
		progress = parseInt(progress_edit);
		if (isNaN(progress))
		{
			progress = null;
			c_Error('Invalid progress: ' + c_Strip(progress_edit));
		}
		if (progress < -1)
			progress = -1;
		if (progress > 100)
			progress = 100;
	}

	if (this.elEdit_annotation.textContent != st_MultiValue)
	{
		this.elEdit_annotation.innerHTML = this.elEdit_annotation.textContent;
		annotation = c_Strip(this.elEdit_annotation.textContent);
	}

	if (g_admin)
	{
		this.elEdit_adinfo.innerHTML = this.elEdit_adinfo.textContent;
		adinfo = c_Strip(this.elEdit_adinfo.textContent);
		adinfo = btoa(adinfo);
	}


	if (this.editAtrists)
		artists = this.editAtrists.getSelectedObjects();
	if (this.editFlags)
		flags = this.editFlags.getSelectedObjects();
	if (this.editTags)
		tags = this.editTags.getSelectedObjects();

	// Collect statuses to change
	// ( this and may be others selected )
	let statuses = [this];
	if (i_args && i_args.statuses && i_args.statuses.length)
	{
		statuses = i_args.statuses;
		if (statuses.indexOf(this) == -1)
			statuses.push(this);
	}

	if (1)//document.location.hostname == 'localhost')
	{
	let obj = {};

	obj.paths = [];
	for (let st of statuses)
		obj.paths.push(st.path);

	if (annotation !== null)
		obj.annotation = annotation;
	if (adinfo !== null)
		obj.adinfo = adinfo;
	if (finish !== null)
		obj.finish = finish;
	if (progress !== null)
		obj.progress = progress;

	if (this.editAtrists)
	{
		obj.artists = this.editAtrists.getSelectedNames();
		obj.artists_keep = this.editAtrists.getHalfSelectedNames();
	}
	if (this.editFlags)
	{
		obj.flags = this.editFlags.getSelectedNames();
		obj.flags_keep = this.editFlags.getHalfSelectedNames();
	}
	if (this.editTags)
	{
		obj.tags = this.editTags.getSelectedNames();
		obj.tags_keep = this.editTags.getHalfSelectedNames();
	}

	if (this.elEdit_Color.m_color_changed)
	{
		obj.color = this.elEdit_Color.m_color;
		if (obj.color == null)
			obj.color = [];
	}

	if (nw_disabled)
		obj.nonews = true;

	n_Request({'send':{'setstatus':obj},'func':st_StatusesSaved,'info':'status.setStatuses','wait':false});
	//console.log(JSON.stringify(obj));
	return
	}

	// Set values to statuses
	var some_progress_changed = false;
	var progresses = {};

	for (let i = 0; i < statuses.length; i++)
	{
		if (statuses[i].obj == null)
			statuses[i].obj = {};

		// Store original progress to compare later
		// and find out that is was changed.
		let _progress = statuses[i].obj.progress;

		if (annotation !== null)
			statuses[i].obj.annotation = annotation;
		if (adinfo !== null)
			statuses[i].obj.adinfo = adinfo;
		if (finish !== null)
			statuses[i].obj.finish = finish;
		if (progress !== null)
			statuses[i].obj.progress = progress;

		st_SetStatusFlags(statuses[i].obj, flags);

		if (artists)
		{
			if (statuses[i].obj.artists == null)
				statuses[i].obj.artists = [];

			for (let a = 0; a < statuses[i].obj.artists.length;)
				if (artists[statuses[i].obj.artists[a]] == null)
					statuses[i].obj.artists.splice(a, 1);
				else
					a++;

			for (let id in artists)
				if ((artists[id].selected) && (statuses[i].obj.artists.indexOf(id) == -1))
					statuses[i].obj.artists.push(id);
		}

		if (tags)
		{
			if (statuses[i].obj.tags == null)
				statuses[i].obj.tags = [];

			for (let a = 0; a < statuses[i].obj.tags.length;)
				if (tags[statuses[i].obj.tags[a]] == null)
					statuses[i].obj.tags.splice(a, 1);
				else
					a++;

			for (let id in tags)
				if ((tags[id].selected) && (statuses[i].obj.tags.indexOf(id) == -1))
					statuses[i].obj.tags.push(id);
		}

		// If shot progress is 100% all tasks should be 100% done.
		if ((statuses[i].obj.progress == 100) && (statuses[i].obj.tasks))
			for (let t in statuses[i].obj.tasks)
			{
				if (statuses[i].obj.tasks[t].progress != 100)
				{
					statuses[i].obj.tasks[t].progress = 100;
					statuses[i].obj.tasks[t].changed = true;
				}
				if (statuses[i].obj.tasks[t].flags.indexOf('done') == -1)
				{
					statuses[i].obj.tasks[t].flags = ['done'];
					statuses[i].obj.tasks[t].changed = true;
				}
			}

		// If shot has OMIT flags, all tasks should be omitted
		if (statuses[i].obj.flags && (statuses[i].obj.flags.indexOf('omit') != -1) && (statuses[i].obj.tasks))
			for (let t in statuses[i].obj.tasks)
				if ((statuses[i].obj.tasks[t].flags.indexOf('omit') == -1) ||
					(statuses[i].obj.tasks[t].progress != -1))
				{
					let task = statuses[i].obj.tasks[t];

					// Skip done tasks
					if (task.progress && (task.progress == 100))
						continue;

					task.flags = ['omit'];
					task.progress = -1;
					task.changed = true;
				}
/*
		// Shot "super" flags resets tasks flags
		if (statuses[i].obj.flags && statuses[i].obj.flags.length)
			for (let f of statuses[i].obj.flags)
				if (f && RULES.flags[f] && statuses[i].obj.tasks && RULES.flags[f].mode && (RULES.flags[f].mode == "super"))
					for (let t in statuses[i].obj.tasks)
					{
						let task = statuses[i].obj.tasks[t];

						// Skip done tasks
						if (task.progress && (task.progress == 100))
							continue;

						if (task.flags &&
							task.flags.length)
						{
							task.flags = [];
							task.changed = true;
						}
					}
*/
		// If progress was changed we should update upper progress:
		if (_progress != statuses[i].obj.progress)
		{
			progresses[statuses[i].path] = statuses[i].obj.progress;
			some_progress_changed = true;
		}

		if (this.elEdit_Color.m_color_changed)
			statuses[i].obj.color = this.elEdit_Color.m_color;

		// Store modification user and time
		statuses[i].obj.muser = g_auth_user.id;
		statuses[i].obj.mtime = c_DT_CurSeconds();

		// This is needed for news&bookmarks
		// Maybe just one task was changed, not the entire status
		statuses[i].obj.changed = true;
	}

	// News & Bookmarks:
	// At first we should emit news,
	// as some temporary could be added for news.
	// For example task.changed = true
	nw_StatusesChanged(statuses);

	// Saving will filter objects.
	// Temporary parameters will be removed.
	for (let i = 0; i < statuses.length; i++)
	{
		statuses[i].save();
		statuses[i].show();
		//^ Status showing causes values redraw,
		// and destroys edit GUI if any.
	}

	if (some_progress_changed)
		st_UpdateProgresses(this.path, progresses);

	c_Log('Status(es) saved.');
};

Status.prototype.save = function() {
	if (this.path == g_CurPath())
		RULES.status = this.obj;

	st_Save(this.obj, this.path);
};

function st_FilterStatusForSave(i_status)
{
	// Delete temporary items:
	delete i_status.error;
	delete i_status.changed;

	if (i_status.body)
		delete i_status.body.data;

	// Task changed property needed for news.
	// Artist that is assigned to task only,
	// should receive news only if it task changed.
	if (i_status.tasks)
		for (let t in i_status.tasks)
			delete i_status.tasks[t].changed;
}

function st_Save(i_status, i_path, i_func, i_args, i_navig_params_update)
{
	if (i_status == null)
		i_status = RULES.status;
	if (i_path == null)
		i_path = g_CurPath();
	if (i_func == null)
		i_func = st_SaveFinished;

	st_FilterStatusForSave(i_status);

	// If path is current, we take status global current status.
	// As only some part of status object can be edited and provided for save.
	g_FolderSetStatusPath(i_path == g_CurPath() ? RULES.status : i_status, i_path, i_navig_params_update);

	// Clear network walks cache
	n_walks[i_path] = null;

	var obj = {};
	obj.object = {"status": i_status};
	obj.add = true;
	obj.file = c_GetRuFilePath('status.json', i_path);
	n_Request(
		{"send": {"editobj": obj}, "func": i_func, "args": i_args, "wait": false, 'info': 'status save'});
}

function st_SaveFinished(i_data)
{
	if (i_data.info)
		c_Info(i_data.info);

	if(i_data.error)
	{
		c_Error(i_data.error);
		return;
	}
}

function st_StatusesSaved(i_data)
{
	// Update statuses:
	if (i_data.statuses)
	{
		for (let sdata of i_data.statuses)
		{
			// Update current status:
			if (sdata.path == g_CurPath())
			{
				RULES.status = sdata.status;
				st_Show(sdata.status);
			}

			// Update other statuses (scene shots view):
			if (st_Statuses[sdata.path])
			{
				st_Statuses[sdata.path].show(sdata.status);
			}

			// Update navigation folders
			let el = g_elFolders[sdata.path];
			if (el == null) continue;

			let fstat = el.m_fobject.status;
			if (fstat)
			{
				// Update only if status time > folder status time
				if (fstat.ctime && (fstat.ctime >= sdata.status.time)) continue;
				if (fstat.mtime && (fstat.mtime >= sdata.status.time)) continue;
			}

			// Update folder status:
			g_FolderSetStatus(sdata.status, el);
		}
	}

	if (i_data.error)
	{
		c_Error(i_data.error);
		return;
	}

	// Get news if subscribed:
	if (i_data.users_subscribed && (i_data.users_subscribed.indexOf(g_auth_user.id) != -1))
	{
		nw_NewsLoad();
		nw_RecentLoad({"file_check": false});
	}

	// Get bookmarks:
	if (i_data.users_changed && (i_data.users_changed.indexOf(g_auth_user.id) != -1))
	{
		bm_Load({'info': 'statuses'});
	}

	if (i_data.users_subscribed && i_data.users_subscribed.length)
	{
		let info = 'Subscribed users: ';
		for (let i = 0; i < i_data.users_subscribed.length; i++)
		{
			if (i) info += ', ';
			info += c_GetUserTitle(i_data.users_subscribed[i]);
		}
		c_Info(info);
	}

	// Update navigation folders progresses:
	let progresses = i_data.progresses;
	if (progresses)
	{
		for (let path in progresses)
		{
			g_FolderSetStatusPath({'progress':progresses[path]}, path, {'progress':true});
		}
	}
}

function st_SetFramesNumber(i_num)
{
	if (RULES.status == null)
		RULES.status = {};

	if (RULES.status.frames_num == i_num)
		return;

	RULES.status.frames_num = i_num;
	st_Save();
	st_Show(RULES.status);

	$('status_framesnum_div').classList.add('updated');
}

function st_SetTimeCode(i_tc)
{
	if ((i_tc == null) || (i_tc.length < 1))
	{
		return;
	}

	var tc = i_tc;
	tc = tc.split('-');
	if (tc.length != 2)
	{
		c_Error('Invalid time code: ' + i_tc);
		return;
	}

	var frame_start = c_TC_FromSting(tc[0].replace());
	if (frame_start === null)
	{
		c_Error('Invalid start time code: ' + tc[0]);
		return;
	}

	var frame_finish = c_TC_FromSting(tc[1]);
	if (frame_finish === null)
	{
		c_Error('Invalid finish time code: ' + tc[1]);
		return;
	}
	var timecode_start = c_TC_FromFrame(frame_start);

	var frames_num = frame_finish - frame_start + 1;
	if (frames_num <= 0)
	{
		c_Error('Start time code is grater than finish: ' + tc[1]);
		return;
	}
	var timecode_finish = c_TC_FromFrame(frame_finish);

	if (RULES.status == null)
		RULES.status = {};

	if ((RULES.status.timecode_start == timecode_start) && (RULES.status.timecode_finish == timecode_finish))
		return;

	RULES.status.timecode_start = timecode_start;
	RULES.status.timecode_finish = timecode_finish;

	var save_fields = {};
	save_fields.timecode_start = timecode_start;
	save_fields.timecode_finish = timecode_finish;

	var navig_params_update = {};

	if ((RULES.status.frames_num == null) || (RULES.status.frames_num <= 0))
	{
		RULES.status.frames_num = frames_num;
		save_fields.frames_num = frames_num;
		navig_params_update.frames_num = true;

		st_SetElFramesNum(RULES.status, $('status_framesnum'));
	}

	st_Save(save_fields, null, null, null, navig_params_update);
}

function st_SetStatusFlags(o_status, i_flags)
{
	if (null == i_flags)
		return;

	if (null == o_status)
		return;

	if (o_status.flags == null)
		o_status.flags = [];

	for (let a = 0; a < o_status.flags.length;)
		if (i_flags[o_status.flags[a]] == null)
			o_status.flags.splice(a, 1);
		else
			a++;

	// Store existing flags to check was it ON before:
	let _flags = [];
	for (let a = 0; a < o_status.flags.length; a++)
		_flags.push(o_status.flags[a]);

	for (let id in i_flags)
		if ((i_flags[id].selected) && (_flags.indexOf(id) == -1))
		{
			if (RULES.flags[id])
			{
				// Flag can limit minimum and maximum progress percentage:
				let p_min = RULES.flags[id].p_min;
				let p_max = RULES.flags[id].p_max;
				let progress = o_status.progress;

				if (p_min && ((progress == null) || (progress < p_min)))
					progress = p_min;

				if (p_max && ((progress == null) || (p_max < 0) || (progress > p_max)))
					progress = p_max;

				if (progress != null)
					o_status.progress = progress;

				// Flag can be exclusive, so we should delete other flags:
				if (RULES.flags[id].mode && (
							(RULES.flags[id].mode == 'stage') ||
							(RULES.flags[id].mode == 'super')
							))
					o_status.flags = [];
			}

			o_status.flags.push(id);
		}
}

function st_UpdateProgresses(i_path, i_progresses)
{
	var paths = [];
	var progresses = {};
	if (i_progresses)
		progresses = i_progresses;

	var paths_skip_save = [];
	// Skip saving of provided progresses:
	// ( as they provided after edit and already saved )
	for (var path in progresses)
		paths_skip_save.push(path);

	var folders = i_path.split('/');
	var path = '';
	for (var i = 1; i < folders.length - 1; i++)
	{
		path += '/' + folders[i];
		paths.push(path);
	}
	// console.log( paths);
	// console.log(JSON.stringify(i_progresses));
	n_WalkDir({
		"paths": paths,
		"wfunc": st_UpdateProgressesWalkReceived,
		"progresses": progresses,
		"paths_skip_save": paths_skip_save,
		"info": 'walk up statuses',
		"rufiles": ['status'],
		"lookahead": ['status']
	});
}
function st_UpdateProgressesWalkReceived(i_walks, i_args)
{
	if (i_walks == null)
		return;

	// console.log(JSON.stringify(i_args));
	var paths = i_args.paths;
	var progresses = i_args.progresses;

	// Update only progress in navig:
	var navig_params_update = {};
	navig_params_update.progress = true;

	for (var w = i_walks.length - 1; w >= 0; w--)
	{
		// console.log(JSON.stringify(i_walks[w]));
		if (i_walks[w].error)
		{
			c_Error(i_walks[w].error);
			return;
		}
		if ((i_walks[w].folders == null) || (i_walks[w].folders.length == 0))
		{
			c_Error('Can`t find folders in ' + paths[w]);
			return;
		}

		var progress = 0;
		var progress_count = 0;
		for (var f = 0; f < i_walks[w].folders.length; f++)
		{
			var folder = i_walks[w].folders[f];

			if (folder.name == RUFOLDER)
				continue;
			if (c_AuxFolder(folder))
				continue;

			let path = paths[w] + '/' + folder.name;

			let cur_progress = 0;

			if ((progresses[path] != null) && (progresses[path] != -1))
			{
				cur_progress = progresses[path];
			}
			else
			{
				if ((folder.status == null) || (folder.status.progress == null))
				{
					// Here we set and save 0% progress on a neighbour folders,
					// if status or progress is not set at all:

					// Siblings are only at last walk ( earlier are parents )
					if (w != (i_walks.length - 1))
						continue;

					// Save only progress:
					st_Save({"progress": 0}, path, null, null, navig_params_update);
				}
				else
					cur_progress = folder.status.progress;
			}

			// Skip aux folders:
			if (cur_progress < 0)
				continue;

			if (cur_progress > 100)
				cur_progress = 100;

			if (folder.status)
			{
				if (folder.status.tasks)
				{
					if (cur_progress < 100)
						cur_progress = 0;
				}
			}

			progress += cur_progress;

			progress_count++;
		}

		if (progress_count == 0)
		{
			return;
		}

		progress = Math.floor(progress / progress_count);
		progresses[paths[w]] = progress;

		// console.log(paths[w]+': '+progress_count+': '+progress);
	}

	for (var path in progresses)
	{
		if (i_args.paths_skip_save.indexOf(path) != -1)
			continue;

		st_Save({"progress": progresses[path]}, path, /*func=*/null, /*args=*/null, navig_params_update);
		// console.log(path);
	}
}
