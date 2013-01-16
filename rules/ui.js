u_elements = ['asset','assets','content','info','open','log','navig','rules','playlist','status',
	'content_status','thumbnail','sidepanel','sidepanel_playlist'];
u_el = {};

function u_Init()
{
	for( var i = 0; i < u_elements.length; i++)
		u_el[u_elements[i]] = document.getElementById( u_elements[i]);

	if( localStorage.sidepanel_opened_width == null ) localStorage.sidepanel_opened_width = 200;
	if( localStorage.sidepanel_closed_width == null ) localStorage.sidepanel_closed_width = 20;

	if( localStorage.sidepanel_opened == "true" ) u_SidePanelOpen();
	else u_SidePanelClose();
}

function u_Process()
{
	if( RULES.status )
		u_el.status.innerHTML = RULES.status.annotation;
	else
		u_el.status.innerHTML = '';
//		u_el.status.innerHTML = '<span style="font-size:10px">STATUS</span>';
}

function u_HeaderOpenButtonOnClick( i_elBtn, i_id, i_pos)
{
	if( i_elBtn.classList.contains('opened'))
	{
		i_elBtn.classList.remove('opened');
		if( i_id == 'header')
		{
			i_elBtn.innerHTML = '&darr;';
			document.getElementById( i_id).style.top = i_pos;
		}
		else
		{
			i_elBtn.innerHTML = '&uarr;';
			document.getElementById( i_id).style.bottom = i_pos;
		}
	}
	else
	{
		i_elBtn.classList.add('opened');
		if( i_id == 'header')
		{
			i_elBtn.innerHTML = '&uarr;';
			document.getElementById( i_id).style.top = '0px';
		}
		else
		{
			i_elBtn.innerHTML = '&darr;';
			document.getElementById( i_id).style.bottom = '0px';
		}
	}
}

function u_SidePanelHideOnClick() { u_SidePanelClose()}
function u_SidePanelClose()
{
	u_el['sidepanel'].classList.remove('opened');
	localStorage.sidepanel_opened = false;
	u_el.content.style.right = '0';
	u_el.sidepanel.style.width = localStorage.sidepanel_closed_width + 'px';
}
function u_SidePanelOpen()
{
	u_el['sidepanel'].classList.add('opened');
	localStorage.sidepanel_opened = true;
	u_el.content.style.right = localStorage.sidepanel_opened_width + 'px';
	u_el.sidepanel.style.width = localStorage.sidepanel_opened_width + 'px';
}

function u_RulesOnClick()
{
	if( u_el.rules.m_opened )
	{
		u_el.rules.m_opened = false;
		u_el.rules.style.display = 'none';
	}
	else
	{
		u_el.rules.m_opened = true;
		u_el.rules.style.display = 'block';
	}
}

function u_StatusEditOnClick()
{
	if( u_el.status.m_editing )
		return;

	var text = '';
	if( RULES.status ) text = RULES.status.annotation;

	u_el.content_status.classList.add('opened');
	u_el.status.m_text = text;
	u_el.status.textContent = text;
	u_el.status.classList.add('editing');
	u_el.status.m_editing = true;
	u_el.status.contentEditable = 'true';
	u_el.status.focus();
}

function u_StatusCancelOnClick()
{
	u_el.status.innerHTML = u_el.status.m_text;
	u_el.status.classList.remove('editing');
	u_el.content_status.classList.remove('opened');
	u_el.status.m_editing = false;
	u_el.status.contentEditable = 'false';
}

function u_StatusSaveOnClick()
{
	var text = u_el.status.innerHTML;
	u_el.status.m_text = text;
	u_StatusCancelOnClick();
	RULES.status = {};
	RULES.status.annotation = text;
//return;

	var obj = {};
	obj.object = {"status":RULES.status};
	obj.add = true;
	obj.file = RULES.root + g_elCurFolder.m_path + '/' + RULES.rules + '/status.json';
	n_Request({"editobj":obj});
}

