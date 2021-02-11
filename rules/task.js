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

var CurTasks = [];

function task_ShowTasks(i_statusClass)
{
	$('status_tasks_div').style.display = 'none';
	for (let task of CurTasks)
		task.destroy();
	CurTasks = [];

	if ((null == i_statusClass) || (null == i_statusClass.obj) || (null == i_statusClass.obj.tasks))
		return;

	for (let task of i_statusClass.obj.tasks)
		CurTasks.push(new Task(i_statusClass, task));

	if (CurTasks.length)
		$('status_tasks_div').style.display = 'block';
}

function Task(i_statusClass, i_task, i_args)
{
	this.statusClass = i_statusClass;
	this.obj = i_task;
console.log(this.obj);

	if (null == this.obj.artists)
		this.obj.artists = [];
	if (null == this.obj.flags)
		this.obj.flags = [];
	if (null == this.obj.progress)
		this.obj.progress = 0;


	this.elParent = $('status_tasks');


	this.elRoot = document.createElement('div');
	this.elRoot.classList.add('task');
	this.elParent.appendChild(this.elRoot);


	this.elShow = document.createElement('div');
	this.elShow.classList.add('show');
	this.elRoot.appendChild(this.elShow);


	this.elTags = document.createElement('div');
	this.elTags.classList.add('tags');
	this.elShow.appendChild(this.elTags);


	this.elArtists = document.createElement('div');
	this.elArtists.classList.add('artists');
	for (let artist of this.obj.artists)
	{
		let el = document.createElement('div');
		el.classList.add('artist');
		this.elArtists.appendChild(el);

		let elName = document.createElement('div');
		elName.classList.add('name');
		elName.textContent = c_GetUserTitle(artist);
		el.appendChild(elName);
	}
	this.elShow.appendChild(this.elArtists);


	this.elFlags = document.createElement('div');
	this.elFlags.classList.add('flags');
	for (let flag of this.obj.flags)
	{
		let el = document.createElement('div');
		el.classList.add('flag');
		el.textContent = c_GetFlagTitle(flag);
		el.title = c_GetFlagTip(flag);
		this.elFlags.appendChild(el);
	}
	this.elShow.appendChild(this.elFlags);

	this.elBtnEdit = document.createElement('button');
	this.elBtnEdit.classList.add('button','edit');
	this.elBtnEdit.m_task = this;
	this.elBtnEdit.onclick = function(e){e.currentTarget.m_task.edit();}
	this.elShow.appendChild(this.elBtnEdit);

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
	this.elEdit.classList.add('edit');
	this.elRoot.appendChild(this.elEdit);


	this.editing = false;

	this.show();
}

Task.prototype.show = function()
{
	st_SetElTags(this.obj, this.elTags);
	st_SetElArtists(this.obj, this.elArtists);
	st_SetElFlags(this.obj, this.elFlags);
	st_SetElProgress(this.obj, this.elProgressBar, this.elProgress, this.elPercent);
}

Task.prototype.destroy = function()
{
	this.elParent.removeChild(this.elRoot);
}

Task.prototype.edit = function()
{
	if (this.editing)
		return;

	this.editing = true;
	this.elShow.style.display = 'none';


	this.elEditTags = document.createElement('div');
	this.elEditTags.classList.add('tags');
	this.elEdit.appendChild(this.elEditTags);
	st_SetElTags(this.obj, this.elEditTags);


	this.elBtnCancel = document.createElement('div');
	this.elBtnCancel.classList.add('button');
	this.elBtnCancel.textContent = 'Cancel';
	this.elBtnCancel.m_task = this;
	this.elBtnCancel.onclick = function(e){e.currentTarget.m_task.editCancel();}
	this.elEdit.appendChild(this.elBtnCancel);


	this.elBtnSave = document.createElement('div');
	this.elBtnSave.classList.add('button');
	this.elBtnSave.textContent = 'Save';
	this.elBtnSave.m_task = this;
	this.elBtnSave.onclick = function(e){e.currentTarget.m_task.editSave();}
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
}

Task.prototype.editCancel = function()
{
	if ( ! this.editing)
		return;

	this.editing = false;
	this.elEdit.textContent = '';
	this.elShow.style.display = 'block';
}

Task.prototype.editSave = function()
{
	// Iint nulls
	let progress = null;

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

	// Set values:
	if (progress !== null)
	{
		this.obj.progress = progress;
console.log(this.obj);
	}

	this.editCancel();
	this.show();

	this.statusClass.save();
}

