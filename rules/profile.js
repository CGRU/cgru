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
	profile.js - profile settgins
*/

"use strict";

var prof_props = {
	id /**********/: {"disabled": true, "lwidth": '170px', "label": 'ID'},
	role /********/: {"disabled": true, "lwidth": '170px'},
	title /*******/: {"lwidth": '170px'},
	avatar /******/: {},
	news_limit /**/: {},
	email /*******/: {"width": '70%'},
	email_news /**/: {"width": '30%', 'type': "bool", 'default': false},
	signature /***/: {}
};

function prof_Open()
{
	if (g_auth_user == null)
	{
		c_Error('No authenticated user found.');
		return;
	}

	var wnd = new cgru_Window({"name": 'profile', "title": 'My Profile'});
	wnd.elContent.classList.add('profile');

	var avatar = c_GetAvatar();
	if (avatar)
	{
		var el = document.createElement('img');
		wnd.elContent.appendChild(el);
		el.classList.add('avatar');
		el.src = avatar;
	}

	gui_Create(wnd.elContent, prof_props, [g_auth_user]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.style.clear = 'both';

	wnd.elContent.m_wnd = wnd;
	wnd.elContent.onkeydown = function(e) {
		if (e.keyCode == 13) // Enter
			prof_Save(e.currentTarget.m_wnd);
	}

	var el = document.createElement('div');
	elBtns.appendChild(el);
	el.textContent = 'Save';
	el.classList.add('button');
	el.onclick = function(e) { prof_Save(e.currentTarget.m_wnd); };
	el.m_wnd = wnd;

	var el = document.createElement('div');
	elBtns.appendChild(el);
	el.textContent = 'Cancel';
	el.classList.add('button');
	el.onclick = function(e) { e.currentTarget.m_wnd.destroy(); };
	el.m_wnd = wnd;

	if (c_CanSetPassword())
	{
		var el = document.createElement('div');
		elBtns.appendChild(el);
		el.textContent = 'Set Password';
		el.classList.add('button');
		el.onclick = function(e) { ad_SetPasswordDialog(g_auth_user.id); };
		el.m_wnd = wnd;
	}
}

function prof_Save(i_wnd)
{
	let params = gui_GetParams(i_wnd.elContent, prof_props);

	if (params.news_limit.length == 0)
		params.news_limit = '-1';

	params.news_limit = parseInt(params.news_limit);
	if (isNaN(params.news_limit))
	{
		c_Error('Invalid news limit number.');
		return;
	}

	for (var p in params)
	{
		g_auth_user[p] = params[p];
	}
	g_users[g_auth_user.id] = g_auth_user;

	ad_SaveUser();
	ad_UpdateProfileSettings();
	i_wnd.destroy();
}
