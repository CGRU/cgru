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

'use strict';

var prof_avatar_max_size = 24 * 1024;
var prof_avatar_min_size = 1 * 1024;
var prof_avatars_location = 'users/avatars'

var prof_props = {
	id /**********/: {'disabled': true, 'label': 'ID'},
	role /********/: {'disabled': true},
	title /*******/: {},
	avatar /******/: {},
	news_limit /**/: {},
	email /*******/: {'width': '70%'},
	email_news /**/: {'width': '30%', 'type': 'bool', 'default': false},
	signature /***/: {}
};

var prof_avatar_exts = ['jpg', 'png', 'gif'];

var prof_wnd = null;

function prof_Open()
{
	if (g_auth_user == null)
	{
		c_Error('No authenticated user found.');
		return;
	}

	if (g_admin)
		prof_props.avatar.disabled = false;
	else
		prof_props.avatar.disabled = true;

	prof_wnd = new cgru_Window({'name': 'profile', 'title': 'My Profile'});
	prof_wnd.elContent.classList.add('profile');

	let elAvatarDiv = document.createElement('div');
	prof_wnd.elContent.appendChild(elAvatarDiv);
	elAvatarDiv.classList.add('avatar_div');

	prof_wnd.elAvatarImg = document.createElement('img');
	elAvatarDiv.appendChild(prof_wnd.elAvatarImg);
	let avatar = c_GetAvatar();
	if (avatar)
		prof_wnd.elAvatarImg.src = avatar;
	else
		prof_wnd.elAvatarImg.style.display = 'none';


	let elSelectLabel = document.createElement('label');
	elAvatarDiv.appendChild(elSelectLabel);
	elSelectLabel.classList.add('button', 'upload');
	elSelectLabel.innerHTML = 'Select<br>Avatar';

	let elAvatarInput = document.createElement('input');
	elSelectLabel.appendChild(elAvatarInput);
	elAvatarInput.type = 'file';
	elAvatarInput.innerHTML = 'Avatar Input File';
	elAvatarInput.onchange = prof_AvatarSelected;

	let pav_info = 'Square Image:';
	pav_info += '<br>Max size: ' + c_Bytes2KMG(prof_avatar_max_size);
	pav_info += '<br>Extensions:<br>jpg, png, gif';

	let elUploadInfo = document.createElement('div');
	elAvatarDiv.appendChild(elUploadInfo);
	elUploadInfo.classList.add('pav_info');
	elUploadInfo.innerHTML = pav_info;

	prof_wnd.elUploadStatus = document.createElement('div');
	elAvatarDiv.appendChild(prof_wnd.elUploadStatus);

	prof_wnd.elUploadAvatar = document.createElement('div');
	elAvatarDiv.appendChild(prof_wnd.elUploadAvatar);
	prof_wnd.elUploadAvatar.classList.add('pav_upload', 'button');
	prof_wnd.elUploadAvatar.innerHTML = 'Upload Avatar';
	prof_wnd.elUploadAvatar.style.display = 'none';
	prof_wnd.elUploadAvatar.onclick = prof_UploadAvatar;

	gui_Create(prof_wnd.elContent, prof_props, [g_auth_user]);

	let elBtns = document.createElement('div');
	prof_wnd.elContent.appendChild(elBtns);
	elBtns.classList.add('buttons_div');
	elBtns.style.clear = 'both';

	prof_wnd.elContent.onkeydown =
		function(e) {
		if (e.keyCode == 13)  // Enter
			prof_Save();
	}

	let el = document.createElement('div');
	elBtns.appendChild(el);
	el.textContent = 'Save';
	el.classList.add('button');
	el.onclick = function(e) {
		prof_Save();
	};

	el = document.createElement('div');
	elBtns.appendChild(el);
	el.textContent = 'Cancel';
	el.classList.add('button');
	el.onclick = function(e) {
		prof_wnd.destroy();
	};

	if (c_CanSetPassword())
	{
		el = document.createElement('div');
		elBtns.appendChild(el);
		el.textContent = 'Set Password';
		el.classList.add('button');
		el.onclick = function(e) {
			ad_SetPasswordDialog(g_auth_user.id);
		};
	}

	prof_StatusClear();
}

function prof_Save()
{
	let params = gui_GetParams(prof_wnd.elContent, prof_props);

	if (params.news_limit.length == 0)
		params.news_limit = '-1';

	params.news_limit = parseInt(params.news_limit);
	if (isNaN(params.news_limit))
	{
		c_Error('Invalid news limit number.');
		return;
	}

	let save_obj = {'id': g_auth_user.id};
	for (let p in params)
	{
		g_auth_user[p] = params[p];
		save_obj[p] = params[p];
	}
	g_users[g_auth_user.id] = g_auth_user;

	ad_SaveUser(save_obj);
	ad_UpdateProfileSettings();
	prof_wnd.destroy();
}

function prof_AvatarSelected(e)
{
	prof_StatusClear();
	prof_wnd.elUploadAvatar.style.display = 'none';
	prof_wnd.avatar_file = null;

	let el = e.currentTarget;
	let file = el.files[0];
	let split = c_PathSplitExt(file.name);
	let ext = split[1].toLowerCase();
	if (ext == 'jpeg')
		ext = 'jpg';

	if (ext.length < 1)
	{
		prof_StatusError('Empty file extension.');
		return;
	}
	if (prof_avatar_exts.indexOf(ext) == -1)
	{
		prof_StatusError('Extension "' + ext + '"<br>not allowed.');
		return;
	}
	if (file.size < prof_avatar_min_size)
	{
		prof_StatusError(
			'File size < ' + c_Bytes2KMG(prof_avatar_min_size) + '<br>' + c_Bytes2KMG(file.size));
		return;
	}
	if (file.size > prof_avatar_max_size)
	{
		prof_StatusError(
			'File size > ' + c_Bytes2KMG(prof_avatar_max_size) + '<br>' + c_Bytes2KMG(file.size));
		return;
	}

	let path = prof_avatars_location + '/' + g_auth_user.id + '.' + ext;

	prof_wnd.elContent.m_elements['avatar'].textContent = path;

	prof_wnd.elAvatarImg.src = URL.createObjectURL(file);
	prof_wnd.elAvatarImg.style.display = 'block';

	prof_wnd.elUploadAvatar.style.display = 'block';

	prof_wnd.avatar_file = file;
}

function prof_UploadAvatar()
{
	prof_wnd.elUploadAvatar.style.display = 'none';

	if (prof_wnd.avatar_file == null)
	{
		prof_StatusError('Avatar file is null.');
		return;
	}

	// Check path:
	let params = gui_GetParams(prof_wnd.elContent, prof_props);
	let path = params.avatar.toLowerCase();
	if (path.length == 0)
	{
		prof_StatusError('Zero Avatar path.');
		return;
	}
	let split = c_PathSplitExt(path);
	let ext = split[1];
	if (ext == 'jpeg')
		ext = 'jpg';
	if (prof_avatar_exts.indexOf(ext) == -1)
	{
		prof_StatusError('Extension "' + ext + '"<br>not allowed.');
		return;
	}
	path = prof_avatars_location + '/' + c_PathBase(split[0]) + '.' + ext;
	prof_wnd.elContent.m_elements['avatar'].textContent = path;

	let formData = new FormData();
	formData.append('upload_path', path);
	formData.append('upload_file', prof_wnd.avatar_file);
	formData.append('upload_replace', 1);

	let xhr = new XMLHttpRequest();
	xhr.addEventListener('load', prof_AvatarUpload_Load, false);
	xhr.addEventListener('error', prof_AvatarUpload_Error, false);
	xhr.addEventListener('abort', prof_AvatarUpload_Abort, false);
	xhr.open('POST', n_server);
	xhr.send(formData);

	xhr.onreadystatechange = function() {
		if (xhr.readyState == 4)
		{
			if (xhr.status == 200)
			{
				c_Log('<b style="color:#404"><i>upload:</i></b> ' + xhr.responseText);
				prof_AvatarUploadFinished(c_Parse(xhr.responseText));
				return;
			}
		}
	};
}
function prof_AvatarUpload_Load() {}
function prof_AvatarUpload_Error()
{
	prof_StatusError('Upload error.');
}
function prof_AvatarUpload_Abort()
{
	prof_StatusError('Upload aborted.');
}
function prof_AvatarUploadFinished(i_args)
{
	if (i_args.error)
	{
		prof_StatusError(i_args.error);
		return;
	}

	prof_wnd.elContent.m_elements['avatar'].textContent = i_args.upload.path;
	prof_wnd.elAvatarImg.src = i_args.upload.path;

	prof_StatusSuccess('Avatar uploaded.');
}

function prof_StatusClear()
{
	prof_wnd.elUploadStatus.style.display = 'none';
	prof_wnd.elUploadStatus.innerHTML = '';
	prof_wnd.elUploadStatus.className = 'pav_status';
}
function prof_StatusSuccess(i_msg)
{
	prof_StatusClear();
	prof_wnd.elUploadStatus.innerHTML = i_msg;
	prof_wnd.elUploadStatus.classList.add('pav_status_success');
	prof_wnd.elUploadStatus.style.display = 'block';
}
function prof_StatusError(i_msg)
{
	prof_StatusClear();
	prof_wnd.elUploadStatus.innerHTML = i_msg;
	prof_wnd.elUploadStatus.classList.add('pav_status_error');
	prof_wnd.elUploadStatus.style.display = 'block';
}
