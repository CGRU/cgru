function RendersList()
{
	this.valid = false;
	this.name = 'renders';
	for( i = 0; i < recievers.lenght; i++)
	{
		if( recievers.name == this.name )
		{
			info('ERROR: Renders list already exists.');
			return;
		}
	}
	recievers.push( this);

	this.items = [];

	var obj = {};
	obj.get = {};
	obj.get.type = 'renders';
	send(obj);
}

RendersList.prototype.processMsg = function( obj)
{
//	document.getElementById('data').innerHTML=obj.renders;

	if( obj.renders == null )
		return;

	items = [];

	for( i = 0; i < obj.renders.length; i++)
	{
		items.push( new RenderNode(obj.renders[i]));
	}

	info('renders processed: ' + obj.renders.length);
}

function RenderNode( obj)
{
	this.element = document.createElement('p');
	document.getElementById('data').appendChild(this.element);
//	this.element.style.width = '50%';
	this.element.style.outlineStyle = 'solid';
	this.element.style.outlineWidth = '1px';
	this.element.style.outlineColor = '#777777';
	this.element.style.backgroundColor = '#EEEEEE';
	this.element.style.margin = '5px';
	this.element.style.padding = '3px';

	this.params = obj

//	this.table = document.createElement('table');
//	this.element.appendChild( this.table);
//	this.table.style.width = '100%';
//	this.row1 = document.createElement('tr');
//	this.table.appendChild( this.row1);

	var name = obj.name;
	if( obj.version != null )
		name += ' ' + obj.version;

	var user = obj.user_name;

	if( obj.offline === true )
		this.element.style.backgroundColor = '#999999';
//	else
//	{
//		this.row2 = document.createElement('tr');
//		this.table.appendChild( this.row2);
//	}
	if( obj.busy === true )
		this.element.style.backgroundColor = '#99EE77';

	if( obj.NIMBY === true )
	{
		if( obj.offline !== true) this.element.style.backgroundColor = '#8888DD';
		user = '(' + user + ')N';
	}
	else if( obj.nimby === true )
	{
		if( obj.offline !== true) this.element.style.backgroundColor = '#9999DD';
		user = '(' + user + ')n';
	}

	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.innerHTML = name;
//	this.name.style.backgroundColor = '#EEEE99';

	this.user_name = document.createElement('span');
	this.element.appendChild( this.user_name);
	this.user_name.style.cssFloat = 'right';
	this.user_name.innerHTML = user;
//	this.user_name.style.backgroundColor = '#EEEEBB';

	this.center = document.createElement('span');
	this.element.appendChild( this.center);
	this.center.style.backgroundColor = '#EEEE99';
	this.center.style.textAlign = 'center';
	this.center.style.position = 'absolute';
	this.center.style.left = '40%';
	this.center.style.width = '20%';

	if( obj.offline === true )
	{
		this.center.innerHTML = 'offline';
		return;
	}

	this.center.style.height = '2.4em';
	this.center.innerHTML = '.';

	this.element.appendChild( document.createElement('br'));

	var capacity = obj.capacity;
	if( capacity == null )
		capacity = obj.host.capacity;
	capacity = obj.capacity_used + '/' + capacity;
	var max_tasks = obj.max_tasks;
	if( max_tasks == null )
		max_tasks = obj.host.max_tasks;
	if( obj.busy === true )
		max_tasks = '(' + obj.tasks.length + '/' + max_tasks + ')';
	else
		max_tasks = '(0/' + max_tasks + ')';
	capacity += ' ' + max_tasks;
	this.capacity = document.createElement('span');
	this.element.appendChild( this.capacity);
	this.capacity.innerHTML = capacity;
	this.capacity.style.textAlign = 'center';

	var state = 'NEW';
	if(( obj.task_start_finish_time != null ) && ( obj.task_start_finish_time > 0 ))
		state = obj.task_start_finish_time;
	this.state = document.createElement('span');
	this.element.appendChild( this.state);
	this.state.style.cssFloat = 'right';
	this.state.innerHTML = state;
}

