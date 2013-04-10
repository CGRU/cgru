function Monitor( i_window, i_element, i_type, i_id, i_name)
{
	this.window = i_window;
	this.document = this.window.document;
	this.type = i_type;
	this.elParent = i_element;
	this.name = i_name ? i_name : i_type;
	this.elementsSU = [];

	this.nodeConstructor = null;
	if     ( this.type == 'jobs'   ) this.nodeConstructor = JobNode;
	else if( this.type == 'renders') this.nodeConstructor = RenderNode;
	else if( this.type == 'users'  ) this.nodeConstructor = UserNode;
	else if( this.type == 'tasks'  ) this.nodeConstructor = TaskItem;

	if( this.nodeConstructor.createActions)
		this.nodeConstructor.createActions();

	this.elMonitor = this.document.createElement('div');
	this.elParent.appendChild( this.elMonitor);
	this.elMonitor.classList.add('monitor');
	this.elMonitor.monitor = this;

	this.elList = this.document.createElement('div');
	this.elCtrl = this.document.createElement('div');
	this.elInfo = this.document.createElement('div');
	this.elMonitor.appendChild( this.elList);
	this.elMonitor.appendChild( this.elCtrl);
	this.elMonitor.appendChild( this.elInfo);
	this.elList.classList.add('list');
	this.elCtrl.classList.add('ctrl');
	this.elInfo.classList.add('info');
	this.elementsSU.push( this.elCtrl);
	this.elementsSU.push( this.elInfo);

	this.elList.monitor = this;
	this.elList.oncontextmenu = function(e){ return e.currentTarget.monitor.noneSelected(e);}
	this.elList.onmousedown   = function(e){ return e.currentTarget.monitor.noneSelected(e);}

	this.elCtrlButtons = this.document.createElement('div');
	this.elCtrl.appendChild( this.elCtrlButtons);
	this.elCtrlButtons.style.position = 'absolute';

	var buttons_width = 0;
	if( this.type == 'jobs' || this.type == 'renders' || this.type == 'users')
	{
		this.elCtrlSet = this.document.createElement('div');
		this.elCtrlButtons.appendChild( this.elCtrlSet);
		this.elCtrlSet.classList.add('ctrl_button');
		this.elCtrlSet.textContent = 'SET';
		this.elCtrlSet.monitor = this;
		this.elCtrlSet.onmouseover = function(e){ return e.currentTarget.monitor.onMouseOverSet(e);}

		buttons_width += 50;
	}

	if( this.type == 'renders')
	{
		this.elCtrlCmd = this.document.createElement('div');
		this.elCtrlButtons.appendChild( this.elCtrlCmd);
		this.elCtrlCmd.classList.add('ctrl_button');
		this.elCtrlCmd.textContent = 'CMD';
		this.elCtrlCmd.monitor = this;
		this.elCtrlCmd.onmouseover = function(e){ return e.currentTarget.monitor.onMouseOverSet(e,'cmd');}

		buttons_width += 50;

		if( g_GOD())
		{
			this.elCtrlPow = this.document.createElement('div');
			this.elCtrlButtons.appendChild( this.elCtrlPow);
			this.elCtrlPow.classList.add('ctrl_button');
			this.elCtrlPow.textContent = 'POW';
			this.elCtrlPow.monitor = this;
			this.elCtrlPow.onmouseover = function(e){ return e.currentTarget.monitor.onMouseOverSet(e,'pow');}

			buttons_width += 50;
		}
	}
//	this.elCtrlButtons.style.width = 50+buttons_width+'px';

	this.elCtrlSortFilter = this.document.createElement('div');
	this.elCtrl.appendChild( this.elCtrlSortFilter);
	this.elCtrlSortFilter.style.position = 'absolute';
	this.elCtrlSortFilter.style.left = buttons_width+'px';
	this.elCtrlSortFilter.style.right = '0';
	this.elCtrlSortFilter.style.top = '0';
	this.elCtrlSortFilter.style.bottom = '0';
//this.elCtrlSortFilter.style.border = '1px solid #333';

	this.elCtrlSort = this.document.createElement('div');
	this.elCtrlSortFilter.appendChild( this.elCtrlSort);
	this.elCtrlSort.classList.add('ctrl_sort');

	this.elCtrlSortLabel = this.document.createElement('span');
	this.elCtrlSort.appendChild( this.elCtrlSortLabel);
	this.elCtrlSortLabel.classList.add('label');
	this.elCtrlSortLabel.textContent = 'Sort:';

	this.elCtrlSortParam = this.document.createElement('span');
	this.elCtrlSort.appendChild( this.elCtrlSortParam);
	this.elCtrlSortParam.classList.add('param');
	this.elCtrlSortParam.textContent = 'null';
	this.elCtrlSortParam.monitor = this;
	this.elCtrlSortParam.ondblclick = function(e){return e.currentTarget.monitor.sortDirChanged(e);}
	this.elCtrlSortParam.title = 'Sort Parameter\nContext menu to select.\nDouble click to toggle direction.';
	this.elCtrlSortParam.oncontextmenu = function(e){return e.currentTarget.monitor.sortFilterParmMenu(e,'sort');}

	this.elCtrlFilter = this.document.createElement('div');
	this.elCtrlSortFilter.appendChild( this.elCtrlFilter);
	this.elCtrlFilter.classList.add('ctrl_filter');

	this.elCtrlFilterLabel = this.document.createElement('div');
	this.elCtrlFilter.appendChild( this.elCtrlFilterLabel);
	this.elCtrlFilterLabel.classList.add('label');
	this.elCtrlFilterLabel.textContent = 'Filter:';

	this.elCtrlFilterParam = this.document.createElement('div');
	this.elCtrlFilter.appendChild( this.elCtrlFilterParam);
	this.elCtrlFilterParam.classList.add('param');
	this.elCtrlFilterParam.textContent = 'null';
	this.elCtrlFilterParam.title = 'Filter Parameter\nContext menu to select.\nDouble click to toggle exclude.';
	this.elCtrlFilterParam.monitor = this;
	this.elCtrlFilterParam.oncontextmenu = function(e){return e.currentTarget.monitor.sortFilterParmMenu(e,'filter');}
	this.elCtrlFilterParam.ondblclick = function(e){return e.currentTarget.monitor.filterExcludeChanged(e);}

	this.elCtrlFilterInput = this.document.createElement('div');
	this.elCtrlFilter.appendChild( this.elCtrlFilterInput);
	this.elCtrlFilterInput.classList.add('input');
	this.elCtrlFilterInput.contentEditable = true;
	this.elCtrlFilterInput.monitor = this;
	this.elCtrlFilterInput.onkeyup = function(e){return e.currentTarget.monitor.filterKeyUp(e);}
	this.elCtrlFilterInput.onmouseout = function(e){return e.currentTarget.blur();}

	this.elInfoText = this.document.createElement('div');
	this.elInfoText.classList.add('text');	
	this.elInfoText.textContent = this.type;
	this.elInfo.appendChild( this.elInfoText);
///*
//this.elCtrl.textContent='ctrl';
//this.elList.textContent='list';
//this.elInfo.textContent='info';
//*/
//	this.valid = false;
	for( var i = 0; i < g_recievers.length; i++)
	{
		if( g_recievers[i].name == this.name )
		{
			g_Info('ERROR: Monitor[' + this.name + '] list already exists.');
			return;
		}
	}

	for( var i = 0; i < this.elementsSU.length; i++)
		if( g_GOD())
			this.elementsSU[i].classList.add('su_god');
		else if ( g_VISOR())
			this.elementsSU[i].classList.add('su_visor');

	
	if( this.nodeConstructor.sortVisor && g_VISOR() )
		this.sortParm = this.nodeConstructor.sortVisor;
	else
		this.sortParm = this.nodeConstructor.sort[0];
	for( var i = 0; i < cm_Attrs.length; i++)
		if( cm_Attrs[i][0] == this.sortParm )
			this.elCtrlSortParam.textContent = cm_Attrs[i][1];
	this.sortDirection = false;
	if( this.sortParm == 'order')
		this.sortDirection = true;

	if( this.nodeConstructor.filterVisor && g_VISOR())
		this.filterParm = this.nodeConstructor.filterVisor;
	else
		this.filterParm = this.nodeConstructor.filter[0];
	for( var i = 0; i < cm_Attrs.length; i++)
		if( cm_Attrs[i][0] == this.filterParm )
			this.elCtrlFilterParam.textContent = cm_Attrs[i][1];

	this.items = [];
	g_recievers.push( this);
	g_monitors.push( this);
	this.setWindowTitle();
	g_cur_monitor = this;
	this.menu = null;
	this.cycle = 0;

	if( this.type == 'tasks')
	{
		this.job_id = i_id;
		nw_GetNodes( 'jobs', [this.job_id], 'full');
	}
	else
	{
		g_refreshers.push( this);
		nw_Subscribe( this.type, true);
		nw_GetNodes( this.type);
	}
}

Monitor.prototype.destroy = function()
{
	if( this.menu ) this.menu.destroy();
	if( g_cur_monitor == this ) g_cur_monitor = null;
	cm_ArrayRemove(	g_recievers, this);
	cm_ArrayRemove(	g_refreshers, this);
	cm_ArrayRemove(	g_monitors, this);
	if( this.type == 'tasks')
		nw_Subscribe( this.type, false, [this.job_id]);
	else
		nw_Subscribe( this.type, false);

//	for( var i = 0; i < this.items.length; i++)
//		this.element.removeChild(this.items[i].element);

	this.items = [];

	if( this.elParent && this.elMonitor )
		try
		{
			this.elParent.removeChild( this.elMonitor);
		}
		catch( err)
		{
			g_Error(err.message);
		}

	g_MonitorClosed( this);
//g_Info('Destroying "'+this.name+'"');
}

Monitor.prototype.refresh = function()
{
	if( this.type == 'users' )
	{
		this.max_tasks = 0;
		for( var i = 0; i < this.items.length; i++)
			if( this.items[i].params.running_tasks_num )
				if( this.items[i].params.running_tasks_num > this.max_tasks )
					this.max_tasks = this.items[i].params.running_tasks_num;
	}
	else if(( this.type == 'renders') && ( this.cycle % 5 == 0 ))
	{
		nw_ReqestRendersResources();
	}

	for( var i = 0; i < this.items.length; i++)
		this.items[i].refresh();

	this.cycle++;
}

Monitor.prototype.processMsg = function( obj)
{
//this.info('New message '+this.cycle+':'+JSON.stringify(obj));
	if( obj.events != null )
	{
		if( this.type == 'tasks')
		{
			if( obj.events.tasks_progress != null )
			{
				this.tasksProgress( obj.events.tasks_progress );
			}
			if( obj.events.block_ids != null )
			{
				this.getBlocks( obj.events.block_ids );
			}
			return;
		}
		this.delNodes( eval('obj.events.'+this.type+'_del'));
		ids = cm_IdsMerge( eval('obj.events.'+this.type+'_change'), eval('obj.events.'+this.type+'_add'));
		if( ids.length > 0 )
			nw_GetNodes( this.type, ids);

		if(( this.type == 'jobs') && obj.events.jobs_order )
			if( g_uid == obj.events.jobs_order.uids[0] )
				this.sortByIds( obj.events.jobs_order.jids[0] );

		return;
	}

	if( this.type == 'tasks')
	{
		if( this.job == null )
		{
			if( obj.jobs != null )
				if( obj.jobs.length == 1 )
					this.jobConstruct( obj.jobs[0]);
		}
		else if( obj.job_progress != null )
		{
				if( obj.job_progress.id == this.job_id )
					this.jobProgress( obj.job_progress.progress);
		}
		else if( obj.blocks != null )
		{
			this.updateBlocks( obj.blocks);
		}
		return;
	}

	var nodes = null;
	nodes = eval('obj.' + this.type);
	if( nodes == null ) return;

	var new_ids = [];
	var updated = [];

	for( var j = 0; j < nodes.length; j++)
	{
		founded = false;
		for( var i = 0; i < this.items.length; i++)
		{
			if( this.items[i].params.id == nodes[j].id )
			{
				this.items[i].update( nodes[j]);
				updated.push( this.items[i]);
				this.filterItem( this.items[i]);
				founded = true;
				break;
			}
		}
		if( founded == false )
			new_ids.push(j);
	}

	if( this.sortParm != 'order')
		for( var i = 0; i < updated.length; i++)
			this.sortItem( updated[i]);

	var new_nodes = [];
	for( var i = 0; i < new_ids.length; i++)
		new_nodes.push( this.createNode( nodes[new_ids[i]]));

	if(( this.type == 'jobs' ) && ( this.sortParm == 'order'))
		if( new_ids.length || updated.length )
			nw_GetNodes('users',[g_uid],'jobs_order');

	if( false == this.hasSelection())
		if( new_nodes.length )
			this.cur_item = new_nodes[new_nodes.length-1];
		else if( updated.length )
			this.cur_item = updated[updated.length-1];

	this.setWindowTitle();
//this.info( 'c' + this.cycle + ': nodes processed: ' + nodes.length + ' new:' + new_ids.length + ' up:' + updated.length);
}

Monitor.prototype.setWindowTitle = function()
{
	var title = null;
	if( this.type == 'jobs' )
	{
		title = 'AJ';
		var tasks = 0;
		for( var i = 0; i < this.items.length; i++)
			if( this.items[i].state.RUN )
				tasks += this.items[i].running_tasks;
		if( tasks > 0 )
			title += ' '+tasks;
		if( this.cur_item )
		{
			if( this.cur_item.state.RUN )
				title += ' '+this.cur_item.percentage+'%';
			title += this.cur_item.params.state;
		}
	}
	else if( this.type == 'renders' )
	{
		title = 'AR';
		var tasks = 0;
		for( var i = 0; i < this.items.length; i++)
			if( this.items[i].state.RUN )
				tasks += this.items[i].params.tasks.length;
		if( tasks > 0 )
			title += ' '+tasks;
	}
	else if( this.type == 'users' )
	{
		title = 'AU';
		var tasks = 0;
		for( var i = 0; i < this.items.length; i++)
			if( this.items[i].params.running_tasks_num )
				tasks += this.items[i].params.running_tasks_num;
		if( tasks > 0 )
			title += ' '+tasks;
	}
	else if( this.type == 'tasks' && this.job )
	{
		var count = 0;
		var percent = 0;
		var run = 0;
		var error = 0;
		for( var i = 0; i < this.items.length; i++)
			if( this.items[i].task_num )
			{
				if( this.items[i].state.DON )
					percent += 100;
				else if( this.items[i].state.RUN )
				{
					run++;
					if( this.items[i].percent )
						percent += this.items[i].percent;
				}
				else if( this.items[i].state.ERR )
					error++;
				count++;
			}
		if( count )
		{
			title = '';
			if( error )
				title += 'E'+error+' ';
			if( run )
				title +='R'+run+' ';
			percent = Math.round( percent / count );
			title += percent+'%';
			title += ' '+this.job.name;
		}
	}

	if( title )
		this.document.title = title;
}

Monitor.prototype.delNodes = function( i_ids)
{
	if( i_ids == null ) return;
	if( i_ids.length == null ) return;
	if( i_ids.length == 0 ) return;
	if( this.items.length == 0 ) return;

	for( var d = 0; d < i_ids.length; d++ )
		for( var i = 0; i < this.items.length; i++)
			if( this.items[i].params.id == i_ids[d] )
			{
				this.elList.removeChild( this.items[i].element);
				this.items.splice(i,1);
				break;
			}
}

Monitor.prototype.createNode = function( i_obj)
{
	var node = new this.nodeConstructor();
	this.createItem( node, i_obj, false);
	this.filterItem( node);
	this.addItemSorted( node);
	return node;
}
Monitor.prototype.addItemSorted = function( i_item)
{
	var index = this.items.length;
	var nodeBefore = null;

//window.console.log('addItemSorted='+i_item.params.name+' (len='+this.items.length+')');
	if( this.sortParm )
		for( var i = 0; i < this.items.length; i++)
		{
//var log = i_item.params.name+'['+this.sortParm+']='+i_item.params[this.sortParm]+' <> '+this.items[i].params[this.sortParm]+'=['+this.sortParm+']'+this.items[i].params.name;
			if( cm_CompareItems( i_item, this.items[i], this.sortParm, false == this.sortDirection ))
			{
//log += ' TRUE index='+index;window.console.log(log);
				index = i;
				break;
			}
//else log += ' FALSE'; window.console.log(log);
		}

	if( index < this.items.length )
		nodeBefore = this.items[index].element;

//if( index < this.items.length ) g_Info('Monitor.prototype.addItemSorted = '+i_item.params.name+'['+index+']');
	this.items.splice( index, 0, i_item);
	this.elList.insertBefore( i_item.element, nodeBefore);
}

Monitor.prototype.createItem = function( i_item, i_obj, i_appendChild)
{
	i_item.element = this.document.createElement('div');
	i_item.element.className = 'item';
	if( i_appendChild == true )
		this.elList.appendChild( i_item.element);

	i_item.params = i_obj;
	i_item.monitor = this;
	i_item.element.monitor = this;
	i_item.element.item = i_item;
	i_item.element.onmousedown = function(e){ if(e.button==0) return e.currentTarget.monitor.onMouseDown(e,e.currentTarget);}
	i_item.element.onmouseover = function(e){ if(e.button==0) return e.currentTarget.monitor.onMouseOver(e,e.currentTarget);}
	i_item.element.ondblclick = function(e){ return e.currentTarget.item.onDoubleClick();}
	i_item.element.oncontextmenu = function(e){ return e.currentTarget.monitor.onContextMenu(e,e.currentTarget);}

	i_item.init();
	i_item.update();
}

Monitor.prototype.info = function( i_str)
{
	this.elInfoText.textContent = i_str;
}

Monitor.prototype.error = function( i_str)
{
	this.elInfoText.textContent = 'Error: ' + i_str;
}

Monitor.prototype.onMouseDown = function( i_evt, i_el)
{
	i_evt.stopPropagation();
	g_cur_monitor = this;
	if( this.menu )
	{
		this.menu.destroy();
		return;
	}

	if( false == i_evt.ctrlKey )
		this.selectAll( false);
	if( i_evt.shiftKey && this.cur_item )
	{
		var i = this.items.indexOf( this.cur_item);
		var ci = this.items.indexOf( i_el.item);
		if(( i != ci ) && ( i != -1) && ( ci != -1))
		{
			this.elSetSelected( i_el, true);
			var d = 1;
			if( i > ci ) d = -1;
			while( i != ci )
			{
				this.elSetSelected( this.items[i].element, true);
				i += d;
			}
			return;
		}
	}
		
	this.elSelectToggle( i_el);
}

Monitor.prototype.onMouseOver = function( i_evt, i_el)
{
	if( i_evt.buttons != 1 ) return;
	this.elSetSelected( i_evt.currentTarget, i_evt.ctrlKey == false);
}

Monitor.prototype.elSetSelected = function( el, on)
{
	if( on )
	{
		this.cur_item = el.item;
		this.info( this.cur_item.params.name);
		if( el.selected ) return;
		el.selected = true;
		if( false == el.classList.contains('selected'))
			el.classList.add('selected');
		if( this.type == 'jobs' )
			this.setWindowTitle();
	}
	else
	{
		if( false == el.selected ) return;
		el.selected = false;
		el.classList.remove('selected');
	}
}

Monitor.prototype.elSelectToggle = function( el)
{
	if( !el ) return;
	if( el.selected )
		this.elSetSelected( el, false);
	else
		this.elSetSelected( el, true);
}

Monitor.prototype.selectAll = function( on)
{
	for( var i = 0; i < this.items.length; i++)
		this.elSetSelected( this.items[i].element, on);
}

Monitor.prototype.selectNext = function( i_evt, previous)
{
	if( this.items.length == 0 ) return;

	var next_index = 0;
	if( this.cur_item )
	{
		var cur_index = 0;
		for( var i = 0; i < this.items.length; i++)
			if( this.cur_item == this.items[i])
			{
				cur_index = i;
				break;
			}
		var next_index = cur_index+1;
		if( previous )
			next_index = cur_index-1;
	}

	if( next_index < 0 ) return;
	if( next_index >= this.items.length ) return;
	if( next_index == cur_index ) return;

	this.cur_item = this.items[next_index]; 
	if( false == i_evt.shiftKey )
		this.selectAll( false);
	this.elSetSelected( this.cur_item.element, true);
}

Monitor.prototype.onContextMenu = function( i_evt, i_el)
{
	i_evt.stopPropagation();
	g_cur_monitor = this;
	if( i_el.selected != true )
		this.selectAll( false);
	this.elSetSelected( i_el, true);

	var menu = this.createMenu( i_evt, 'context');
	if( i_el.item.onContextMenu )
		i_el.item.onContextMenu( menu);
	else
	{
		var actions = i_el.item.constructor.actions;
		for( var i = 0; i < actions.length; i++)
			if( actions[i][0] == 'context' )
				this.addMenuItem( menu, actions[i]);
	}
	menu.show();

	return false;
}
Monitor.prototype.onMenuDestroy = function() { this.menu = null;}

Monitor.prototype.addMenuItem = function( i_menu, i_action)
{
	var name = i_action[1];
	if( name == null )
	{
		i_menu.addItem();
		return;
	}
	var receiver = this.cur_item;
	var handle = i_action[3];
	var title = i_action[4];
	var permission = i_action[5];

	if( permission )
	{
		if( g_VISOR())
		{
			if( permission == 'user') return;
			if( g_GOD())
			{
				if( false == (( permission == 'visor') || ( permission == 'god')))
					return;
			}
			else if( permission != 'visor')
				return;
		}
		else if( permission != 'user')
			return;
	}

	if( i_action[0] == 'cmd')
	{
		var cmds = [];
		for( var i = 0; i < this.items.length; i++)
			if( this.items[i].element.selected == true )
			{
				cmd = handle;
				cmd = cmd.replace(/@ARG@/g, this.items[i].params.name);
				if( this.items[i].params.address.ip )
					cmd = cmd.replace(/@IP@/g, this.items[i].params.address.ip);
				cmds.push( cmd);
			}
		i_menu.addItem( name, 'cmdexec', cmds, title);
		return;
	}
	if( receiver[handle] == null ) receiver = this;
	if( receiver[handle] )
		i_menu.addItem( name, receiver, handle, title);
	else
		i_menu.addItem('invalid', 'invalid', 'invalid', 'invalid '+name, false);
}

Monitor.prototype.onMouseOverSet = function( i_evt, i_name)
{
	if( this.cur_item == null ) return;
	if( this.hasSelection() == false ) return;
	if( i_name == null ) i_name = 'set';

	var menu = this.createMenu( i_evt, 'set');
	var actions = this.cur_item.constructor.actions;
	for( var i = 0; i < actions.length; i++)
		if( actions[i][0] == i_name )
			this.addMenuItem( menu, actions[i]);
	menu.show();
}
Monitor.prototype.menuHandleParam = function( i_name)
{
	var ptype = null;
	var actions = this.cur_item.constructor.actions;
	
	for( var i = 0; i < actions.length; i++)
		if( i_name == actions[i][1])
		{
			var parameter = i_name;
			if( actions[i][6] ) parameter = actions[i][6];
			this.setParameter( parameter, actions[i][2]);
			return;
		}
}
Monitor.prototype.menuHandleDialog = function( i_name)
{
	var ptype = null;
	var parameter = i_name;
	var actions = this.cur_item.constructor.actions;
	var reciever = this;
	var handle = 'setParameter';
	var value = this.cur_item.params[parameter];

	// Search actions by name to get other values
	for( var i = 0; i < actions.length; i++)
	{
		if( i_name == actions[i][1])
		{
			ptype = actions[i][2];
			// Parameter can be overriden:
			if( actions[i][6] ) parameter = actions[i][6];
		}
	}

	new cgru_Dialog( this.window, reciever, handle, parameter, ptype, value, this.name+'_parameter');
}
Monitor.prototype.setParameter = function( i_parameter, i_value)
{
	var params = {};
	params[i_parameter] = i_value;
//this.info('params.'+i_parameter+'="'+i_value+'";');
	this.action( null, params);
}
Monitor.prototype.menuHandleOperation = function( i_name)
{
//this.info('Operation = ' + i_name);
	var operation = {};
	operation.type = i_name;
	this.action( operation, null);
}
Monitor.prototype.menuHandleGet = function( i_name)
{
//this.info('Get = ' + i_name);
	nw_GetNodes( this.type, [this.cur_item.params.id], i_name);
}

Monitor.prototype.action = function( i_operation, i_params)
{
	nw_Action( this.type, this.getSelectedIds(), i_operation, i_params);
}

Monitor.prototype.getSelectedIds = function()
{
	var ids = [];
	for( var i = 0; i < this.items.length; i++)
		if( this.items[i].element.selected == true )
			ids.push( this.items[i].params.id);
	return ids;
}

Monitor.prototype.hasSelection = function()
{
	for( var i = 0; i < this.items.length; i++)
		if( this.items[i].element.selected == true )
			return true;
	return false;
}

Monitor.prototype.noneSelected = function( i_evt)
{//return false;
	i_evt.stopPropagation();
	if( i_evt.ctrlKey  ) return false;
	if( i_evt.shiftKey ) return false;
//	this.selectAll( false);
	if( this.menu)
		this.menu.destroy();
	return false;
}

Monitor.prototype.sortItems = function()
{
//this.info('Sort '+(this.sortDirection ? 'ascending':'descending')+': '+this.sortParm);
	this.elCtrlSort.classList.add('sorting');
	if( this.type == 'tasks' )
	{
		var pos = 0;
		for( var b = 0; b < this.job.blocks.length; b++)
		{
			var tasks_num = this.job.blocks[b].tasks_num;
			if( tasks_num > 1 )
				this.sortItemsSet( pos + 1, pos + tasks_num);
//g_Info('st: '+pos+'-'+(pos+tasks_num));
			pos += tasks_num + 1;
		}
	}
	else
		this.sortItemsSet( 0, this.items.length-1);
	this.elCtrlSort.classList.remove('sorting');
}
Monitor.prototype.sortItemsSet = function( i_a, i_b)
{
	for( var i = i_b; i > i_a; i--)
		for( var j = i_a; j < i; j++)
		{
			var itemA = this.items[j];
			var itemB = this.items[j+1];
			if( cm_CompareItems( itemA, itemB, this.sortParm, this.sortDirection ))
			{
//window.console.log( itemA.params.name+' '+itemA.params[this.sortParm] + ' < ' + itemB.params[this.sortParm] +' '+itemB.params.name+ ' TRUE');
				this.elList.removeChild( itemB.element);
				this.elList.insertBefore( itemB.element, itemA.element);
				this.items[j] = itemB;
				this.items[j+1] = itemA;
			}
//window.console.log( itemA.params.name+' '+itemA.params[this.sortParm] + ' < ' + itemB.params[this.sortParm] +' '+itemB.params.name+ ' FALSE');
		}
}
Monitor.prototype.sortItem = function( i_item)
{
	var i = this.items.indexOf( i_item);
	var needsort = false;
	if( i > 0 )
		if( cm_CompareItems( i_item, this.items[i-1], this.sortParm, this.sortDirection ) == false )
			needsort = true;
	if( i < (this.items.length-1))
		if( cm_CompareItems( i_item, this.items[i+1], this.sortParm, this.sortDirection ) == true )
			needsort = true;

	if( false == needsort )
		return;

	this.elList.removeChild( i_item.element);
	this.items.splice( i, 1);

	this.addItemSorted( i_item);
}
Monitor.prototype.sortByIds = function( i_ids)
{
	if( i_ids.length != this.items.length )
	{
		g_Error('Order Ids and items size mismatch.');
		return;
	}

	var items = this.items.slice();

	for( var i = 0; i < this.items.length; i++)
		this.elList.removeChild( this.items[i].element );
	this.items = [];

	for( var i = 0; i < i_ids.length; i++)
	{
		for( var j = 0; j < items.length; j++)
		{
			if( i_ids[i] == items[j].params.id )
			{
				if( items[j].params['order'] )
					items[j].params['order'] = i;
				this.items.push( items[j]);
				this.elList.appendChild( items[j].element);
				break;
			}
		}
	}
}

Monitor.prototype.createMenu = function( i_evt, i_name)
{
	if( this.menu ) this.menu.destroy();
	var menu = new cgru_Menu( this.document, this.document.body, i_evt, this, this.type+'_'+i_name, 'onMenuDestroy');
	this.menu = menu;
	return menu;
}



// --------------- Sorting: -------------------//
Monitor.prototype.sortDirChanged = function( i_evt)
{
	if( this.sortParm == 'order') return;
	if( this.sortDirection ) this.sortDirection = false;
	else this.sortDirection = true;
	this.sortItems();
	return false;
}
Monitor.prototype.sortFilterParmMenu = function( i_evt, i_type)
{
	var menu = this.createMenu( i_evt, i_type);
	for( var i = 0; i < this.nodeConstructor[i_type].length; i++)
		for( var j = 0; j < cm_Attrs.length; j++)
			if( this.nodeConstructor[i_type][i] == cm_Attrs[j][0] )
				menu.addItem( cm_Attrs[j][0], this, i_type+'ParmChanged', cm_Attrs[j][2]);
	menu.show();
	i_evt.stopPropagation();
	return false;
}
Monitor.prototype.sortParmChanged = function( i_name)
{
	this.elCtrlSortParam.textContent = i_name;
	for( var i = 0; i < cm_Attrs.length; i++)
		if( cm_Attrs[i][0] == i_name )
			this.elCtrlSortParam.textContent = cm_Attrs[i][1];
	this.sortParm = i_name;
	if( this.sortParm == 'order') this.sortDirection = true;
	this.sortItems();
}
// --------------- Filtering: -------------------//
Monitor.prototype.filterExcludeChanged = function( i_evt)
{
	if( this.filterExclude ) this.filterExclude = false;
	else this.filterExclude = true;
	this.filterItems();
}
Monitor.prototype.filterParmChanged = function( i_name)
{
	this.elCtrlFilterParam.textContent = i_name;
	for( var i = 0; i < cm_Attrs.length; i++)
		if( cm_Attrs[i][0] == i_name )
			this.elCtrlFilterParam.textContent = cm_Attrs[i][1];
	this.filterParm = i_name;
	this.info('Filter'+(this.filterExclude?' E':'')+': '+i_name);
	this.filterItems();
}
Monitor.prototype.filterKeyUp = function( i_evt)
{
	if( i_evt.keyCode == 13 || i_evt.keyCode == 27 ) /* ENTER or ESC */
	{
		i_evt.currentTarget.blur();
		i_evt.stopPropagation();
	}
	this.filterExpr = null;
	var expr = this.elCtrlFilterInput.textContent;
	if( expr && expr.length )
	{
		this.info('Filter: '+expr);
		try { this.filterExpr = new RegExp( expr);}
		catch( err ) { this.filterExpr = null; this.error( err.message);}
	}
	this.filterItems();
}
Monitor.prototype.filterItem = function( i_item)
{
//g_Info('filtering "'+i_item.params.name+'" p"'+this.filterParm+'"');
	var hide = false;
	if( i_item.params.hidden && ( i_item.params.hidden == true ))
		hide = true;
	else if( this.filterExpr && this.filterParm && i_item.params[this.filterParm] )
	{
		if( false == this.filterExpr.test( i_item.params[this.filterParm]))
			hide = ( this.filterExclude ? false : true );
		else
			hide = ( this.filterExclude ? true : false );
	}
	if( hide )
		i_item.element.style.display = 'none';
	else
		i_item.element.style.display = 'block';
}
Monitor.prototype.filterItems = function()
{
	if( this.filterExpr && this.filterParm )
	{
		this.elCtrlFilter.classList.add('filtering');
	}
	else
	{
		this.elCtrlFilter.classList.remove('filtering');
	}
	for( var i = 0; i < this.items.length; i++)
		this.filterItem( this.items[i]);
}



// --------------- Job Tasks specific: -------------------//
Monitor.prototype.jobConstruct = function( job)
{
	this.job = job;
	this.blocks = [];
	for( var b = 0; b < this.job.blocks.length; b++)
	{
		var block = new BlockItem(b);
		this.createItem( block, this.job.blocks[b], true);
		this.items.push( block);
		this.blocks.push( block);
		block.tasks = [];
		for( var t = 0; t < this.job.blocks[b].tasks_num; t++)
		{
			var task = new TaskItem( job, block, t);
			this.createItem( task, null, true);
			this.items.push( task);
			block.tasks.push( task);
		}
	}

	nw_GetNodes( 'jobs', [this.job_id], 'progress');
	nw_Subscribe( this.type, true, [this.job_id]);
}

Monitor.prototype.jobProgress = function( progress)
{
	if( this.blocks.length != progress.length )
	{
		g_Error('Job progress bocks length mismatch: job_id='
			+ this.job_id + ' ' + this.blocks.length + '!=' + progress.length);
		return;
	}

	for( var b = 0; b < this.blocks.length; b++)
	{
		if( this.blocks[b].tasks.length != progress[b].length )
		{
			g_Error('Job progress tasks length mismatch: job_id='
				+ this.job_id + ' block=' + b + ' ' + this.blocks[b].tasks.length + '!=' + progress[b].length);
			return;
		}

		for( var t = 0; t < this.blocks[b].tasks.length; t++)
		{
			this.blocks[b].tasks[t].updateProgress( progress[b][t]);
		}
	}
	this.setWindowTitle();
}

Monitor.prototype.tasksProgress = function( tasks_progress)
{
//g_Info('Monitor.prototype.tasksProgress = function( tasks_progress)');
	var j = -1;
	
	for( var i = 0; i < tasks_progress.length; i++)
	{
//g_Info(' jid='+tasks_progress[i].job_id+' this='+this.job_id);
		if( tasks_progress[i].job_id == this.job_id)
		{
			j = i;
			break;
		}
	}

	if( j == -1 ) return;

	for( var i = 0; i < tasks_progress[j].progress.length; i++)
	{
		var b = tasks_progress[j].blocks[i];
		var t = tasks_progress[j].tasks[i];
		var p = tasks_progress[j].progress[i];
		this.blocks[b].tasks[t].updateProgress( p);
	}
	this.setWindowTitle();
}

Monitor.prototype.getBlocks = function( i_block_ids)
{
	blocks = [];
	modes = [];

//var test = 'block ids:'
//this.document.getElementById('test').textContent = test;

	for( var i = 0; i < i_block_ids.job_id.length; i++)
	{
		if( this.job_id != i_block_ids.job_id[i]) continue;
//test += ' n' + i_block_ids.block_num[i];
//test += ' m' + i_block_ids.mode[i];
		blocks.push( i_block_ids.block_num[i]);
		modes.push( i_block_ids.mode[i]);
	}

//this.document.getElementById('test').textContent = test;

	if( blocks.length == 0 ) return;

	nw_GetBlocks( this.job_id, blocks, modes);
}

Monitor.prototype.updateBlocks = function( i_blocks)
{
	for( var i = 0; i < i_blocks.length; i++)
	{
		if( i_blocks[i].job_id != this.job_id ) continue;

		for( var b = 0; b < this.blocks.length; b++)
		{
			if( i_blocks[i].block_num != b ) continue;

//			this.blocks[b].getData( i_blocks[i]);
			this.blocks[b].params = i_blocks[i];
			this.blocks[b].update();
			break;
		}
	}
}
