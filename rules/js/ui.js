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
	ui.js - TODO: description
*/

"use strict";

var u_elements = ['asset', 'assets', 'info', 'log', 'navig', 'cycle', 'thumbnail'];
var u_el = {};
var u_views = ['asset', 'files', 'body', 'comments'];

var u_guest_attrs = [
	{"name": 'id', "label": 'Login Name', "required": true}, {"name": 'title', "label": 'Full Name'},
	{"name": 'email', "label": 'Email', "info": 'gravarar, hidden'},
	{"name": 'avatar', "label": 'Avatar', "info": 'link'}, {"name": 'signature', "label": 'Signature'}
];

var u_body_filename = 'body.html';
var u_body_text = '';
var u_body_editing = false;
var u_body_edit_markup = 0;

var u_background = '#A0A0A0';
var u_textColor = '#000000';
cgru_params.push(['back_asset', 'Asset', '', 'Enter background style']);
cgru_params.push(['back_body', 'Body', '', 'Enter background style']);
cgru_params.push(['back_files', 'Files', '', 'Enter background style']);
cgru_params.push(['back_comments', 'Comments', '', 'Enter background style']);

var u_gui_size_handle = '6px';  // with a border

var u_thumbstime = {};
var u_resizing_name = null;
var u_resizing_koeff = null;
var u_resizing_x = null;

var u_navig_filter_flags_exclude = [];
var u_navig_filter_flags_include = [];

function View_body_Open()
{
	u_BodyLoad();
}

function View_body_Close()
{
	u_BodyEditCancel('');
}

function View_files_Open()
{
	if (g_elCurFolder)
		new FilesView(
			{"el": $('files'), "path": g_elCurFolder.m_path, "walk": g_elCurFolder.m_dir, "limits": false, 'name':'files'});
}

function View_files_Close()
{
	$('files').textContent = '';
}

function u_Init()
{
	for (var i = 0; i < u_elements.length; i++)
		u_el[u_elements[i]] = document.getElementById(u_elements[i]);

	if (localStorage.store_opened == 'ON')
	{
		if (localStorage.header_opened == 'true')
			 u_OpenCloseHeader();
		if (localStorage.footer_opened == 'true')
			 u_OpenCloseFooter();
	}

	if (p_PLAYER)
		return;

	if (localStorage.navig_width == null)
		localStorage.navig_width = 360;
	if (localStorage.sidepanel_width == null)
		localStorage.sidepanel_width = 260;
	if (localStorage.sidepanel_closed_width == null)
		localStorage.sidepanel_closed_width = 20;

	if (localStorage.sidepanel_opened == null)
		localStorage.sidepanel_opened = 'false';
	if (localStorage.sidepanel_opened == 'true')
		u_SidePanelOpen();
	else
		u_SidePanelClose();

	if (localStorage.thumb_file_size == null)
		localStorage.thumb_file_size = '80';
	if (localStorage.thumb_file_crop == null)
		localStorage.thumb_file_crop = 'false';

	if (localStorage.store_opened == null)
		localStorage.store_opened = 'OFF';
	$('store_opened').textContent = localStorage.store_opened;

	if (localStorage.show_hidden == null)
		localStorage.show_hidden = 'OFF';
	$('show_hidden').textContent = localStorage.show_hidden;

	if (localStorage.has_filesystem == null)
		localStorage.has_filesystem = 'OFF';
	$('has_filesystem').textContent = localStorage.has_filesystem;

	if (localStorage.execute_soft == null)
		localStorage.execute_soft = 'OFF';
	$('execute_soft').textContent = localStorage.execute_soft;
	
	if (localStorage.navig_filter_flags_include)
		u_navig_filter_flags_include = localStorage.navig_filter_flags_include.split(',');
	if (localStorage.navig_filter_flags_exclude)
		u_navig_filter_flags_exclude = localStorage.navig_filter_flags_exclude.split(',');

	u_CalcGUI();

	$('body_panel_edit').m_panel_edit = u_EditPanelCreate($('body_panel_edit'));
	$('body_body').onkeydown = u_BodyEditOnKeyDown;
	$('body_timecode_edit_value').onkeydown = u_BodyEditOnKeyDown;

	for (var i = 0; i < u_views.length; i++)
		u_OpenCloseView(u_views[i], false, false);
}
function u_BodyEditOnKeyDown(i_e)
{
	if (u_body_editing)
	{
		if ((i_e.keyCode == 13) && i_e.ctrlKey)  // CTRL + ENTER
		{
			u_BodyEditSave();
			i_e.currentTarget.blur();
		}
	}
}
function u_InitAuth()
{
	st_InitAuth();
	$('body_edit').style.display = 'block';
	$('search_artists_grp').style.display = 'block';
	$('auth_user').textContent = c_GetUserTitle() + ' [' + g_auth_user.id + ']';

	if (c_CanExecuteSoft())
		$('execute_div').style.display = 'block';
}

function u_InitConfigured()
{
}

function u_Process()
{
	if (RULES.annotations)
		for (var name in RULES.annotations)
		{
			var elDiv = document.createElement('div');
			$('annotations').appendChild(elDiv);

			var el = document.createElement('div');
			elDiv.appendChild(el);
			el.classList.add('name');
			el.textContent = name + ':';

			var el = document.createElement('div');
			elDiv.appendChild(el);
			el.classList.add('text');
			el.textContent = RULES.annotations[name];
		}

	if (c_RuFileExists(RULES.thumbnail.filename))
	{
		u_el.thumbnail.src = c_GetRuFilePath(RULES.thumbnail.filename);
		u_el.thumbnail.style.display = 'inline';
	}
	else
	{
		u_el.thumbnail.style.display = 'none';
	}

	if (g_elCurFolder.classList.contains('dummy'))
	{
		$('content').style.display = 'none';
		return;
	}
	else
		$('content').style.display = 'block';

	// Restore may be hidden by asset views:
	for (var i = 0; i < u_views.length; i++)
		$(u_views[i] + '_div').style.display = 'block';

	$('body_avatar_c').style.display = 'none';
	$('body_avatar_m').style.display = 'none';

	st_Show(RULES.status);

	u_ViewsFuncsOpen();

	var path = c_PathPM_Rules2Client(g_elCurFolder.m_path);
	c_Info(path);

	if (c_HasFileSystem())
	{
		$('open').style.display = 'block';
		cgru_CmdExecProcess({"element": $('open'), "open": path});
	}
	else
	{
		$('open').style.display = 'none';
		$('open').ondblclick = null;
	}

	u_ExecuteShow(false);

	u_NavigFiltersRefresh();

	if (c_RuFileExists('location.json'))
	{
		n_GetFile({
			"path": c_GetRuFilePath('location.json'),
			"func": u_LocationInfoReceived,
			"info": 'location.json',
			"local": true
		});
	}
}

function u_LocationInfoReceived(i_data, i_args)
{
	let info = '';
	if (i_data.cuser)
		info += ' by ' + c_GetUserTitle(i_data.cuser);
	if (i_data.ctime)
		info += ' at ' + c_DT_StrFromSec(i_data.ctime);
	if (info.length)
		$('location_info').innerHTML = 'Created ' + info;
}

function u_Finish()
{
	$('annotations').textContent = '';
	u_el.thumbnail.style.display = 'none';

	activity_Finish();
	st_Finish();
	nw_Finish();
	a_Finish();
	fv_Finish();
	cm_Finish();

	u_ViewsFuncsClose();

	$('location_info').textContent = '';
	$('body_body').innerHTML = '';
	u_body_text = '';

	$('body_avatar_c').style.display = 'none';
	$('body_avatar_m').style.display = 'none';
}

function u_StoreOpened()
{
	if (localStorage.store_opened == 'ON')
		localStorage.store_opened = 'OFF';
	else
		localStorage.store_opened = 'ON';
	$('store_opened').textContent = localStorage.store_opened;
}
function u_OpenCloseHeader()
{
	u_OpenCloseHeaderFooter($('headeropenbtn'), 'header', -200, 0);
}
function u_OpenCloseFooter()
{
	u_OpenCloseHeaderFooter($('footeropenbtn'), 'footer', 38, 238);
}
function u_ShowHiddenToggle()
{
	if (localStorage.show_hidden == 'ON')
		localStorage.show_hidden = 'OFF';
	else
		localStorage.show_hidden = 'ON';
	$('show_hidden').textContent = localStorage.show_hidden;
}

function u_CalcGUI()
{
	let barW = u_el.navig.offsetWidth - u_el.navig.clientWidth;
	let sideW = parseInt(localStorage.sidepanel_width);
	let sideClosedW = parseInt(localStorage.sidepanel_closed_width);
	let navigW = parseInt(localStorage.navig_width);

	if (localStorage.sidepanel_opened == 'true')
	{
		$('sidepanel_handle').style.display = 'block';
		$('sidepanel').style.left = u_gui_size_handle;
	}
	else
	{
		$('sidepanel_handle').style.display = 'none';
		$('sidepanel').style.left = '0px';
	}

	$('navig_div').style.width = navigW + 'px';
	$('navig').style.marginRight = '0px';

	$('content').style.left = navigW + 'px';

	if (localStorage.sidepanel_opened == 'true')
	{
		$('content').style.right = sideW + 'px';
		$('sidepanel_div').style.width = sideW + 'px';
		$('sidepanel').style.marginRight = '0px';
	}
	else
	{
		$('content').style.right = sideClosedW + 'px';
		$('sidepanel_div').style.width = sideClosedW + 'px';
		$('sidepanel').style.marginRight = '-10px';
	}
}

function u_ResizeGUIStart(i_name, i_coeff)
{
	u_resizing_name = i_name;
	u_resizing_koeff = i_coeff;
	u_resizing_x = null;
	document.onmousemove = u_ResizeGUI;
	document.onmouseup = u_ResizeGUIFinish;
}
function u_ResizeGUI(i_e)
{
	if (u_resizing_name == null)
		u_ResizeGUIFinish();
	if (u_resizing_x == null)
		u_resizing_x = i_e.screenX;

	var size = parseInt(localStorage[u_resizing_name]);
	var delta = i_e.screenX - u_resizing_x;
	var size = size + delta * u_resizing_koeff;
	if (size < 128)
		return;
	if (size > 768)
		return;
	u_resizing_x = i_e.screenX;
	// console.log( size+'+'+i_e.screenX+'-'+u_resizing_x+'='+size);
	localStorage[u_resizing_name] = '' + size;
	u_CalcGUI();
}
function u_ResizeGUIFinish()
{
	u_resizing_name = null;
	document.onmousemove = null;
	document.onmouseup = null;
}

function u_ApplyStyles()
{
	if (p_PLAYER)
		return;

	if (localStorage.background && localStorage.background.length)
	{
		u_background = localStorage.background;
		document.body.style.background = localStorage.background;
/* It can be just inherited, no need to set it to children (background: inherit;)
		var backs =
			['header', 'footer', 'navig_div', 'sidepanel_div', 'content', 'navig_handle', 'sidepanel_handle'];
		for (var i = 0; i < backs.length; i++)
			$(backs[i]).style.background = localStorage.background;
*/
	}

	if (localStorage.text_color && localStorage.text_color.length)
	{
		u_textColor = localStorage.text_color;
		document.body.style.color = localStorage.text_color;
	}

	var backs = ['asset', 'body', 'files', 'comments'];
	for (var i = 0; i < backs.length; i++)
	{
		var back = localStorage['back_' + backs[i]];
		if (back && back.length)
			$(backs[i] + '_div').style.background = localStorage['back_' + backs[i]];
	}
}

function u_OpenCloseHeaderFooter(i_elBtn, i_id, i_closed, i_opened)
{
	if (i_elBtn.classList.contains('opened'))
	{
		i_elBtn.classList.remove('opened');
		if (i_id == 'header')
		{
			localStorage.header_opened = 'false';
			document.getElementById(i_id).style.top = i_closed + 'px';
		}
		else
		{
			localStorage.footer_opened = 'false';
			document.getElementById('footer').style.height = i_closed + 'px';
			document.getElementById('log').style.display = 'none';
			$('log_ctrl').style.display = 'none';
		}
	}
	else
	{
		i_elBtn.classList.add('opened');
		if (i_id == 'header')
		{
			localStorage.header_opened = 'true';
			document.getElementById(i_id).style.top = i_opened + 'px';
		}
		else
		{
			localStorage.footer_opened = 'true';
			document.getElementById('footer').style.height = i_opened + 'px';
			document.getElementById('log').style.display = 'block';
			$('log_ctrl').style.display = 'block';
		}
	}
}

function u_SidePanelHideOnClick()
{
	u_SidePanelClose()
}
function u_SidePanelClose()
{
	localStorage.sidepanel_opened = 'false';
	$('sidepanel').classList.remove('opened');
	u_CalcGUI();
}
function u_SidePanelOpen()
{
	localStorage.sidepanel_opened = 'true';
	$('sidepanel').classList.add('opened');
	u_CalcGUI();
}

function u_RulesShow()
{
	cgru_ShowObject(RULES, 'RULES ' + g_CurPath());
}


function u_NavigSettingsOnClick()
{
	let elSettings = $('navig_settings');
	if (elSettings.m_opened)
	{
		elSettings.m_opened = false;
		elSettings.style.display = 'none';
		return;
	}

	elSettings.m_opened = true;
	elSettings.style.display = 'block';

	u_NavigFiltersRefresh();
}
function u_NavigFiltersRefresh()
{
//	if ($('navig_settings').m_opened != true)
//		return;

	localStorage.navig_filter_flags_exclude = u_navig_filter_flags_exclude.join(',');
	localStorage.navig_filter_flags_include = u_navig_filter_flags_include.join(',');

	let elFlagsEx = st_SetElFlags({"flags":u_navig_filter_flags_exclude}, $('navig_filter_flags_exclude'), true);
	for (let el of elFlagsEx)
	{
		el.title = "Double click to remove.";
		el.ondblclick = function(e)
		{
			u_navig_filter_flags_exclude = u_navig_filter_flags_exclude.filter(i => i !== e.currentTarget.m_name);
			u_NavigFiltersRefresh();
		}
	}
	let elFlagsIn = st_SetElFlags({"flags":u_navig_filter_flags_include}, $('navig_filter_flags_include'), true);
	for (let el of elFlagsIn)
	{
		el.title = "Double click to remove.";
		el.ondblclick = function(e)
		{
			u_navig_filter_flags_include = u_navig_filter_flags_include.filter(i => i !== e.currentTarget.m_name);
			u_NavigFiltersRefresh();
		}
	}

	let elParent = g_elCurFolder.m_elParent;
	if (elParent == null)
		return;
	let elFolders = elParent.m_elFolders;
	if (elFolders == null)
		return;

	let flags_coll = [];
	for (let elFolder of elFolders)
	{
		let fobject = elFolder.m_fobject;
		if (fobject == null) continue;

		let flags = [];
		if (fobject.status && fobject.status.flags)
			flags = fobject.status.flags;

//		let toHide = false;
		let toHide = (u_navig_filter_flags_include.length > 0);
		for (let flag of flags)
		{
			if (u_navig_filter_flags_exclude.indexOf(flag) != -1)
			{
				toHide = true;
				continue;
			}

			if (u_navig_filter_flags_include.indexOf(flag) != -1)
			{
				toHide = false;
				continue;
			}

			if (flags_coll.indexOf(flag) == -1)
				flags_coll.push(flag);
		}

		if (toHide && ASSET && (ASSET.type == 'shot'))
			elFolder.style.display = 'none';
		else
			elFolder.style.display = 'block';
	}

	let elFlagsColl = $('navig_filter_flags_collected');

	let elFlags = st_SetElFlags({"flags":flags_coll}, elFlagsColl, true);

	for (let elFlag of elFlags)
		elFlag.onclick = function(e){c_ElToggleSelected(e.currentTarget)};
}
function u_NavigFilterIncludeOnClick() {u_NavigFilterInExOnClick('include');}
function u_NavigFilterExcludeOnClick() {u_NavigFilterInExOnClick('exclude');}
function u_NavigFilterInExOnClick(i_inex)
{
	let elFlagsColl = $('navig_filter_flags_collected');
	if (elFlagsColl.m_elFlags == null)
		return;

	let flags = [];
	for (let el of elFlagsColl.m_elFlags)
		if (el.m_selected && (flags.indexOf(el.m_name) == -1))
			flags.push(el.m_name);

	for (let flag of flags)
		if (window['u_navig_filter_flags_'+i_inex].indexOf(flag) == -1)
			window['u_navig_filter_flags_'+i_inex].push(flag);

	u_NavigFiltersRefresh();
}


function u_DrawColorBars(i_args)
{
	var height = i_args.height;
	var elParent = i_args.el;
	var onclick = i_args.onclick;
	var data = i_args.data;

	if (height == null)
		height = '20px';
	else
		height = Math.round(height / 3) + 'px';
	elParent.classList.add('colorbars');
	var ccol = 35;
	var crow = 3;
	var cstep = 5;
	var cnum = crow * ccol;
	for (var cr = 0; cr < crow; cr++)
	{
		var elRaw = document.createElement('div');
		elParent.appendChild(elRaw);
		for (var cc = 0; cc < ccol; cc++)
		{
			var el = document.createElement('div');
			elRaw.appendChild(el);
			el.style.width = 100 / ccol + '%';
			el.style.height = height;

			var r = 0, g = 0, b = 0;
			r = ((cc % cstep) + 1) / (cstep + 1);

			if (cc < cstep)
			{
				r = cc / (cstep - 1);
				g = r;
				b = r;
			}
			else if (cc < cstep * 2)
			{
				r = r;
			}
			else if (cc < cstep * 3)
			{
				g = r;
				r = 0;
			}
			else if (cc < cstep * 4)
			{
				b = r;
				r = 0;
			}
			else if (cc < cstep * 5)
			{
				g = r;
			}
			else if (cc < cstep * 6)
			{
				b = r;
			}
			else if (cc < cstep * 7)
			{
				g = r;
				b = r;
				r = 0;
			}

			if (cr > 0)
			{
				var avg = (r + g + b) / 2.5;
				var sat = 2, add = .1;
				if (cr > 1)
				{
					sat = 1.2;
					add = .2
				};
				r += add + (avg - r) / sat;
				g += add + (avg - g) / sat;
				b += add + (avg - b) / sat;
			}

			r = Math.round(255 * r);
			g = Math.round(255 * g);
			b = Math.round(255 * b);
			if (r > 255)
				r = 255;
			if (g > 255)
				g = 255;
			if (b > 255)
				b = 255;

			if (cr && (cc < cstep))
				el.m_color = null;
			else
			{
				el.style.background = 'rgb(' + r + ',' + g + ',' + b + ')';
				el.m_color = [r, g, b];
			}
			// window.console.log('rgb('+r+','+g+','+b+')');

			el.m_data = data;
			el.onclick = function(e) { onclick(e.currentTarget.m_color, e.currentTarget.m_data); };
		}
	}
}

function u_BodyLoad(i_args)
{
	if (i_args == null)
		i_args = {};

	if (u_body_edit_markup)
		u_BodyEditMarkup();

	if (false == c_RuFileExists(u_body_filename))
	{
		u_BodyShowInfo();
		return;
	}

	var cache = RULES.cache_time;
	if (i_args.cache === false)
		cache = 0;

	n_GetFile({
		"path": c_GetRuFilePath(u_body_filename),
		"func": u_BodyReceived,
		"cache_time": cache,
		"info": 'body.html',
		"parse": false,
		"local": true
	});
}

function u_BodyReceived(i_data, i_args)
{
	u_body_text = i_data;
	$('body_body').innerHTML = u_body_text;
	u_BodyShowInfo();
}

function u_BodyShowInfo()
{
	var info = '';
	if (RULES.status && RULES.status.body)
	{
		var avatar = c_GetAvatar(RULES.status.body.cuser, RULES.status.body.guest);
		if (avatar)
		{
			$('body_avatar_c').style.display = 'block';
			$('body_avatar_c').src = avatar;
		}

		info += '<i>';
		info += 'Created by ' + c_GetUserTitle(RULES.status.body.cuser, RULES.status.body.guest);
		if (g_admin && RULES.status.body.guest && RULES.status.body.guest.email)
			info += ' ' + c_EmailDecode(RULES.status.body.guest.email);
		info += ' at ' + c_DT_StrFromSec(RULES.status.body.ctime);
		if (RULES.status.body.muser)
		{
			if (RULES.status.body.cuser != RULES.status.body.muser)
			{
				var avatar = c_GetAvatar(RULES.status.body.muser);
				if (avatar)
				{
					$('body_avatar_m').style.display = 'block';
					$('body_avatar_m').src = avatar;
				}
			}

			info += '<br>Modified by ' + c_GetUserTitle(RULES.status.body.muser);
			info += ' at ' + c_DT_StrFromSec(RULES.status.body.mtime);
		}
		info += '</i>';
	}
	if (RULES.status && RULES.status.timecode_start)
	{
		var timecode = RULES.status.timecode_start + ' - ' + RULES.status.timecode_finish;
		var frame_start = c_TC_FromSting(RULES.status.timecode_start);
		var frame_finish = c_TC_FromSting(RULES.status.timecode_finish);
		var frames_num = frame_finish - frame_start + 1;
		var duration = c_TC_FromFrame(frames_num).replace('00:', '');
		$('body_timecode').style.display = 'block';
		$('body_timecode_value').textContent = timecode;
		$('body_timecode_framesnum').textContent = frames_num;
		$('body_timecode_framesnum').m_frames_num = frames_num;
		$('body_timecode_duration').textContent = duration;
		$('body_timecode_fps').textContent = RULES.fps;
		if (RULES.status.frames_num && (frames_num != RULES.status.frames_num))
		{
			$('body_timecode_framesnum').classList.add('error');
			if (RULES.status.frames_num > frames_num)
				$('body_timecode_framesnum').classList.add('greater');
			else
				$('body_timecode_framesnum').classList.remove('greater');
		}
		else
		{
			$('body_timecode_framesnum').classList.remove('error');
		}
		$('body_timecode_edit_value').textContent = timecode;
	}
	else
	{
		$('body_timecode').style.display = 'none';
		$('body_timecode_edit_value').textContent = '';
	}

	$('body_info').innerHTML = info;
	// console.log(info);
}

function u_BodyEditStart()
{
	if (g_auth_user == null)
		return;

	$('body_btn_edit').style.display = 'none';
	$('body_btn_edit_cancel').style.display = 'block';
	$('body_panel').style.display = 'none';
	$('body_panel_edit').style.display = 'block';

	ec_EditingStart({'el':$('body_body'),'form':'body'});

	u_body_editing = true;
}

function u_BodyEditCancel(i_text)
{
	u_body_editing = false;

	if (u_body_edit_markup)
		u_BodyEditMarkup();
	if (i_text == null)
		i_text = u_body_text;
	$('body_body').innerHTML = i_text;

	if (c_CanEditBody())
		$('body_btn_edit').style.display = 'block';
	$('body_btn_edit_cancel').style.display = 'none';
	$('body_panel').style.display = 'block';
	$('body_panel_edit').style.display = 'none';

	ec_EditingFinish({'el':$('body_body')});
}

function u_BodyEditSave()
{
	if (g_auth_user == null)
		return;

	u_body_editing = false;

	if (u_body_edit_markup)
		u_BodyEditMarkup();

	var text = c_LinksProcess($('body_body').innerHTML);

	st_SetTimeCode($('body_timecode_edit_value').textContent);

	var res = n_Request({
		"send": {"save": {"file": c_GetRuFilePath(u_body_filename), "data": text}},
		"func": u_BodyEditSaveFinished,
		"info": 'body save'
	});
	// console.log('RES:'+JSON.stringify( res));
}

function u_BodyEditSaveFinished(i_data, i_args)
{
	if ((i_data == null) || (i_data.error))
	{
		c_Error(i_data.error);
		return;
	}
	// console.log('DAT'+JSON.stringify( i_data));
	// console.log('ARG'+JSON.stringify( i_args));
	st_BodyModified();

	nw_MakeNews({"title": 'body'});

	u_BodyEditCancel();

	// Add body file to .rules folder files list.
	c_RuFileAdd(u_body_filename);
	// Body file be new and does exist before saving.

	u_BodyLoad({"cache": false});
}

function u_BodyEditMarkup()
{
	u_body_edit_markup = 1 - u_body_edit_markup;
	if (u_body_edit_markup)
	{
		$('body_body').textContent = $('body_body').innerHTML;
		$('body_edit_markup').classList.add('selected');
		$('body_panel_edit').m_panel_edit.style.display = 'none';
	}
	else
	{
		$('body_body').innerHTML = $('body_body').textContent;
		$('body_edit_markup').classList.remove('selected');
		$('body_panel_edit').m_panel_edit.style.display = 'block';
	}
}
function u_BodyEditMarkupRemove()
{
	if (u_body_edit_markup)
		return;
	c_elMarkupRemove($('body_body'));
}
function u_ViewsFuncsOpen()
{
	for (var i = 0; i < u_views.length; i++)
		if (localStorage['view_' + u_views[i]] === 'true')
			if (window['View_' + u_views[i] + '_Open'])
				window['View_' + u_views[i] + '_Open']();
}
function u_ViewsFuncsClose()
{
	for (var i = 0; i < u_views.length; i++)
		if (localStorage['view_' + u_views[i]] === 'true')
			if (window['View_' + u_views[i] + '_Close'])
				window['View_' + u_views[i] + '_Close']();
}
function u_OpenCloseView(i_id, i_toggle, i_callfuncs)
{
	// console.log(i_id+': '+localStorage['view_'+i_id]);
	if (localStorage['view_' + i_id] == null)
		localStorage['view_' + i_id] = 'true';
	if (i_toggle !== false)
	{
		if (localStorage['view_' + i_id] !== 'true')
			localStorage['view_' + i_id] = 'true';
		else
			localStorage['view_' + i_id] = 'false';
	}

	if (localStorage['view_' + i_id] === 'true')
	{
		$(i_id + '_div').classList.add('opened');

		if (i_callfuncs !== false)
			if (window['View_' + i_id + '_Open'])
				window['View_' + i_id + '_Open']();
	}
	else
	{
		$(i_id + '_div').classList.remove('opened');

		if (i_callfuncs !== false)
			if (window['View_' + i_id + '_Close'])
				window['View_' + i_id + '_Close']();
	}
}

function u_EditPanelCreate(i_el)
{
	var elPanel = document.createElement('div');
	i_el.appendChild(elPanel);
	elPanel.classList.add('textedit_panel');

	var cmds = [];
	cmds.push(['Tx', 'removeFormat', null, 'Remove Formatting']);
	cmds.push(['<b>B</b>', 'bold', null, 'Bold']);
	cmds.push(['<i>I</i>', 'italic', null, 'Italic']);
	//	cmds.push(['>','indent', null]);
	//	cmds.push(['<','outdent', null]);
	//	cmds.push(['1','insertOrderedList', '']);
	//	cmds.push(['*','insertUnorderedList', null]);
	cmds.push(['&uarr;', 'superscript', null, 'Super']);
	cmds.push(['&darr;', 'subscript', null, 'Sub']);
	cmds.push(['s', 'decreaseFontSize', null, 'Smaller']);
	cmds.push(['S', 'increaseFontSize', null, 'Bigger']);
	//	cmds.push(['k','formatBlock', 'kbd']);
	//	cmds.push(['c','formatBlock', 'code']);
	cmds.push(['F', 'formatBlock', 'pre', 'Pre Formatted']);
	cmds.push(['p', 'formatBlock', 'p', 'Paragraph']);
	cmds.push(['C', 'foreColor', '#990000', 'Text Color']);
	cmds.push(['C', 'foreColor', '#666600', 'Text Color']);
	cmds.push(['C', 'foreColor', '#008800', 'Text Color']);
	cmds.push(['C', 'foreColor', '#007777', 'Text Color']);
	cmds.push(['C', 'foreColor', '#000099', 'Text Color']);
	cmds.push(['C', 'foreColor', '#880088', 'Text Color']);
	cmds.push(['C', 'foreColor', '#FFFFFF', 'Text Color']);
	cmds.push(['C', 'foreColor', '#000000', 'Text Color']);
	cmds.push(['B', 'backColor', '#DD4444', 'Back Color']);
	cmds.push(['B', 'backColor', '#DDDD44', 'Back Color']);
	cmds.push(['B', 'backColor', '#66DD66', 'Back Color']);
	cmds.push(['B', 'backColor', '#66DDDD', 'Back Color']);
	cmds.push(['B', 'backColor', '#6666FF', 'Back Color']);
	cmds.push(['B', 'backColor', '#DD66DD', 'Back Color']);
	cmds.push(['B', 'backColor', '#FFFFFF', 'Back Color']);
	cmds.push(['B', 'backColor', '#000000', 'Back Color']);
	//	cmds.push(['Q','formatBlock', 'DL']);

	for (var i = 1; i < 4; i++)
		cmds.push([('h' + i), 'heading', ('h' + i), 'Header ' + i]);

	for (var i = 0; i < cmds.length; i++)
	{
		var el = document.createElement('div');
		elPanel.appendChild(el);
		el.classList.add('button');
		el.innerHTML = cmds[i][0];
		el.title = cmds[i][3];
		el.m_cmd = cmds[i];
		//		el.style.cssFloat = 'left';
		if ((el.m_cmd[1] == 'foreColor') && (el.m_cmd[1] != null))
			el.style.color = el.m_cmd[2];
		if ((el.m_cmd[1] == 'backColor') && (el.m_cmd[1] != null))
			el.style.background = el.m_cmd[2];
		el.onclick = function(e) {
			var el = e.currentTarget;
			document.execCommand(el.m_cmd[1], false, el.m_cmd[2]);
		}
	}

	return elPanel;
}

function u_GuestAttrsDraw(i_el)
{
	i_el.classList.add('guest_attrs');
	i_el.m_guest_attrs = {};

	var el = document.createElement('div');
	i_el.appendChild(el);
	el.classList.add('caption');
	el.textContent = 'Guest attributes:';

	for (var i = 0; i < u_guest_attrs.length; i++)
	{
		var attr = u_guest_attrs[i];

		var el = document.createElement('div');
		i_el.appendChild(el);
		el.classList.add('attr');

		var elLabel = document.createElement('div');
		el.appendChild(elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = attr.label;

		var elInfo = document.createElement('div');
		el.appendChild(elInfo);
		elInfo.classList.add('info');
		var info = 'optional';
		if (attr.required)
			info = 'required';
		if (attr.info)
			info += ', ' + attr.info;
		elInfo.textContent = info;

		var elEdit = document.createElement('div');
		el.appendChild(elEdit);
		elEdit.contentEditable = true;
		elEdit.classList.add('editing');
		elEdit.m_attr = attr.name;
		elEdit.onblur = u_GuestAttrValidate;
		if (localStorage['guest_' + attr.name])
		{
			if (attr.name == 'email')
				elEdit.textContent = c_EmailDecode(localStorage['guest_' + attr.name]);
			else
				elEdit.textContent = localStorage['guest_' + attr.name];
		}

		i_el.m_guest_attrs[attr.name] = elEdit;
	}
}
function u_GuestAttrValidate(i_e)
{
	var el = i_e.currentTarget;
	var value = el.textContent;
	value = value.substr(0, 99);
	value = c_Strip(value);
	if (el.m_attr == 'id')
	{
		value = value.toLowerCase();
		value = value.substr(0, 12);
		value = c_Strip(value);
		value = value.replace(/\W/g, '_');
	}
	el.textContent = value;
}
function u_GuestAttrsGet(i_el)
{
	for (var i = 0; i < u_guest_attrs.length; i++)
		i_el.m_guest_attrs[u_guest_attrs[i].name].classList.remove('error');

	var guest = {};
	for (var i = 0; i < u_guest_attrs.length; i++)
	{
		var attr = u_guest_attrs[i];
		var el = i_el.m_guest_attrs[attr.name];
		var value = el.textContent;
		value = c_Strip(value);

		if ((attr.name == 'id') && (value.length == 0))
		{
			c_Error('Required guest ID attribute is empty.');
			el.classList.add('error');
			return null;
		}
		if ((attr.name == 'email') && (value.length != 0))
		{
			if (c_EmailValidate(value))
				value = c_EmailEncode(value);
			else
			{
				c_Error('Invalid guest email.');
				el.classList.add('error');
				return null;
			}
		}

		localStorage['guest_' + attr.name] = value;
		guest[attr.name] = value;
	}
	return guest;
}

function u_ThumbnailMake(i_args)
{
	var file = c_GetRuFilePath(RULES.thumbnail.filename);

	if (i_args.no_cache !== true)
	{
		var cache_time = RULES.cache_time;
		if (ASSET.cache_time)
			cache_time = ASSET.cache_time;
		if (u_thumbstime[file] && (c_DT_CurSeconds() - u_thumbstime[file] < cache_time))
		{
			c_Log('Thumbnail cached ' + cache_time + 's: ' + file);
			return;
		}
	}

	var input = null;
	for (var i = 0; i < i_args.paths.length; i++)
	{
		if (input)
			input += ',';
		else
			input = '';
		input += RULES.root + i_args.paths[i];
	}

	var cmd = RULES.thumbnail.cmd_asset.replace(/@INPUT@/g, input).replace(/@OUTPUT@/g, file);
	cmd += ' -c ' + RULES.thumbnail.colorspace;
	if (i_args.no_cache)
		cmd += ' -f';
	if (i_args.skip_movies)
		cmd += ' --nomovie';

	n_Request({
		"send": {"cmdexec": {"cmds": [cmd]}},
		"func": u_ThumbnailShow,
		"info": i_args.info + ' thumbnail',
		"local": true
	});
}
function u_ThumbnailShow(i_data)
{
	if (i_data.error)
	{
		c_Error('Make thumbnail: ' + i_data.error);
		return;
	}

	if (i_data.status == 'skipped')
		return;

	var file = c_GetRuFilePath(RULES.thumbnail.filename);
	u_el.thumbnail.src = file + '#' + (new Date().getTime());
	u_el.thumbnail.style.display = 'inline';

	// Update time
	u_thumbstime[file] = c_DT_CurSeconds();

	// Ensure that it exists in walk rules files:
	if (g_elCurFolder && g_elCurFolder.m_dir)
	{
		if (g_elCurFolder.m_dir.rufiles == null)
			g_elCurFolder.m_dir.rufiles = [];
		if (g_elCurFolder.m_dir.rufiles.indexOf(RULES.thumbnail.filename) == -1)
			g_elCurFolder.m_dir.rufiles.push(RULES.thumbnail.filename);
	}
}

function u_HasFilesystem()
{
	if (localStorage.has_filesystem == 'OFF')
		localStorage.has_filesystem = 'ON';
	else
		localStorage.has_filesystem = 'OFF';
	$('has_filesystem').textContent = localStorage.has_filesystem;
}

function u_ExecuteSoft()
{
	if (localStorage.execute_soft == 'OFF')
		localStorage.execute_soft = 'ON';
	else
		localStorage.execute_soft = 'OFF';
	$('execute_soft').textContent = localStorage.execute_soft;
}

function u_ExecuteShow(i_show)
{
	let elItems = $('execute_items');
	let elFavor = $('execute_favor');

	elItems.textContent = '';
	elFavor.textContent = '';

	if (false == c_CanExecuteSoft())
		return;

	let favourites = [];
	if (localStorage.execute_favourites)
		favourites = JSON.parse(localStorage.execute_favourites);

	let actions = [];
	let afavors = [];
	for (let action of RULES.execute)
	{
		if (action.asset && (ASSETS[action.asset] == null))
			continue;

		if (favourites.indexOf(action.name) == -1)
			actions.push(action);
		else
			afavors.push(action);
	}

	u_CreateActions(afavors, elFavor);

	let elBtn = $('execute_btn');
	if (i_show == null)
	{
		if (elBtn.classList.contains('pushed'))
			i_show = false;
		else
			i_show = true;
	}

	if (i_show)
	{
		elBtn.classList.add('pushed');
		u_CreateActions(actions, elItems);

		let el = document.createElement('div');
		el.classList.add('info');
		el.textContent = 'Use CTRL or SHIFT click to add to favourites or remove from.'
		elItems.appendChild(el);
	}
	else
	{
		elBtn.classList.remove('pushed');
	}
}

function u_CreateActions(i_actions, i_el)
{
	let elements = [];

	for (let action of i_actions)
	{
		let el = document.createElement('div');
		i_el.appendChild(el);
		el.textContent = action.label;
		if (action.title)
			el.title = action.title;

		// Process icon:
		let icon = action.icon;
		if (icon)
		{
			el.style.backgroundImage = 'url(' + icon + ')';
			el.style.paddingLeft = '28px';
		}

		// Process command:
		let cmd = null;
		if (action.cmd)
		{
			cmd = c_PathPM_Server2Client(action.cmd);
			cmd = cmd.replace(/@PATH@/g, c_PathPM_Rules2Client(g_CurPath()));
			cmd = cmd.replace(/@SHOT@/g, c_PathPM_Rules2Client(ASSETS.shot.path));
			cmd = cmd.replace(/@USER@/g, g_auth_user.id);

			if (action.show_on_activity)
			{
				if (activity_Selected == null)
					el.style.display = 'none';

				el.classList.add('show_on_activity');
			}
		}

		if (action.server)
		{
			el.m_cmd = cmd;
			el.classList.add('cmdexec_server');
			el.onclick = u_CmdExecServerOnClick;
			el.ondblclick = u_CmdExecServerOnDblClick;
		}
		else
		{
			// Process open:
			let open = null;
			if (action.open)
				open = action.open.replace(/@PATH@/g, c_PathPM_Rules2Client(g_CurPath()));

			// Process terminal:
			let terminal = null;
			if (action.terminal)
				terminal = action.terminal.replace(/@PATH@/g, c_PathPM_Rules2Client(g_CurPath()));

			// Make an executable button:
			cgru_CmdExecProcess({'element':el,'cmd':cmd,'open':open,'terminal':terminal});
		}

		// Add action on CTRL or SHIFT click to add/remove favourites
		el.m_action = action;
		el.addEventListener('click', u_ExecClicked);

		elements.push(el);
	}

	return elements;
}

function u_ExecClicked(i_evt)
{
	if ((i_evt.ctrlKey == false) && (i_evt.shiftKey == false))
		return;

	i_evt.stopPropagation();
	let action = i_evt.currentTarget.m_action;

	let favourites = [];
	if (localStorage.execute_favourites)
		favourites = JSON.parse(localStorage.execute_favourites);

	let index = favourites.indexOf(action.name);
	if (index == -1)
		favourites.push(action.name);
	else
		favourites.splice(index, 1);

	localStorage.execute_favourites = JSON.stringify(favourites);

	u_ExecuteShow($('execute_btn').classList.contains('pushed'));
}


// Button to execute command on server side:
function u_CmdExecServerCreate(i_exec, i_path)
{
	let el = document.createElement('div');
	el.classList.add('cmdexec_server');
	el.textContent = i_exec.name;
	el.title = i_exec.tooltip;

	let cmd = i_exec.cmd;
	cmd = cmd.replace('@PATH@', c_PathPM_Rules2Server(i_path));
	cmd = cmd.replace('@USER@', g_auth_user.id);
	el.m_cmd = cmd;

	el.onclick = u_CmdExecServerOnClick;
	el.ondblclick = u_CmdExecServerOnDblClick;

	return el;
}

function u_CmdExecServerOnClick(i_evt)
{
	i_evt.stopPropagation();
	let el = i_evt.currentTarget;
	c_Info(el.m_cmd, false);
}

function u_CmdExecServerOnDblClick(i_evt)
{
	i_evt.stopPropagation();
	let el = i_evt.currentTarget;
	c_Info('Executing:\n' + el.m_cmd);

	n_Request({
		"send": {"cmdexec": {"cmds": [el.m_cmd]}},
		"func": u_CmdExecServerFinished,
		"el"  : el,
		"info": 'cmdexec_server'
	});

	el.classList.add('running');
}

function u_CmdExecServerFinished(i_data, i_args)
{
	//console.log(i_args);
	//console.log(i_data);

	let el = i_args.el;
	el.classList.remove('running');
}

