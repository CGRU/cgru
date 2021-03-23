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
	bookmarks.js - TODO: description
*/

"use strict";

var bm_initialized = false;

var bm_projects = [];
var bm_elements = [];

function bm_Init()
{
	// Init localStorage:
	if (localStorage.bookmarks_opened == null)
		localStorage.bookmarks_opened = 'false';

	if (localStorage.bookmarks_projects_opened == null)
		localStorage.bookmarks_projects_opened = '';

	if (localStorage.bookmarks_thumbnails_show == null)
		localStorage.bookmarks_thumbnails_show = 'false';


	// Bookmarks are not available for guests:
	if (g_auth_user == null)
		return;

	// Bookmarks are available only for artists:
	if (c_IsNotAnArtist())
		return;

	$('sidepanel_bookmarks').style.display = 'block';
	bm_initialized = true;
}

function bm_InitConfigured()
{
	if (false == bm_initialized)
		return;

	if (localStorage.bookmarks_opened == 'true')
		bm_Open(false);
	else
		bm_Show();

	if (RULES.bookmarks.refresh == null)
		return;
	if (RULES.bookmarks.refresh < 1)
		return;
	setInterval(bm_Load, RULES.bookmarks.refresh * 1000);
}

function bm_OnClick()
{
	if ($('sidepanel').classList.contains('opened'))
	{
		if ($('sidepanel_bookmarks').classList.contains('opened'))
			bm_Close();
		else
			bm_Open();
	}
	else
	{
		u_SidePanelOpen();
		bm_Open();
	}
}

function bm_Close()
{
	$('sidepanel_bookmarks').classList.remove('opened');
	$('bookmarks').innerHTML = '';
	localStorage.bookmarks_opened = 'false';
}

function bm_Open(i_load)
{
	if (false == bm_initialized)
		return;

	$('sidepanel_bookmarks').classList.add('opened');
	localStorage.bookmarks_opened = 'true';

	if (i_load !== false)
		bm_Load({"info": 'open'});
	else
		bm_Show();
}

function bm_Load(i_args)
{
	if (false == bm_initialized)
		return;

	if (i_args == null)
		i_args = {};

	if (i_args.func == null)
		i_args.func = bm_Show;

	if (i_args.info == null)
		i_args.info = 'load';

	var filename = 'users/' + g_auth_user.id + '.json';
	n_Request({
		'send': {'getobjects': {'file': filename, 'objects': ['bookmarks']}},
		'func': bm_Received,
		'args': i_args,
		'info': 'bookmarks ' + i_args.info
	});
}

function bm_Received(i_user, i_args)
{
	if (false == bm_initialized)
		return;

	// console.log('nw_NewsReceived()');
	if (i_user == null)
		return;
	if (i_user.error)
	{
		c_Error(i_user.error);
		return;
	}

	if (i_user.bookmarks == null)
		i_user.bookmarks = [];

	g_auth_user.bookmarks = [];
	for (var i = 0; i < i_user.bookmarks.length; i++)
		if (i_user.bookmarks[i])
			g_auth_user.bookmarks.push(i_user.bookmarks[i]);

	//	i_args.args.func( i_args.args);
	bm_Show();
}

function bm_Compare(a, b)
{
	if (a == null)
		return 1;
	if (b == null)
		return -1;
	if (a.path > b.path)
		return 1;
	if (a.path < b.path)
		return -1;
	return 0;
}

function bm_Show()
{
	if (false == bm_initialized)
		return;

	// console.log(JSON.stringify(g_auth_user.bookmarks));
	$('bookmarks').innerHTML = '';
	$('bookmarks_label').textContent = 'Bookmarks';
	bm_projects = [];
	bm_elements = [];

	if ((g_auth_user.bookmarks == null) || (g_auth_user.bookmarks.length == 0))
		return;

	$('bookmarks_label').textContent = 'Bookmarks - ' + g_auth_user.bookmarks.length;

	g_auth_user.bookmarks.sort(bm_Compare);

	// Collect projects:
	let project = null;
	for (let i = 0; i < g_auth_user.bookmarks.length; i++)
	{
		let bm = g_auth_user.bookmarks[i];

		if (bm == null)
			continue;

		let names = bm.path.split('/');

		if ((project == null) || (project.name != names[1]))
		{
			project = {};
			project.name = names[1];
			project.bms = [];

			bm_projects.push(project);
		}

		project.bms.push(bm);
	}

	// Construct elements:
	let opened = localStorage.bookmarks_projects_opened.split('|');
	for (let p = 0; p < bm_projects.length; p++)
	{
		let project = bm_projects[p];

		// Project element:
		project.el = document.createElement('div');
		$('bookmarks').appendChild(project.el);
		project.el.classList.add('project');
		if (opened.indexOf(project.name) != -1)
			project.el.classList.add('opened');
		else
			project.el.classList.add('closed');

		// Project label:
		let el = document.createElement('div');
		project.elLabel = el;
		project.el.appendChild(el);
		el.classList.add('label');
		el.onclick = bm_ProjectClicked;
		el.m_project = project;

		// Project bookmarks:
		let highlighted = 0;
		let folder_label = null;
		for (let b = 0; b < project.bms.length; b++)
		{
			let bm = project.bms[b];

			// Folder label:
			let label = bm.path.split('/');
			if (label.length > 3)
				label = label.slice(3,-1);
			label = label.join(' / ');
			if (label != folder_label)
			{
				let el = document.createElement('div');
				project.el.appendChild(el);
				el.classList.add('bm_folder_label');
				el.textContent = label;
				folder_label = label;
			}

			// Bookmark element:
			let el = bm_CreateBookmark(bm);
			bm_elements.push(el);
			project.el.appendChild(el);
			if (el.highlighted)
				highlighted++;
		}

		let label = project.name + ' - ' + project.bms.length;
		if (highlighted)
			label += ' (' + highlighted + ')';
		project.elLabel.textContent = label;
	}

	bm_HighlightCurrent();
	bm_ThumbnailsShowHide();

	bm_DeleteObsoleteForTime();
}

function bm_CreateBookmark(i_bm)
{
	let name = i_bm.path.split('/');
	name = name[name.length-1];

	let el = document.createElement('div');
	el.classList.add('bookmark');
	el.highlighted = false;

	let elDel = document.createElement('div');
	el.appendChild(elDel);
	elDel.classList.add('button');
	elDel.classList.add('delete');
	elDel.m_path = i_bm.path;
	elDel.ondblclick = function(e) { bm_Delete([e.currentTarget.m_path]); };
	elDel.title = 'Double click to delete.';

	var elPath = document.createElement('a');
	el.appendChild(elPath);
	elPath.classList.add('name');
	elPath.textContent = name;
	elPath.href = '#' + i_bm.path;

	// Display status:
	st_SetElStatus(el, i_bm.status, /*show all tasks = */ false);

	var tooltip = '';
	if (i_bm.cuser)
		tooltip += 'Created by: ' + c_GetUserTitle(i_bm.cuser) + '\n';
	if (i_bm.ctime)
		tooltip += 'Created at: ' + c_DT_StrFromSec(i_bm.ctime) + '\n';
	if (i_bm.muser)
		tooltip += 'Modified by: ' + c_GetUserTitle(i_bm.muser) + '\n';
	if (i_bm.mtime)
		tooltip += 'Modified at: ' + c_DT_StrFromSec(i_bm.mtime) + '\n';
	el.title = tooltip;

	if (false == bm_ActualStatus(i_bm.status))
		el.classList.add('obsolete');

	el.m_bookmark = i_bm;

	return el;
}

function bm_ProjectClicked(i_evt)
{
	var el = i_evt.currentTarget.m_project.el;
	el.classList.toggle('opened');
	el.classList.toggle('closed');

	var list = '';
	for (var p = 0; p < bm_projects.length; p++)
	{
		if (bm_projects[p].el.classList.contains('closed'))
			continue;

		if (list.length)
			list += '|';

		list += bm_projects[p].name;
	}

	localStorage.bookmarks_projects_opened = list;
}

function bm_NavigatePost()
{
	if ($('sidepanel').classList.contains('opened') != true)
		return;

	bm_HighlightCurrent();
}

function bm_ActualStatus(i_status)
{
	if (i_status == null)
		return false;

	if (i_status.tasks && (typeof i_status.tasks == 'object'))
	{
		for (let t in i_status.tasks)
		{
			let task = i_status.tasks[t];

			if (task.deleted)
				continue;
			if (task.artists && (task.artists.indexOf(g_auth_user.id) == -1))
				continue;
			if (task.progress && (task.progress >= 100))
				continue;
			if (task.flags && (task.flags.indexOf('omit')) != -1)
				continue;

			return true;
		}
	}

	if (i_status.artists == null)
		return false;

	if (i_status.artists.indexOf(g_auth_user.id) == -1)
		return false;

	if (i_status.progress)
	{
		if (i_status.progress >= 100)
			return false;
	}

	if (i_status.flags)
	{
		if (i_status.flags.indexOf('omit') != -1)
			return false;
	}

	return true;
}

function bm_StatusesChanged(i_args)
{
	bm_Load({'info': 'statuses'});
}

function bm_HighlightCurrent()
{
	var path = g_CurPath();

	for (var i = 0; i < bm_elements.length; i++)
	{
		if (path == bm_elements[i].m_bookmark.path)
		{
			bm_elements[i].classList.add('cur_path');
			if (g_CurPathDummy() || (false == bm_ActualStatus(RULES.status)))
			{
				bm_elements[i].classList.add('obsolete');
			}
			// bm_elements[i].scrollIntoView();
		}
		else
		{
			bm_elements[i].classList.remove('cur_path');
		}
	}
}

function bm_Delete(i_paths)
{
	if (i_paths.length == 0)
	{
		c_Error('No bookmarks to delete.');
		bm_Load({"info": 'not deleted'});
		return;
	}

	var obj = {};
	obj.objects = [];
	for (var i = 0; i < i_paths.length; i++)
		obj.objects.push({"path": i_paths[i]});
	obj.delarray = 'bookmarks';

	obj.file = 'users/' + g_auth_user.id + '.json';
	n_Request({"send": {"editobj": obj}, "func": bm_DeleteFinished});
}

function bm_DeleteFinished(i_data)
{
	if ((i_data == null) || (i_data.error))
	{
		c_Error(i_data.error);
		return;
	}

	c_Info('Bookmarks deleted');
	bm_Load({"info": 'deleted'});
}

function bm_DeleteObsoleteOnClick()
{
	let bookmarks_deleted = bm_DeleteObsoleteForTime(true);
	if (bookmarks_deleted == 0)
		c_Info('No obsolete bookmarks found.');
	else
		c_Info('Bookmarks deleted: ' + bookmarks_deleted);
}
function bm_DeleteObsoleteForTime(i_delete_any_time)
{
	let paths = [];
	let cur_seconds = c_DT_CurSeconds();
	for (let i = 0; i < bm_elements.length; i++)
	{
		let el = bm_elements[i];
		let bm = el.m_bookmark;

		if ( ! bm.mtime)
		{
			paths.push(bm.path);
			c_Log('Deleting invalid bookmark: ' + bm.path + ' - no modification time.');
			continue;
		}

		if (false == el.classList.contains('obsolete'))
		{
			// Delete bookmarks that are inactive for a long time
			if ((cur_seconds - bm.mtime) > (RULES.bookmarks.inactive_delete_days * 24 * 60 * 60))
			{
				paths.push(bm.path);
				c_Log('Deleting inactive bookmark: ' + bm.path + ' - '
						+ c_DT_StrFromSec(bm.mtime) + ' - ' + RULES.bookmarks.inactive_delete_days + ' days.');
			}

			continue;
		}

		// Deleta all obsolete bookmarks
		if (i_delete_any_time)
		{
			paths.push(bm.path);
			c_Log('Deleting obsolete bookmark: ' + bm.path);
			continue;
		}

		// Delete obsolete bookmarks if modified time later than configured
		if ((cur_seconds - bm.mtime) > (RULES.bookmarks.obsolete_delete_days * 24 * 60 * 60))
		{
			paths.push(bm.path);
			c_Log('Deleting obsolete bookmark: ' + bm.path + ' - '
					+ c_DT_StrFromSec(bm.mtime) + ' - ' + RULES.bookmarks.obsolete_delete_days + ' days.');
			continue;
		}
	}

	if (paths.length)
		bm_Delete(paths);

	return paths.length;
}

/* ---------------- [ thumbnail functions ] -------------------------------------------------------------- */

function bm_ThumbnailsOnClick()
{
	if (localStorage.bookmarks_thumbnails_show == 'true')
		localStorage.bookmarks_thumbnails_show = 'false';
	else
		localStorage.bookmarks_thumbnails_show = 'true';
	bm_ThumbnailsShowHide();
}

function bm_ThumbnailsShowHide()
{
	if (localStorage.bookmarks_thumbnails_show == 'true')
		bm_ThumbnailsShow();
	else
		bm_ThumbnailsHide();
}

function bm_ThumbnailsShow()
{
	c_ElSetSelected($('bookmarks_thumbs_btn'), true);

	for (var i = 0; i < bm_elements.length; i++)
	{
		var el = bm_elements[i];

		if (el.m_elTh == null)
		{
			el.m_elTh = document.createElement('a');
			el.appendChild(el.m_elTh);
			el.m_elTh.href = '#' + el.m_bookmark.path;

			var img = document.createElement('img');
			el.m_elTh.appendChild(img);
			img.src = RULES.root + el.m_bookmark.path + '/' + RULES.rufolder + '/thumbnail.jpg';
			img.style.display = 'none';
			img.onload = function(i_el) { i_el.currentTarget.style.display = 'block'; }
		}

		el.m_elTh.style.display = 'block';
	}
}

function bm_ThumbnailsHide()
{
	c_ElSetSelected($('bookmarks_thumbs_btn'), false);

	for (var i = 0; i < bm_elements.length; i++)
	{
		var el = bm_elements[i];
		if (el.m_elTh)
			el.m_elTh.style.display = 'none';
	}
}
