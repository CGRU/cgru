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

	if( this.type == 'tasks' )
	{
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
		var node = this.createNode( nodes[new_ids[i]]);
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

Monitor.prototype.createNode = function( i_obj)
{
	var node = null
	if     ( this.type == 'jobs'   ) node = new    JobNode( i_obj);
	else if( this.type == 'renders') node = new RenderNode( i_obj);
	else return null;

	node.element = document.createElement('div');
//	node.element.className = 'item';
	this.element.appendChild( node.element);

	node.init();
	node.params = i_obj;
	node.update();
	node.parentElement = this.element;
	node.element.node = node;
	node.element.onmousedown = cm_ItemMouseDown;
	node.element.onmouseover = cm_ItemMouseOver;
	node.element.ondblclick = this.onDoubleClick;

	return node;
}

Monitor.prototype.onDoubleClick = function(e)
{
	if( e == null ) return;
	var node = e.currentTarget.node;
	if( node == null ) return;
document.getElementById('test').innerHTML = node.params.name;
	node.onDoubleClick();
}
