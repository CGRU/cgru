function Monitor( i_element, i_type, i_id)
{
	this.type = i_type;
	this.elParent = i_element;

	this.elMonitor = document.createElement('div');
	this.elParent.appendChild( this.elMonitor);
	this.elMonitor.classList.add('monitor');
	this.elMonitor.monitor = this;

	this.elList = document.createElement('div');
	this.elCtrl = document.createElement('div');
	this.elInfo = document.createElement('div');
	this.elMonitor.appendChild( this.elList);
	this.elMonitor.appendChild( this.elCtrl);
	this.elMonitor.appendChild( this.elInfo);
	this.elList.classList.add('list');
	this.elCtrl.classList.add('ctrl');
	this.elInfo.classList.add('info');

//	this.elList.onmousedown = function(e) { return false;}
//	this.elList.onmousedown = function(e) { if(e.button==0){g_mouse_down=true;  return false;}}
//	this.elList.onmouseup   = function(e) { if(e.button==0){g_mouse_down=false; return false;}}
/*	this.elList.oncontextmenu = this.onContextMenu;
	this.elList.monitor = this;
	this.elList.EType = 'list';*/

	this.elCtrlSet = document.createElement('span');
	this.elCtrl.appendChild( this.elCtrlSet);
	this.elCtrlSet.classList.add('menubutton');
	this.elCtrlSet.innerHTML = 'SET';
	this.elCtrlSet.style.top = '7px';
	this.elCtrlSet.onmouseover = this.onMouseOverSet;

	this.elCtrlSort = document.createElement('div');
	this.elCtrl.appendChild( this.elCtrlSort);
	this.elCtrlSort.classList.add('sort');
	this.elCtrlSort.innerHTML = 'Sort:';

	this.elCtrlFilter = document.createElement('div');
	this.elCtrl.appendChild( this.elCtrlFilter);
//	this.elCtrlFilter.classList.add('sort');
	this.elCtrlFilter.classList.add('filter');
	this.elCtrlFilter.innerHTML = 'Filter:';

	this.elInfoText = document.createElement('div');
	this.elInfoText.classList.add('text');	
	this.elInfoText.innerHTML = this.type;
	this.elInfo.appendChild( this.elInfoText);
///*
//this.elCtrl.innerHTML='ctrl';
//this.elList.innerHTML='list';
//this.elInfo.innerHTML='info';
//*/
//	this.valid = false;
	for( i = 0; i < g_recievers.length; i++)
	{
		if( g_recievers[i].type == this.type )
		{
			info('ERROR: Monitor[' + this.type + '] list already exists.');
			return;
		}
	}

	this.items = [];
	g_recievers.push( this);
	g_monitors.push( this);

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

	g_cur_monitor = this;

	this.cycle = 0;
}

Monitor.prototype.destroy = function()
{
	if( g_cur_monitor == this )
		g_cur_monitor = null;
	cm_ArrayRemove(	g_recievers, this);
	cm_ArrayRemove(	g_refreshers, this);
	cm_ArrayRemove(	g_monitors, this);
	if( this.type == 'tasks')
		nw_Subscribe( this.type, false, [this.job_id]);
	else
		nw_Subscribe( this.type, false);

//	for( i = 0; i < this.items.length; i++)
//		this.element.removeChild(this.items[i].element);

	this.items = [];

	this.elParent.removeChild( this.elMonitor);
}

Monitor.prototype.refresh = function()
{
	if( this.type == 'users' )
	{
		this.max_tasks = 0;
		for( i = 0; i < this.items.length; i++)
			if( this.items[i].params.running_tasks_num )
				if( this.items[i].params.running_tasks_num > this.max_tasks )
					this.max_tasks = this.items[i].params.running_tasks_num;
	}
	else if(( this.type == 'renders') && ( this.cycle % 5 == 0 ))
	{
		nw_ReqestRendersResources();
	}

	for( i = 0; i < this.items.length; i++)
		this.items[i].refresh();

	this.cycle++;
}

Monitor.prototype.processMsg = function( obj)
{
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
	var updated = 0;

	for( j = 0; j < nodes.length; j++)
	{
		founded = false;
		for( i = 0; i < this.items.length; i++)
		{
			if( this.items[i].params.id == nodes[j].id )
			{
				this.items[i].params = nodes[j];
				this.items[i].update();
				founded = true;
				updated = updated + 1;
				break;
			}
		}
		if( founded == false )
			new_ids.push(j);
	}

	for( i = 0; i < new_ids.length; i++)
	{
		var node = this.newNode( nodes[new_ids[i]]);
		if( node != null )
			this.items.push( node);
	}

//g_Info(this.type + ':' + g_cycle + ' nodes processed ' + nodes.length + ': old:' + this.items.length + ' new:' + new_ids.length + ' up:' + updated);
}

Monitor.prototype.delNodes = function( i_ids)
{
	if( i_ids == null ) return;
	if( i_ids.length == null ) return;
	if( i_ids.length == 0 ) return;
	if( this.items.length == 0 ) return;

	for( d = 0; d < i_ids.length; d++ )
		for( i = 0; i < this.items.length; i++)
			if( this.items[i].params.id == i_ids[d] )
			{
				this.elList.removeChild( this.items[i].element);
				this.items.splice(i,1);
				break;
			}
}

Monitor.prototype.newNode = function( i_obj)
{
	var node = null
	if     ( this.type == 'jobs'   ) node = new    JobNode();
	else if( this.type == 'renders') node = new RenderNode();
	else if( this.type == 'users'  ) node = new   UserNode();
	else return null;

	this.createItem( node, i_obj);

	return node;
}

Monitor.prototype.createItem = function( i_item, i_obj)
{
	i_item.element = document.createElement('div');
	i_item.element.className = 'item';
	this.elList.appendChild( i_item.element);

	i_item.params = i_obj;
	i_item.monitor = this;
	i_item.element.monitor = this;
	i_item.element.item = i_item;
	i_item.element.onmousedown = this.onMouseDown;
	i_item.element.onmouseover = this.onMouseOver;
	i_item.element.ondblclick = this.onDoubleClick;
	i_item.element.oncontextmenu = this.onContextMenu;

	i_item.init();
	i_item.update();
}

Monitor.prototype.info = function( i_str)
{
	this.elInfoText.innerHTML = i_str;
}

Monitor.prototype.onDoubleClick = function(evt)
{
	if( evt == null ) return;
	var item = evt.currentTarget.item;
	if( item == null ) return;
	item.onDoubleClick();
}

Monitor.prototype.onMouseDown = function(evt)
{
	if( evt.button != 0 ) return;
	var el = evt.currentTarget;
	if( el == null ) return;
	if( el.monitor == null ) return

	g_cur_monitor = el.monitor;
	if( false == g_key_ctrl )
		g_cur_monitor.selectAll( false);
	if( g_key_shift && g_cur_monitor.cur_item )
	{
		var i = g_cur_monitor.items.indexOf( g_cur_monitor.cur_item);
		var ci = g_cur_monitor.items.indexOf( el.item);
		if(( i != ci ) && ( i != -1) && ( ci != -1))
		{
			g_cur_monitor.elSetSelected( el, true);
			var d = 1;
			if( i > ci ) d = -1;
//info('i='+i+' ci='+ci+' d='+d);
			while( i != ci )
			{
				g_cur_monitor.elSetSelected( g_cur_monitor.items[i].element, true);
				i += d;
			}
			return;
		}
	}
		
	g_cur_monitor.elSelectToggle( el);
}

Monitor.prototype.onMouseOver = function(evt)
{
	if( evt.button != 0 ) return;
	if( false == g_mouse_down ) return;
	g_cur_monitor = evt.currentTarget.monitor;
	g_cur_monitor.elSetSelected( evt.currentTarget, g_key_ctrl == false);
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

Monitor.prototype.selectNext = function( previous)
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
	if( false == g_key_shift )
		this.selectAll( false);
	this.elSetSelected( this.cur_item.element, true);
}

Monitor.prototype.onContextMenu = function( evt)
{
	var el = evt.currentTarget;
	if( el == null ) return;
	if( el.monitor == null ) return

	g_cur_monitor = el.monitor;
	if( el.selected != true )
		g_cur_monitor.selectAll( false);
	g_cur_monitor.elSetSelected( el, true);
	el.item.onContexMenu( evt);

	return false;
}


Monitor.prototype.onMouseOverSet = function( evt)
{
	if( g_cur_monitor == null ) return;
g_cur_monitor.info('onMouseOverSet');
	if( g_cur_monitor.cur_item == null ) return;

	g_cur_monitor.cur_item.openDialogSet();
}

Monitor.prototype.action = function( i_name)
{
	nw_Action( this.type, i_name, this.getSelectedIds());
}

Monitor.prototype.getSelectedIds = function()
{
	var ids = [];
	for( var i = 0; i < this.items.length; i++)
		if( this.items[i].element.selected == true )
			ids.push( this.items[i].params.id);
	return ids;
}

Monitor.prototype.jobConstruct = function( job)
{
	this.job = job;
	this.blocks = [];
	for( var b = 0; b < this.job.blocks.length; b++)
	{
		var block = new BlockItem(b);
		this.createItem( block, this.job.blocks[b]);
		this.items.push( block);
		this.blocks.push( block);
		block.tasks = [];
		for( var t = 0; t < this.job.blocks[b].tasks_num; t++)
		{
			var task = new TaskItem( t);
			this.createItem( task, this.job.blocks[b]);
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
		cm_Error('Job progress bocks length mismatch: job_id='
			+ this.job_id + ' ' + this.blocks.length + '!=' + progress.length);
		return;
	}

	for( var b = 0; b < this.blocks.length; b++)
	{
		if( this.blocks[b].tasks.length != progress[b].length )
		{
			cm_Error('Job progress tasks length mismatch: job_id='
				+ this.job_id + ' block=' + b + ' ' + this.blocks[b].tasks.length + '!=' + progress[b].length);
			return;
		}

		for( var t = 0; t < this.blocks[b].tasks.length; t++)
		{
			this.blocks[b].tasks[t].updateProgress( progress[b][t]);
		}
	}
}

Monitor.prototype.tasksProgress = function( tasks_progress)
{
g_Info('Monitor.prototype.tasksProgress = function( tasks_progress)');
	var j = -1;
	
	for( var i = 0; i < tasks_progress.length; i++)
	{
g_Info(' jid='+tasks_progress[i].job_id+' this='+this.job_id);
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
}

Monitor.prototype.getBlocks = function( i_block_ids)
{
	blocks = [];
	modes = [];

//var test = 'block ids:'
//document.getElementById('test').innerHTML = test;

	for( var i = 0; i < i_block_ids.job_id.length; i++)
	{
		if( this.job_id != i_block_ids.job_id[i]) continue;
//test += ' n' + i_block_ids.block_num[i];
//test += ' m' + i_block_ids.mode[i];
		blocks.push( i_block_ids.block_num[i]);
		modes.push( i_block_ids.mode[i]);
	}

//document.getElementById('test').innerHTML = test;

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
