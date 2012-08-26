function RenderNode() {}

RenderNode.prototype.init = function()
{
	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild( this.elName);
//	this.elName.style.cssFloat = 'left';
	this.elName.title = 'Client host name';

	this.elVersion = document.createElement('span');
	this.element.appendChild( this.elVersion);
//	this.elVersion.style.cssFloat = 'left';
	this.elVersion.title = 'Client version';

	this.elPriority = document.createElement('span');
	this.element.appendChild( this.elPriority);
	this.elPriority.style.cssFloat = 'right';
	this.elPriority.title = 'Priority';

	this.elUserName = document.createElement('span');
	this.element.appendChild( this.elUserName);
	this.elUserName.style.cssFloat = 'right';
	this.elUserName.title = 'User name and "Nimby" status'

	this.elResources = document.createElement('div');
	this.element.appendChild( this.elResources);
	this.elResources.className = 'resources';
//	this.elResources.style.textAlign = 'center';

	this.elPower = document.createElement('div');
	this.element.appendChild( this.elPower);
//	this.elResources.appendChild( this.elPower);
	this.elPower.style.position = 'absolute';
	this.elPower.style.textAlign = 'center';
	this.elPower.style.top = '1px';
	this.elPower.style.left = '0';
	this.elPower.style.right = '0';

	this.element.appendChild( document.createElement('br'));

	this.elCapacity = document.createElement('span');
	this.element.appendChild( this.elCapacity);
	this.elCapacity.title = 'Capacity used / total'

	this.elMaxTasks = document.createElement('span');
	this.element.appendChild( this.elMaxTasks);
	this.elMaxTasks.title = 'Running tasks / maximum'

	this.elStateTime = document.createElement('span');
	this.element.appendChild( this.elStateTime);
	this.elStateTime.style.cssFloat = 'right';
	this.elStateTime.title = 'Busy / free status and time';

	this.elAnnotation = document.createElement('div');
	this.element.appendChild( this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.style.textAlign = 'center';

	this.plotters = [];
	this.plotterC = new Plotter( this.plotters, this.elResources, 'C');
	this.plotterM = new Plotter( this.plotters, this.elResources, 'M');
	this.plotterS = new Plotter( this.plotters, this.elResources, 'S');
	this.plotterH = new Plotter( this.plotters, this.elResources, 'H');
	this.plotterN = new Plotter( this.plotters, this.elResources, 'N');
	this.plotterD = new Plotter( this.plotters, this.elResources, 'D');

	this.plotterC.addGraph( 200,   0,  0);
	this.plotterC.addGraph(  50, 200, 20);

	this.state = {};
}

RenderNode.prototype.update = function()
{
	cm_GetState( this.params.state, this.element, this.state);

	this.elName.innerHTML = this.params.name;
	this.elName.title = this.params.host.os;

	if( this.params.version != null )
		this.elVersion.innerHTML = ' ' + this.params.version;
	else
		this.elVersion.innerHTML = '';

	this.elPriority.innerHTML = '-' + this.params.priority;

	var user = this.params.user_name;
	if( this.state.NBY )
		user = 'NIMBY(' + user + ')N';
	this.elUserName.innerHTML = user;

	if( this.params.annotation )
		this.elAnnotation.innerHTML = this.params.annotation;
	else
		this.elAnnotation.innerHTML = '';

	for( var i = 0; i < this.plotters.length; i++)
		this.plotters[i].setHidden( this.state.OFF)

	if( this.state.WWK ) this.offlineState = 'Waking Up';
	else if( this.state.WSL || this.state.WFL) this.offlineState = 'Sleeping';
	else this.offlineState = 'Offline';


	if( this.state.OFF == true )
	{
		this.elPower.innerHTML = 'Offline';
		this.elCapacity.innerHTML = '';
		this.elMaxTasks.innerHTML = '';
		this.state.innerHTML = '';
		this.elStateTime.innerHTML = '';
		return;
	}
	this.elPower.innerHTML = '';

	var capacity = this.params.capacity;
	if( capacity == null )
		capacity = this.params.host.capacity;
	capacity = this.params.capacity_used + '/' + capacity;
	this.elCapacity.innerHTML = capacity;

	var max_tasks = this.params.max_tasks;
	if( max_tasks == null )
		max_tasks = this.params.host.max_tasks;
	if( this.params.busy === true )
		max_tasks = '(' + this.params.tasks.length + '/' + max_tasks + ')';
	else
		max_tasks = '(0/' + max_tasks + ')';
	max_tasks = ' ' + max_tasks;
	this.elMaxTasks.innerHTML = max_tasks;
	
//	this.plotterC.update( this.params.host_resources);

	this.refresh();
}

RenderNode.prototype.refresh = function()
{
	if( this.state.OFF == true )
	{
		var power = this.offlineState;
		if( this.params.wol_operation_time )
			power += ' ' + cm_TimeStringInterval( this.params.wol_operation_time);
		this.elPower.innerHTML = power;
		return;
	}

	var stateTime = 'NEW';
	if(( this.params.task_start_finish_time != null ) && ( this.params.task_start_finish_time > 0 ))
	{
		stateTime = cm_TimeStringInterval( this.params.task_start_finish_time);
		if( this.state.RUN == true )
			stateTime += ' busy';
		else
			stateTime += ' free';
	}
	this.elStateTime.innerHTML = stateTime;
}

RenderNode.prototype.onDoubleClick = function()
{
}
