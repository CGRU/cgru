
function gui_Create( i_wnd, i_params, i_values)
{
	i_wnd.classList.add('gui');
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
		if( i_params[p].tooltip )
			elLabel.title = i_params[p].tooltip;
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
		if( i_params[p].type && ( i_params[p].type == 'bool'))
		{
			if( i_params[p].default )
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

		if( i_params[p].pulldown )
			gui_PullDownCreate({"elParent":elDiv,"elValue":elValue,"menu":i_params[p].pulldown});

		var values = [];
		if( i_params[p].default )
			values.push( i_params[p].default )
		if( i_values )
			for( var v = 0; v < i_values.length; v++)
				if( i_values[v] && ( i_values[v][p] != null ))
					values.push( i_values[v][p]);

		for( var v = 0; v < values.length; v++)
		{
			if( i_params[p].type == 'bool' )
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

function gui_PullDownCreate( i_args)
{
	i_args.elParent.classList.add('pulldown');

	var elBtn = document.createElement('div');
	i_args.elParent.insertBefore( elBtn, i_args.elValue);
	elBtn.classList.add('pulldown_btn');
	elBtn.textContent = '[<>]';
	elBtn.m_args = i_args;

	elBtn.onclick = function( i_evt)
	{
		var args = i_evt.currentTarget.m_args;
		if( args.elDiv )
		{
			args.elParent.removeChild( args.elDiv);
			args.elDiv = null;
			return;
		}

		args.elDiv = document.createElement('div');
		args.elParent.appendChild( args.elDiv);
		args.elDiv.classList.add('pulldown_menu');

		for( var i = 0; i < args.menu.length; i++ )
		{
			var el = document.createElement('div');
			args.elDiv.appendChild( el);
			el.classList.add('pulldown_item');
			el.textContent = args.menu[i];
			el.m_args = args;
			el.onclick = gui_PullDownClicked;
		}
	}
}
function gui_PullDownClicked( i_evt)
{
	var args = i_evt.currentTarget.m_args;
	args.elValue.textContent = i_evt.currentTarget.textContent;
	if( args.elDiv )
		args.elParent.removeChild( args.elDiv);
	args.elDiv = null;
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
		if( i_wnd.m_elements[p] == null )
			continue;

		if( i_params[p].list )
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
		else if( i_params[p].type )
		{
			if( i_params[p].type == 'bool')
			{
				if( i_wnd.m_elements[p].textContent == 'ON' )
					params[p] = true;
				else
					params[p] = false;
			}
			else if( i_params[p].type == 'int')
			{
				var str = i_wnd.m_elements[p].textContent;
				var num = parseInt(str);
				if( isNaN(num))
					c_Error('Invalid number: ' + str);
				else
					params[p] = num;
			}
		}
		else
		{
			params[p] = i_wnd.m_elements[p].textContent;
		}

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
		if( keys[key].disabled ) continue;

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

function gui_CreateTabs( i_args)
{
	var o_elTabs = [];

	i_args.elParent.classList.add('gui');
	i_args.elParent.classList.add('tab');

	var elLabels = [];
	for( var tab in i_args.tabs)
	{
		var elLabel = document.createElement('div');
		i_args.elParent.appendChild( elLabel);
		elLabel.classList.add('tablabel');
		if( i_args.tabs[tab].label )
			elLabel.textContent = i_args.tabs[tab].label;
		else
		{
			elLabel.textContent = tab.replace(/_/g,' ');
			elLabel.style.textTransform = 'capitalize';
		}
		if( i_args.tabs[tab].tooltip )
			elLabel.title = i_args.tabs[tab].tooltip;

		elLabel.m_tab = tab;
		elLabel.onclick = function(e){
			var el = e.currentTarget;
			for( var i = 0; i < el.m_elLabels.length; i++ )
			{
				el.m_elLabels[i].classList.remove('active');
				el.m_elLabels[i].m_elTab.style.display = 'none';
			}
			el.m_elTab.style.display = 'block';
			el.classList.add('active');
			if( i_args.name )
				localStorage[i_args.name] = el.m_tab;
		}

		elLabels.push( elLabel);
	}

	var active_index = 0;
	for( var i = 0; i < elLabels.length; i++)
	{
		var tab = elLabels[i].m_tab;
		if( i_args.name && localStorage[i_args.name] )
			if( localStorage[i_args.name] == tab )
				active_index = i;

		var elTab = document.createElement('div');
		i_args.elParent.appendChild( elTab);
		elTab.classList.add('tabpanel');
		elTab.style.display = 'none';

		elLabels[i].m_elTab = elTab;
		elLabels[i].m_elLabels = elLabels;
		o_elTabs[tab] = elTab;
//elTab.textContent = 'Panel for ' + tab;
	}

	elLabels[active_index].classList.add('active');
	o_elTabs[elLabels[active_index].m_tab].style.display = 'block';

	return o_elTabs;
}

