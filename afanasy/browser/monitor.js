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
		this.jobid = i_id;
		nw_GetNodes( 'jobs', [this.jobid], 'full');
	}
	else
	{
		g_updaters.push( this);
		nw_Subscribe( this.type);
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

	if(( this.type == 'tasks') && ( this.job == null ))
	{
		if( obj.jobs != null )
			if( obj.jobs.length == 1 )
				this.constructJob( obj.jobs[0]);
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

info(this.type + ':' + g_cycle + ' nodes processed ' + nodes.length + ': old:' + this.items.length + ' new:' + new_ids.length + ' up:' + updated);
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
	i_item.element.onmousedown = cm_ItemMouseDown;
	i_item.element.onmouseover = cm_ItemMouseOver;
	i_item.element.ondblclick = this.onDoubleClick;
}

Monitor.prototype.onDoubleClick = function(e)
{
	if( e == null ) return;
	var item = e.currentTarget.item;
	if( item == null ) return;
//document.getElementById('test').innerHTML = item.params.name;
	item.onDoubleClick();
}

Monitor.prototype.constructJob = function( job)
{
	this.job = job;
	for( var b = 0; b < this.job.blocks.length; b++)
	{
		var block = new BlockItem();
		this.createItem( block, this.job.blocks[b]);
		this.items.push( block);
		for( var t = 0; t < this.job.blocks[b].tasks_num; t++)
		{
			var task = new TaskItem( t);
			this.createItem( task, this.job.blocks[b]);
			this.items.push( task);
		}
	}
}
