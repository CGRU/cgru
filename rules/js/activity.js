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
	activity.js - Task is a GUI item, it stands for an artist activity
*/

"use strict";

var activity_Current = null;
var activity_Selected = null;
var activity_Filter = [];
var activity_Items = {};

function activity_Init()
{
//console.log(localStorage.activity);

	let array = c_Parse(localStorage.activity);
	if (null == array)
		return;

	for (let item of array)
		activity_AddItem(item);
}

function activity_Finish()
{
//console.log('activity_Finish')
	if (activity_Selected)
	{
		activity_Items[activity_Selected].el.classList.remove('selected');
		activity_Selected = null;
	}
}

function activity_ChangeCmd(i_cmd)
{
	if (null == i_cmd)
		return null;

	let activity = '--activity '
	if (activity_Selected)
		activity += activity_Selected;
	else
		activity = '';

	return i_cmd.replace(/@ACTIVITY@/g, activity);
}

function activity_Set(i_activity)
{
	if (activity_Selected == i_activity)
		return;


	// Deselect all pinned items and remove unpinned
	for (let a in activity_Items)
	{
		let item = activity_Items[a];
		if (item.pinned)
		{
			item.selected = false;
			item.el.classList.remove('selected');
			continue;
		}

		$('activity').removeChild(item.el);
		delete activity_Items[a];
	}

	if (null == i_activity)
	{
		activity_Changed();
		return;
	}

	let item = activity_Items[i_activity];
	if (item)
	{
		activity_Current = i_activity;
		item.el.classList.add('selected');
		item.selected = true;
		activity_Changed();
		return;
	}

	item = {};
	item.name = i_activity;
	item.selected = true;

	activity_AddItem(item);
}

function activity_AddItem(i_item)
{
	let el = document.createElement('div');
	el.onclick = function(){activity_Clicked(i_item.name);}
	el.classList.add('notselectable');
	$('activity').appendChild(el);
	i_item.el = el;

	let elBtnDel = document.createElement('div');
	elBtnDel.classList.add('button','delete','right');
	elBtnDel.onclick = function(){activity_Delete(i_item.name);}
	el.appendChild(elBtnDel);

	let elName = document.createElement('div');
	elName.classList.add('name');
	elName.textContent = i_item.name;
	el.appendChild(elName);

	if (i_item.selected) i_item.el.classList.add('selected');
	if (i_item.pinned  ) i_item.el.classList.add('pinned');
	if (i_item.filter  ) i_item.el.classList.add('filter');

	activity_Items[i_item.name] = i_item;

	activity_Changed();
}

function activity_Clicked(i_activity)
{
	let item = activity_Items[i_activity];
	if (null == item)
		return;

	if ( ! item.pinned)
	{
		item.pinned = true;
		item.el.classList.add('pinned');
		activity_Changed();
		return;
	}

	if (item.filter)
	{
		item.filter = false;
		item.el.classList.remove('filter');
	}
	else
	{
		item.filter = true;
		item.el.classList.add('filter');
	}

	activity_Changed();
}

function activity_Delete(i_activity)
{
	let item = activity_Items[i_activity];
	if (null == item)
		return;

	$('activity').removeChild(item.el);
	delete activity_Items[item.name];

	activity_Changed();
}

function activity_Changed()
{
	activity_Current = null;
	activity_Selected = null;
	activity_Filter = [];

	let array = [];

	for (let i in activity_Items)
	{
		let item = activity_Items[i];

		if (item.selected)
		{
			activity_Current = item.name;
			activity_Selected = item.name;
		}

		if (item.filter)
			activity_Filter.push(item.name);

		let obj = {};
		obj.name = item.name;
		if (item.selected) obj.selected = true;
		if (item.pinned  ) obj.pinned   = true;
		if (item.filter  ) obj.filter   = true;

		array.push(obj);
	}

	for (let elBadge of document.getElementsByClassName('task_badge'))
		activity_TaskBadgeFilter(elBadge);

	activity_ApplyFilter();


	for (let el of document.getElementsByClassName('show_on_activity'))
		if (activity_Selected)
			el.style.display = 'block';
		else
			el.style.display = 'none';

	//console.log(JSON.stringify(array));
	localStorage.activity = JSON.stringify(array);
}

function activity_TaskBadgeFilter(i_badge)
{
	if ((activity_Filter.length == 0) || (activity_Filter.indexOf(i_badge.m_name) != -1))
		i_badge.classList.remove('activity_filtered');
	else
		i_badge.classList.add('activity_filtered');
}

function activity_ApplyFilter()
{
	for (let el of document.getElementsByClassName('activity_filter'))
		activity_ApplyFilterElement(el);
}
function activity_ApplyFilterElement(i_el)
{
	i_el.classList.remove('activity_filtered');

	if (activity_Filter.length == 0)
		return;

	if (i_el.m_tags)
		for (let tag of i_el.m_tags)
			if (activity_Filter.indexOf(tag) != -1)
				return;

	i_el.classList.add('activity_filtered');
}
