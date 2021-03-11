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
	task.js - Task is a GUI item, it stands for a status task
*/

"use strict";

var task_CurrentTasks = [];

var _OLD_TASTKS_ = false;

function tasks_Init()
{
	if (c_IsNotAnArtist())
		$('status_tasks_btn_add_artist').style.display = 'none';
}

// Called on location leave (change)
function tasks_Finish()
{
_OLD_TASTKS_ = false;
$('status_tasks_buttons').style.display = 'block';

	for (let task of task_CurrentTasks)
		task.destroy();

	task_CurrentTasks = [];

	$('status_tasks_label').style.display = 'none';
	$('status_tasks').textContent = '';
}

function task_ShowTasks(i_statusClass)
{
	tasks_Finish();

	if ((null == i_statusClass) || (null == i_statusClass.obj) || (null == i_statusClass.obj.tasks))
		return;

	if (Array.isArray(i_statusClass.obj.tasks) && (i_statusClass.obj.tasks.length == 0))
	{
		delete i_statusClass.obj.tasks;
		return;
	}

	// OLD TASKS
	if (Array.isArray(i_statusClass.obj.tasks))
	{
		_OLD_TASTKS_ = true;
		$('status_tasks_buttons').style.display = 'none';
		let new_tasks = {};
		for (let task of i_statusClass.obj.tasks)
		{
			let tags = task.tags;
			if ((null == tags) || (tags.length == 0))
			{
				$('status_tasks').innerHTML = '<b style="color:darkred;font-size:32px;">TASK(s) HAS NO TAG(s)!!!</b>';
				return;
			}
			let task_name = tags.join('_');
			task.name = task_name;
			new_tasks[task_name] = task;
		}
		for (let task in new_tasks)
			new Task(i_statusClass, new_tasks[task]);

		let el = document.createElement('div');
		el.classList.add('button');
		el.textContent = 'CONVERT OLD TASKS';
		el.onclick = task_CONVERT_OLD_TASKS;
		$('status_tasks').appendChild(el);
	}
	else
	for (let task in i_statusClass.obj.tasks)
		new Task(i_statusClass, i_statusClass.obj.tasks[task]);
}

var tasks_flags_map = {'roto':['masks'],'sim':['fx']};
function task_CONVERT_OLD_TASKS(i_evt)
{
	let statusClass = task_CurrentTasks[0].statusClass;
	statusClass.obj.tasks = {};
	for (let task of task_CurrentTasks)
	{
		statusClass.obj.tasks[task.obj.name] = task.obj;

		// Steal tags and aritsts from status:
		if (statusClass.obj.artists && statusClass.obj.artists.length)
			for (let artist of task.obj.artists)
			{
				let index = statusClass.obj.artists.indexOf(artist);
				if (index != -1)
					statusClass.obj.artists.splice(index, 1);
			}
		if (statusClass.obj.tags && statusClass.obj.tags.length)
			for (let tag of task.obj.tags)
			{
				let index = statusClass.obj.tags.indexOf(tag);
				if (index != -1)
					statusClass.obj.tags.splice(index, 1);
			}

		// Grab flags:
		if (statusClass.obj.flags && statusClass.obj.flags.length)
		{
			let f = 0;
			while (f < statusClass.obj.flags.length)
			{
				let names = statusClass.obj.flags[f].split('_');
				if (names.length != 2) {f++;continue;}

				let tag = names[0];
				let flag = names[1];
				let name = task.obj.name;
				if (name != tag)
					if ((null == tasks_flags_map[name]) || (tasks_flags_map[name].indexOf(tag) == -1))
						{f++;continue;}

				if ( ! RULES.flags[flag]) {f++;continue;}

				task.obj.flags.push(flag);
				if (RULES.flags[flag].p_min)
					task.obj.progress = RULES.flags[flag].p_min;

				statusClass.obj.flags.splice(f, 1);
			}
		}
	}

	statusClass.show();

	let elBtn = document.createElement('div');
	elBtn.classList.add('button');
	elBtn.textContent = 'SAVE NEW STATUS';
	elBtn.onclick = task_CONVERT_OLD_TASKS_save;
	$('status_tasks').appendChild(elBtn);
}

function task_CONVERT_OLD_TASKS_save(i_evt)
{
	let statusClass = task_CurrentTasks[0].statusClass;

	statusClass.save();
	statusClass.show();
}

function task_AddTask()
{
	new Task();
}

function task_AddArtistTask()
{
	let task = {};
	task.artists = [g_auth_user.id];
	task.tags = [];

	if (g_auth_user.tag)
		task.tags = [g_auth_user.tag];

	if (activity_Current)
		task.tags = [activity_Current];

	new Task(null, task);
}

function Task(i_statusClass, i_task)
{
	task_CurrentTasks.push(this);

	this.statusClass = i_statusClass;
	if (this.statusClass == null)
		this.statusClass = st_Status;
	if (this.statusClass == null)
		this.statusClass = {};

	this.obj = i_task;
	if (this.obj == null)
	{
		// This is a new added task
		this.obj = {};
	}

	// If at least one task exists we should unhide tasks label, as it is hidden by default.
	$('status_tasks_label').style.display = 'block';


	// Init
	if (null == this.obj.tags)
		this.obj.tags = [];
	if (null == this.obj.artists)
		this.obj.artists = [];
	if (null == this.obj.flags)
		this.obj.flags = [];
	if (null == this.obj.progress)
		this.obj.progress = 0;
	if (null == this.obj.deleted)
		this.obj.deleted = false;


	this.elParent = $('status_tasks');


	this.elRoot = document.createElement('div');
	this.elRoot.classList.add('task');
	this.elParent.appendChild(this.elRoot);


	this.elShow = document.createElement('div');
	this.elShow.classList.add('show_div');
	this.elRoot.appendChild(this.elShow);


	this.elName = document.createElement('div');
	this.elName.classList.add('name','notselectable');
	this.elName.m_task = this;
	this.elName.onclick = function(e){e.currentTarget.m_task.select()};
	this.elShow.appendChild(this.elName);


	this.elArtists = document.createElement('div');
	this.elArtists.classList.add('artists');
	this.elShow.appendChild(this.elArtists);


	this.elFlags = document.createElement('div');
	this.elFlags.classList.add('flags');
	this.elShow.appendChild(this.elFlags);


if ( ! _OLD_TASTKS_ )
	if (c_CanEditTasks())
	{
		this.elBtnEdit = document.createElement('button');
		this.elBtnEdit.classList.add('button','edit','right');
		this.elBtnEdit.m_task = this;
		this.elBtnEdit.onclick = function(e){e.stopPropagation();e.currentTarget.m_task.edit();}
		this.elShow.appendChild(this.elBtnEdit);
	}


	this.elPercent = document.createElement('div');
	this.elPercent.classList.add('percent');
	this.elShow.appendChild(this.elPercent);


	this.elProgress = document.createElement('div');
	this.elProgress.classList.add('progress');
	this.elShow.appendChild(this.elProgress);

	this.elProgressBar = document.createElement('div');
	this.elProgressBar.classList.add('progressbar');
	this.elProgress.appendChild(this.elProgressBar);


	this.elEdit = document.createElement('div');
	this.elEdit.classList.add('edit_div');
	this.elRoot.appendChild(this.elEdit);


	this.elInfoLeft = document.createElement('div');
	this.elInfoLeft.classList.add('info','left');
	this.elRoot.appendChild(this.elInfoLeft);

	this.elInfoRight = document.createElement('div');
	this.elInfoRight.classList.add('info','right');
	this.elRoot.appendChild(this.elInfoRight);

	this.editing = false;

	// We should start to edit new added task
	if (this.obj.name == null)
		this.edit();
	else
		this.show();

	if (this.obj.deleted)
		this.elRoot.classList.add('deleted');
}

Task.prototype.show = function()
{
	if (activity_Current == this.obj.name)
		this.select();

	//st_SetElTags(this.obj, this.elName);
	this.elName.textContent = this.obj.name;

	st_SetElArtists(this.obj, this.elArtists,/*short = */false,/*clickable = */true);
	st_SetElFlags(this.obj, this.elFlags,/*short = */false,/*clickable = */true);
	st_SetElProgress(this.obj, this.elProgressBar, this.elProgress, this.elPercent);

	let info_left = '';
	if (this.obj.cuser)
		info_left += 'By ' + c_GetUserTitle(this.obj.cuser);
	if (this.obj.cuser)
		info_left += ' ' + c_DT_StrFromSec(this.obj.ctime);
	this.elInfoLeft.textContent = info_left;

	let info_right = '';
	if (this.obj.muser)
		info_right += 'Edit: ' + c_GetUserTitle(this.obj.muser);
	if (this.obj.mtime)
		info_right += ' ' + c_DT_StrFromSec(this.obj.mtime);
	this.elInfoRight.textContent = info_right;
}

Task.prototype.destroy = function()
{
	this.deselect();

	this.elParent.removeChild(this.elRoot);
}

Task.prototype.select = function()
{
	//console.log('Task.prototype.select = ' + this.obj.name);
	// NULL activity set, on task deselect
	if (this.elRoot.classList.contains('selected'))
	{
		for (let task of task_CurrentTasks)
			task.deselect();

		activity_Set(null);
		return;
	}

	for (let task of task_CurrentTasks)
		task.deselect();

	this.elRoot.classList.add('selected');

	activity_Set(this.obj.name);
}

Task.prototype.deselect = function()
{
	this.elRoot.classList.remove('selected');
}

Task.prototype.edit = function()
{
	if (this.editing)
		return;

	this.editing = true;
	this.elShow.style.display = 'none';


	this.elBtnCancel = document.createElement('div');
	this.elBtnCancel.classList.add('button','right');
	this.elBtnCancel.textContent = 'Cancel';
	this.elBtnCancel.m_task = this;
	this.elBtnCancel.onclick = function(e){e.stopPropagation();e.currentTarget.m_task.editCancel();}
	this.elEdit.appendChild(this.elBtnCancel);


	this.elBtnSave = document.createElement('div');
	this.elBtnSave.classList.add('button','right');
	this.elBtnSave.textContent = 'Save';
	this.elBtnSave.m_task = this;
	this.elBtnSave.onclick = function(e){e.stopPropagation();e.currentTarget.m_task.editProcess();}
	this.elEdit.appendChild(this.elBtnSave);


	this.elEditPercentDiv = document.createElement('div');
	this.elEditPercentDiv.classList.add('percent');
	this.elEdit.appendChild(this.elEditPercentDiv);

	this.elEditPercentLabel = document.createElement('div');
	this.elEditPercentLabel.textContent = '%';
	this.elEditPercentLabel.classList.add('label');
	this.elEditPercentDiv.appendChild(this.elEditPercentLabel);

	this.elEditPercentContent = document.createElement('div');
	this.elEditPercentContent.classList.add('content','editing');
	this.elEditPercentContent.contentEditable = true;
	this.elEditPercentContent.textContent = this.obj.progress;
	this.elEditPercentDiv.appendChild(this.elEditPercentContent);

	if (this.obj.name == null)
	{
		// This is a new just added task
		this.editTags = new EditList({
				"name"    : 'tags',
				"label"   : 'Tags:',
				"list"    : this.obj.tags,
				"list_all": RULES.tags,
				"elParent": this.elEdit});
		// Start to edit tags immediately, if no tags provided.
		// As task can't exit without tag(s)
		if (this.obj.tags.length == 0)
			this.editTags.showAllItems();
	}
	else
	{
		// We should not edit tags of an existing task
		this.elEditTags = document.createElement('div');
		this.elEditTags.classList.add('tags');
		this.elEdit.appendChild(this.elEditTags);
		st_SetElTags(this.obj, this.elEditTags);
	}


	if (c_CanAssignArtists())
		this.editAritsts = new EditList({
			"name"    : 'artists',
			"label"   : 'Artists:',
			"list"    : this.obj.artists,
			"list_all": g_users,
			"elParent": this.elEdit});

	this.editFlags = new EditList({
			"name"    : 'flags',
			"label"   : 'Flags:',
			"list"    : this.obj.flags,
			"list_all": RULES.flags,
			"elParent": this.elEdit});


	// If there is no name, we just adding this task.
	// So we should not delete it, we can cancel adding.
	if (this.obj.name)
	{
		this.elBtnDelete = document.createElement('div');
		this.elBtnDelete.classList.add('button','right');
		this.elBtnDelete.textContent = 'Delete';
		this.elBtnDelete.title = 'Double click to delete task.';
		this.elBtnDelete.m_task = this;
		this.elBtnDelete.onclick = function(e){e.stopPropagation();}
		this.elBtnDelete.ondblclick = function(e){e.stopPropagation();e.currentTarget.m_task.editDelete();}
		this.elEdit.appendChild(this.elBtnDelete);
	}
}

Task.prototype.editCancel = function()
{
	if ( ! this.editing)
		return;

	if (this.obj.name == null)
	{
		// New just added task was cancelled
		let index = task_CurrentTasks.indexOf(this);
		if (index != -1)
			task_CurrentTasks.splice(index, 1);
		this.destroy();
		return;
	}

	this.editing = false;
	this.elEdit.textContent = '';
	this.elShow.style.display = 'block';
}

Task.prototype.editProcess = function()
{
	// Initialize nulls
	let progress = null;
	let artists = null;
	let flags = null;

	// Set values to statuses
	let progress_changed = false;

	if (this.obj.name == null)
	{
		// This is a new added task
		let selected_tags = this.editTags.getSelectedNames();
		if (selected_tags)
			this.obj.tags = selected_tags;
		if ((null == this.obj.tags) || (this.obj.tags.length == 0))
		{
			// Tag(s) must be selected.
			// It is a mandatory attribute.
			this.elInfoLeft.innerHTML = '<b style="font-size: 14px; color: darkred">You should select task tag(s).<b>';
			return;
		}

		// Construct name from tags
		this.obj.name = this.obj.tags.join('_');

		// Add this new task object to status object
		if (null == this.statusClass.obj)
		{
			// There was not any status at all
			this.statusClass.obj = {};
			RULES.status = this.statusClass.obj;
		}
		if (null == this.statusClass.obj.tasks)
			this.statusClass.obj.tasks = {};
		this.statusClass.obj.tasks[this.obj.name] = this.obj;

		// On a new task, total progress should be recalculated
		progress_changed = true;

		// Set creation user and time:
		this.obj.cuser = g_auth_user.id;
		this.obj.ctime = c_DT_CurSeconds();
	}
	else
	{
		// Set modification user and time:
		this.obj.muser = g_auth_user.id;
		this.obj.mtime = c_DT_CurSeconds();
	}


	// Get values
	let progress_edit = this.elEditPercentContent.textContent;
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

	artists = this.editAritsts.getSelectedNames();
	flags   = this.editFlags.getSelectedNames();


	// Set values:
	// Flags min, max progress and exclusiveness:
	if (null !== flags){
		this.obj.flags = [];
		let p_min = null;
		let p_max = null;
		for (let f of flags)
		{
			let rFlag = RULES.flags[f];
			if (rFlag)
			{
				if (rFlag.excl)
				{
					this.obj.flags = [];
					p_min = null;
					p_max = null;
				}
				if (rFlag.p_min && ((null === p_min) || (p_min > rFlag.p_min)))
					p_min = rFlag.p_min;
				if (rFlag.p_max && ((null === p_max) || (p_max > rFlag.p_max)))
					p_max = rFlag.p_max;
			}
			this.obj.flags.push(f);
		}

		if (p_min && (progress < p_min))
			progress = p_min;
		else if (p_max && (progress > p_max))
			progress = p_max;
	}
	// Progress:
	if ((null !== progress) && (this.obj.progress != progress))
	{
		this.obj.progress = progress;
		progress_changed = true;
	}
	// Artists:
	if (null !== artists )
		this.obj.artists = artists;

	this.save(progress_changed);
}

Task.prototype.save = function(i_progress_changed)
{
	// Save constructed status
	//this.statusClass.save();
	let obj = {};
	obj.tasks = {};
	obj.tasks[this.obj.name] = this.obj;


	// Set status modification time
	obj.muser = g_auth_user.id;
	obj.mtime = c_DT_CurSeconds();
	this.statusClass.obj.muser = obj.muser;
	this.statusClass.obj.mtime = obj.mtime;


	// Grab ready and done flags (OLD style TRACKDONE, ANIMREADY)
	if (this.statusClass.obj.flags && this.statusClass.obj.flags.length)
	{
		let found = false;
		let f = 0;
		while (f < this.statusClass.obj.flags.length)
		{
			let keys = this.statusClass.obj.flags[f].split('_');

			if (keys[0] == 'compoz') keys[0] = 'comp';
			if (keys[0] == 'masks')  keys[0] = 'roto';

			if ((keys.length == 2) && (this.obj.name == keys[0]) && (RULES.flags[keys[1]]))
			{
				if (this.obj.flags.indexOf(keys[1]) == -1)
					this.obj.flags.push(keys[1]);
				this.statusClass.obj.flags.splice(f, 1);
				if (RULES.flags[keys[1]].p_min)
				{
					this.obj.progress = RULES.flags[keys[1]].p_min;
					i_progress_changed = true;
				}
				found = true;
				continue;
			}
			f++;
		}

		obj.flags = this.statusClass.obj.flags;
	}


	// Progress
	let progresses = {};
	if (i_progress_changed)
	{
		let avg_progress = 0;
		let num_tasks = 0;
		for (let t in this.statusClass.obj.tasks)
		{
			let task = this.statusClass.obj.tasks[t];
			if (task.deleted)
				continue;

			avg_progress += task.progress;
			num_tasks += 1;
		}
		avg_progress = Math.floor(avg_progress / num_tasks);
		progresses[this.statusClass.path] = avg_progress;
		this.statusClass.obj.progress = avg_progress;
		obj.progress = avg_progress;
	}

	// Remove status tags, artists and flags if the same has task
	for (let arr of ['tags','artists','flags'])
		if (this.statusClass.obj[arr] && this.statusClass.obj[arr].length)
		{
			let found = false;
			for (let item of this.obj[arr])
			{
				let index = this.statusClass.obj[arr].indexOf(item);
				if (index != -1)
				{
					this.statusClass.obj[arr].splice(index, 1);
					found = true;
				}
			}
			if (found)
				obj[arr] = this.statusClass.obj[arr];
		}

	// Save constucted status object
	st_Save(obj);

	// Task will be recreated on status show
	//	this.editCancel();
	//	this.show();
	this.statusClass.show();

	if (i_progress_changed)
		st_UpdateProgresses(this.statusClass.path, progresses);

	// News & Bookmarks:
	nw_StatusesChanged([this.statusClass]);
}

Task.prototype.editDelete = function()
{
	let index = task_CurrentTasks.indexOf(this);
	if (index != -1)
		task_CurrentTasks.splice(index, 1);

	// Set modification user and time:
	this.obj.muser = g_auth_user.id;
	this.obj.mtime = c_DT_CurSeconds();

	this.obj.deleted = true;

	this.save(this.obj.progress);
}

function task_DrawBadges(i_status, i_el, i_args)
{
	if (null == i_args)
		i_args = {};

	let update = i_args.update;
	let short_names = ! i_args.full_names;
	let only_my = i_args.only_my;

	if ( ! update)
		i_el.textContent = '';

	if (null == i_status)
		return;
	if (null == i_status.tasks)
		return;

	if (null == i_el.m_elTasks)
		i_el.m_elTasks = {};

	for (let t in i_status.tasks)
	{
		let task = i_status.tasks[t];
		if (update && i_el.m_elTasks[task.name])
			i_el.m_elTasks.removeChild(i_el.m_elTasks[task.name]);

		if (task.deleted)
			continue;

		if (only_my)
		{
			if (null == g_auth_user) continue;
			if (null == task.artists) continue;
			if (task.artists.indexOf(g_auth_user.id) == -1) continue;
		}

		let elTask = document.createElement('div');
		elTask.classList.add('task_badge');
		elTask.m_name = task.name;
		i_el.appendChild(elTask);
		i_el.m_elTasks[task.name] = this;

		let elName = document.createElement('div');
		elName.classList.add('name');
		elName.textContent = task.tags.join('_');
		elTask.appendChild(elName);

		if ( ! i_args.hide_artists)
		{
			let elArtists = document.createElement('div');
			elArtists.classList.add('artists');
			st_SetElArtists(task, elArtists, short_names);
			elTask.appendChild(elArtists);
		}

		let elFlags = document.createElement('div');
		st_SetElFlags(task, elFlags, short_names);
		elTask.appendChild(elFlags);

		let elProgress = document.createElement('div');
		elProgress.classList.add('progress');
		elTask.appendChild(elProgress);

		let elProgressBar = document.createElement('div');
		elProgressBar.classList.add('progressbar');
		elProgress.appendChild(elProgressBar);

		st_SetElProgress(task, elProgressBar, elProgress);

		activity_TaskBadgeFilter(elTask);
	}
}

function task_StatusTagClicked(i_name)
{
	let artists = [];

	if (st_Status && st_Status.obj && st_Status.obj.artists && st_Status.obj.artists.length)
		for (let art of st_Status.obj.artists)
			if (g_users[art] && (g_users[art].tag == i_name))
				artists.push(art);

	new Task(null, {'tags':[i_name],'artists':artists});
}
function task_StatusArtistClicked(i_name)
{
	let tags = [];

	if (g_users[i_name] && g_users[i_name].tag)
		tags.push(g_users[i_name].tag);

	new Task(null, {'artists':[i_name],'tags':tags});
}

