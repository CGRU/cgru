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

function tasks_Init()
{
	if (c_IsNotAnArtist())
		$('status_tasks_btn_add_artist').style.display = 'none';
}

// Called on location leave (change)
function tasks_Finish()
{
	for (let task of task_CurrentTasks)
		task.destroy();

	task_CurrentTasks = [];

	$('status_tasks_label').style.display = 'none';
	$('status_tasks').textContent = '';
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

function Task(i_statusClass, i_task, i_args)
{
	this.statusClass = i_statusClass;
	if (this.statusClass == null)
		this.statusClass = st_Status;
	if (this.statusClass == null)
		this.statusClass = {};

	this.multi = false;
	if (this.statusClass.multi)
		this.multi = true;

	if (false == this.multi)
		task_CurrentTasks.push(this);

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


	this.elParent = this.statusClass.elTasks;


	this.elRoot = document.createElement('div');
	this.elRoot.classList.add('task');
	this.elParent.appendChild(this.elRoot);


	this.elShow = document.createElement('div');
	this.elShow.classList.add('show_div');
	this.elRoot.appendChild(this.elShow);


	this.elName = document.createElement('div');
	this.elName.classList.add('name','notselectable');
	this.elName.m_task = this;
	if (false == this.multi)
		this.elName.onclick = function(e){e.currentTarget.m_task.select()};
	this.elShow.appendChild(this.elName);


	this.elArtists = document.createElement('div');
	this.elArtists.classList.add('artists');
	this.elShow.appendChild(this.elArtists);


	this.elFlags = document.createElement('div');
	this.elFlags.classList.add('flags');
	this.elShow.appendChild(this.elFlags);


	if (c_CanEditTask(this.obj))
	{
		this.elBtnEdit = document.createElement('button');
		this.elBtnEdit.classList.add('button','edit','right');
		this.elBtnEdit.m_task = this;
		this.elBtnEdit.onclick = function(e){e.stopPropagation();e.currentTarget.m_task.reloadBeforeEdit();}
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


	this.elAnnotation = document.createElement('div');
	this.elAnnotation.classList.add('annotation');
	this.elShow.appendChild(this.elAnnotation);


	this.elInfoLeft = document.createElement('div');
	this.elInfoLeft.classList.add('info','left');
	this.elRoot.appendChild(this.elInfoLeft);

	this.elInfoRight = document.createElement('div');
	this.elInfoRight.classList.add('info','right');
	this.elRoot.appendChild(this.elInfoRight);

	this.editing = false;

	// We should start to edit new added task
	if (this.obj.name == null)
		this.edit(i_args);
	else
		this.show();

	if (this.obj.deleted)
		this.elRoot.classList.add('deleted');
}

Task.prototype.show = function()
{
	if ((activity_Current == this.obj.name) && (false == this.multi))
		this.select();

	//st_SetElTags(this.obj, this.elName);
	this.elName.textContent = this.obj.name;

	st_SetElArtists(this.obj, this.elArtists,/*short = */false,/*clickable = */true);
	st_SetElFlags(this.obj, this.elFlags,/*short = */false,/*clickable = */true);
	st_SetElProgress(this.obj, this.elProgressBar, this.elProgress, this.elPercent);

	this.elAnnotation.textContent = this.obj.annotation;

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

Task.prototype.reloadBeforeEdit = function()
{
	n_GetFile({
		"path": c_GetRuFilePath('status.json', this.statusClass.path),
		"func": task_StatusReceived,
		"task_object": this,
		"info": 'status.task',
		"parse": true,
		"local": true,
		"cache_time":-1
	});
}

function task_StatusReceived(i_data, i_args)
{
	if (null == i_data.status)
		c_Error("Invalid status received:<br>" + JSON.stringify(i_data));
	else
		i_args.task_object.updateOrEdit(i_data.status);
}

Task.prototype.updateOrEdit = function(i_status)
{
	if (i_status.mtime > this.statusClass.obj.mtime)
		this.statusClass.update(i_status);
	else
		this.edit();
}

Task.prototype.edit = function(i_args)
{
	if (this.editing)
		return;

	this.editing = true;
	this.elRoot.classList.add('edit');
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
	this.elBtnSave.onclick = function(e){e.stopPropagation();e.currentTarget.m_task.editProcess(i_args);}
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


	this.elEditAnnotationDiv = document.createElement('div');
	this.elEditAnnotationDiv.classList.add('annotation');
	this.elEdit.appendChild(this.elEditAnnotationDiv);

	this.elEditAnnotationLabel = document.createElement('div');
	this.elEditAnnotationLabel.classList.add('label');
	this.elEditAnnotationLabel.contentEditable = true;
	this.elEditAnnotationLabel.textContent = 'Annotation:';
	this.elEditAnnotationDiv.appendChild(this.elEditAnnotationLabel);

	this.elEditAnnotationContent = document.createElement('div');
	this.elEditAnnotationContent.classList.add('content','editing');
	this.elEditAnnotationContent.contentEditable = true;
	this.elEditAnnotationContent.textContent = this.obj.annotation;
	this.elEditAnnotationDiv.appendChild(this.elEditAnnotationContent);


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
		this.elBtnDelete.ondblclick = function(e){e.stopPropagation();e.currentTarget.m_task.editDelete(i_args);}
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
	this.elRoot.classList.remove('edit');
	this.elEdit.textContent = '';
	this.elShow.style.display = 'block';
}

Task.prototype.editProcess = function(i_args)
{
	if (i_args == null)
		i_args = {};

	let this_is_a_new_task = false;

	// Store previous progress to find out that it was changed
	let progress_prevous = this.obj.progress;

	if (this.obj.name == null)
	{
		// This is a new added task
		this_is_a_new_task = true;

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

	// Get progress
	let progress_edit = this.elEditPercentContent.textContent;
	if (progress_edit.length && (progress_edit != st_MultiValue))
	{
		progress_edit = c_Strip(progress_edit);
		let progress = parseInt(progress_edit);
		if (isNaN(progress))
		{
			progress = 0;
			c_Error('Invalid progress: ' + c_Strip(progress_edit));
		}
		if (progress < -1)
			progress = -1;
		if (progress > 100)
			progress = 100;

		this.obj.progress = progress;
	}

	// Get flags.
	// We should do it after progress, as flags can limit it.
	st_SetStatusFlags(this.obj, this.editFlags.getSelectedObjects());

	// Artists:
	if (c_CanAssignArtists())
	{
		let artists = this.editAritsts.getSelectedNames();
		if (null !== artists)
			this.obj.artists = artists;
	}

	// Annotation:
	let annotation = this.elEditAnnotationContent.textContent;
	if (annotation)
	{
		annotation = c_Strip(annotation);
		if (annotation.length)
			this.obj.annotation = annotation;
		else
			this.obj.annotation = '';
	}
	else if (this.obj.annotation)
		this.obj.annotation = '';


	if (0)//document.location.hostname != 'localhost')
	{
	// We should calculate status progress
	// if task progress is changed
	// or if it is a new task
		this.save(this_is_a_new_task || (progress_prevous != this.obj.progress));
		return;
	}

	let obj = {};
	if (i_args.paths && i_args.paths.length)
		obj.paths = i_args.paths;
	else
		obj.paths = [this.statusClass.path];

	let fields = ['name','artists','flags','tags','progress','annotation'];
	for (let f of fields)
		obj[f] = this.obj[f];
	if (nw_disabled)
		obj.nonews = true;

	n_Request({'send':{'settask':obj},'func':st_StatusesSaved,'info':'status.setTask','wait':false});
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

			let koeff = 1;
			if (RULES.tags[t] && RULES.tags[t].koeff)
				koeff = RULES.tags[t].koeff;

			avg_progress += koeff * task.progress;
			num_tasks += koeff;
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

	// News & Bookmarks:
	// Setting change to true, we telling to news what task was changes in status.
	// This way artists that are assigned to task only will
	// receive news only when their task is changed.
	this.obj.changed = true;
	nw_StatusesChanged([this.statusClass]);

	// Save constucted status object
	st_Save(obj);

	// Task will be recreated on status show
	//	this.editCancel();
	//	this.show();
	this.statusClass.show();

	if (i_progress_changed)
		st_UpdateProgresses(this.statusClass.path, progresses);
}

Task.prototype.editDelete = function(i_args = {})
{
	let index = task_CurrentTasks.indexOf(this);
	if (index != -1)
		task_CurrentTasks.splice(index, 1);

	// Set modification user and time:
	this.obj.muser = g_auth_user.id;
	this.obj.mtime = c_DT_CurSeconds();

	this.obj.deleted = true;

	if (0)//document.location.hostname != 'localhost')
	{
	this.save(this.obj.progress);
	return;
	}

	let obj = {'name':this.obj.name,'deleted':true};
	if (i_args.paths && i_args.paths.length)
		obj.paths = i_args.paths;
	else
		obj.paths = [this.statusClass.path];

	if (nw_disabled)
		obj.nonews = true;

	n_Request({'send':{'settask':obj},'func':st_StatusesSaved,'info':'status.setTask','wait':false});
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

	let elBages = [];

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
		if (g_auth_user && (task.artists.indexOf(g_auth_user.id) != -1))
			elTask.classList.add('my');
		else
			elTask.classList.add('notmy');
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

		elBages.push(elTask);
	}

	return elBages;
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

