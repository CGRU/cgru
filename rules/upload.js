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
//	elInput.size = '99';
	elInput.onchange = up_FileSelected;
	elInput.m_elFile = el;
	elInput.title = 'Drag and drop files or click to browse';
}

function up_FileSelected( e)
{
	var el = e.currentTarget.m_elFile;
	var files = el.m_elInput.files;

	el.m_elInput.style.display = 'none';
	for( var i = 0; i < files.length; i++)
	{
		if( i > 0 )
		{
			el = up_InsertElement();
		}

		var title = 'upload';
		var path = g_CurPath();
		if( ASSET && ASSET.uploads && ( ASSET.path == path ))
			for( var up in ASSET.uploads )
				for( var m = 0; m < ASSET.uploads[up].masks.length; m++ )
					if( files[i].name.match( new RegExp(ASSET.uploads[up].masks[m], 'i')))
					{
						path = g_CurPath() + '/' + ASSET.uploads[up].folder;
						title = ASSET.uploads[up].news_title;
						if( title == null ) title = up;
					}

		path = path.replace('@DATE@', c_DT_FormStrNow().split(' ')[0]);
		path = path.replace('@USER@', g_auth_user.id);

		up_CreateFile({'file':files[i],'path':path,'title':title,'el':el});
	}
	up_CreateInput();
}

function up_CreateFile( i_args)
{
	var file = i_args.file;
	var path = i_args.path;
	var el = i_args.el;

	el.m_selected = true;
	el.m_upfile = file;
	el.m_uppath = path;
	el.m_uptitle = i_args.title;
	el.m_curpath = g_CurPath();
	el.title = path;

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
	elInfo.href = '#' + path;

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
	i_el.m_time = (new Date() / 1000);

	var formData = new FormData();
	formData.append('upload_path', RULES.root + i_el.m_uppath);
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
	i_el.m_xhr = xhr;

	xhr.onreadystatechange = function()
	{
		if( xhr.readyState == 4 )
		{
			if( xhr.status == 200 )
			{
				c_Log('<b style="color:#404"><i>upload'+(up_counter++)+':</i></b> '+ xhr.responseText);
				up_Received( c_Parse( xhr.responseText), i_el);
				return;
			}
		}
	}

	i_el.m_elProgress.style.display = 'block';
}

function up_Progress( e)
{
	var el = e.currentTarget.m_elFile;

	var dur = c_DT_DurFromNow( el.m_time);
	var text = dur;

    if( e.lengthComputable )
	{
		if( e.total > 0 )
		{
			var percent = Math.round( 100 * e.loaded / e.total );
			if( el.m_percent && ( el.m_percent > percent ))
			{
				c_Error('Upload: New progress lowered (' + el.m_percent + ' > ' + percent + ') at ' + dur);
			}
			text += ' ' + percent + '%';
			el.m_elBar.style.width = percent + '%';
			el.m_percent = percent;
		}
    }

	el.m_elUpInfo.textContent = text;
}

function up_Load( e) { up_Finished( e.currentTarget.m_elFile,'saving', this);}
function up_Error( e) { up_Finished( e.currentTarget.m_elFile,'error', this);}
function up_Abort( e) { up_Finished( e.currentTarget.m_elFile,'abort', this);}

function up_Finished( i_el, i_status, i_xhr)
{
	if( i_el.m_done ) return;

	var info = i_status;
	if( info == null )
		info = '';
	else
		info += ': '

	var time = (new Date()) / 1000 - i_el.m_time;
	var speed = i_el.m_upfile.size;
	if(( time > 0 ) && ( speed > 0 ))
	{
		speed = c_Bytes2KMG( speed / time) + '/s';
		info += c_DT_DurFromNow( i_el.m_time) + ' x ' + speed;
	}

	if( i_xhr )
		i_el.title += '\n' + i_xhr.status + ': ' + i_xhr.statusText;

	i_el.m_elUpInfo.innerHTML = info;
	i_el.m_upfinished = true;
	i_el.m_elBar.style.width = '100%';
	if( i_status )
		i_el.m_elBar.classList.add( i_status);
	i_el.m_elBtnDel.style.display = 'block';
}

function up_Received( i_args, i_el)
{
	if( i_args == null )
	{
		c_Error('Upload undefined error.');
		return;
	}
	if(( i_args.files == null ) || ( i_args.files.length == 0 ))
	{
		if( i_args.error )
			c_Error('Upload server: ' + i_args.error);
		else
			c_Error('Uploaded no files.');
		return;
	}

	var news = [];
	for( var f = 0; f < i_args.files.length; f++)
	{
		up_Done( i_el, i_args.files[f]);

		var filename = i_args.files[f].filename;
		if( filename == null ) continue;

		var news_link = g_GetLocationArgs({"fv_Goto":i_el.m_uppath + '/' + c_PathBase(i_args.files[f].filename)}, false, i_el.m_curpath);
		news.push( nw_CreateNews({'title':i_el.m_uptitle,'path':i_el.m_curpath,'link':news_link}));
	}

	if( news.length )
		nw_SendNews( news);
//console.log( JSON.stringify( i_args));
}

function up_Done( i_el, i_msg)
{
	up_Finished( i_el);

	i_el.m_done = true;
	i_el.classList.remove('started');

	if( i_msg.error )
	{
		c_Error('Upload: ' + i_msg.error + ': file="'+i_el.m_upfile.name+'" path="'+i_el.m_uppath+'"');
		i_el.classList.add('error');
		i_el.m_elProgress.textContent = i_msg.error;
		return;
	}

	c_Info('Uploaded "'+i_el.m_upfile.name+'" to "' + i_el.m_uppath + '"');
	i_el.classList.add('done');

	// Refresh files views same path:
	fv_RefreshPath( i_el.m_uppath);

//	c_MakeThumbnail( i_el.m_uppath + '/' + i_el.m_upfile.name);
}

function up_Remove( i_el)
{
	var index = up_elFiles.indexOf( i_el);
	if( index == -1 )
	{
		c_Error('Upload: Removing index not found.');
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

