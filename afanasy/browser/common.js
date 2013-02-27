cm_Attrs = [];
cm_Attrs.push(['name',            'Name',     'Name']);
cm_Attrs.push(['user_name',       'User',     'User Name']);
cm_Attrs.push(['host_name',       'Host',     'Host Name']);
cm_Attrs.push(['priority',        'Priority', 'Priority']);
cm_Attrs.push(['time_creation',   'Created',  'Time Created']);
cm_Attrs.push(['time_started',    'Started',  'Time Started']);
cm_Attrs.push(['time_done',       'Finished', 'Time Finished']);
cm_Attrs.push(['order',           'Order',    'Order']);

// This is task progress parameters short names:
cm_Attrs.push(['str',             'Starts',   'Starts Count']);
cm_Attrs.push(['err',             'Errors',   'Errors Count']);
cm_Attrs.push(['hst',             'Host',     'Host Name']);

function cm_ApplyStyles()
{
	document.body.style.background = localStorage.background;
	document.body.style.color = localStorage.text_color;
//	$('header').style.background = localStorage.background;
//	$('footer').style.background = localStorage.background;
//	$('navig').style.background = localStorage.background;
//	$('sidepanel').style.background = localStorage.background;
}

function cm_CompareItems( i_itemA, i_itemB, i_param, i_greater )
{
	if( i_greater == null ) i_greater = false;

	if( i_itemA == null || i_itemB == null )
		return false;

	if( i_itemA.params[i_param] > i_itemB.params[i_param] )
		return i_greater == true;
	if( i_itemA.params[i_param] < i_itemB.params[i_param] )
		return i_greater == false;

	if( i_param != 'name' )
		if( i_itemA.params['name'] < i_itemB.params['name'] )
			return true;

	return false;
}

function cm_ArrayRemove( io_arr, i_value)
{
	var index = io_arr.indexOf( i_value);
	if( index >= 0 )
		io_arr.splice( index, 1);
}

function cm_IdsMerge( i_ids1, i_ids2)
{
	var o_ids = i_ids1;
	if( o_ids == null )
		o_ids = [];
	if(( i_ids2 != null ) && ( i_ids2.length > 0 ))
	{
		if( o_ids.length > 0 )
		{
			for( var i = 0; i < i_ids2.length; i++)
			{
				founded = false;
				for( var o = 0; o < o_ids.length; o++)
				{
					if( o_ids[o] == i_ids2[i] )
					{
						founded = true;
						break;
					}
				}
				if( founded == false )
					o_ids.push( i_ids2[i])
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

cm_States = ['ONL','RDY','RUN','DON','ERR','SKP','OFF','WDP','WTM','DRT','NbY','NBY','PER','PBR','PSC','WFL','WSL','WWK'];
function cm_GetState( i_state, o_state, o_element)
{
	for( var i = 0; i < cm_States.length; i++)
	{
		if( o_element )
			o_element.classList.remove( cm_States[i]);
		if( i_state )
		{
			if( i_state.indexOf( cm_States[i]) != -1 )
			{
				o_state[cm_States[i]] = true;
				if( o_element )
					o_element.classList.add( cm_States[i]);
				continue;
			}
		}
		o_state[cm_States[i]] = false;
	}
}

function cm_ElCreateFloatText( i_elParent, i_side, i_title)
{
	var element = document.createElement('span');
	element.classList.add('text');
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

function cm_CreateStart( i_item)
{
	i_item.elStar = document.createElement('div');
	i_item.element.appendChild( i_item.elStar);
	i_item.elStar.classList.add('star');
	i_item.elStar.innerHTML = localStorage.run_symbol;
	i_item.elStarCount = document.createElement('div');
	i_item.elStar.appendChild( i_item.elStarCount);
	i_item.elStarCount.classList.add('count');
}

function cm_FillNumbers( i_string, i_number)
{
	var pattern = /@#+@/g;
	var match = i_string.match(pattern);
	for(i in match)
	{
		var replace_mask = match[i];
		if(replace_mask.length > i_number.toString().length)
		{
			var replace_string = Array(replace_mask.length - i_number.toString().length - 1).join("0") + i_number.toString();
			i_string = i_string.replace(replace_mask, replace_string);
		}
	}
	return i_string;
}
