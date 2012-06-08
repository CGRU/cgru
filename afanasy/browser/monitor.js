function Monitor( i_element, i_type, i_id)
{
	this.element = i_element;
	this.element.monitor = this;
//	this.valid = false;
	this.type = i_type;
	for( i = 0; i < g_recievers.lenght; i++)
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
		g_updaters.push( this);
		nw_Subscribe( this.type, true);
		nw_GetNodes( this.type);
	}

	g_cur_monitor = this;
}

Monitor.prototype.destroy = function()
{
	for( i = 0; i < this.items.length; i++)
		this.element.removeChild(this.items[i].element);
	this.items = [];
	if( g_cur_monitor == this )
		g_cur_monitor = null;
	cm_ArrayRemove(	g_recievers, this);
	cm_ArrayRemove(	g_updaters, this);
	cm_ArrayRemove(	g_monitors, this);
	if( this.type == 'tasks')
		nw_Subscribe( this.type, false, [this.job_id]);
	else
		nw_Subscribe( this.type, false);
}

Monitor.prototype.update = function()
{
}

Monitor.prototype.processMsg = function( obj)
{
	if( obj.events != null )
	{
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
		else
		{
			if( obj.job_progress != null )
			{
				if( obj.job_progress.id == this.job_id )
					this.jobProgress( obj.job_progress.progress);
			}
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

g_Info(this.type + ':' + g_cycle + ' nodes processed ' + nodes.length + ': old:' + this.items.length + ' new:' + new_ids.length + ' up:' + updated);
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
				this.element.removeChild(this.items[i].element);
				this.items.splice(i,1);
				break;
			}
}

Monitor.prototype.newNode = function( i_obj)
{
	var node = null
	if     ( this.type == 'jobs'   ) node = new    JobNode();
	else if( this.type == 'renders') node = new RenderNode();
	else return null;

	this.createItem( node, i_obj);

	return node;
}

Monitor.prototype.createItem = function( i_item, i_obj)
{
	i_item.element = document.createElement('div');
//	i_item.element.className = 'item';
	this.element.appendChild( i_item.element);

	i_item.init();
	i_item.params = i_obj;
	i_item.update();
	i_item.parentElement = this.element;
	i_item.element.item = i_item;
	i_item.element.onmousedown = this.onMouseDown;
	i_item.element.onmouseover = this.onMouseOver;
	i_item.element.ondblclick = this.onDoubleClick;
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
	g_cur_monitor = el.parentElement.monitor;
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
	g_cur_monitor = evt.currentTarget.parentElement.monitor;
	g_cur_monitor.elSetSelected( evt.currentTarget, g_key_ctrl == false);
}

Monitor.prototype.elSetSelected = function( el, on)
{
	if( on )
	{
		this.cur_item = el.item;
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
	if( this.cur_item == null )
		this.cur_item = this.items[0];

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

	if( next_index < 0 ) return;
	if( next_index >= this.items.length ) return;
	if( cur_index == next_index ) return;

	this.cur_item = this.items[next_index]; 
	if( false == g_key_shift )
		this.selectAll( false);
	this.elSetSelected( this.cur_item.element, true);
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
