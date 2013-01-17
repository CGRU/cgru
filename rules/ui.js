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
	if( RULES.status && RULES.status.annotation )
		u_el.status.innerHTML = RULES.status.annotation;
	else
		u_el.status.innerHTML = '';
	
	if( RULES.status )
		u_StatusSetColor( RULES.status.color );
}
function u_StatusSetColor( c)
{
	if( c )
		u_el.content_status.parentNode.style.background = 'rgb('+c[0]+','+c[1]+','+c[2]+')';
	else
		u_el.content_status.parentNode.style.background = '';
}

function u_Finish()
{
	u_StatusCancelOnClick();
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

	elColor = document.getElementById('status_color');
	var ccol = 30;
	var crow = 3;
	var cnum = crow * ccol;
	var i = 0;
	for( var cr = 0; cr < crow; cr++)
	{
		elRaw = document.createElement('div');
		elColor.appendChild( elRaw);
		for( var cc = 0; cc < ccol; cc++)
		{
			el = document.createElement('div');
			elRaw.appendChild( el);
			el.style.width = 100/ccol + '%';
			el.onclick = u_StatusColorOnClick;

			var r = (i / cnum * 9) % 1;
			var g = (i / cnum * 3) % 1;
			var b = (i / cnum * 1) % 1;
			r = Math.round( 255*r);
			g = Math.round( 255*g);
			b = Math.round( 255*b);

			el.style.background = 'rgb('+r+','+g+','+b+')';
			el.m_color = [r,g,b];

			i++;
//window.console.log('rgb('+r+','+g+','+b+')');
		}
	}

	u_el.status.contentEditable = 'true';
	u_el.status.focus();
}

function u_StatusColorOnClick( i_evt)
{
	var el = i_evt.currentTarget;
	u_el.status.m_color = el.m_color;
	u_StatusSetColor( el.m_color);
}

function u_StatusCancelOnClick()
{
	u_el.status.innerHTML = u_el.status.m_text;
	u_el.status.classList.remove('editing');
	u_el.content_status.classList.remove('opened');
	u_el.status.m_editing = false;
	u_el.status.m_color = null;
	u_el.status.contentEditable = 'false';
	document.getElementById('status_color').innerHTML = '';
}

function u_StatusSaveOnClick()
{
	var text = u_el.status.innerHTML;
	var color = u_el.status.m_color;
	u_el.status.m_text = text;
	u_StatusCancelOnClick();
	RULES.status = {};
	RULES.status.annotation = text;
	RULES.status.color = color;
//return;

	var obj = {};
	obj.object = {"status":RULES.status};
	obj.add = true;
	obj.file = RULES.root + g_elCurFolder.m_path + '/' + RULES.rules + '/status.json';
	n_Request({"editobj":obj});
}

