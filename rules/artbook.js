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
	ab_wnd = new cgru_Window({"name": 'artbook', "title": 'Artists Bookmarks', 'padding': '3% 1%'});
	ab_wnd.elContent.classList.add('artbook');

	let elBtnsDiv = document.createElement('div');
	ab_wnd.elContent.appendChild(elBtnsDiv);

	let el = document.createElement('div');
	elBtnsDiv.appendChild(el);
	el.classList.add('button');
	el.textContent = 'Refresh';
	el.ondblclick = ab_WndRefresh;
	el.style.cssFloat = 'right';


	ab_wnd.elInfo = document.createElement('div');
	ab_wnd.elContent.appendChild(ab_wnd.elInfo);

	ab_wnd.elArtists = document.createElement('div');
	ab_wnd.elContent.appendChild(ab_wnd.elArtists);
	ab_wnd.elArtists.classList.add('artbook_users');

	ab_WndRefresh();
}

function ab_WndRefresh()
{
	n_Request({
		"send": {'getallusers':true},
		"func": ab_WndArtistsReceived
	});
}

function ab_WndArtistsReceived(i_data)
{
	if (i_data == null)
	{
		ab_wnd.elContent.innerHTML = 'Error getting artists.';
		return;
	}
	if (i_data.error)
	{
		ab_wnd.elContent.innerHTML = 'Error getting artists:<br>' + i_data.error;
		return;
	}
	if (i_data.users == null)
	{
		ab_wnd.elContent.innerHTML = '"artists" are NULL.';
		return;
	}

	g_users = i_data.users;

	if (ab_wnd == null)
		return;

	ab_WndDrawArtists();
}

function ab_WndSortArtists(i_prop)
{
	// console.log('ab_WndSortArtists: ' + i_prop);
	if (ab_wnd_sort_prop == i_prop)
		ab_wnd_sort_dir = 1 - ab_wnd_sort_dir;
	else
		ab_wnd_sort_prop = i_prop;
	ab_WndDrawArtists();
}

function ab_WndDrawArtists()
{
	ab_wnd.elArtists.innerHTML = '';
	ab_wnd.elUsrRows = [];

	let users = [];
	for (let uid in g_users)
	{
		let user = g_users[uid];
		if (user.disabled)
			continue;
		if (c_IsNotAnArtist(user))
			continue;

		users.push(user);
	}

	users.sort(function(a, b) {
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

	for (let i = 0; i < users.length; i++)
	{
		let ap = new ArtPage(ab_wnd.elArtists, users[i]);

		if (i % 2)
			ap.elRoot.style.backgroundColor = 'rgba(0,0,0,.1)';
		else
			ap.elRoot.style.backgroundColor = 'rgba(255,255,255,.1)';
	}
}

function ArtPage(i_el, i_user)
{
	this.user = i_user;

	this.elParent = i_el;

	this.elRoot = document.createElement('div');
	this.elRoot.classList.add('artpage');
	this.elParent.appendChild(this.elRoot);

	// Info:
	this.elInfo = document.createElement('div');
	this.elRoot.appendChild(this.elInfo);
	this.elInfo.classList.add('info');

	this.elAvatar = document.createElement('div');
	this.elInfo.appendChild(this.elAvatar);
	this.elAvatar.classList.add('avatar');
	let avatar = c_GetAvatar(this.user.id);
	if (avatar)
		this.elAvatar.style.backgroundImage = 'url(' + avatar + ')';

	this.elTitle = document.createElement('div');
	this.elInfo.appendChild(this.elTitle);
	this.elTitle.textContent = c_GetUserTitle(this.user.id);

	this.elTag = document.createElement('div');
	this.elInfo.appendChild(this.elTag);
	this.elTag.textContent = i_user.tag;

	this.elRole = document.createElement('div');
	this.elInfo.appendChild(this.elRole);
	this.elRole.textContent = i_user.role;

	// Bookmarks:
	this.elBmrks = document.createElement('div');
	this.elRoot.appendChild(this.elBmrks);
	this.elBmrks.classList.add('ap_bmrks_div');

	if (null == this.user.bookmarks)
		return;

	for (let bm of this.user.bookmarks)
	{
		let el = document.createElement('div');
		this.elBmrks.appendChild(el);
		el.textContent = bm.path;
	}
}
