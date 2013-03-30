up_elFiles = [];
up_counter = 0;

function up_Init()
{
	$('sidepanel_upload').style.display = 'block';
	if( localStorage.upload_opened == 'true' ) up_Open();
	else up_Close();

	up_CreateInput();
}

function up_Close()
{
	$('sidepanel_upload').classList.remove('opened');
	localStorage.upload_opened = 'false';
}
function up_Open()
{
	$('sidepanel_upload').classList.add('opened');
	localStorage.upload_opened = 'true';
}

function up_OnClick()
{
	if( $('sidepanel').classList.contains('opened'))
	{
		if( $('sidepanel_upload').classList.contains('opened'))
			up_Close();
		else
			up_Open();
	}
	else
	{
		u_SidePanelOpen();
		up_Open();
	}
}

function up_InsertElement()
{
	var el = document.createElement('div');
	if( up_elFiles.length )
		$('upload').insertBefore( el, up_elFiles[up_elFiles.length-1]);
	else
		$('upload').appendChild( el);
	up_elFiles.push( el);
	return el;
}


function up_CreateInput()
{
	var el = up_InsertElement();
	var elInput = document.createElement('input');
	el.appendChild( elInput);
	el.m_elInput = elInput;
	elInput.type = 'file';
	elInput.size = '99';
	elInput.onchange = up_FileSelected;
	elInput.m_elFile = el;
	elInput.title = 'Drag and drop files or click to browse';
}

function up_FileSelected( e)
{
	var path = g_CurPath();
	if( ASSET && ASSET.upload_dir && ( ASSET.path == path ))
	{
		path += '/' + ASSET.upload_dir;
		path = path.replace('@DATE@', c_DT_FormStrNow().split(' ')[0]);
		path = path.replace('@USER@', g_auth_user.id);
	}
	var el = e.currentTarget.m_elFile;
	var files = el.m_elInput.files;

	el.m_elInput.style.display = 'none';
	for( var i = 0; i < files.length; i++)
	{
		if( i > 0 )
		{
			el = up_InsertElement();
		}
		up_CreateFile( files[i], path, el );
	}
	up_CreateInput();
}

function up_CreateFile( i_file, i_path, i_el)
{
	var file = i_file;
	var el = i_el;

	el.m_selected = true;
	el.m_upfile = file;
	el.m_path = i_path + '/' + file.name;
	el.m_uppath = RULES.root + i_path;
	el.title = i_path;

	var elBtnAdd = document.createElement('div');
	el.appendChild( elBtnAdd);
	el.m_elBtnAdd = elBtnAdd;
	elBtnAdd.classList.add('button');
	elBtnAdd.textContent = '+';
	elBtnAdd.m_elFile = el;
	elBtnAdd.onclick = function(e){ up_Start( e.currentTarget.m_elFile);};
	elBtnAdd.style.cssFloat = 'left';

	var elBtnDel = document.createElement('div');
	el.appendChild( elBtnDel);
	el.m_elBtnDel = elBtnDel;
	elBtnDel.classList.add('button');
	elBtnDel.textContent = '-';
	elBtnDel.m_elFile = el;
	elBtnDel.onclick = function(e){ up_Remove( e.currentTarget.m_elFile);};
	elBtnDel.style.cssFloat = 'right';

	var elInfo = document.createElement('a');
	el.appendChild( elInfo);
	el.m_elInfo = elInfo;
	elInfo.classList.add('info');
	elInfo.innerHTML = c_Bytes2KMG(file.size) + ' ' + file.name;
	elInfo.href = '#' + i_path;

	var elProgress = document.createElement('div');
	el.appendChild( elProgress);
	elProgress.classList.add('progress');
	elProgress.style.display = 'none';
	el.m_elProgress = elProgress;
	var elBar = document.createElement('div');
	elProgress.appendChild( elBar);
	elBar.classList.add('bar');
	el.m_elBar = elBar;
	var elUpInfo = document.createElement('div');
	elProgress.appendChild( elUpInfo);
	elUpInfo.classList.add('upinfo');
	el.m_elUpInfo = elUpInfo;
}

function up_Start( i_el)
{
	i_el.m_uploading = true;
	i_el.m_elBtnAdd.style.display = 'none';
	i_el.m_elBtnDel.style.display = 'none';
	i_el.m_elBtnAdd.onclick = null;
	i_el.classList.add('started');

	var formData = new FormData();
	formData.append('upload_path', i_el.m_uppath);
	formData.append('upload_file', i_el.m_upfile);

	var xhr = new XMLHttpRequest();
	xhr.upload.addEventListener('progress', up_Progress, false);
	xhr.upload.m_elFile = i_el;
	xhr.m_elFile = i_el;
	xhr.addEventListener('load', up_Load, false);
	xhr.addEventListener('error', up_Error, false);
	xhr.addEventListener('abort', up_Abort, false);
	xhr.open('POST', n_server);
	xhr.send( formData);

	xhr.onreadystatechange = function()
	{
		if( xhr.readyState == 4 )
		{
			if( xhr.status == 200 )
			{
				c_Log('<b style="color:#404"><i>upload'+(up_counter++)+':</i></b> '+ xhr.responseText);
				up_Received( c_Parse( xhr.responseText));
				return;
			}
		}
	}

	i_el.m_elProgress.style.display = 'block';
}

function up_Progress( e)
{
	var el = e.currentTarget.m_elFile;

    if( e.lengthComputable )
	{
		if( e.total > 0 )
		{
			var percent = Math.round( 100 * e.loaded / e.total ) + '%';
			el.m_elUpInfo.textContent = percent;
			el.m_elBar.style.width = percent;
		}
    }
    else
	{
		el.m_elUpInfo.textContent = 'N/A';
	}
}

function up_Load( e) { up_Finished( e.currentTarget.m_elFile,'saving'); }
function up_Error( e) { up_Finished( e.currentTarget.m_elFile,'error');}
function up_Abort( e) { up_Finished( e.currentTarget.m_elFile,'abort');}

function up_Finished( i_el, i_status)
{
	i_el.m_elUpInfo.textContent = i_status;
	i_el.m_upfinished = true;
	i_el.m_elBar.style.width = '100%';
	i_el.m_elBar.classList.add( i_status);
	i_el.m_elBtnDel.style.display = 'block';
}

function up_Received( i_msg)
{
	if( i_msg == null )
	{
		c_Error('Upload undefined error.');
		return;
	}
	if(( i_msg.files == null ) || ( i_msg.files.length == 0 ))
	{
		if( i_msg.error )
			c_Error('Upload: ' + i_msg.error);
		else
			c_Error('Uploaded no files.');
		return;
	}

	var els = [];
	for( var f = 0; f < i_msg.files.length; f++)
	{
		for( var e = 0; e < up_elFiles.length; e++)
		{
//			if( ( up_elFiles[e].m_upfinished ) &&
			if( ( i_msg.path == up_elFiles[e].m_uppath ) &&
				( i_msg.files[f].name == up_elFiles[e].m_upfile.name))
			{
				els.push( up_elFiles[e]);
				up_Done( up_elFiles[e], i_msg.files[f]);
			}
		}
	}

	if( els.length == 0 )
	{
		c_Error('Upload elemants not fonded.');
		return;
	}

//console.log( JSON.stringify( i_msg));
}

function up_Done( i_el, i_msg)
{
	i_el.m_done = true;
	i_el.classList.remove('started');

	if( i_msg.error )
	{
		c_Error('Upload: ' + i_msg.error + ': file="'+i_el.m_upfile.name+'" path="'+i_el.m_uppath+'"');
		i_el.classList.add('error');
		i_el.m_elProgress.textContent = i_msg.error;
	}
	else
	{
		c_Info('Uploaded "'+i_el.m_upfile.name+'" to "' + i_el.m_uppath + '"');
		i_el.m_elProgress.style.display = 'none';
		i_el.classList.add('done');
	}

	c_MakeThumbnail( i_el.m_uppath + '/' + i_el.m_upfile.name);
}

function up_Remove( i_el)
{
	var index = up_elFiles.indexOf( i_el);
	if( index == -1 )
	{
		c_Error('Upload: Removing index not founded.');
		return;
	}
	$('upload').removeChild( up_elFiles[index]);
	up_elFiles.splice( index, 1);
}

function up_StartAll()
{
	for( var i = 0; i < up_elFiles.length; i++)
		if( up_elFiles[i].m_selected == true )
		if( up_elFiles[i].m_uploading !== true )
			up_Start( up_elFiles[i]);
}

function up_ClearAll()
{
	var dels = [];
	for( var i = 0; i < up_elFiles.length; i++)
		if( up_elFiles[i].m_selected == true )
		if(( up_elFiles[i].m_done == true ) || ( up_elFiles[i].m_uploading !== true ))
			dels.push( up_elFiles[i]);

	for( var i = 0; i < dels.length; i++)
		up_Remove( dels[i]);
}

