
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
	$('upload').appendChild( el);

	var input = document.createElement('input');
	el.appendChild( input);
	el.m_elInput = input;
	input.type = 'file';
	input.onchange = up_FileSelected;
	input.m_elFile = el;

	var info = document.createElement('div');
	el.appendChild( info);
	el.m_elInfo = info;
	info.classList.add('info');
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
}

function up_FileLoaded( e)
{
	var el = e.currentTarget.m_elFile;
	var file = el.m_file;

	var info = 'Info: ';
	info += ' size='+c_Bytes2KMG(file.size);
	info += ' name='+file.name;
	info += ' type='+file.type;

	el.m_elInfo.textContent = info;
}

