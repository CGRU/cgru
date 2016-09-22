function Monitor( i_args)
{
	this.window = i_args.wnd;
	this.type = i_args.type;
	this.elParent = i_args.elParent;
	this.name = i_args.name ? i_args.name : i_args.type;

	this.elementsSU = [];
	this.document = this.window.document;

	this.nodeConstructor = null;
	if     ( this.type == 'jobs'    ) this.nodeConstructor = JobNode;
	else if( this.type == 'renders' ) this.nodeConstructor = RenderNode;
	else if( this.type == 'users'   ) this.nodeConstructor = UserNode;
	else if( this.type == 'tasks'   ) this.nodeConstructor = TaskItem;
	else if( this.type == 'monitors') this.nodeConstructor = MonitorNode;

	if( this.nodeConstructor.createActions)
		this.nodeConstructor.createActions();

	this.elMonitor = this.document.createElement('div');
	this.elParent.appendChild( this.elMonitor);
	this.elMonitor.classList.add('monitor');
	this.elMonitor.classList.add( this.type);
	this.elMonitor.monitor = this;

	this.elView = this.document.createElement('div');
	this.elView.classList.add('view');
	this.elMonitor.appendChild( this.elView);

	this.elPanelL = this.document.createElement('div');
	this.elMonitor.appendChild( this.elPanelL);
	this.elPanelL.classList.add('panel');
	this.elPanelL.classList.add('left');

	this.elPanelR = this.document.createElement('div');
	this.elMonitor.appendChild( this.elPanelR);
	this.elPanelR.classList.add('panel');
	this.elPanelR.classList.add('right');
	this.elPanelR.classList.add('text_selectable');

	this.elList = this.document.createElement('div');
	this.elCtrl = this.document.createElement('div');
	this.elInfo = this.document.createElement('div');
	this.elView.appendChild( this.elList);
	this.elView.appendChild( this.elCtrl);
	this.elView.appendChild( this.elInfo);
	this.elList.classList.add('list');
	this.elCtrl.classList.add('ctrl');
	this.elInfo.classList.add('info');
	this.elementsSU.push( this.elCtrl);
	this.elementsSU.push( this.elInfo);

	this.elList.monitor = this;
	this.elList.oncontextmenu = function(e){ return e.currentTarget.monitor.noneSelected(e);}
	this.elList.onmousedown   = function(e){ return e.currentTarget.monitor.noneSelected(e);}

	this.view_opts = {};
	if( this.nodeConstructor.view_opts )
	{
		for( var opt in this.nodeConstructor.view_opts )
		{
			var view_opt = this.nodeConstructor.view_opts[opt];

			// Get value from a browser storage or use default:
			if( localStorage[opt] != null )
			{
				if( view_opt.type == 'num' )
					this.view_opts[opt] = parseInt( localStorage[opt]);
				else
					this.view_opts[opt] = localStorage[opt];
			}
			else
				this.view_opts[opt] = view_opt.default;
		}

		this.createCtrlBtn({'name':'view_opts','label':'VIEW','tooltip':'View options.',
			'sub_menu':this.nodeConstructor.view_opts,'handle':'mh_Opt','always_active':true});
	}

	if( this.type != 'tasks' )
		this.createCtrlBtn({"name":'log',"label":'LOG',"tooltip":'Show node log.',"handle":'mh_Get'});

	var el = document.createElement('div');
	this.elPanelR.appendChild( el);
	this.elPanelR.m_elName = el;
	el.classList.add('name');

	if( this.nodeConstructor.createPanels )
		this.nodeConstructor.createPanels( this);

	// Parameters section:
	var el = document.createElement('div');
	this.elPanelR.appendChild( el);
	this.elPanelR.m_elParams = el;
	this.elPanelR.m_elParams.m_elPMap = {};
	el.classList.add('section');
	// Show raw JSON onject:
	this.createCtrlBtn({"name":'obj',"label":'OBJ',"tooltip":'Show object.',
		"handle":'showObject','elParent':this.elPanelR.m_elParams,'always_active':true});
	// Label:
	var el = document.createElement('div');
	this.elPanelR.m_elParams.appendChild( el);
	el.textContent = 'Parameters';
	el.classList.add('caption');
	el.title = 'Click to edit all paramters.';
	el.m_elParams = this.elPanelR.m_elParams;
	el.onclick = function(e){
		var el = e.currentTarget;
		if( el.m_elParams.classList.contains('active') != true ) return false;
		if( el.m_elParams.m_all_shown == true ) return;
		el.m_elParams.m_all_shown = true;
		var elParams = el.m_elParams.m_elPMap;
		for( var p in elParams )
			elParams[p].style.display = 'block';
		return false;
	}
	el.oncontextmenu = el.onclick;

	if( this.nodeConstructor.params )
	for( var p in this.nodeConstructor.params )
	{
		var param = this.nodeConstructor.params[p];
		if( false == cm_CheckPermissions( param.permissions )) continue;

		var elDiv = document.createElement('div');
		this.elPanelR.m_elParams.appendChild( elDiv);
		elDiv.classList.add('param');
		elDiv.style.display = 'none';

		var elLabel = document.createElement('div');
		elDiv.appendChild( elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = param.label;

		var elValue = document.createElement('div');
		elDiv.appendChild( elValue);
		elValue.classList.add('value');
		elDiv.m_elValue = elValue;

		this.elPanelR.m_elParams.m_elPMap[p] = elDiv;

		var el = elDiv;
		el.title = 'Double click to edit.'
		el.monitor = this;
		el.name = p;
		el.param = param;
		el.ondblclick = function(e){
			var el = e.currentTarget;
			el.monitor.mh_Dialog({'name':el.name,'type':el.param.type});
		}
	}

	// Info section:
	var el = document.createElement('div');
	this.elPanelR.appendChild( el);
	this.elPanelR.m_elInfo = el;
	el.classList.add('section');
	// Label:
	var el = document.createElement('div');
	this.elPanelR.m_elInfo.appendChild( el);
	el.textContent = 'Info';
	el.classList.add('caption');
	el.title = 'Node information.';
	// Body:
	var el = document.createElement('div');
	this.elPanelR.m_elInfo.appendChild(el);
	this.elPanelR.m_elInfo.m_elBody = el;


	// Sort&Filter:
	this.elCtrlSort = this.document.createElement('div');
	this.elCtrl.appendChild( this.elCtrlSort);
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
	this.elCtrl.appendChild( this.elCtrlFilter);
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
	this.elCtrlFilterInput.classList.add('text_selectable');
	this.elCtrlFilterInput.monitor = this;
	this.elCtrlFilterInput.onkeyup = function(e){return e.currentTarget.monitor.filterKeyUp(e);}
	this.elCtrlFilterInput.onmouseout = function(e){return e.currentTarget.blur();}

	this.elCtrlHidden = this.document.createElement('div');
	this.elCtrl.appendChild( this.elCtrlHidden);
	this.elCtrlHidden.textContent = 'H';
	this.elCtrlHidden.title = 'Show hidden items.';
	this.elCtrlHidden.classList.add('ctrl_hidden');
	this.elCtrlHidden.m_monitor = this;
	this.elCtrlHidden.onclick = function(e)
	{
		var el = e.currentTarget;
		el.m_monitor.selectAll( false);
		el.m_monitor.elMonitor.classList.toggle('show_hidden');
		if( el.m_monitor.elMonitor.classList.contains('show_hidden'))
			el.m_monitor.show_hidden = true;
		else
			el.m_monitor.show_hidden = false;
	}

	this.elInfoText = this.document.createElement('div');
	this.elInfoText.classList.add('text');	
	this.elInfoText.classList.add('text_selectable');
	this.elInfoText.textContent = this.type;
	this.elInfo.appendChild( this.elInfoText);

	for( var i = 0; i < g_receivers.length; i++)
	{
		if( g_receivers[i].name == this.name )
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
	{
		if( localStorage[this.type + '_sort_param_visor'])
			this.sortParm = localStorage[this.type + '_sort_param_visor'];
		else
			this.sortParm = this.nodeConstructor.sortVisor;
	}
	else
	{
		if( localStorage[this.type + '_sort_param'])
			this.sortParm = localStorage[this.type + '_sort_param'];
		else
			this.sortParm = this.nodeConstructor.sort[0];
	}

	for( var i = 0; i < cm_Attrs.length; i++)
		if( cm_Attrs[i][0] == this.sortParm )
			this.elCtrlSortParam.textContent = cm_Attrs[i][1];

	if( localStorage[this.type + '_sort_dir'] == 'ON')
		this.sortDirection = true;
	else
		this.sortDirection = false;
	if( this.sortParm == 'order')
		this.sortDirection = true;

	if( this.nodeConstructor.filterVisor && g_VISOR())
	{
		if( localStorage[this.type + '_filter_param_visor'] )
			this.filterParm = localStorage[this.type + '_filter_param_visor'];
		else
			this.filterParm = this.nodeConstructor.filterVisor;
	}
	else
	{
		if( localStorage[this.type + '_filter_param'])
			this.filterParm = localStorage[this.type + '_filter_param'];
		else
			this.filterParm = this.nodeConstructor.filter[0];
	}

	if( localStorage[this.type + '_filter_exclude'] == 'ON')
		this.filterExclude = true;
	else
		this.filterExclude = false;

	for( var i = 0; i < cm_Attrs.length; i++)
		if( cm_Attrs[i][0] == this.filterParm )
			this.elCtrlFilterParam.textContent = cm_Attrs[i][1];

	this.items = [];
	this.selected_items = [];
	this.cur_item = null;

	g_receivers.push( this);
	g_monitors.push( this);
	this.setWindowTitle();
	g_cur_monitor = this;
	this.menu = null;
	this.cycle = 0;

	if( this.type == 'tasks')
	{
		this.job_id = i_args.id;
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
	cm_ArrayRemove(	g_receivers, this);
	cm_ArrayRemove(	g_refreshers, this);
	cm_ArrayRemove(	g_monitors, this);
	if( this.type == 'tasks')
		nw_Subscribe( this.type, false, [this.job_id]);
	else
		nw_Subscribe( this.type, false);

	for( var i = 0; i < this.items.length; i++)
		if( this.items[i].monitorDestroy )
			this.items[i].monitorDestroy( this);

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
		// Get resources of online renders:
		var ids = [];
		for( var i = 0; i < this.items.length; i++)
			if( this.items[i].state.ONL )
				ids.push( this.items[i].params.id)
		if( ids.length )
			nw_GetNodes('renders', ids, 'resources');
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

		if(( this.type == 'jobs') && obj.events.jobs_order_ids && obj.events.jobs_order_ids.length )
			this.sortByIds( obj.events.jobs_order_ids);

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
		found = false;
		for( var i = 0; i < this.items.length; i++)
		{
			if( this.items[i].params.id == nodes[j].id )
			{
				this.items[i].update( nodes[j]);

				if( this.panel_item == this.items[i] )
					this.updatePanels( this.items[i]);

				if(( this.type == 'jobs' ) && this.elPanelR.m_elBlocks.m_cur_block )
					if( this.elPanelR.m_elBlocks.m_cur_block.job.params.id == this.items[i].params.id )
						this.elPanelR.m_elBlocks.m_cur_block.updatePanels();

				updated.push( this.items[i]);
				this.filterItem( this.items[i]);

				found = true;
				break;
			}
		}
		if( found == false )
			new_ids.push(j);
	}

	if( this.sortParm != 'order')
		for( var i = 0; i < updated.length; i++)
			this.sortItem( updated[i]);

	var new_nodes = [];
	for( var i = 0; i < new_ids.length; i++)
		new_nodes.push( this.createNode( nodes[new_ids[i]]));

	if(( this.type == 'jobs' ) && ( this.sortParm == 'order'))
		if( new_ids.length )//|| updated.length )
			nw_GetNodes('users',[g_uid],'jobs_order');

	if( false == this.hasSelection())
	{
		if( new_nodes.length )
			this.cur_item = new_nodes[new_nodes.length-1];
		else if( updated.length )
			this.cur_item = updated[updated.length-1];
	}

	if(( this.firstNodesReceived != true ) && new_nodes.length && ( g_VISOR() != true ))
	{
		this.items[this.items.length-1].element.scrollIntoView();
		this.firstNodesReceived = true;
	}

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
				if( this.panel_item == this.items[i] )
					this.resetPanels({'hide_params':true});

				if( this.items[i].selected )
					this.selected_items.splice( this.selected_items.indexOf( this.items[i]), 1);

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
	i_item.element.ondblclick = function(e){ return e.currentTarget.item.onDoubleClick(e);}
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

	if( i_evt.shiftKey && this.cur_item )
	{
		var i = this.items.indexOf( this.cur_item);
		var ci = this.items.indexOf( i_el.item);
		if(( i != ci ) && ( i != -1) && ( ci != -1))
		{
			this.setSelected( i_el.item, true);
			var d = 1;
			if( i > ci ) d = -1;
			while( i != ci )
			{
				this.setSelected( this.items[i], true);
				i += d;
			}
			return;
		}
	}
	else if( ! i_evt.ctrlKey )
		this.selectAll( false);

		
	this.selectToggle( i_el.item);
}

Monitor.prototype.onMouseOver = function( i_evt, i_el)
{
	if( i_evt.buttons != 1 ) return;
	this.setSelected( i_evt.currentTarget.item, i_evt.ctrlKey == false);
}

Monitor.prototype.setSelected = function( i_item, on)
{
	this.window.getSelection().removeAllRanges();

	if( on )
	{
		if(( this.show_hidden != true ) && ( i_item.hidden ))
			return;

		this.cur_item = i_item;

		this.updatePanels( null, {'hide_params':true});

		this.info( this.cur_item.params.name);

		if( i_item.selected ) return;

		i_item.selected = true;
		i_item.element.classList.add('selected');
		this.selected_items.push( i_item);

		if( this.type == 'jobs' )
			this.setWindowTitle();

		if( this.cur_item.element.offsetTop < this.elList.scrollTop )
			this.cur_item.element.scrollIntoView();
		if( this.cur_item.element.offsetTop + this.cur_item.element.clientHeight > this.elList.scrollTop + this.elList.clientHeight)
			this.cur_item.element.scrollIntoView(false);
	}
	else
	{
		if( ! i_item.selected ) return;

		this.resetPanels({'hide_params':true});

		i_item.selected = false;
		i_item.element.classList.remove('selected');
		this.selected_items.splice( this.selected_items.indexOf( i_item), 1);

		if( this.selected_items.length )
		{
			this.cur_item = this.selected_items[this.selected_items.length-1];
			this.updatePanels( null, {'hide_params':true});
		}
		else
			this.cur_item = null;
	}
}

Monitor.prototype.selectToggle = function( i_item)
{
	if( ! i_item ) return;
	if( i_item.selected )
		this.setSelected( i_item, false);
	else
		this.setSelected( i_item, true);
}

Monitor.prototype.selectAll = function( on)
{
	for( var i = 0; i < this.items.length; i++)
	{
		if( on && ( this.items[i].show_hidden != true ) && ( this.items[i].hidden ))
			continue;

		this.setSelected( this.items[i], on);
	}

	if( this.type == 'jobs' )
		JobBlock.deselectAll( this);
}

Monitor.prototype.selectNext = function( i_evt, previous)
{
	if( this.items.length == 0 ) return;

	var next_index = 0;
	if( this.cur_item )
	{
		var cur_index = this.items.indexOf( this.cur_item);
		next_index = cur_index;
		if( this.hasSelection())
		{
			while(( next_index >= 0 ) && ( next_index < this.items.length ))
			{
				if( previous )
					next_index--;
				else
					next_index++;
				if( this.show_hidden || ( this.items[next_index] && ( this.items[next_index].hidden != true )))
					break;
			}
		}
//		if( next_index == cur_index ) return;
	}

	if( next_index < 0 ) return;
	if( next_index >= this.items.length ) return;

	if( false == i_evt.shiftKey )
		this.selectAll( false);

	this.setSelected( this.items[next_index], true);
}

Monitor.prototype.showObject = function( i_act, i_evt)
{
	if( this.hasSelection() == false ) return false;
	if( this.cur_item && this.cur_item.params )
		g_ShowObject({"object":this.cur_item.params},{"evt":i_evt,"wnd":this.window});
}

Monitor.prototype.resetPanels = function( i_args)
{
	if( this.panel_item == null ) return;

	if( i_args == null ) i_args = {};

	this.elPanelR.m_elName.style.display = 'none';
	this.elPanelR.m_elInfo.m_elBody.textContent = '';

	var els = this.elPanelL.getElementsByClassName('ctrl_button');
	for( var i = 0; i < els.length; i++)
		if( els[i].m_always_active != true )
			els[i].classList.remove('active');
	var els = this.elPanelR.getElementsByClassName('section');
	for( var i = 0; i < els.length; i++)
		if( els[i].m_always_active != true )
			els[i].classList.remove('active');

	if( i_args.hide_params )
	{
		this.elPanelR.m_elParams.m_all_shown = false;
		var elParams = this.elPanelR.m_elParams.m_elPMap;
		for( var p in elParams )
			elParams[p].style.display = 'none';
	}

	if( this.nodeConstructor.resetPanels )
		this.nodeConstructor.resetPanels( this);

	this.panel_item = null;
}
Monitor.prototype.updatePanels = function( i_item, i_args)
{
	this.resetPanels( i_args);

	if( i_item == null )
		i_item = this.cur_item;
	if( i_item == null )
		return;

	this.panel_item = i_item;

	this.elPanelR.m_elName.textContent = i_item.params.name;
	this.elPanelR.m_elName.title = 'Current job name:\n' + i_item.params.name;
	this.elPanelR.m_elName.style.display = 'block';

	var els = this.elPanelL.getElementsByClassName('ctrl_button');
	for( var i = 0; i < els.length; i++)
		els[i].classList.add('active');
	var els = this.elPanelR.getElementsByClassName('section');
	for( var i = 0; i < els.length; i++)
		els[i].classList.add('active');

	var elParams = this.elPanelR.m_elParams.m_elPMap;
	for( var p in elParams )
	{
		if( i_item.params[p] == null )
		{
			if( this.elPanelR.m_elParams.m_all_shown != true )
				elParams[p].style.display = 'none';
			elParams[p].m_elValue.textContent = '';
			continue;
		}

		var value = i_item.params[p];
		if( this.nodeConstructor.params[p].type == 'hrs')
			value = cm_TimeStringFromSeconds( value, true);
		else if(( typeof value ) == 'string' )
		{
			// word-wrap long regular expressions:
			value = value.replace(/\./g,'.&shy;');
			value = value.replace(/\|/g,'|&shy;');
			value = value.replace(/\)/g,')&shy;');
		}
		elParams[p].m_elValue.innerHTML = value;
		elParams[p].style.display = 'block';
	}
	if( i_item.updatePanels )
		i_item.updatePanels();
}
Monitor.prototype.setPanelInfo = function( i_html) { this.elPanelR.m_elInfo.m_elBody.innerHTML = i_html; }
Monitor.prototype.resetPanelInfo = function()      { this.elPanelR.m_elInfo.m_elBody.textContent = '';   }

Monitor.prototype.onContextMenu = function( i_evt, i_el)
{
	i_evt.stopPropagation();
	g_cur_monitor = this;

	if( i_el.item.selected != true )
		this.selectAll( false);
	this.setSelected( i_el.item, true);

	if( i_el.item.onContextMenu )
	{
		var menu = this.createMenu( i_evt,'context');
		i_el.item.onContextMenu( menu);
		menu.show();
	}

	return false;
}
Monitor.prototype.onMenuDestroy = function() { this.menu = null;}

Monitor.prototype.addMenuItem = function( i_menu, i_action)
{
	if( i_action.name == null )
	{
		i_menu.addItem();
		return;
	}

	var item = {};
	for( var key in i_action ) item[key] = i_action[key];
	item.receiver = this;
	if( this.cur_item && this.cur_item[item.handle])
		item.receiver = this.cur_item;
	item.param = i_action;

	if( item.permissions )
	{
		if( g_VISOR())
		{
			if( item.permissions == 'user') return;
			if( g_GOD())
			{
				if( false == (( item.permissions == 'visor') || ( item.permissions == 'god')))
					return;
			}
			else if( item.permissions != 'visor')
				return;
		}
		else if( item.permissions != 'user')
			return;
	}

	if( i_action.mode == 'cgru_cmdexec')
	{
		var cmds = [];
		for( var i = 0; i < this.items.length; i++)
			if( this.items[i].selected == true )
			{
				var cmd = i_action.handle;
				cmd = cmd.replace(/@ARG@/g, this.items[i].params.name);
				if( this.items[i].params.address.ip )
					cmd = cmd.replace(/@IP@/g, this.items[i].params.address.ip);
				cmds.push( cmd);
			}
		i_menu.addItem({"name":i_action.name,"receiver":'cgru_cmdexec',"handle":cmds,"label":i_action.label});
		return;
	}

	if( item.receiver[item.handle] )
		i_menu.addItem( item);
	else
		i_menu.addItem({"label":'invalid '+name,"enabled":false});
}

Monitor.prototype.showMenu = function( i_evt, i_name, i_need_selection)
{
	if( i_need_selection !== false )
	{
		if( this.cur_item == null ) return;
		if( this.hasSelection() == false ) return;
	}

	if( i_name == null ) i_name = 'set';

	var menu = this.createMenu( i_evt, 'set');
//	var actions = this.cur_item.constructor.actions;
	var actions = this.nodeConstructor.actions;
	for( var i = 0; i < actions.length; i++)
		if( actions[i].mode == i_name )
			this.addMenuItem( menu, actions[i]);
	menu.show();
}
Monitor.prototype.mh_Param = function( i_param)
{
	this.setParameter( i_param.value, i_param.name);
}
Monitor.prototype.mh_Dialog = function( i_param)
{
	var args = {};

	args.param    = i_param.name;
	args.type     = i_param.type;
	args.receiver = this;
	args.wnd      = this.window;
	args.handle   = 'setParameter';
	args.value    = this.cur_item.params[i_param.name];
	args.name     = this.name + '_parameter';

	new cgru_Dialog( args);
}
Monitor.prototype.setParameter = function( i_value, i_parameter)
{
	var params = {};
	params[i_parameter] = i_value;
//this.info('params.'+i_parameter+'="'+i_value+'";');
	this.action( null, params);
}
Monitor.prototype.mh_Oper = function( i_param)
{
//this.info('Operation = ' + i_param.name);
	var operation = {};
	operation.type = i_param.name;
	this.action( operation, null);
}
Monitor.prototype.mh_Get = function( i_param, i_evt)
{
//this.info('Get = ' + i_param.name);
	if(( this.hasSelection() == false ) || ( this.cur_item == null ))
	{
		g_Info('No object selected.');
		return;
	}

	get = {"type":this.type,"ids":[this.cur_item.params.id],"mode":i_param.name};
	nw_request({"send":{"get":get},"func":g_ShowObject,"evt":i_evt,"wnd":this.window});
}

Monitor.prototype.action = function( i_operation, i_params)
{
	nw_Action( this.type, this.getSelectedIds(), i_operation, i_params);
}

Monitor.prototype.mh_Opt = function( i_param)
{
	var args = {};

	args.param    = i_param.name;
	args.type     = i_param.type;
	args.receiver = this;
	args.wnd      = this.window;
	args.handle   = 'setOption';
	args.value    = this.view_opts[i_param.name];
	args.name     = this.name + '_parameter';

	new cgru_Dialog( args);
}
Monitor.prototype.setOption = function( i_value, i_param)
{
	this.view_opts[i_param] = i_value;
	localStorage[i_param] = i_value;
}
Monitor.prototype.getSelectedItems = function()
{
	var items = [];
	for( var i = 0; i < this.selected_items.length; i++)
		items.push( this.selected_items[i]);
	return items;
}
Monitor.prototype.getSelectedIds = function()
{
	var ids = [];
	for( var i = 0; i < this.selected_items.length; i++)
		ids.push( this.selected_items[i].params.id);
	return ids;
}
Monitor.prototype.hasSelection = function() { return ( this.selected_items.length > 0 ); }

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
	var menu = new cgru_Menu({"doc":this.document,"parent":this.document.body,"evt":i_evt,"name":this.type+'_'+i_name,"receiver":this,"destroy":'onMenuDestroy'});
	this.menu = menu;
	return menu;
}

Monitor.prototype.createCtrlBtns = function( i_acts)
{
	for( var a in i_acts )
	{
		if( i_acts[a].name == null )
			i_acts[a].name = a;
		this.createCtrlBtn( i_acts[a]);
	}
}

Monitor.prototype.createCtrlBtn = function( i_args)
{
	if( this.ctrl_btns == null ) this.ctrl_btns = {};

	var elBtn = document.createElement('div');

	if( i_args.elParent )
		i_args.elParent.appendChild( elBtn);
	else
		this.elPanelL.appendChild( elBtn);

	this.ctrl_btns[i_args.name] = elBtn;

	if( i_args.sub_button )
		elBtn.classList.add('sub_button');
	else
		elBtn.classList.add('ctrl_button');

	if( i_args.always_active )
	{
		elBtn.classList.add('active');
		elBtn.m_always_active = true;
	}

	elBtn.textContent = i_args.label;
	elBtn.title = i_args.tooltip;
	elBtn.m_monitor = this;
	elBtn.m_act = i_args;

	if( i_args.sub_menu )
	{
		elBtn.onclick = function(e){ e.currentTarget.classList.toggle('hide_childs'); return false; }
		elBtn.oncontextmenu = elBtn.onclick;
		elBtn.classList.add('hide_childs');

		var acts = i_args.sub_menu;
		for( var a in acts )
		{
			acts[a].sub_button = true;
			if( acts[a].name == null ) acts[a].name = a;
			acts[a].elParent = elBtn;
			if( acts[a].handle == null ) acts[a].handle = i_args.handle;
			var el = this.createCtrlBtn( acts[a]);
			el.m_elBtn = elBtn;
		}
	}
	else
	{
		if( i_args.ondblclick )
			elBtn.ondblclick = Monitor.ctrlBtnClicked;
		else
			elBtn.onclick = Monitor.ctrlBtnClicked;
		elBtn.oncontextmenu = function(e){return false;}
	}

	return elBtn;
}
Monitor.ctrlBtnClicked = function(e)
{
	e.stopPropagation();

	var el = e.currentTarget;

	var elBtn = el;
	if( el.m_elBtn ) elBtn = el.m_elBtn; // <-- this is sub-button in this case
	if( elBtn.classList.contains('active') != true )
		return false;

	var args = {'name':el.m_act.name,'type':el.m_act.type,'value':el.m_act.value,'monitor':el.m_monitor};

	var handle = el.m_act.handle;
	if( handle == null )
		handle = 'mh_Oper';

	var action_accepted = true;
	if( el.m_monitor.nodeConstructor[handle] )
		action_accepted = el.m_monitor.nodeConstructor[handle]( args, e);
	else
		action_accepted = el.m_monitor[handle]( args, e);

	if( action_accepted !== false )
	{
		setInterval( Monitor.ctrlBtnRelease, 1234, el);
		el.classList.add('clicked');
	}

	return false;
}
Monitor.ctrlBtnRelease = function( i_el)
{
	i_el.classList.remove('clicked');
}

// --------------- Sorting: -------------------//
Monitor.prototype.sortDirChanged = function( i_evt)
{
	if( this.sortParm == 'order') return;
	if( this.sortDirection ) this.sortDirection = false;
	else this.sortDirection = true;
	localStorage[this.type + '_sort_dir'] = this.sortDirection ? 'ON' : 'OFF';
	this.sortItems();
	return false;
}
Monitor.prototype.sortFilterParmMenu = function( i_evt, i_type)
{
	var menu = this.createMenu( i_evt, i_type);
	for( var i = 0; i < this.nodeConstructor[i_type].length; i++)
		for( var j = 0; j < cm_Attrs.length; j++)
			if( this.nodeConstructor[i_type][i] == cm_Attrs[j][0] )
				menu.addItem({"name":cm_Attrs[j][0],"receiver":this,"handle":i_type+'ParmChanged',"label":cm_Attrs[j][2]});
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

	// Store:
	if( this.nodeConstructor.sortVisor && g_VISOR() )
		localStorage[this.type + '_sort_param_visor'] = this.sortParm;
	else
		localStorage[this.type + '_sort_param'] = this.sortParm;

	this.sortItems();
}

// --------------- Filtering: -------------------//
Monitor.prototype.filterExcludeChanged = function( i_evt)
{
	if( this.filterExclude ) this.filterExclude = false;
	else this.filterExclude = true;
	localStorage[this.type + '_filter_exclude'] = this.filterExclude ? 'ON' : 'OFF';
	this.filterItems();
}
Monitor.prototype.filterParmChanged = function( i_name)
{
	this.elCtrlFilterParam.textContent = i_name;
	for( var i = 0; i < cm_Attrs.length; i++)
		if( cm_Attrs[i][0] == i_name )
			this.elCtrlFilterParam.textContent = cm_Attrs[i][1];
	this.filterParm = i_name;

	// Store:
	if( this.nodeConstructor.filterVisor && g_VISOR() )
		localStorage[this.type + '_filter_param_visor'] = this.filterParm;
	else
		localStorage[this.type + '_filter_param'] = this.filterParm;

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
	if( i_item.params.hidden )
		hide = true;
	else if( this.filterExpr && this.filterParm && i_item.params[this.filterParm] )
	{
		if( false == this.filterExpr.test( i_item.params[this.filterParm]))
			hide = ( this.filterExclude ? false : true );
		else
			hide = ( this.filterExclude ? true : false );
	}
	if( hide )
	{
//		i_item.element.style.display = 'none';
		i_item.element.classList.add('hidden');
		i_item.hidden = true;
	}
	else
	{
//		i_item.element.style.display = 'block';
		i_item.element.classList.remove('hidden');
		i_item.hidden = false;
	}
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
