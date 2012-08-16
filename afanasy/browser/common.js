function cm_Init()
{
//	document.body.style.MozUserSelect='none';
	document.body.onkeydown = cm_OnKeyDown;
	document.body.onkeyup = cm_OnKeyUp;
//	document.body.onmousedown = function(e) { if(e.button==0){g_mouse_down=true;  return false;}}
//	document.body.onmouseup   = function(e) { if(e.button==0){g_mouse_down=false; return false;}}
	document.body.onmousedown = function(e) { if(e.button==0){g_mouse_down=true; }}
	document.body.onmouseup   = function(e) { if(e.button==0){g_mouse_down=false;}}
}

function cm_OnKeyDown(e)
{ 
	if(!e) return;
	if(e.keyCode==27) // ESC
	{
		g_key_shift=false;
		g_key_ctrl=false;
		g_mouse_down=false;
		for( var i = 0; i < g_monitors.length; i++)
		{
			g_monitors[i].selectAll( false);
		}
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

function cm_TimeStringInterval( time1, time2)
{
	if( time2 == null )
		time2 = new Date();
	else
		time2 = new Date( time2 * 1000);

	time = time2 - new Date( time1 * 1000);
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

cm_States = ['RDY','RUN','DON','ERR','SKP','OFF','WDP','WTM','DRT','NbY','NBY'];
function cm_GetState( i_state, i_elParent, i_elChild)
{
	if( i_state == null )
		i_elChild.innerHTML = '-';
	else
		i_elChild.innerHTML = i_state;

	for( var i = 0; i < cm_States.length; i++)
	{
		i_elParent.classList.remove( cm_States[i]);
		if( i_state )
		{
			if( i_state.indexOf( cm_States[i]) != -1 )
			{
				eval('i_elChild.' + cm_States[i] + ' = true;');
				i_elParent.classList.add( cm_States[i]);
			}
			else
				eval('i_elChild.' + cm_States[i] + ' = false;');
		}
	}
}

