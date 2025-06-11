/**
 * '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                                      Copyright © 2012-17 by The CGRU team
 *    '          '
 * common.js - common js stuff for the afanasy web-page
 * This file contains common structures, misc formatting and user permission functions used throughout
 * all afanasy/browser/*.js files.
 * .......................................................................................................
 */

'use strict';

var cm_Attrs = [
	['name' /***********/, 'Name' /******/, 'Name' /***********/],
	['user_name' /******/, 'User' /******/, 'User Name' /******/],
	['host_name' /******/, 'Host' /******/, 'Host Name' /******/],
	['priority' /*******/, 'Priority' /**/, 'Priority' /*******/],
	['service' /********/, 'Service' /***/, 'Service Name' /***/],
	['time_creation' /**/, 'Created' /***/, 'Time Created' /***/],
	['time_started' /***/, 'Started' /***/, 'Time Started' /***/],
	['time_done' /******/, 'Finished' /**/, 'Time Finished' /**/],
	['order' /**********/, 'Order' /*****/, 'Order' /**********/],

	// This is task progress parameters short names:
	['str' /************/, 'Starts' /****/, 'Starts Count' /***/],
	['err' /************/, 'Errors' /****/, 'Errors Count' /***/],
	['hst' /************/, 'Host' /******/, 'Host Name' /******/]
];

var cm_UILevels = ['Padawan', 'Jedi', 'Sith'];

var cm_States = [
	'ONL', 'RDY', 'RUN', 'DON', 'WRN', 'ERR', 'SKP', 'OFF', 'WDP', 'WTM', 'DRT', 'NbY', 'SUS',
	'NBY', 'PAU', 'PER', 'PBR', 'PSC', 'WFL', 'WSL', 'WWK', 'PPA', 'RER', 'WRC', 'SIC'
];

var cm_SoftwareIcons = [];
var cm_TicketsIcons = [];

var cm_blockFlags = {
	numeric /*************/: 1 << 0,
	varCapacity /*********/: 1 << 1,
	multiHost /***********/: 1 << 2,
	masterOnSlave /*******/: 1 << 3,
	dependSubTask /*******/: 1 << 4,
	skipThumbnails /******/: 1 << 5,
	skipExistingFiles /***/: 1 << 6,
	checkRenderedFiles /**/: 1 << 7
};

var $ = function(id) {
	return document.getElementById(id);
};

function cm_Init()
{
	if (localStorage.ui_level == null)
		localStorage.ui_level = cm_UILevels[0];

	var uiLevelHandle = $('ui_level');
	uiLevelHandle.m_elBtns = [];
	for (var i = 0; i < cm_UILevels.length; i++)
	{
		var el = document.createElement('div');
		uiLevelHandle.appendChild(el);
		uiLevelHandle.m_elBtns.push(el);
		el.textContent = cm_UILevels[i];
		if (cm_UILevels[i] == localStorage.ui_level)
		{
			el.classList.add('current');
			el.title = 'Current UI level.';
		}
		else
		{
			el.classList.add('button');
			el.title = 'Double click to set ' + cm_UILevels[i] + ' UI level.';
			el.ondblclick = function(e) {
				cm_ApplyUILevel(e.currentTarget.textContent);
			};
		}
	}
}

function cm_ApplyUILevel(i_level)
{
	var uiLevelHandle = $('ui_level');
	for (var i = 0; i < uiLevelHandle.m_elBtns.length; i++)
	{
		var el = uiLevelHandle.m_elBtns[i];
		el.classList.remove('button');
		if (el.textContent != i_level)
			el.style.display = 'none';
	}

	if (cm_UILevels.indexOf(i_level) == -1)
	{
		g_Error('Invalid UI Level: ' + i_level);
		return;
	}

	g_Info('UI level set: ' + i_level);

	if (localStorage.ui_level == i_level)
		return;

	localStorage.ui_level = i_level;

	var text = 'Powerful you have become, the dark side I sense in you.';
	if (i_level == 'Padawan')
		text = 'Patience you must have, my young padawan.';
	else if (i_level == 'Jedi')
		text = 'May the Force be with you.';
	else
		g_Info('Welcome to the Dark Side.');

	var elB = document.createElement('div');
	document.body.appendChild(elB);
	elB.classList.add('ui_level_back');
	var elT = document.createElement('div');
	elB.appendChild(elT);
	elT.classList.add('ui_level_text');
	elT.textContent = text;

	elB.onclick = function(e) {
		document.location.reload();
	};
	elB.onkeydown = function(e) {
		document.location.reload();
	};
	elT.onclick = function(e) {
		document.location.reload();
	};
	elT.onkeydown = function(e) {
		document.location.reload();
	};
	setTimeout(function() {
		document.location.reload()
	}, 3000);
}

function cm_IsPadawan()
{
	return cm_UILevels.indexOf(localStorage.ui_level) == 0;
}

function cm_IsJedi()
{
	return cm_UILevels.indexOf(localStorage.ui_level) == 1;
}

function cm_IsSith()
{
	return cm_UILevels.indexOf(localStorage.ui_level) == 2;
}

function cm_ApplyStyles()
{
	document.body.style.background = localStorage.background;
	document.body.style.color = localStorage.text_color;
	// $('header').style.background = localStorage.background;
	// $('footer').style.background = localStorage.background;
	// $('navig').style.background = localStorage.background;
	// $('sidepanel').style.background = localStorage.background;
}

function cm_CompareItems(i_itemA, i_itemB, i_param, i_greater)
{
	if (i_greater == null)
		i_greater = false;

	if (i_itemA == null || i_itemB == null)
		return false;

	// Sorting is forced (branches&jobs, pools&renders)
	if (i_itemA.sort_force || i_itemB.sort_force)
	{
		if (i_itemA.sort_force != i_itemB.sort_force)
		{
			if (i_itemA.sort_force < i_itemB.sort_force)
				return true;
			else
				return false;
		}
		else if (i_itemA.node_type != i_itemB.node_type)
		{
			// If path is the same we should show parent node first:
			if (i_itemA.node_type == 'branches')
				return true;
			if (i_itemA.node_type == 'pools')
				return true;
			return false;
		}
	}

	if (i_itemA.params[i_param] > i_itemB.params[i_param])
		return i_greater == true;
	if (i_itemA.params[i_param] < i_itemB.params[i_param])
		return i_greater == false;

	if (i_param != 'name')
		if (i_itemA.params['name'] < i_itemB.params['name'])
			return true;

	return false;
}

function cm_ArrayRemove(io_arr, i_value)
{
	var index = io_arr.indexOf(i_value);
	if (index >= 0)
		io_arr.splice(index, 1);
}

function cm_IdsMerge(i_ids1, i_ids2)
{
	var o_ids = i_ids1;
	if (o_ids == null)
		o_ids = [];
	if ((i_ids2 != null) && (i_ids2.length > 0))
	{
		if (o_ids.length > 0)
		{
			for (var i = 0; i < i_ids2.length; i++)
			{
				var found = false;
				for (var o = 0; o < o_ids.length; o++)
				{
					if (o_ids[o] == i_ids2[i])
					{
						found = true;
						break;
					}
				}
				if (found == false)
					o_ids.push(i_ids2[i])
			}
		}
		else
			o_ids = i_ids2;
	}
	return o_ids;
}

function cm_TimeStringInterval(time1, time2)
{
	if (time2 == null)
		time2 = new Date();
	else
		time2 = new Date(time2 * 1000);

	var time = time2 - new Date(time1 * 1000);
	var seconds = Math.floor(time / 1000);
	var minutes = Math.floor(seconds / 60);
	var hours = Math.floor(minutes / 60);
	var days = Math.floor(hours / 24);
	if (days == 1)
		days = 0;
	seconds = seconds - minutes * 60;
	if (seconds < 10)
		seconds = '0' + seconds;
	minutes = minutes - hours * 60;
	if (minutes < 10)
		minutes = '0' + minutes;
	hours = hours - days * 24;
	time = hours + ':' + minutes + '.' + seconds;
	if (days > 1)
		time = days + 'd ' + time;
	return time;
}

function cm_TimeStringFromSeconds(i_seconds, i_full)
{
	var str = '';

	if (i_seconds == 0)
		return '0';

	var seconds = Math.round(i_seconds);
	var days = 0;
	var hours = 0;
	var minutes = Math.floor(seconds / 60);
	if (minutes > 0)
	{
		seconds -= minutes * 60;
		hours = Math.floor(minutes / 60);
		if (hours > 0)
		{
			minutes -= hours * 60;
			days = Math.floor(hours / 24);
			if (days > 0)
				hours -= days * 24;
		}
	}

	if (days)
	{
		if (i_full)
			str += days + ' Days ';
		else
			str += days + 'd ';
	}

	if (hours)
	{
		str += hours;
		if (minutes || seconds)
		{
			str += ':';
			if (minutes < 10)
				str += '0';
			str += minutes;
			if (seconds)
			{
				str += '.';
				if (seconds < 10)
					str += '0';
				str += seconds;
			}
		}
		else
		{
			if (i_full)
				str += ' Hours';
			else
				str += 'h';
		}
	}
	else if (minutes)
	{
		str += minutes;
		if (seconds)
		{
			str += '.';
			if (seconds < 10)
				str += '0';
			str += seconds;
		}
		str += 'm';
	}
	else if (seconds)
	{
		str += seconds + 's';
	}

	return str;
}

function cm_DateTimeStrFromSec(i_sec, i_nosec)
{
	return cm_DateTimeStrFromMSec(i_sec * 1000, i_nosec);
}

function cm_DateTimeStrFromMSec(i_sec, i_nosec)
{
	var date = new Date(i_sec);
	date = date.toString();
	date = date.substr(0, date.indexOf(' GMT'));
	if (i_nosec)
		date = date.substr(0, date.lastIndexOf(':'));
	return date;
}

function cm_ToKMG(i_value, i_args)
{
	var lables = ['', 'K', 'M', 'G', 'T', 'P'];

	if (i_args == null)
		i_args = {};

	var i_base = i_args.base;
	var i_floats = i_args.floats;
	var i_stripzeros = i_args.stripzeros;

	if (i_base == null)
		i_base = 1000.0;
	if (i_floats == null)
		i_floats = 1;
	if (i_stripzeros == null)
		i_stripzeros = true;

	var th = 1, pow = 0;
	while (th * i_base < i_value)
	{
		th *= i_base;
		pow++;

		if (pow == lables.length - 1)
			break;
	}

	var str = (i_value / th).toFixed(i_floats);
	if (i_stripzeros)
		str = str.replace(/\.0+/, '');
	str += lables[pow];

	return str;
}

function cm_ValueToString(i_value, i_type)
{
	if (i_value == null)
		return '';

	let o_str = '' + i_value;
	if (i_type == 'hrs')
	{
		o_str = cm_TimeStringFromSeconds(i_value, true);
	}
	else if (i_type == 'tim')
	{
		o_str = cm_DateTimeStrFromSec(i_value);
	}
	else if (i_type == 'mib')
	{
		o_str = (i_value / 1024.0).toFixed(1);
	}
	else if (i_type == 'meg')
	{
		o_str = (i_value / 1000.0).toFixed(1);
	}
	else if ((typeof i_value) == 'string')
	{
		// word-wrap long regular expressions:
		o_str = i_value.replace(/\./g, '.&shy;');
		o_str = o_str.replace(/\|/g, '|&shy;');
		o_str = o_str.replace(/\)/g, ')&shy;');
	}
	else if (i_type == 'msi')
	{
		o_str = '';
		for (let s in i_value)
			o_str += ' <b>' + s + '</b>:' + i_value[s];
	}

	return o_str;
}

function cm_GetState(i_state, o_state, o_element)
{
	for (var i = 0; i < cm_States.length; i++)
	{
		if (o_element)
			o_element.classList.remove(cm_States[i]);
		if (i_state)
		{
			if (i_state.indexOf(cm_States[i]) != -1)
			{
				o_state[cm_States[i]] = true;
				if (o_element)
					o_element.classList.add(cm_States[i]);
				continue;
			}
		}
		o_state[cm_States[i]] = false;
	}
}

function cm_ElCreateFloatText(i_elParent, i_side, i_title, i_text)
{
	var element = document.createElement('span');
	element.classList.add('text');
	i_elParent.appendChild(element);
	if (i_side)
		element.style.cssFloat = i_side;
	if (i_title)
		element.title = i_title;
	if (i_side == 'right')
		element.style.marginLeft = '4px';
	else
		element.style.marginRight = '4px';

	if (i_text)
		element.innerHTML = i_text;

	return element;
}

function cm_ElCreateText(i_elParent, i_title)
{
	return cm_ElCreateFloatText(i_elParent, null, i_title);
}

function cm_CreateStart(i_item)
{
	i_item.elStar = document.createElement('div');
	i_item.element.appendChild(i_item.elStar);
	i_item.elStar.classList.add('star');
	i_item.elStar.innerHTML = localStorage.run_symbol;
	i_item.elStarCount = document.createElement('div');
	i_item.elStar.appendChild(i_item.elStarCount);
	i_item.elStarCount.classList.add('count');
}

function cm_FillNumbers(i_string, i_number)
{
	var pattern = /@#+@/g;
	var match = i_string.match(pattern);
	for (var i in match)
	{
		var replace_mask = match[i];
		if (replace_mask.length > i_number.toString().length)
		{
			var replace_string =
				Array(replace_mask.length - i_number.toString().length - 1).join('0') + i_number.toString();
			i_string = i_string.replace(replace_mask, replace_string);
		}
	}
	return i_string;
}

function cm_PathBase(i_file)
{
	return i_file.substr(i_file.lastIndexOf('/') + 1).substr(i_file.lastIndexOf('\\') + 1);
}

function cm_PathDir(i_file)
{
	return i_file.substr(0, i_file.lastIndexOf('/')).substr(0, i_file.lastIndexOf('\\'));
}

function cm_CheckBlockFlag(i_flags, i_name)
{
	if (cm_blockFlags[i_name])
		return cm_blockFlags[i_name] & i_flags;

	g_Error('Block flag "' + i_name + '" does not exist.');
	g_Log('Valid flags are: ' + JSON.stringify(cm_blockFlags));

	return false;
}

function cm_CheckPermissions(i_perm)
{
	if (i_perm == null)
		return true;

	if (g_VISOR())
	{
		if (i_perm == 'user')
			return false;
		if (g_GOD())
		{
			if (false == ((i_perm == 'visor') || (i_perm == 'god')))
				return false;
		}
		else if (i_perm != 'visor')
			return false;
	}
	else if (i_perm != 'user')
		return false;

	return true;
}

function cm_SoftwareIconsReceived(i_obj)
{
	if (i_obj.files && i_obj.files.length)
		cm_SoftwareIcons = i_obj.files;
	else
		g_Error('Invalid softwate icons: ' + JSON.stringify(i_obj));
}

function cm_TicketsIconsReceived(i_obj)
{
	if (i_obj.files && i_obj.files.length)
		cm_TicketsIcons = i_obj.files;
	else
		g_Error('Invalid softwate icons: ' + JSON.stringify(i_obj));
}
