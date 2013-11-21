
function gui_Create( i_wnd, i_params, i_defaults)
{
	i_wnd.classList.add('dialog');
	if( i_wnd.m_elements == null ) i_wnd.m_elements = {};
	for( var p in i_params)
	{
		var elDiv = document.createElement('div');
		i_wnd.appendChild( elDiv);
		elDiv.classList.add('param');

		if( i_params[p].width != null )
		{
			elDiv.style.cssFloat = 'left';
			elDiv.style.width = i_params[p].width;
		}
		else
			elDiv.style.clear = 'both';

		var elLabel = document.createElement('div');
		elDiv.appendChild( elLabel);
		elLabel.classList.add('label');
		if( i_params[p].label )
			elLabel.textContent = i_params[p].label + ':';
		else
		{
			elLabel.textContent = p.replace(/_/g,' ') + ':';
			elLabel.style.textTransform = 'capitalize';
		}
		if( i_params[p].lwidth )
			elLabel.style.width = i_params[p].lwidth;

		if( i_params[p].info )
		{	
			var elInfo = document.createElement('div');
			elDiv.appendChild( elInfo);
			elInfo.classList.add('info');
			elInfo.textContent = i_params[p].info;
			if( i_params[p].iwidth )
				elInfo.style.width = i_params[p].iwidth;
		}

		if( i_params[p].list )
		{
			var elList = document.createElement('div');
			elDiv.appendChild( elList);
			elList.classList.add('list');
			elList.m_params = i_params[p][i_params[p].list];

			var elCtrl = document.createElement('div');
			elList.appendChild( elCtrl);
			elCtrl.classList.add('ctrl');

			var btns = ['insert','append','move_up','move_down','delete'];
			for( var b = 0; b < btns.length; b++ )
			{
				var elBtn = document.createElement('div');
				elCtrl.appendChild( elBtn);
				elBtn.classList.add('button');
				elBtn.textContent = btns[b].replace(/_/g,' ');
				elBtn.m_elList = elList;
				elBtn.m_action = btns[b];
				elBtn.ondblclick = gui_ListAction;
			}

			var elParams = document.createElement('div');
			elList.appendChild( elParams);
			elParams.classList.add('item');

			gui_Create( elParams, i_params[p][i_params[p].list]);
//			i_wnd.m_elements[p] = elValue;
			continue;
		}

		var elValue = document.createElement('div');
		elDiv.appendChild( elValue);
		elValue.classList.add('value');
		elValue.classList.add('editing');
		if( i_params[p].bool != null )
		{
			if( i_params[p].bool)
				elValue.textContent = 'ON';
			else
				elValue.textContent = 'OFF';
			elValue.classList.add('checkbox');
			elValue.onclick = function( e)
			{
				if( e.currentTarget.textContent == 'ON')
					e.currentTarget.textContent = 'OFF';
				else
					e.currentTarget.textContent = 'ON';
			};
		}
		else
		{
			if( i_params[p].disabled )
				elValue.classList.add('disabled');
			else
				elValue.contentEditable = 'true';
		}

		if( i_defaults )
			for( var d = 0; d < i_defaults.length; d++)
				if( i_defaults[d][p] != null )
				{
					if( i_params[p].bool != null )
					{
						if( i_defaults[d][p])
							elValue.textContent = 'ON';
						else
							elValue.textContent = 'OFF';
					}
					else
						elValue.textContent = i_defaults[d][p];
					break;
				}

		i_wnd.m_elements[p] = elValue;
	}
}

function gui_ListAction( i_evt)
{
	var elBtn = i_evt.currentTarget;
	var elList = elBtn.m_elList;
	var action = elBtn.m_action;
console.log( action);

	if( action == 'append')
	{
		var elParams = document.createElement('div');
		elList.appendChild( elParams);
		elParams.classList.add('item');
		gui_Create( elParams, elList.m_params);
	}
}

function gui_GetParams( i_wnd, i_params, o_params)
{
	var params = {};

	for( var p in i_params)
	{
		if( i_params[p].bool != null )
		{
			if( i_wnd.m_elements[p].textContent == 'ON' )
				params[p] = true;
			else
				params[p] = false;
		}
		else
			params[p] = i_wnd.m_elements[p].textContent;

		if( o_params )
			o_params[p] = params[p];
	}

	return params;
}

