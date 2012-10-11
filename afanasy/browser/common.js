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

function cm_TimeStringFromSeconds( i_seconds)
{
	var str = '';

	if( i_seconds == 0 )
		return '0';

	var seconds = i_seconds;
	var days = 0; var hours = 0;
	var minutes = Math.floor( seconds / 60 );
	if( minutes > 0 )
	{
		seconds -= minutes * 60;
		hours = Math.floor( minutes / 60 );
		if( hours > 0 )
		{
			minutes -= hours * 60;
			days = Math.floor( hours / 24 );
			if( days > 0 )
				hours -= days * 24;
		}
	}

	if( days )
		str += days + 'd ';

	if( hours )
	{
		str += hours;
		if( minutes || seconds )
		{
			str += ':'; if( minutes < 10 ) str += '0'; str += minutes;
			if( seconds ) { str += '.'; if( seconds < 10 ) str += '0'; str += seconds;}
		}
		else str += 'h';
	}
	else if( minutes )
	{
		str += minutes;
		if( seconds ) { str += '.'; if( seconds < 10 ) str += '0'; str += seconds;}
		str += 'm';
	}
	else if( seconds )
	{
		str += seconds + 's ';
	}

	return str;
}

cm_States = ['ONL','RDY','RUN','DON','ERR','SKP','OFF','WDP','WTM','DRT','NbY','NBY'];
function cm_GetState( i_state, i_elParent, i_elChild)
{
	if( i_state == null )
		i_elChild.textContent = '-';
	else
		i_elChild.textContent = i_state;

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

function cm_ElCreateFloatText( i_elParent, i_side, i_title)
{
	var element = document.createElement('span');
	i_elParent.appendChild( element);
	if( i_side )
		element.style.cssFloat = i_side;
	if( i_title )
		element.title = i_title;
	if( i_side == 'right')
		element.style.marginLeft = '4px';
	else
		element.style.marginRight = '4px';
	return element;
}

function cm_ElCreateText( i_elParent, i_title)
{
	return cm_ElCreateFloatText( i_elParent, null, i_title);
}
