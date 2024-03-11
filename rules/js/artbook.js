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

var ab_flags_order = ['precomp','check','comment','ready','inprogress','on_farm','sent'];

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

function ab_CollectFlags(i_bm, i_uid)
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

		if (task.artists == null)
			continue;
		if (task.artists.indexOf(i_uid) == -1)
			continue;

		if (task.flags)
			for (let flag of task.flags)
				if (flags.indexOf(flag) == -1)
					flags.push(flag);
	}

	return flags;
}

function ab_CompareFlags(a,b)
{
	let max_index_a = -1;
	let max_index_b = -1;

	for (let flag of a)
		if (ab_flags_order.indexOf(flag) > max_index_a)
			max_index_a = ab_flags_order.indexOf(flag);

	for (let flag of b)
		if (ab_flags_order.indexOf(flag) > max_index_b)
			max_index_b = ab_flags_order.indexOf(flag);

	if (max_index_a < max_index_b)
		return 1;
	if (max_index_a > max_index_b)
		return -1;

	return 0;
}

function ab_CompareBookmarks(i_uid) {
return function (a,b)
{
	return ab_CompareFlags(ab_CollectFlags(a, i_uid), ab_CollectFlags(b, i_uid));
}}

function ab_ProcessArtists()
{
	ab_artists = [];

	let total_obsolete_bookmarks = 0;
	let total_invalid_bookmarks = 0;

	let shown_artists = 0;
	let shown_disabled_artists = 0;
	let shown_projects = 0;
	let shown_bookmarks = 0;

	let prj_infos_obj = [];
	for (let uid in ab_users)
	{
		let user = ab_users[uid];

		// Skip not an artists
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
				total_invalid_bookmarks += 1;
				continue;
			}

			if (false == bm_ActualStatus(bm.status, user))
			{
				total_obsolete_bookmarks += 1;
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

		if (artist.disabled)
		{
			if (artist.bm_count == 0)
				continue;
			shown_disabled_artists += 1;
		}

		ab_artists.push(artist);
		shown_artists += 1;
		shown_bookmarks += artist.bm_count;
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
			shown_projects += 1;
		}
	}
	if (shown_projects == 0)
		shown_projects = prj_infos_arr.length;

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
	info += '<b>' + shown_artists + '</b> Artists';
	if (shown_disabled_artists)
		info += ' (' + shown_disabled_artists + ' disabled)';
	info += ', <b>' + shown_projects + '</b> Projects';
	info += ', <b>' + shown_bookmarks + '</b> Bookmarks';
	ab_wnd.elInfo.innerHTML = info;

	info = ''
	if (total_obsolete_bookmarks)
		info += '<b>' + total_obsolete_bookmarks + '</b> Obsolete Bookmarks';
	if (total_invalid_bookmarks)
		info += ', <b>' + total_invalid_bookmarks + '</b> Invalid Bookmarks';
	if (info.length)
		ab_wnd.elBotPanel.innerHTML = 'Totals: ' + info;
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

	// Body:
	this.elBody = document.createElement('div');
	this.elRoot.appendChild(this.elBody);

	// Show bookmarks per project:
	for (let project of this.artist.projects)
	{
		let prj = new ArtPagePrj(this.elBody, project, this.artist);
		ab_art_projects.push(prj);
	}

	if (this.artist.projects.length == 0)
		return;

	// Activity:
	this.elActRoot = document.createElement('div');
	this.elBody.appendChild(this.elActRoot);
	this.elActRoot.classList.add('artpage_actroot');

	let elActPanel = document.createElement('div');
	this.elActRoot.appendChild(elActPanel);
	elActPanel.classList.add('panel');

	this.elBtnActLoad = document.createElement('div');
	elActPanel.appendChild(this.elBtnActLoad);
	this.elBtnActLoad.classList.add('button');
	this.elBtnActLoad.textContent = 'Load Activity';
	this.elBtnActLoad.m_this = this;
	this.elBtnActLoad.onclick = function(e){e.currentTarget.m_this.activityLoad();}

	this.elBtnActClose = document.createElement('div');
	elActPanel.appendChild(this.elBtnActClose);
	this.elBtnActClose.classList.add('button','close');
	this.elBtnActClose.textContent = 'Close Activity';
	this.elBtnActClose.m_this = this;
	this.elBtnActClose.onclick = function(e){e.currentTarget.m_this.activityClose();}
	this.elBtnActClose.style.display = 'none';

	this.elActivityInfo = document.createElement('div');
	elActPanel.appendChild(this.elActivityInfo);
	this.elActivityInfo.classList.add('info');

	this.elActivity = document.createElement('div');
	this.elActRoot.appendChild(this.elActivity);
	this.elActivity.classList.add('artpage_activity');
}

ArtPage.prototype.activityClose = function()
{
	this.elBtnActLoad.style.display = 'block';
	this.elBtnActClose.style.display = 'none';
	this.elActivityInfo.textContent = '';
	this.elActivity.textContent = '';
}

function ad_GetUserActivityFileName(i_id){return ad_GetUserFileName(i_id, 'activity');}
ArtPage.prototype.activityLoad = function()
{
	n_GetFile({
		'path': ad_GetUserActivityFileName(this.artist.id),
		'func': ad_UserActivityReceived,
		'object': this,
		'info': 'activity',
		'cache_time': -1,
		'parse': true,
		'local': false
	});
}
function ad_UserActivityReceived(i_data, i_args)
{
	i_args.object.activityReceived(i_data);
}
ArtPage.prototype.activityReceived = function(i_data)
{
	if ((i_data == null) || (i_data.error))
	{
		this.elActRoot.textContent = 'No activity found.';
		c_Log(JSON.stringify(i_data));
		return;
	}

	this.elBtnActLoad.style.display = 'none';
	this.elBtnActClose.style.display = 'block';
	this.elActivity.textContent = '';

	let stat = {};
	stat.count = 0;
	stat.time_min = 0;
	stat.time_max = 0;
	stat.flags = {};

	for (let path in i_data)
	{
		let act = i_data[path];

		let elAct = document.createElement('div');
		this.elActivity.appendChild(elAct);
		elAct.classList.add('artpage_act');

		let elPath = document.createElement('a');
		elAct.appendChild(elPath);
		elPath.classList.add('path');
		elPath.textContent = path;
		elPath.href = '#' + path;
		elPath.target = '_blank';

		if (act.task.cuser)
		{
			let elCUser = st_CreateElArtist(act.task.cuser, true);
			elAct.appendChild(elCUser);
			elCUser.classList.add('user','cuser');
			let tip = 'Created by ' + c_GetUserTitle(act.task.cuser);
			if (act.task.ctime)
				tip += '\n' + c_DT_StrFromSec(act.task.ctime);
			elCUser.title = tip;
		}

		let elTask = document.createElement('div');
		elAct.appendChild(elTask);
		elTask.classList.add('task');

		task_DrawBadges({'tasks':[act.task]}, elTask, {'full_names':true});

		if (act.task.muser)
		{
			let elMUser = st_CreateElArtist(act.task.muser, true);
			elAct.appendChild(elMUser);
			elMUser.classList.add('user','muser');
			let tip = 'Modified by ' + c_GetUserTitle(act.task.muser);
			if (act.task.mtime)
				tip += '\n' + c_DT_StrFromSec(act.task.mtime);
			elMUser.title = tip;
		}

		let elTime = document.createElement('div');
		elAct.appendChild(elTime);
		elTime.classList.add('time');
		let time = '';
		if (act.ctime)
			time = c_DT_StrFromSec(act.ctime);
		time += ' - ';
		if (act.mtime)
			time += c_DT_StrFromSec(act.mtime);
		elTime.textContent = time;

		// Collect stat:
		stat.count += 1;
		if (act.mtime)
		{
			if (stat.count == 1)
			{
				stat.time_min = act.mtime;
				stat.time_max = act.mtime;
			}
			else
			{
				if (stat.time_min > act.mtime)
					stat.time_min = act.mtime;
				if (stat.time_max < act.mtime)
					stat.time_max = act.mtime;
			}
		}
		if (act.task.flags)
		{
			for (let flag of act.task.flags)
			{
				if (flag in stat.flags)
					stat.flags[flag] += 1;
				else
					stat.flags[flag] = 1;
			}
		}
	}


	let elCount = document.createElement('div');
	this.elActivityInfo.appendChild(elCount);
	elCount.textContent = 'Count = ' + stat.count;

	let elTime = document.createElement('div');
	this.elActivityInfo.appendChild(elTime);
	elTime.textContent = c_DT_StrFromSec(stat.time_min) + ' - ' + c_DT_StrFromSec(stat.time_max);

	for (let flag in stat.flags)
	{
		let elFlag = document.createElement('div');
		this.elActivityInfo.appendChild(elFlag);
		elFlag.classList.add('tag','flag');
		elFlag.textContent = c_GetFlagTitle(flag) + ' x ' + stat.flags[flag];
		elFlag.title = c_GetFlagTip(flag);
		let clr = null;
		if (RULES.flags[flag] && RULES.flags[flag].clr)
			clr = RULES.flags[flag].clr;
		if (clr)
			st_SetElColor({"color": clr}, elFlag);
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

	this.elPanel = document.createElement('div');
	this.elRoot.appendChild(this.elPanel);
	this.elPanel.classList.add('panel');

	this.elBtnExpand = document.createElement('div');
	this.elPanel.appendChild(this.elBtnExpand);
	this.elBtnExpand.classList.add('button');
	this.elBtnExpand.textContent = 'Expand';
	this.elBtnExpand.m_this = this;
	this.elBtnExpand.onclick = function(e){e.currentTarget.m_this.expand();}

	this.elBtnCollapse = document.createElement('div');
	this.elPanel.appendChild(this.elBtnCollapse);
	this.elBtnCollapse.classList.add('button');
	this.elBtnCollapse.textContent = 'Collapse';
	this.elBtnCollapse.m_this = this;
	this.elBtnCollapse.onclick = function(e){e.currentTarget.m_this.collapse();}
	this.elBtnCollapse.style.display = 'none';

	this.elTitle = document.createElement('div');
	this.elPanel.appendChild(this.elTitle);
	this.elTitle.textContent = this.project.name;
	this.elTitle.classList.add('title');

	this.elStat = document.createElement('div');
	this.elRoot.appendChild(this.elStat);
	this.elStat.classList.add('prj_stat');

	this.showStat();
}

ArtPagePrj.prototype.showStat = function()
{
	let prj_bms = [];
	for (let scene of this.project.scenes)
		prj_bms = prj_bms.concat(scene.bms);

	prj_bms.sort(ab_CompareBookmarks(this.artist.id));

	let acts = {};

	for (let bm of prj_bms)
	{
		if (bm.status == null) continue;
		if (bm.status.tasks == null) continue;
		for (let name in bm.status.tasks)
		{
			let t = bm.status.tasks[name];
			if (t.deleted) continue;
			if (t.artists == null) continue;
			if (t.artists.indexOf(this.artist.id) == -1) continue;

			let act = {};
			if (name in acts)
				act = acts[name]
			else
			{
				act.count = 0;
				act.flags = {};
			}

			act.count += 1;

			if (t.flags )
				for (let f of t.flags)
					if (f in act.flags)
						act.flags[f] += 1;
					else
						act.flags[f] = 1;

			acts[name] = act;
		}
	}

	for (let act in acts)
	{
		let elAct = document.createElement('div');
		this.elStat.appendChild(elAct);
		elAct.classList.add('prj_act');

		let elName = document.createElement('div');
		elAct.appendChild(elName);
		elName.classList.add('name');
		elName.innerHTML = '<b>' + act + '</b> x ' + acts[act].count;

		if ('flags' in acts[act])
			for (let flag in acts[act].flags)
			{
				let elFlag = document.createElement('div');
				elAct.appendChild(elFlag);
				elFlag.classList.add('tag','flag');
				elFlag.textContent = c_GetFlagTitle(flag) + ' x ' + acts[act].flags[flag];
				elFlag.title = c_GetFlagTip(flag);
				let clr = null;
				if (RULES.flags[flag] && RULES.flags[flag].clr)
					clr = RULES.flags[flag].clr;
				if (clr)
					st_SetElColor({"color": clr}, elFlag);
			}
	}
}

ArtPagePrj.prototype.expand = function()
{
	this.elBtnExpand.style.display = 'none';
	this.elBtnCollapse.style.display = 'block';

	if (this.elBmrks)
	{
		this.elBmrks.style.display = 'block';
	}
	else
	{
		this.elBmrks = document.createElement('div');
		this.elRoot.appendChild(this.elBmrks);
		this.showFull();
	}
}

ArtPagePrj.prototype.collapse = function()
{
	this.elBtnCollapse.style.display = 'none';
	this.elBtnExpand.style.display = 'block';
	this.elBmrks.style.display = 'none';
}

ArtPagePrj.prototype.showFull = function()
{
	// Collect bookmarks of all project scenes:
	let prj_bms = [];
	for (let scene of this.project.scenes)
		prj_bms = prj_bms.concat(scene.bms);

	prj_bms.sort(ab_CompareBookmarks(this.artist.id));

	for (let bm of prj_bms)
	{
		let apbm = new ArtPageBM(this.elBmrks, bm, this.artist);
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

