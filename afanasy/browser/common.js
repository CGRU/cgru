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
	if(e.keyCode==27) // ESC
	{
		g_key_shift=false;
		g_key_ctrl=false;
		g_mouse_down=false;
		cm_ItemSelectAll( null, false);
	}
	else if(e.keyCode==16) g_key_shift=true; // SHIFT
	else if(e.keyCode==17) g_key_ctrl=true; // CTRL
	else if(e.keyCode==65 && g_key_ctrl) // CTRL+A
	{
		cm_ItemSelectAll( null, true);
		return false;
	}
	else if(e.keyCode==38) cm_ItemSelectNext( true); // DOWN
	else if(e.keyCode==40) cm_ItemSelectNext( false); // UP
//	else if(evt.keyCode==116) return false; // F5
document.getElementById('test').innerHTML='key down: ' + e.keyCode;
//	return false; 
}

function cm_OnKeyUp(e)
{ 
	if(!e) return;
	if(e.keyCode==16) g_key_shift=false; // SHIFT
	if(e.keyCode==17) g_key_ctrl=false; // CTRL
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
	g_cur_monitor = el.parentElement.monitor;
	if( on )
	{
		if( el.selected ) return;
		el.selected = true;
		if( false == el.classList.contains('selected'))
			el.classList.add('selected');
//		el.innerHTML='selected';
	}
	else
	{
		if( false == el.selected ) return;
		el.selected = false;
		el.classList.remove('selected');
//		el.innerHTML='';
	}
}

function cm_ItemSelectAll( el, on)
{
	if( el )
		g_cur_monitor = el.parentElement.monitor;
	if( g_cur_monitor == null ) return;
	for( var i = 0; i < g_cur_monitor.items.length; i++)
		cm_ItemSetSelected( g_cur_monitor.items[i].element, on);
}

function cm_ItemSelectNext( previous )
{
	if( g_cur_monitor == null ) return;
	if( g_cur_monitor.items.length == 0 ) return;
	if( g_cur_monitor.cur_item == null )
		g_cur_monitor.cur_item = g_cur_monitor.items[0];

//get index of current element
	cur_index = 0;
	for( var i = 0; i < g_cur_monitor.items.length; i++)
		if( g_cur_monitor.cur_item == g_cur_monitor.items[i])
		{
			cur_index = i;
			break;
		}

	if( previous )
		next_index = cur_index-1;
	else
		next_index = cur_index+1;

	if( next_index < 0 ) return;
	if( next_index >= g_cur_monitor.items.length ) return;
	if( cur_index == next_index ) return;

	g_cur_monitor.cur_item = g_cur_monitor.items[next_index]; 
	element = g_cur_monitor.cur_item.element;
	if( false == g_key_shift )
		cm_ItemSelectAll( element, false);
	cm_ItemSetSelected( element, true);
//	cm_ItemToggleSelection( element);
}

function cm_ArrayRemove( io_arr, i_value)
{
	var index = io_arr.indexOf( i_value);
	if( index >= 0 )
		io_arr.splice( index, 1);
}

function cm_IdsMerge( i_ids1, i_ids2)
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

function cm_TimeStringFromNow( time)
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
