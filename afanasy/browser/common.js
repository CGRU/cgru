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
		if( g_cur_monitor) g_cur_monitor.selectAll( false);
	}
	else if(e.keyCode==16) g_key_shift=true; // SHIFT
	else if(e.keyCode==17) g_key_ctrl=true; // CTRL
	else if(e.keyCode==65 && g_key_ctrl) // CTRL+A
	{
		if( g_cur_monitor) g_cur_monitor.selectAll( true);
		return false;
	}
	else if((e.keyCode==38) && g_cur_monitor) g_cur_monitor.selectNext( true); // DOWN
	else if((e.keyCode==40) && g_cur_monitor) g_cur_monitor.selectNext( false); // UP
//	else if(evt.keyCode==116) return false; // F5
//document.getElementById('test').innerHTML='key down: ' + e.keyCode;
//	return false; 
}

function cm_OnKeyUp(e)
{ 
	if(!e) return;
	if(e.keyCode==16) g_key_shift=false; // SHIFT
	if(e.keyCode==17) g_key_ctrl=false; // CTRL
//document.getElementById('test').innerHTML='key up: ' + e.keyCode;
//	return false; 
}

function info( i_msg, i_elem)
{
	if( i_elem == null )
		i_elem = 'info';
	document.getElementById(i_elem).innerHTML=i_msg;
}

function cm_Error( i_msg)
{
	document.getElementById('error').innerHTML='Error:' + i_msg;
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

function cm_GetState( i_state)
{
	var state = {};
	state.string = i_state;
	if( i_state.indexOf('RDY') != -1 ) state.rdy = true;
	if( i_state.indexOf('RUN') != -1 ) state.run = true;
	if( i_state.indexOf('DON') != -1 ) state.don = true;
	if( i_state.indexOf('ERR') != -1 ) state.err = true;
	if( i_state.indexOf('SKP') != -1 ) state.skp = true;
	return state;
}

