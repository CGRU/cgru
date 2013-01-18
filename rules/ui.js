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
	else
		u_StatusSetColor();
}
function u_StatusSetColor( c, i_elB, i_elC)
{
	if( i_elB == null ) i_elB = u_el.content_status.parentNode;
	if( i_elC == null ) i_elC = u_el.content_status.parentNode;

	if( c )
	{
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
	var color = null;
	if( RULES.status )
	{
		text = RULES.status.annotation;
		color = RULES.status.color;
	}

	u_el.content_status.classList.add('opened');
	u_el.status.m_text = text;
	u_el.status.m_color_saved = color;
	u_el.status.m_color = color;
	u_el.status.textContent = text;
	u_el.status.classList.add('editing');
	u_el.status.m_editing = true;

	elColor = document.getElementById('status_color');
	elColor.style.display = 'block';
	var ccol = 35;
	var crow = 3;
	var cstep = 5;
	var cnum = crow * ccol;
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

			var r = 0, g = 0, b = 0;
			r = ( ( cc % cstep ) + 1 ) / ( cstep + 1 );

			if     (cc < cstep  ) { r = cc/(cstep-1); g = r; b = r; }
			else if(cc < cstep*2) { r = r; }
			else if(cc < cstep*3) { g = r; r = 0; }
			else if(cc < cstep*4) { b = r; r = 0; }
			else if(cc < cstep*5) { g = r; }
			else if(cc < cstep*6) { b = r; }
			else if(cc < cstep*7) { g = r; b = r; r = 0; }

			if( cr > 0 )
			{
				var avg = (r+g+b)/2.5;
				var sat = 2, add = .1;
				if( cr > 1 ) { sat = 1.2; add = .2};
				r += add+(avg-r)/sat;
				g += add+(avg-g)/sat;
				b += add+(avg-b)/sat;
			}

			r = Math.round( 255*r);
			g = Math.round( 255*g);
			b = Math.round( 255*b);
			if( r > 255 ) r = 255;
			if( g > 255 ) g = 255;
			if( b > 255 ) b = 255;

			if( cr && (cc < cstep))
				el.m_color = null;
			else
			{
				el.style.background = 'rgb('+r+','+g+','+b+')';
				el.m_color = [r,g,b];
			}
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
	u_StatusSetColor( u_el.status.m_color_saved);
	u_el.status.contentEditable = 'false';
	document.getElementById('status_color').innerHTML = '';
	document.getElementById('status_color').style.display  = 'none';
}

function u_StatusSaveOnClick()
{
	var text = u_el.status.innerHTML;
	var color = u_el.status.m_color;
	u_el.status.m_color_saved = color;

	u_el.status.m_text = text;
	u_StatusCancelOnClick();
	g_StatusSetColor( color);
	g_elCurFolder.m_elStatus.innerHTML = text;

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

