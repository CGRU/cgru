st_wnd = null;
st_elRoot = null;
st_elParent = null;
st_elToHide = null
st_path = null;
st_status = null;

function st_StatusSetElLabel( i_el, i_status, i_full)
{
	if( i_status && i_status.annotation)
	{
		if( i_full )
			i_el.innerHTML = i_status.annotation;
		else
			i_el.innerHTML = i_status.annotation.split(' ')[0];
	}
	else
		i_el.innerHTML = '';
}
function st_StatusSetColor( i_status, i_elB, i_elC)
{
	if( i_elB == null ) i_elB = st_elRoot;
	if( i_elC == null ) i_elC = i_elB;

	if( i_status &&  i_status.color)
	{
		var c = i_status.color;
		i_elB.style.background = 'rgb('+c[0]+','+c[1]+','+c[2]+')';
		if( c[0]+c[1]+c[2] > 200 )
			i_elC.style.color = '#000';
		else
			i_elC.style.color = '#FFF';
//window.console.log(c[0]+c[1]+c[2])
	}
	else
	{
		i_elB.style.background = '';
		i_elC.style.color = '#000';
	}
}

function st_CreateEditUI( i_elParent, i_path, i_status, i_elToHide)
{
	if( i_elToHide )
		i_elToHide.style.display = 'none';

	st_DestroyEditUI();

	st_elParent = i_elParent;
	st_elToHide = i_elToHide;
	st_path = i_path;
	st_status = i_status;

	st_elRoot = document.createElement('div');
	st_elParent.appendChild( st_elRoot);
	st_elRoot.classList.add('status_edit');

	st_elAnnLabel = document.createElement('div');
	st_elRoot.appendChild( st_elAnnLabel);
	st_elAnnLabel.textContent = 'Status:';

	st_elAnn = document.createElement('div');
	st_elRoot.appendChild( st_elAnn);
	st_elAnn.contentEditable = 'true';

	st_elColor = document.createElement('div');
	u_DrawColorBars( st_elColor, st_EditColorOnClick);

	st_FillEditUI( i_status);
}

function st_FillEditUI( i_status )
{
	st_status = i_status;
	if( st_status == null )
		st_status = {};

	st_StatusSetElLabel( st_elAnn, st_status, true);
	st_StatusSetColor( st_status);

	st_elAnn.focus();
}

function st_DestroyEditUI()
{
	if( st_elRoot && st_elParent )
		st_elParent.removeChild( st_elRoot);

	if( st_elToHide )
		st_elToHide.style.display = 'block';

	st_elRoot = null;
	st_elParent = null;
	st_elToHide = null;
}

function st_EditColorOnClick( i_evt)
{
	var el = i_evt.currentTarget;
	st_status.color = el.m_color
	st_StatusSetColor( st_status);
}

function st_SaveOnClick()
{
	var text = st_elAnn.innerHTML;
	var color = st_elColor.m_status.color;

	RULES.status = {};
	RULES.status.annotation = text;
	RULES.status.color = color;

//	u_StatusCancelOnClick();

//	g_FolderSetStatus( RULES.status);

	var obj = {};
	obj.object = {"status":RULES.status};
	obj.add = true;
	obj.file = RULES.root + st_path + '/' + RULES.rufolder + '/status.json';

c_Info(JSON.stringify(obj));
//	n_Request({"editobj":obj});
}
