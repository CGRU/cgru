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

var bm_clicked = false;

var bm_projects = [];
var bm_elements = [];

function bm_Init()
{
	// Init localStorage:
	if (localStorage.bookmarks_opened == null)
		localStorage.bookmarks_opened = 'false';

	if (localStorage.bookmarks_projects_closed == null)
		localStorage.bookmarks_projects_closed = '';

	if (localStorage.bookmarks_scenes_closed == null)
		localStorage.bookmarks_scenes_closed = '';

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

function bm_GetUserFileName()
{
	return ad_GetUserFileName(g_auth_user.id, 'bookmarks');
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

	n_GetFile({
		'path': bm_GetUserFileName(),
		'func': bm_Received,
		'info': 'news',
		'cache_time': -1,
		'parse': true,
		'local': false
	});
}

function bm_Received(i_user, i_args)
{
	if (false == bm_initialized)
		return;

	if (i_user == null)
		return;

	if (i_user.error)
	{
		c_Log(i_user.error);
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

function bm_CollectProjects(i_bookmarks)
{
	i_bookmarks.sort(bm_Compare);

	let collection = [];
	let project = null;
	let scene = null;
	for (let bm of i_bookmarks)
	{
		if (bm == null)
			continue;

		if (bm.path == null)
			continue;

		let names = bm.path.split('/');

		if ((project == null) || (project.name != names[1]))
		{
			project = {};
			project.name = names[1];
			project.scenes = [];
			scene = null;

			collection.push(project);
		}

		let scene_path = null;
		let scene_name = null;
		if (names.length > 3)
		{
			scene_path = names[1] + '/' + names[2] + '/' + names[3];
			scene_name = names[3];
		}
		if ((scene == null) || (scene.path != scene_path))
		{
			scene = {};
			scene.name = scene_name;
			scene.path = scene_path;
			scene.bms = [];

			project.scenes.push(scene);
		}

		scene.bms.push(bm);
	}

	return collection;
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

	// Collect projects and scenes:
	bm_projects = bm_CollectProjects(g_auth_user.bookmarks);

	// Construct elements:
	let closed_projects = localStorage.bookmarks_projects_closed.split('|');
	for (let p = 0; p < bm_projects.length; p++)
	{
		let project = bm_projects[p];

		// Project element:
		project.el = document.createElement('div');
		$('bookmarks').appendChild(project.el);
		project.el.classList.add('project');
		if (closed_projects.includes(project.name))
			project.el.classList.add('closed');
		else
			project.el.classList.add('opened');

		// Project label:
		let el = document.createElement('div');
		project.elLabel = el;
		project.el.appendChild(el);
		el.textContent = project.name;
		el.classList.add('label');
		el.onclick = bm_ProjectClicked;

		// Project statistics
		let prj_flags = {};
		project.elStat = document.createElement('div');
		project.el.appendChild(project.elStat);
		project.elStat.classList.add('statistics')

		// Project scenes:
		let project_count = 0;
		let closed_scenes = localStorage.bookmarks_scenes_closed.split('|');
		for (let s = 0; s < project.scenes.length; s++)
		{
			let scene = project.scenes[s];

			// Scene element:
			scene.el = document.createElement('div');
			project.el.appendChild(scene.el);
			scene.el.classList.add('scene');
			if (closed_scenes.includes(scene.path))
				scene.el.classList.add('closed');
			else
				scene.el.classList.add('opened');

			// Scene label:
			if (scene.name)
			{
				let el = document.createElement('div');
				scene.elLabel = el;
				scene.el.appendChild(el);
				el.textContent = scene.name;
				el.classList.add('label');
				el.onclick = bm_SceneClicked;
			}

			// Scene bookmarks:
			for (let b = 0; b < scene.bms.length; b++)
			{
				let bm = scene.bms[b];

				// Bookmark element:
				let el = bm_CreateBookmark(bm);
				bm_elements.push(el);
				scene.el.appendChild(el);

				if (bm.status && bm.status.flags)
				{
					for (let flag of bm.status.flags)
					{
						if (flag in prj_flags)
							prj_flags[flag] += 1;
						else
							prj_flags[flag] = 1;
					}
				}

				if (bm.status && bm.status.tasks)
				{
					for (let task in bm.status.tasks)
					{
						task = bm.status.tasks[task];
						if (task.deleted)
							continue;
						if (task.artists == null)
							continue;
						if (task.artists.indexOf(g_auth_user.id) == -1)
							continue;
						if (task.flags)
						{
							for (let flag of task.flags)
							{
								// Skip task if the flag was already added at status:
								if (bm.status.flags && bm.status.flags.includes(flag))
									continue;

								if (flag in prj_flags)
									prj_flags[flag] += 1;
								else
									prj_flags[flag] = 1;
							}
						}
					}
				}
			}

			if (scene.elLabel)
			{
				let label = scene.name + ' - ' + scene.bms.length;
				scene.elLabel.textContent = label;

				scene.elLabel.title = scene.path
						+ '\nBookmarks count: ' + scene.bms.length
						+ '\nClick to toggle collapse';
			}

			project_count += scene.bms.length;
		}

		for (let flag in prj_flags)
		{
			let el = document.createElement('div');
			el.textContent = c_GetFlagShort(flag) + ': ' + prj_flags[flag];
			el.title = c_GetFlagTitle(flag);
			el.classList.add('tag','flag');
			let clr = null;
			if (RULES.flags[flag] && RULES.flags[flag].clr)
				clr = RULES.flags[flag].clr;
			if (clr)
				st_SetElColor({"color": clr}, el);

			project.elStat.appendChild(el);
		}

		let label = project.name + ' - ' + project_count;
		project.elLabel.textContent = label;

		project.elLabel.title = 'Project: ' + project.name
				+ '\nBookmarks count: ' + project_count
				+ '\nClick to toggle collapse';
	}

	bm_HighlightCurrent();
	bm_ThumbnailsShowHide();
	activity_ApplyFilter();

	bm_DeleteObsoleteForTime();
}

function bm_CreateBookmark(i_bm)
{
	let name = i_bm.path.split('/');
	name = name[name.length-1];

	let el = document.createElement('div');
	el.classList.add('bookmark');

	let elDel = document.createElement('div');
	el.appendChild(elDel);
	elDel.classList.add('button');
	elDel.classList.add('delete');
	elDel.m_path = i_bm.path;
	elDel.ondblclick = function(e) { bm_Delete([e.currentTarget.m_path]); };
	elDel.title = 'Double click to delete.';

	let elFav = document.createElement('div');
	el.appendChild(elFav);
	elFav.classList.add('favourite_toggle');
	elFav.m_bm = i_bm;
	elFav.title = 'Double click to toggle favourite.';
	elFav.ondblclick = function(e) {bm_FavouriteToggle(e.currentTarget.m_bm);};

	var elPath = document.createElement('a');
	el.appendChild(elPath);
	elPath.classList.add('name');
	elPath.textContent = name;
	elPath.href = '#' + i_bm.path;
	elPath.onclick = function(e){bm_clicked = true};

	el.m_elThumb_div = document.createElement('div');
	el.appendChild(el.m_elThumb_div);
	el.m_elThumb_div.classList.add('thumb_div','activity_filter');
	el.m_elThumb_div.m_tags = [];
	if (i_bm.status && i_bm.status.tasks)
		for (let t in i_bm.status.tasks)
		{
			let task = i_bm.status.tasks[t];
			if (task.artists && (task.artists.indexOf(g_auth_user.id) != -1))
				if (task.tags) for (let tag of task.tags)
					el.m_elThumb_div.m_tags.push(tag);
		}

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

	if (i_bm.favourite)
		el.classList.add('favourite');

	el.m_bookmark = i_bm;

	return el;
}

function bm_ProjectClicked(i_evt)
{
	let el = i_evt.currentTarget.parentElement;
	el.classList.toggle('opened');
	el.classList.toggle('closed');

	let list = '';
	for (let p = 0; p < bm_projects.length; p++)
	{
		if (false == bm_projects[p].el.classList.contains('closed'))
			continue;

		if (list.length)
			list += '|';

		list += bm_projects[p].name;
	}

	localStorage.bookmarks_projects_closed = list;
}

function bm_SceneClicked(i_evt)
{
	let el = i_evt.currentTarget.parentElement;
	el.classList.toggle('opened');
	el.classList.toggle('closed');

	let list = '';
	for (let p = 0; p < bm_projects.length; p++)
	{
		for (let s = 0; s < bm_projects[p].scenes.length; s++)
		{
			let scene =  bm_projects[p].scenes[s];

			if (scene.path == null)
				continue;

			if (false == scene.el.classList.contains('closed'))
				continue;

			if (list.length)
				list += '|';

			list += scene.path;
		}
	}

	localStorage.bookmarks_scenes_closed = list;
}

function bm_NavigatePost()
{
	if ($('sidepanel').classList.contains('opened') != true)
		return;

	bm_HighlightCurrent();
}

function bm_ActualStatus(i_status, i_user)
{
	if (i_status == null)
		return false;

	if (i_user == null)
		i_user = g_auth_user;

	if (i_status.tasks && (typeof i_status.tasks == 'object'))
	{
		for (let t in i_status.tasks)
		{
			let task = i_status.tasks[t];

			if (task.deleted)
				continue;
			if (task.artists && (task.artists.indexOf(i_user.id) == -1))
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

	if (i_status.artists.indexOf(i_user.id) == -1)
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
	for (let i = 0; i < bm_elements.length; i++)
	{
		if (bm_elements[i].m_bookmark.path == g_CurPath())
		{
			bm_elements[i].classList.add('cur_path');

			if ((bm_clicked == false) && (nw_clicked == false))
			{
				bm_elements[i].scrollIntoView({behavior:'auto',block:'center',inline:'center'});
			}

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

	bm_clicked = false;
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

	obj.file = bm_GetUserFileName();
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

function bm_FavouriteToggle(i_bm)
{
	let favourite = (true != i_bm.favourite);
	let obj = {};
	obj.replace = true;
	obj.objects = [];
	obj.objects.push({"path": i_bm.path,"favourite":favourite});
	obj.id = 'path';
	obj.file = bm_GetUserFileName();
	n_Request({"send": {"editobj": obj}, "func": bm_FavouriteToggle_Finished});
}

function bm_FavouriteToggle_Finished(i_data)
{
	if ((i_data == null) || (i_data.error))
	{
		c_Error(i_data.error);
		return;
	}

	c_Info('Bookmark favourite toggled.');
	bm_clicked = true;
	bm_Load({"info": 'favourite'});
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

		if (bm.favourite)
			continue;

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

	for (let i = 0; i < bm_elements.length; i++)
	{
		let el = bm_elements[i];

		if (el.m_elThumb == null)
		{
			el.m_elThumb = document.createElement('a');
			el.m_elThumb_div.appendChild(el.m_elThumb);
			el.m_elThumb.href = '#' + el.m_bookmark.path;

			let img = document.createElement('img');
			el.m_elThumb.appendChild(img);
			img.src = RULES.root + el.m_bookmark.path + '/' + RUFOLDER + '/thumbnail.jpg';
			img.style.display = 'none';
			img.onload = function(i_el) { i_el.currentTarget.style.display = 'block'; }
		}

		el.m_elThumb.style.display = 'block';
		el.classList.add('with_thumb');
	}
}

function bm_ThumbnailsHide()
{
	c_ElSetSelected($('bookmarks_thumbs_btn'), false);

	for (let i = 0; i < bm_elements.length; i++)
	{
		let el = bm_elements[i];
		if (el.m_elThumb)
			el.m_elThumb.style.display = 'none';
		el.classList.remove('with_thumb');
	}
}
