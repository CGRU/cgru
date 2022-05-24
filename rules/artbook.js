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

var ab_wnd_sort_prop = 'bookmarks';
var ab_wnd_sort_dir = 1;

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

	let el = document.createElement('div');
	ab_wnd.elTopPanel.appendChild(el);
	el.classList.add('button');
	el.textContent = 'Refresh';
	el.ondblclick = ab_WndRefresh;
	el.style.cssFloat = 'right';

	ab_wnd.elProjectsDiv = document.createElement('div');
	ab_wnd.elTopPanel.appendChild(ab_wnd.elProjectsDiv);
	ab_wnd.elProjectsDiv.classList.add('ab_projects_div');

	ab_wnd.elArtistsDiv = document.createElement('div');
	ab_wnd.elTopPanel.appendChild(ab_wnd.elArtistsDiv);
	ab_wnd.elArtistsDiv.classList.add('ab_artists_div');

	ab_wnd.elInfo = document.createElement('div');
	ab_wnd.elContent.appendChild(ab_wnd.elInfo);

	ab_wnd.elPagesDiv = document.createElement('div');
	ab_wnd.elContent.appendChild(ab_wnd.elPagesDiv);
	ab_wnd.elPagesDiv.classList.add('artbook_pages');

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

function ab_ProcessArtists()
{
	ab_artists = [];
	let prj_names = [];
	for (let uid in ab_users)
	{
		let user = ab_users[uid];

		// Skip disabled users
		if (user.disabled)
			continue;

		// Skip not an atrists
		if (c_IsNotAnArtist(user))
			continue;
		
		// Skip no bookmarks
		if ((null == user.bookmarks) || (user.bookmarks.length == 0))
			continue;

		// Show only selected, or all if no selection
		if (ab_filter_artists.length && (ab_filter_artists.indexOf(user.id) == -1))
			continue;

		// Check bookmarks validness:
		for (let bm of user.bookmarks)
		{
			if (null == bm.path)
			{
				bm.path = 'undefined';
				bm.invalid = true;
				console.log(JSON.stringify(bm));
			}
		}

		let artist =  user;
		artist.projects = [];
		let projects = bm_CollectProjects(artist.bookmarks);
		for (let prj of projects)
		{
			if (prj_names.indexOf(prj.name) == -1)
				prj_names.push(prj.name);

			if (ab_filter_projects.length && (ab_filter_projects.indexOf(prj.name) == -1))
				continue;

			artist.projects.push(prj);
		}

		ab_artists.push(artist);
	}

	prj_names.sort();
	ab_wnd.elProjectsButtons = [];
	ab_wnd.elProjectsDiv.innerHTML = '';
	for (let pname of prj_names)
	{
		let el = document.createElement('div');
		ab_wnd.elProjectsDiv.appendChild(el);
		el.classList.add('button');
		el.textContent = pname;
		el.onclick = ab_ProjectClicked;
		ab_wnd.elProjectsButtons.push(el);

		if (ab_filter_projects.length && (ab_filter_projects.indexOf(pname) != -1))
			el.classList.add('pushed');
	}

	ab_artists.sort(function(a, b) {
		let val_a = a[ab_wnd_sort_prop];
		let val_b = b[ab_wnd_sort_prop];

		if (val_a == null)
			val_a = '';
		if (val_b == null)
			val_b = '';

		if ((val_a > val_b) == ab_wnd_sort_dir)
			return -1;
		if ((val_a < val_b) == ab_wnd_sort_dir)
			return 1;

		if (ab_wnd_sort_prop != 'role')
		{
			if ((a.role > b.role) == ab_wnd_sort_dir)
				return -1;
			if ((a.role < b.role) == ab_wnd_sort_dir)
				return 1;
		}

		if ((a.disabled && (!b.disabled)) == ab_wnd_sort_dir)
			return -1;
		if ((b.disabled && (!a.disabled)) == ab_wnd_sort_dir)
			return 1;

		if ((a.title > b.title) == ab_wnd_sort_dir)
			return -1;
		if ((a.title < b.title) == ab_wnd_sort_dir)
			return 1;

		return 0;
	});


	ab_WndDrawArtists();
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
			ab_filter_projects.push(el.textContent);

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

	// Info:
	this.elInfo = document.createElement('div');
	this.elRoot.appendChild(this.elInfo);
	this.elInfo.classList.add('info');

	this.elAvatar = document.createElement('div');
	this.elInfo.appendChild(this.elAvatar);
	this.elAvatar.classList.add('avatar');
	let avatar = c_GetAvatar(this.artist.id);
	if (avatar)
		this.elAvatar.style.backgroundImage = 'url(' + avatar + ')';

	this.elTitle = document.createElement('div');
	this.elInfo.appendChild(this.elTitle);
	this.elTitle.textContent = c_GetUserTitle(this.artist.id);

	this.elTag = document.createElement('div');
	this.elInfo.appendChild(this.elTag);
	this.elTag.textContent = i_artist.tag;

	this.elRole = document.createElement('div');
	this.elInfo.appendChild(this.elRole);
	this.elRole.textContent = i_artist.role;

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

	this.elTitle = document.createElement('div');
	this.elRoot.appendChild(this.elTitle);
	this.elTitle.textContent = this.project.name;

	this.elBmrks = document.createElement('div');
	this.elRoot.appendChild(this.elBmrks);
	this.elBmrks.classList.add('ap_bmrks_div');

	for (let scene of this.project.scenes)
	{
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

	this.elPath = document.createElement('a');
	this.elRoot.appendChild(this.elPath);
	this.elPath.classList.add('name');
	this.elPath.textContent = this.name;
	this.elPath.href = '#' + this.bm.path;
	this.elPath.target = '_blank';

	st_SetElStatus(this.elRoot, this.bm.status, /*show all tasks = */ false, this.artist);
}

