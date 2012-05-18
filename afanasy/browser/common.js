function cm_Init()
{
	document.body.style.MozUserSelect='none';
	document.body.onkeydown = cm_OnKeyDown;
	document.body.onkeyup = cm_OnKeyUp;
	document.body.onmousedown = function(e) { if(e.button==0){g_mouse_down=true;  return false;}}
	document.body.onmouseup   = function(e) { if(e.button==0){g_mouse_down=false; return false;}}
}

function cm_OnKeyDown(e)
{ 
	if(!e) return;
	if(e.keyCode==27) selectAll( false); // escape 
	else if(e.keyCode==16) g_key_shift=true; // shift
	else if(e.keyCode==17) g_key_ctrl=true; // control
//	else if(evt.keyCode==116) return false; // F5
document.getElementById('test').innerHTML='key down: ' + e.keyCode;
//	return false; 
}

function cm_OnKeyUp(e)
{ 
	if(!e) return;
	if(e.keyCode==16) g_key_shift=false; // shift
	if(e.keyCode==17) g_key_ctrl=false; // control
document.getElementById('test').innerHTML='key up: ' + e.keyCode;
//	return false; 
}

function info( i_msg, i_elem)
{
	if( i_elem == null )
		i_elem = 'info';
	document.getElementById(i_elem).innerHTML=i_msg;
}

function cm_ItemMouseDown(e)
{
	if( e.button != 0 ) return;
	if( false == g_key_ctrl )
		cm_ItemSelectAll( e.currentTarget, false);
	cm_ItemToggleSelection(e.currentTarget);
}

function cm_ItemMouseOver(e)
{
	if( e.button != 0 ) return;
	if( false == g_mouse_down ) return;
	cm_ItemToggleSelection(e.currentTarget)
}

function cm_ItemToggleSelection( el)
{
	if (!el) return;
	if (el.selected )
		cm_ItemSetSelected( el, false)
	else
		cm_ItemSetSelected( el, true)
}

function cm_ItemSetSelected( el, on)
{
	if( on )
	{
		if( el.selected ) return;
		el.selected = true;
		el.className = 'selected';
	}
	else
	{
		if( false == el.selected ) return;
		el.selected = false;
		el.className = '';
	}
}

function cm_ItemSelectAll( el, on)
{
	var divs = el.parentElement.getElementsByTagName('div');
	for (var i = 0; i < divs.length; i++)
		cm_ItemSetSelected( divs[i], on);
}

function mergeIds( i_ids1, i_ids2)
{
	o_ids = i_ids1;
	if( o_ids == null )
		o_ids = [];
	if(( i_ids2 != null ) && ( i_ids2.length > 0 ))
	{
		if( o_ids.length > 0 )
		{
			for( i = 0; i < i_ids2.length; i++)
			{
				founded = false;
				for( o = 0; o < o_ids.length; o++)
				{
					if( o_ids[o] == i_ids2[i] )
					{
						founded = true;
						break;
					}
				}
				if( founded == false )
					o_ids.push(i_ids[i])
			}
		}
		else
			o_ids = i_ids2;
	}
	return o_ids;
}

function timeStringFromNow( time)
{
	time = new Date() - new Date( time * 1000);
	var seconds = Math.floor(time / 1000);
	var minutes = Math.floor(seconds / 60);
	var hours = Math.floor(minutes / 60);
	var days = Math.floor(hours / 24);
	if( days == 1 )
		days = 0;
	seconds = seconds - minutes*60;
	if( seconds < 10 )
		seconds = '0' + seconds;
	minutes = minutes - hours*60;
	if( minutes < 10 )
		minutes = '0' + minutes;
	hours = hours - days*24;
	time = hours + ':' + minutes + '.' + seconds;
	if( days > 1 )
		time = days + 'd ' + time;
	return time;
}
