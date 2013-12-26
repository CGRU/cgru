
function gui_Create( i_wnd, i_params, i_values)
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
			elList.m_elParams = [];

			var elCtrl = document.createElement('div');
			elList.appendChild( elCtrl);
			elCtrl.classList.add('ctrl');

			var elName = document.createElement('div');
			elCtrl.appendChild( elName);
			elName.classList.add('name');
			elName.textContent = i_params[p].list.replace(/_/g,' ');

			var btns = ['insert','append','move_up','move_down','delete'];
			for( var b = 0; b < btns.length; b++ )
			{
				var elBtn = document.createElement('div');
				elCtrl.appendChild( elBtn);
				elBtn.classList.add('button');
				elBtn.textContent = btns[b].replace(/_/g,' ');
				elBtn.m_elList = elList;
				elBtn.m_action = btns[b];
				elBtn.m_item = i_params[p].list;
				elBtn.ondblclick = gui_ListAction;
			}

			i_wnd.m_elements[p] = elList;

			if( i_values )
			for( var v = 0; v < i_values.length; v++)
			if( i_values[v][p])
			for( var l = 0; l < i_values[v][p].length; l++)
			for( var item in i_values[v][p][l])
				gui_ListAdd( elList, item, i_values[v][p][l][item]);

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

		var values = [];
		if( i_params[p].default )
			values.push( i_params[p].default )
		if( i_values )
			for( var v = 0; v < i_values.length; v++)
				if( i_values[v][p] != null )
					values.push( i_values[v][p]);

		for( var v = 0; v < values.length; v++)
		{
			if( i_params[p].bool != null )
			{
				if( values[v])
					elValue.textContent = 'ON';
				else
					elValue.textContent = 'OFF';
			}
			else
				elValue.textContent = values[v];
		}

		i_wnd.m_elements[p] = elValue;
	}
}

function gui_ListAction( i_evt)
{
	var elBtn = i_evt.currentTarget;
	var elList = elBtn.m_elList;
	var action = elBtn.m_action;
	var item = elBtn.m_item;

	var elParams = null;
	for( var i = 0; i < elList.m_elParams.length; i++)
		if( elList.m_elParams[i].classList.contains('selected'))
		{
			elParams = elList.m_elParams[i];
			break;
		}

	if(( action == 'append') || ( action == 'insert'))
	{
		gui_ListAdd( elList, item, null, action, elParams);
		return;
	}

	if( elList.m_elParams.length == 0 )
	{
		c_Error('List has no items.');
		return;
	}

	if( elParams == null )
	{
		c_Error('Nothing selected.');
		return;
	}

	var index = elList.m_elParams.indexOf( elParams);

	if( action == 'delete')
	{
		if( index >= 0 ) elList.m_elParams.splice( index, 1);			
		elList.removeChild( elParams);
		return;
	}

	if( action == 'move_up')
	{
		if( index <= 0 )
		{
			c_Error('Item already has top position.');
			return;
		}
		index_new = index - 1;
	}
	else if( action == 'move_down')
	{
		if( index >= ( elList.m_elParams.length - 1 ))
		{
			c_Error('Item already has bottom position.');
			return;
		}
		index_new = index + 1;
	}

	var prevParams = null;
	if( index_new != elList.m_elParams.length - 1 )
		prevParams = elList.m_elParams[index_new];

	elList.m_elParams.splice( index, 1);
	elList.removeChild( elParams);

	elList.m_elParams.splice( index_new, 0, elParams);
	elList.insertBefore( elParams, prevParams);
}
function gui_ListGetCurrent( i_elList)
{
	return null
}
function gui_ListAdd( i_elList, i_item, i_values, i_action, i_elParams)
{
	var elParams = document.createElement('div');
	if( i_action == 'insert' )
		i_elList.insertBefore( elParams, i_elParams);
	else
		i_elList.appendChild( elParams);
	elParams.classList.add('item');

	var values = null;
	if( i_values ) values = [i_values];
	gui_Create( elParams, i_elList.m_params, values);

	elParams.m_item = i_item;
	elParams.m_params = i_elList.m_params;
	elParams.m_elList = i_elList;
	i_elList.m_elParams.push( elParams);

	elParams.onclick = gui_ListSelect;
}
function gui_ListSelect( i_evt)
{
	i_evt.stopPropagation();
	var el = i_evt.currentTarget;
	var elParams = el.m_elList.m_elParams;
	for( var i = 0; i < elParams.length; i++)
		elParams[i].classList.remove('selected');
	el.classList.add('selected');
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
		else if( i_params[p].list )
		{
			var elParams = i_wnd.m_elements[p].m_elParams;
			var l_items = [];
			for( var i = 0; i < elParams.length; i++)
			{
				var l_params = {};
				l_params[elParams[i].m_item] = gui_GetParams( elParams[i], elParams[i].m_params);
				l_items.push( l_params);
			}
			params[p] = l_items;
		}
		else
			params[p] = i_wnd.m_elements[p].textContent;

		if( o_params )
			o_params[p] = params[p];
	}

	return params;
}


function gui_CreateChoises( i_args)
{
	var wnd = i_args.wnd;
	var name = i_args.name;
	var def_val = i_args.value;
	var label = i_args.label;
	var keys = i_args.keys;

	if( wnd.m_choises == null )
		wnd.m_choises = {};

	wnd.m_choises[name] = {};
	wnd.m_choises[name].value = def_val;
	wnd.m_choises[name].elements = [];
	
	var elDiv = document.createElement('div');
	elDiv.classList.add('param');
	wnd.appendChild( elDiv);
	elDiv.style.clear = 'both';

	var elLabel = document.createElement('div');
	elDiv.appendChild( elLabel);
	elLabel.textContent = label;
	elLabel.classList.add('label');

	var elChoises = document.createElement('div');
	elDiv.appendChild( elChoises);

	for( var key in keys)
	{
		var el = document.createElement('div');
		elChoises.appendChild( el);
		el.classList.add('choise');
		el.classList.add('button');
		el.textContent = keys[key].name;
		if( keys[key].tooltip ) el.title = keys[key].tooltip;
		else if( keys[key].value ) el.title = keys[key].value;
		el.onclick = gui_ChoiseOnClick;
		var value = key;
		if( keys[key].value ) value = keys[key].value;
		if( value == def_val ) el.classList.add('selected');

		el.m_value = value;
		el.m_wnd = wnd;
		el.m_name = name;

		wnd.m_choises[name].elements.push( el);
	}
}
function gui_ChoiseOnClick( i_evt)
{
	var el = i_evt.currentTarget;
	var wnd = el.m_wnd;
	var name = el.m_name;
	var elements = wnd.m_choises[name].elements;

	for( var i = 0; i < elements.length; i++)
		elements[i].classList.remove('selected');
	el.classList.add('selected');
	wnd.m_choises[name].value = el.m_value;
}

