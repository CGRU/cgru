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
	editcontent.js
	Content editable serving functions.
	Used in comments, body.
*/

"use strict";

var ec_form = null;
var ec_process_image = null;

function ec_EditingStart(i_args)
{
	var el = i_args.el;
	ec_form = null;
	if (i_args.form)
		ec_form = i_args.form;

	el.contentEditable = 'true';
	el.classList.add('editing');

	el.addEventListener('paste', ec_OnPaste);
	el.addEventListener('dragenter', ec_OnDragEnter);
	el.addEventListener('dragover',  ec_OnDragOver);
	el.addEventListener('dragleave', ec_OnDragLeave);
	el.addEventListener('drop', ec_OnDrop);

	el.focus();
};

function ec_EditingFinish(i_args)
{
	var el = i_args.el;

	el.contentEditable = 'false';
	el.classList.remove('editing');

	el.removeEventListener('paste', ec_OnPaste);
	el.removeEventListener('dragenter', ec_OnDragEnter);
	el.removeEventListener('dragover',  ec_OnDragOver);
	el.removeEventListener('dragleave', ec_OnDragLeave);
	el.removeEventListener('drop', ec_OnDrop);
};

function ec_OnPaste(i_evt)
{
    i_evt.preventDefault();
	if (ec_process_image)
		return;

	// Process data
	if (ec_DataTransfer(i_evt.clipboardData))
		return;

	// Process text
	let text = (i_evt.clipboardData || window.clipboardData).getData('text/plain');
	text.replace('\n','<br>\n');
	if (text && text.length)
		document.execCommand('insertHTML', false, text);
}

function ec_OnDragEnter(i_evt){ec_DragSetStyle(i_evt, true);}
function ec_OnDragOver( i_evt){ec_DragSetStyle(i_evt, true);}
function ec_OnDragLeave(i_evt){ec_DragSetStyle(i_evt, false);}
function ec_DragSetStyle(i_evt, i_on)
{
//console.log(i_evt);
	// Preventing defaults means that element accept drop
	i_evt.preventDefault();
	i_evt.stopPropagation();

	// This can be not element but element text node
	let el = i_evt.target;
	if (! el.classList)
		return;

	// Child element can accept this event too,
	// so we should search text editing element.
	while ((false == el.classList.contains('editing')) && el.parentElement)
	{
		// We are already at the top of the comment.
		if (el.classList.contains('comment'))
			return;

		// We are already at the top of view.
		if (el.classList.contains('view_view'))
			return;

		el = el.parentElement;
	}

	// Add or remove drag class style
	if (i_on)
		el.classList.add('drag');
	else
		el.classList.remove('drag');
}
function ec_OnDrop(i_evt)
{
	ec_DragSetStyle(i_evt, false);

	if (ec_process_image)
		return;

	ec_DataTransfer(i_evt.dataTransfer);
}

function ec_DataTransfer(i_data)
{
//console.log(i_data);
	if (null == i_data)
	{
		c_Error('Transfered data is null.');
		return false;
	}
	if (null == i_data.items)
	{
		c_Error('Transfered data has null items.');
		return false;
	}
	if (i_data.items.length == 0)
	{
		c_Error('Transfered data has zero items.');
		return false;
	}

	let file = null;
	for (let i = 0; i < i_data.items.length; i++)
	{
		let item = i_data.items[i];
		if (item.type.indexOf('image') == -1)
			continue;

		if (file)
		{
			c_Error('You can paste only one image at once.');
			return false;
		}

		file = item.getAsFile();
	}

	if (file)
	{
		ec_ProcessImage(file);
		return true;
	}

	return false;
}

function ec_ProcessImage(i_file)
{
	//console.log(i_file);
	let name = (new Date()).toISOString().replace(/[:.Z]/g,'-') +  g_auth_user.id + '-' + i_file.name;
	// By default, image will be uploaded in the current folder
	let path = RULES.root + g_CurPath();
	// Asset can specify folder to place images (body and comment form)
	if (ASSET && ASSET.inserted_images && ASSET.inserted_images.forms)
		if (ec_form && ASSET.inserted_images.forms[ec_form])
			if (ASSET.inserted_images.forms[ec_form].folder)
				path += '/' + ASSET.inserted_images.forms[ec_form].folder;
	path += '/' + name;

	let elRoot = document.createElement('div');
	document.body.appendChild(elRoot);
	elRoot.classList.add('insert_image_root');

	let elImg = document.createElement('img');
	elRoot.appendChild(elImg);
	elImg.classList.add('insert_image_img');
	elImg.src = URL.createObjectURL(i_file);
	elImg.m_file = i_file;
	elImg.onload = ec_ProcessImageOnLoad;

	let elWnd = document.createElement('div');
	elRoot.appendChild(elWnd);
	elWnd.classList.add('insert_image_wnd');

	let elInfo = document.createElement('div');
	elWnd.appendChild(elInfo);
	elInfo.classList.add('insert_image_info');
	elInfo.innerHTML = i_file.name + ': ' + c_Bytes2KMG(i_file.size);
	elInfo.title = path;

	let elStatus = document.createElement('div');
	elWnd.appendChild(elStatus);
	elStatus.classList.add('insert_image_status');

	let btn_cancel = document.createElement('div');
	elWnd.appendChild(btn_cancel);
	btn_cancel.textContent = 'Cancel';
	btn_cancel.classList.add('button');
	btn_cancel.classList.add('insert_image_cancel');
	btn_cancel.onclick = ec_ProcessImageClose;

	let btn_upload = document.createElement('div');
	elWnd.appendChild(btn_upload);
	btn_upload.textContent = 'Upload Image';
	btn_upload.classList.add('button');
	btn_upload.classList.add('insert_image_upload');
	btn_upload.onclick = ec_ProcessImageUpload;

	let elProgress = document.createElement('div');
	elWnd.appendChild(elProgress);
	elWnd.style.background = u_background;
	elProgress.classList.add('insert_image_progress');

	ec_process_image = {};
	ec_process_image.el_root = elRoot;
	ec_process_image.file = i_file;
	ec_process_image.name = name;
	ec_process_image.path = path;
	ec_process_image.btn_cancel = btn_cancel;
	ec_process_image.btn_upload = btn_upload;
	ec_process_image.el_info = elInfo;
	ec_process_image.el_status = elStatus;
	ec_process_image.el_progress = elProgress;
	ec_process_image.up_percent = -1;
	ec_process_image.uploading = false;
}

function ec_ProcessImageOnLoad()
{
	ec_process_image.width = this.naturalWidth;
	ec_process_image.height = this.naturalHeight;

	var info = ec_process_image.file.name + ':';
	info += ' ' + this.naturalWidth + 'x' + this.naturalHeight;
	info += ' ' + c_Bytes2KMG(ec_process_image.file.size);

	ec_process_image.el_info.innerHTML = info;
}

function ec_ProcessImageUpload()
{
	ec_process_image.uploading = true;
	ec_process_image.btn_cancel.style.display = 'none';
	ec_process_image.btn_upload.style.display = 'none';
	ec_process_image.up_time = (new Date() / 1000);

	var formData = new FormData();
	formData.append('upload_path', ec_process_image.path);
	formData.append('upload_file', ec_process_image.file);

	var xhr = new XMLHttpRequest();
	xhr.upload.addEventListener('progress', ec_ProcessImageUploadProgress, false);
	xhr.addEventListener('load',  ec_ProcessImageUpload_Load,  false);
	xhr.addEventListener('error', ec_ProcessImageUpload_Error, false);
	xhr.addEventListener('abort', ec_ProcessImageUpload_Abort, false);
	xhr.open('POST', n_server);
	xhr.send(formData);

	xhr.onreadystatechange = function() {
		if (xhr.readyState == 4)
		{
			if (xhr.status == 200)
			{
				c_Log('<b style="color:#404"><i>upload:</i></b> ' + xhr.responseText);
				ec_ProcessImageUploadFinished(c_Parse(xhr.responseText));
				return;
			}
		}
	};
}

function ec_ProcessImageUpload_Load()
{
	if (null == ec_process_image) return;
	ec_process_image.el_status.innerHTML = 'Uploading';
}
function ec_ProcessImageUpload_Error()
{
	if (null == ec_process_image) return;
	ec_process_image.el_status.innerHTML = 'Error';
}
function ec_ProcessImageUpload_Abort()
{
	if (null == ec_process_image) return;
	ec_process_image.el_status.innerHTML = 'Aborting';
}

function ec_ProcessImageUploadProgress(i_evt)
{
	if (null == ec_process_image) return;

	var dur = c_DT_DurFromNow(ec_process_image.up_time);
	var text = dur;

	if (i_evt.lengthComputable)
	{
		if (i_evt.total > 0)
		{
			let percent = Math.round(100 * i_evt.loaded / i_evt.total);
			if (ec_process_image.up_percent > percent)
			{
				c_Error('Upload: New progress lowered (' + ec_process_image.up_percent + ' > ' + percent + ') at ' + dur);
			}
			text += ' ' + percent + '%';
			ec_process_image.el_progress.style.width = percent + '%';
			ec_process_image.up_percent = percent;
		}
	}

	ec_process_image.el_status.innerHTML = text;
}

function ec_ProcessImageUploadFinished(i_args)
{
	if (i_args.files == null)
	{
		c_Error('Uploaded no files.');
		ec_process_image.el_status.innerHTML = 'Uploaded no files.';
		return;
	}
	if (i_args.files.length == 0)
	{
		c_Error('Uploaded zero files.');
		ec_process_image.el_status.innerHTML = 'Uploaded zero files.';
		return;
	}
	if (i_args.files.length > 1)
	{
		c_Error('Uploaded several files at once. Processing only the first one.');
	}

	ec_ProcessImageFileUploaded(i_args.files[0]);
}

function ec_ProcessImageFileUploaded(i_file)
{
	var size_make_thumbnail = 200000;
	if (RULES.inserted_images && RULES.inserted_images.size_make_thumbnail)
		size_make_thumbnail = RULES.inserted_images.size_make_thumbnail;

	i_file.src = i_file.path;
	if (i_file.size < size_make_thumbnail)
	{
		ec_ProcessImageInsertHTML(i_file);
		return;
	}

	ec_process_image.el_status.innerHTML = 'Creating thumbnail...';

	i_file.thumbnail = c_PathDir(i_file.path) + '/' + c_PathBase(i_file.path) + '-thumbnail.jpg';

	var cmd = RULES.inserted_images.thumbnail_cmd;
	cmd = cmd.replace('@INPUT@', i_file.path);
	cmd = cmd.replace('@OUTPUT@', i_file.thumbnail);

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}}, "func": ec_ProcessImageTumbnailed, "file": i_file, "info":'thumbnail'});
}

function ec_ProcessImageTumbnailed(i_data, i_args)
{
//console.log(JSON.stringify(i_args));
//console.log(JSON.stringify(i_data));
	var file = i_args.file;

	if ((null == i_data.cmdexec) || (i_data.cmdexec.length == 0))
	{
		c_Error('Error executing thumbnail creation.');
	}
	else if (i_data.cmdexec[0].search(/error/gi) != -1)
	{
		c_Error(i_data.cmdexec[0]);
	}
	else
	{
		file.src = file.thumbnail;
	}

	ec_ProcessImageInsertHTML(file);
}

function ec_ProcessImageInsertHTML(i_file)
{
	var info = i_file.name + ':';
	info += ' ' + ec_process_image.width + 'x' + ec_process_image.height;
	info += ' ' + c_Bytes2KMG(i_file.size);

	var html = '';
	html += '<div class="inserted_image_div">'
	html += '<a target="_blank"';
	html += ' href="' + i_file.path + '"';
	html += '>';
	html += '<span class="inserted_image_info">' + info + '</span>';
	html += '<br>';
	html += '<img class="inserted_image"';
	html += ' src="' + i_file.src + '"';
	html += ' alt="' + i_file.name + '"';
	if (i_file.thumbnail || ec_process_image.width > 600)
		html += ' width=90%';
	html += '>';
	html += '</a>';
	html += '</div>'
	html += '<div> <br> </div>'

	document.execCommand('insertHTML', false, html);

	ec_ProcessImageClose();
}

function ec_ProcessImageClose()
{
	if (null == ec_process_image) return;

	let elParent = ec_process_image.el_root.parentElement;
	if (elParent)
		elParent.removeChild(ec_process_image.el_root);

	ec_process_image = null;
}
