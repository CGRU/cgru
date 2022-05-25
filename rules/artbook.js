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
	artbook.js - All artists bookmarks.
*/

"use strict";

var ab_wnd = null;
var ab_users = [];
var ab_artists = [];

var ab_filter_artists = [];
var ab_filter_projects = [];

var ab_art_pages = [];
var ab_art_projects = [];

var ab_wnd_sort_prop = 'bm_count';
var ab_wnd_sort_dir = 1;

var ab_flags_order = ['check','comment','ready','inprogress'];

function ab_Init()
{
	if (g_auth_user == null)
		return;

	if (false == g_admin)
		return;

	$('artbook_button').style.display = 'block';
}

function ab_OpenWindow(i_close_header)
{
	if (i_close_header)
		u_OpenCloseHeader();
	ab_wnd = new cgru_Window({"name": 'artbook', "title": 'ArtBook', 'padding': '3% 1%'});
	ab_wnd.elContent.classList.add('artbook');

	ab_wnd.elTopPanel = document.createElement('div');
	ab_wnd.elContent.appendChild(ab_wnd.elTopPanel);
	ab_wnd.elTopPanel.classList.add('ab_top_panel');

	let el = document.createElement('div');
	ab_wnd.elTopPanel.appendChild(el);
	el.classList.add('button');
	el.textContent = 'Refresh';
	el.ondblclick = ab_WndRefresh;
	el.style.cssFloat = 'right';

	ab_wnd.elInfo = document.createElement('div');
	ab_wnd.elTopPanel.appendChild(ab_wnd.elInfo);

	ab_wnd.elProjectsDiv = document.createElement('div');
	ab_wnd.elTopPanel.appendChild(ab_wnd.elProjectsDiv);
	ab_wnd.elProjectsDiv.classList.add('ab_projects_div');

	ab_wnd.elArtistsDiv = document.createElement('div');
	ab_wnd.elTopPanel.appendChild(ab_wnd.elArtistsDiv);
	ab_wnd.elArtistsDiv.classList.add('ab_artists_div');

	ab_wnd.elPagesDiv = document.createElement('div');
	ab_wnd.elContent.appendChild(ab_wnd.elPagesDiv);
	ab_wnd.elPagesDiv.classList.add('artbook_pages');

	ab_wnd.elBotPanel = document.createElement('div');
	ab_wnd.elContent.appendChild(ab_wnd.elBotPanel);
	ab_wnd.elBotPanel.classList.add('ab_bot_panel');

	ab_WndRefresh();
}

function ab_WndRefresh()
{
	ab_filter_artists = [];
	ab_filter_projects = [];

	n_Request({
		"send": {'getallusers':true},
		"func": ab_WndArtistsReceived
	});
}

function ab_WndArtistsReceived(i_data)
{
	if (i_data == null)
	{
		ab_wnd.elContent.innerHTML = 'Error getting users.';
		return;
	}
	if (i_data.error)
	{
		ab_wnd.elContent.innerHTML = 'Error getting users:<br>' + i_data.error;
		return;
	}
	if (i_data.users == null)
	{
		ab_wnd.elContent.innerHTML = '"users" are NULL.';
		return;
	}

	ab_users = i_data.users;
	ab_wnd.elArtistsDiv.innerHTML = '';
	ab_wnd.editAritsts = new EditList({
		"name"    : 'artists',
		"label"   : 'Artists:',
		"list"    : [],
		"list_all": ab_users,
		"elParent": ab_wnd.elArtistsDiv,
		"onChange": ab_ArtistsListChanged});

	if (ab_wnd == null)
		return;

	ab_ProcessArtists();
}

function ab_CollectFlags(i_bm)
{
	let flags = [];

	if (null == i_bm.status)
		return flags;

	if (i_bm.status.flags)
		for (let flag of i_bm.status.flags)
			if (flags.indexOf(flag) == -1)
				flags.push(flag);

	if (null == i_bm.status.tasks)
		return flags;

	for (let t in i_bm.status.tasks)
	{
		let task = i_bm.status.tasks[t];
		if (task.flags)
			for (let flag of task.flags)
				if (flags.indexOf(flag) == -1)
					flags.push(flag);
	}

	return flags;
}

function ab_CompareFlags(a,b)
{
	for (let flag of ab_flags_order)
	{
		if (a.indexOf(flag) > b.indexOf(flag))
			return 1;
		if (a.indexOf(flag) < b.indexOf(flag))
			return -1;
	}

	return 0;
}

function ab_CompareBookmarks(a,b)
{
	return ab_CompareFlags(ab_CollectFlags(a), ab_CollectFlags(b));
}

function ab_ProcessArtists()
{
	ab_artists = [];
	let prj_infos_obj = [];
	let disabled_artists = 0;
	let invalid_bookmarks = 0;
	let total_artists = 0;
	let total_projects = 0;
	let total_bookmarks = 0;
	let obsolete_bookmarks = 0;
	for (let uid in ab_users)
	{
		let user = ab_users[uid];

		// Skip not an atrists
		if (c_IsNotAnArtist(user))
			continue;
		
		// Show only selected, or all if no selection
		if (ab_filter_artists.length && (ab_filter_artists.indexOf(user.id) == -1))
			continue;

		// Create an empty bookmarks array if it does not exist,
		// to not to check each time, that bookmarks are not null.
		if (null == user.bookmarks)
			user.bookmarks = [];

		let bookmarks = [];
		// Check bookmarks validness:
		for (let bm of user.bookmarks)
		{
			if (null == bm.path)
			{
				console.log(JSON.stringify(bm));
				invalid_bookmarks += 1;
				continue;
			}

			if (false == bm_ActualStatus(bm.status, user))
			{
				obsolete_bookmarks += 1;
				continue;
			}

			bookmarks.push(bm);
		}

		let artist =  user;
		artist.projects = [];
		artist.bm_count = 0;
		let projects = bm_CollectProjects(bookmarks);
		for (let prj of projects)
		{
			let prj_info = {};
			if (null == prj.name)
				continue;

			if (null == prj_infos_obj[prj.name])
			{
				prj_infos_obj[prj.name] = {};
				prj_infos_obj[prj.name].name = prj.name;
				prj_infos_obj[prj.name].count_art = 0;
			}

			prj_infos_obj[prj.name].count_art += 1;

			if (ab_filter_projects.length && (ab_filter_projects.indexOf(prj.name) == -1))
				continue;

			for (let sc of prj.scenes)
				for (let bm of sc.bms)
					artist.bm_count += 1;

			artist.projects.push(prj);
		}

		if (ab_filter_projects.length)
			if (artist.bm_count == 0)
				continue;

		ab_artists.push(artist);
		total_artists += 1;
		total_bookmarks += artist.bm_count;

		if (artist.disabled)
			disabled_artists += 1;
	}
	let prj_infos_arr = [];
	for (let pname in prj_infos_obj)
		prj_infos_arr.push(prj_infos_obj[pname]);

	prj_infos_arr.sort(function(a,b){if(a.count_art > b.count_art) return -1; return 1;});
	ab_wnd.elProjectsButtons = [];
	ab_wnd.elProjectsDiv.innerHTML = '';
	for (let prj of prj_infos_arr)
	{
		let el = document.createElement('div');
		ab_wnd.elProjectsDiv.appendChild(el);
		el.classList.add('button','project');
		el.innerHTML = prj.name + ' <small>(' + prj.count_art + 'A)</small>';
		el.m_prj_name = prj.name;
		el.onclick = ab_ProjectClicked;
		ab_wnd.elProjectsButtons.push(el);

		if (ab_filter_projects.length && (ab_filter_projects.indexOf(prj.name) != -1))
		{
			el.classList.add('pushed');
			total_projects += 1;
		}
	}
	if (total_projects == 0)
		total_projects = prj_infos_arr.length;

	ab_artists.sort(function(a, b) {
		let val_a = a[ab_wnd_sort_prop];
		let val_b = b[ab_wnd_sort_prop];

		if ((val_a > val_b) == ab_wnd_sort_dir)
			return -1;
		if ((val_a < val_b) == ab_wnd_sort_dir)
			return 1;

		if ((a.title > b.title) == ab_wnd_sort_dir)
			return -1;
		if ((a.title < b.title) == ab_wnd_sort_dir)
			return 1;

		return 0;
	});

	ab_WndDrawArtists();

	let info = '';
	info += '<b>' + total_artists + '</b> Artists';
	if (disabled_artists)
		info += ' (' + disabled_artists + ' disabled)';
	info += ', <b>' + total_projects + '</b> Projects';
	info += ', <b>' + total_bookmarks + '</b> Bookmarks';
	ab_wnd.elInfo.innerHTML = info;

	info = 'Totals: '
	if (obsolete_bookmarks)
		info += '<b>' + obsolete_bookmarks + '</b> Obsolete Bookmarks';
	if (invalid_bookmarks)
		info += ', <b>' + invalid_bookmarks + '</b> Invalid Bookmarks';
	ab_wnd.elBotPanel.innerHTML = info;
}

function ab_ArtistsListChanged()
{
	ab_filter_artists = ab_wnd.editAritsts.getSelectedNames();
	ab_ProcessArtists();
}

function ab_ProjectClicked(i_e)
{
	let el = i_e.currentTarget;
	el.classList.toggle('pushed');

	ab_filter_projects = [];
	for (el of ab_wnd.elProjectsButtons)
		if (el.classList.contains('pushed'))
			ab_filter_projects.push(el.m_prj_name);

	ab_ProcessArtists();
}

function ab_WndDrawArtists()
{
	ab_wnd.elPagesDiv.innerHTML = '';
	ab_wnd.elUsrRows = [];

	ab_art_pages = [];
	ab_art_projects = [];
	for (let i = 0; i < ab_artists.length; i++)
	{
		if (ab_artists[i].disabled && (ab_artists[i].bm_count == 0))
			continue;

		let ap = new ArtPage(ab_wnd.elPagesDiv, ab_artists[i]);
		ab_art_pages.push(ap);

		if (i % 2)
			ap.elRoot.style.backgroundColor = 'rgba(0,0,0,.1)';
		else
			ap.elRoot.style.backgroundColor = 'rgba(255,255,255,.1)';
	}
}

function ArtPage(i_el, i_artist)
{
	this.artist = i_artist;

	this.elParent = i_el;

	this.elRoot = document.createElement('div');
	this.elParent.appendChild(this.elRoot);
	this.elRoot.classList.add('artpage');
	if (this.artist.disabled)
		this.elRoot.classList.add('disabled');

	// Info:
	this.elBrief = document.createElement('div');
	this.elRoot.appendChild(this.elBrief);
	this.elBrief.classList.add('brief');

	this.elAvatar = document.createElement('div');
	this.elBrief.appendChild(this.elAvatar);
	this.elAvatar.classList.add('avatar');
	let avatar = c_GetAvatar(this.artist.id);
	if (avatar)
		this.elAvatar.style.backgroundImage = 'url(' + avatar + ')';

	this.elTitle = document.createElement('div');
	this.elBrief.appendChild(this.elTitle);
	this.elTitle.classList.add('title');
	this.elTitle.textContent = c_GetUserTitle(this.artist.id);

	let info = this.artist.tag + ' ' + this.artist.role;
	if (this.artist.bm_count)
		info += '<br>Bookmarks: ' + this.artist.bm_count;
	else
		this.elRoot.classList.add('empty');

	this.elInfo = document.createElement('div');
	this.elBrief.appendChild(this.elInfo);
	this.elInfo.classList.add('info');
	this.elInfo.innerHTML = info;

	// Bookmarks:
	this.elBmrks = document.createElement('div');
	this.elRoot.appendChild(this.elBmrks);
	this.elBmrks.classList.add('ap_bmrks_div');

	// Show bookmarks per project:
	for (let project of this.artist.projects)
	{
		let prj = new ArtPagePrj(this.elBmrks, project, this.artist);
		ab_art_projects.push(prj);
	}
}

function ArtPagePrj(i_el, i_project, i_artist)
{
	this.elParent = i_el;
	this.project = i_project;
	this.artist = i_artist;

	this.elRoot = document.createElement('div');
	this.elParent.appendChild(this.elRoot);
	this.elRoot.classList.add('artpage_prj')

	this.elTitle = document.createElement('div');
	this.elRoot.appendChild(this.elTitle);
	this.elTitle.textContent = this.project.name;
	this.elTitle.classList.add('title');

	this.elBmrks = document.createElement('div');
	this.elRoot.appendChild(this.elBmrks);
	this.elBmrks.classList.add('ap_bmrks_div');

	for (let scene of this.project.scenes)
	{
		scene.bms.sort(ab_CompareBookmarks);
		for (let bm of scene.bms)
		{
			let apbm = new ArtPageBM(this.elBmrks, bm, this.artist);
		}
	}
}

function ArtPageBM(i_el, i_bm, i_artist)
{
	this.elParent = i_el;
	this.bm = i_bm;
	this.artist = i_artist;

	this.name = this.bm.path.split('/');
	this.name = this.name[this.name.length-1];

	this.elRoot = document.createElement('div');
	this.elParent.appendChild(this.elRoot);
	this.elRoot.classList.add('artpage_bm');
	if (false == bm_ActualStatus(this.bm.status, this.artist))
		this.elRoot.classList.add('obsolete');

	this.elPath = document.createElement('a');
	this.elRoot.appendChild(this.elPath);
	this.elPath.classList.add('name');
	this.elPath.textContent = this.name;
	this.elPath.href = '#' + this.bm.path;
	this.elPath.target = '_blank';

	st_SetElStatus(this.elRoot, this.bm.status, /*show all tasks = */ false, this.artist);
}

