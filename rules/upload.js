up_elFiles = [];

function up_Init()
{
	$('sidepanel_upload').style.display = 'block';
	if( localStorage.upload_opened == 'true' ) up_Open();
	else up_Close();

	up_AddEmptyFile();
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

function up_AddEmptyFile()
{
	var el = document.createElement('div');
	if( up_elFiles.length )
		$('upload').insertBefore( el, up_elFiles[up_elFiles.length-1]);
	else
		$('upload').appendChild( el);
	up_elFiles.push( el);

	var elInput = document.createElement('input');
	el.appendChild( elInput);
	el.m_elInput = elInput;
	elInput.type = 'file';
	elInput.size = '7';
	elInput.onchange = up_FileSelected;
	elInput.m_elFile = el;

	var elPanel = document.createElement('div');
	el.appendChild( elPanel);
	el.m_elPanel = elPanel;
	elPanel.classList.add('panel');
	elPanel.style.display = 'none';

	var elBtn = document.createElement('div');
	elPanel.appendChild( elBtn);
	el.m_elBtn = elBtn;
	elBtn.classList.add('button');
	elBtn.textContent = '+';
	elBtn.m_elFile = el;
	elBtn.onclick = up_FileBtnOnClick;

	var elInfo = document.createElement('div');
	elPanel.appendChild( elInfo);
	el.m_elInfo = elInfo;
	elInfo.classList.add('info');

	var elProgress = document.createElement('div');
	elPanel.appendChild( elProgress);
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
//elUpInfo.textContent = 'upinfo';
	el.m_elUpInfo = elUpInfo;
}

function up_FileSelected( e)
{
	var el = e.currentTarget.m_elFile;
	var file = el.m_elInput.files[0];
	var reader = new FileReader();

	reader.m_file = file;
	reader.onload = up_FileLoaded;
	reader.readAsDataURL( file);
	reader.m_elFile = el;
	el.m_reader = reader;
	el.m_file = file;
	el.m_path = g_CurPath();
	el.title = el.m_path;
}

function up_FileLoaded( e)
{
	var el = e.currentTarget.m_elFile;
	var file = el.m_file;

	el.m_elInput.style.display = 'none';
	el.m_elPanel.style.display = 'block';

	var info = file.name;
	info += ' ' + c_Bytes2KMG(file.size);
//	info += ' ' + file.type;
	el.m_elInfo.textContent = info;

	up_AddEmptyFile();
}

function up_FileBtnOnClick( e)
{
	var el = e.currentTarget.m_elFile;
	if( el.m_upfinished )
	{
		up_Remove( el);
	}
	else if( el.m_uploading )
	{
		return;
	}
	else
	{
		el.m_elBtn.style.display = 'none';
		up_Start( el);
	}
}

function up_Start( i_el)
{
	i_el.m_uploading = true;

	var formData = new FormData();
	formData.append('upload_path', RULES.root + '/' + i_el.m_path);
	formData.append('upload_file', i_el.m_file);

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
				c_Log('<b style="color:#404"><i>upload'+(n_recvCount++)+':</i></b> '+ xhr.responseText);
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

function up_Load( e) { up_Finished( e.currentTarget.m_elFile,'DONE'); }
function up_Error( e) { up_Finished( e.currentTarget.m_elFile,'ERROR');}
function up_Abort( e) { up_Finished( e.currentTarget.m_elFile,'ABORT');}

function up_Finished( i_el, i_status)
{
	i_el.m_elUpInfo.textContent = i_status;
	i_el.m_elBtn.style.display = 'block';
	i_el.m_elBtn.textContent = '-';
	i_el.m_upfinished = true;
	i_el.m_elBar.style.width = '100%';
	i_el.m_elBar.classList.add( i_status);
}

function up_Received( i_msg)
{
	if( i_msg == null )
	{
		c_Error('Upload undefined error.');
		return;
	}
	if( i_msg.error )
	{
		c_Error('Upload: ' + i_msg.error);
		return;
	}
	c_Info('File uploaded to ' + i_msg.path);
//console.log( JSON.stringify( i_msg));
}

function up_Remove( i_el)
{
	var index = up_elFiles.indexOf( i_el);
	if( index == -1 )
	{
		c_Error('up_Remove: index not founded.');
		return;
	}
	$('upload').removeChild( up_elFiles[index]);
	up_elFiles.splice( index, 1);
}

