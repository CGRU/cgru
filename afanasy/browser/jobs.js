function JobNode() {}

JobNode.prototype.init = function() 
{
	this.element.classList.add('job');

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild( this.elName);
	this.elName.title = 'Job name';

	this.elUserName = document.createElement('span');
	this.element.appendChild( this.elUserName);
	this.elUserName.style.cssFloat = 'right';
	this.elUserName.title = 'User name'

	this.element.appendChild( document.createElement('br'));

	this.elState = document.createElement('span');
	this.element.appendChild( this.elState);
	this.elState.title = 'Job state';

	this.elTime = document.createElement('span');
	this.element.appendChild( this.elTime);
	this.elTime.style.cssFloat = 'right';
	this.elTime.style.marginLeft = '4px';
	this.elTime.title = 'Running time';

	this.elPriority = document.createElement('span');
	this.element.appendChild( this.elPriority);
	this.elPriority.style.cssFloat = 'right';
	this.elPriority.style.marginLeft = '4px';
	this.elPriority.title = 'Priority';

	this.elDependMask = document.createElement('span');
	this.element.appendChild( this.elDependMask);
	this.elDependMask.style.cssFloat = 'right';
	this.elDependMask.style.marginLeft = '4px';
	this.elDependMask.title = 'Depend mask';

	this.elDependMaskGlobal = document.createElement('span');
	this.element.appendChild( this.elDependMaskGlobal);
	this.elDependMaskGlobal.style.cssFloat = 'right';
	this.elDependMaskGlobal.style.marginLeft = '4px';
	this.elDependMaskGlobal.title = 'Global depend mask';

	this.elHostsMask = document.createElement('span');
	this.element.appendChild( this.elHostsMask);
	this.elHostsMask.style.cssFloat = 'right';
	this.elHostsMask.style.marginLeft = '4px';
	this.elHostsMask.title = 'Hosts mask';

	this.elHostsMaskExclude = document.createElement('span');
	this.element.appendChild( this.elHostsMaskExclude);
	this.elHostsMaskExclude.style.cssFloat = 'right';
	this.elHostsMaskExclude.style.marginLeft = '4px';
	this.elHostsMaskExclude.title = 'Hosts mask exclude';

	this.blocks = [];
	for( var b = 0; b < this.params.blocks.length; b++)
		this.blocks.push( new JobBlock( this.element, this.params.blocks[b]));

	this.elAnnotation = document.createElement('div');
	this.element.appendChild( this.elAnnotation);
	this.elAnnotation.title = 'Annotation';
	this.elAnnotation.style.textAlign = 'center';
}

JobNode.prototype.update = function()
{
	cm_GetState( this.params.state, this.element, this.elState);

	var displayFull = false;
	if( this.elState.ERR || this.elState.RUN || this.elState.SKP ||
	  ((this.elState.DON == false) && (this.params.time_started > 0 )))
		displayFull = true;

	this.elName.innerHTML = this.params.name;
	this.elPriority.innerHTML = 'P' + this.params.priority;
	this.elUserName.innerHTML = this.params.user_name;

	if( this.params.depend_mask )
		this.elDependMask.innerHTML = 'D(' + this.params.depend_mask + ') ';
	else
		this.elDependMask.innerHTML = '';

	if( this.params.depend_mask_global )
		this.elDependMaskGlobal.innerHTML = 'G(' + this.params.depend_mask_global + ') ';
	else
		this.elDependMaskGlobal.innerHTML = '';

	if( this.params.hosts_mask )
		this.elHostsMask.innerHTML = 'H(' + this.params.hosts_mask + ') ';
	else
		this.elHostsMask.innerHTML = '';

	if( this.params.hosts_mask_exclude )
		this.elHostsMaskExclude.innerHTML = 'E(' + this.params.hosts_mask_exclude + ') ';
	else
		this.elHostsMaskExclude.innerHTML = '';

	if( this.params.annotation )
		this.elAnnotation.innerHTML = this.params.annotation;
	else
		this.elAnnotation.innerHTML = '';

	for( var b = 0; b < this.params.blocks.length; b++)
	{
		this.blocks[b].params = this.params.blocks[b];
		this.blocks[b].update( displayFull);
	}

	this.refresh();
}

JobNode.prototype.refresh = function()
{
	var time = this.params.time_started;
	if( time )
	{
		if( this.elState.DON == true )
			time = cm_TimeStringInterval( this.params.time_started, this.params.time_done )
		else
			time = cm_TimeStringInterval( time);
		this.elTime.innerHTML = time;
	}
	else
		this.elTime.innerHTML = '';
}

JobNode.prototype.onDoubleClick = function()
{
	g_OpenTasks( this.params.id );
}

function JobBlock( i_elParent, i_block)
{
	this.params = i_block;

	this.tasks_num = this.params.tasks_num;

	this.elRoot = document.createElement('div');
	i_elParent.appendChild( this.elRoot);

	this.service = this.params.service;
	this.elIcon = document.createElement('img');
	this.elRoot.appendChild( this.elIcon);
	this.elIcon.src = 'icons/software/'+this.service+'.png';
	this.elIcon.style.position = 'absolute';
//	this.elIcon.classList.add('icon');

	this.element = document.createElement('div');
	this.elRoot.appendChild( this.element);
	this.element.classList.add('jobblock');

	this.elTasks = document.createElement('span');
	this.element.appendChild( this.elTasks);
	var tasks = 't' + this.tasks_num;
	var tasks_title = 'Block tasks:'
	if( this.params.numeric )
	{
		tasks_title += ' Numeric:';
		tasks += '(' + this.params.frame_first + '-' + this.params.frame_last;
		tasks_title += ' from ' + this.params.frame_first + ' to ' + this.params.frame_last;
		if( this.params.frames_per_task > 1 )
		{
			tasks += ':' + this.params.frames_per_task;
			tasks_title += ' per ' + this.params.frames_per_task;
		}
		if( this.params.frames_inc > 1 )
		{
			tasks += '/' + this.params.frames_inc;
			tasks_title += ' by ' + this.params.frames_inc;
		}
		tasks += ')';
		tasks_title += '.';
	}
	else
	{
		tasks_title += ' Not numeric.';
	}
	if( this.params.non_sequential )
	{
		tasks += '*';
		tasks_title += '\nNon-sequential solving.';
	}
	tasks += ': ';
	this.elTasks.innerHTML = tasks;
	this.elTasks.title = tasks_title;

	this.elName = document.createElement('span');
	this.element.appendChild( this.elName);
	this.elName.title = 'Block name';

	this.elDepends = document.createElement('span');
	this.element.appendChild( this.elDepends);

	this.elProperties = document.createElement('span');
	this.element.appendChild( this.elProperties);
	this.elProperties.style.cssFloat = 'right';

//	this.element.appendChild( document.createElement('br'));
//	this.displayFull = false;
}


JobBlock.prototype.constructFull = function()
{
	this.elIcon.style.width = '48px';
	this.elIcon.style.height = '48px';
	this.element.style.marginLeft = '54px';

	this.elFull = document.createElement('div');
	this.element.appendChild( this.elFull);

	this.elFull.appendChild( document.createElement('br'));

	this.elPercentage = document.createElement('span');
	this.elFull.appendChild( this.elPercentage);

	this.elRunTime = document.createElement('span');
	this.elFull.appendChild( this.elRunTime);
}

JobBlock.prototype.constructBrief = function()
{
	this.elIcon.style.width = '20px';
	this.elIcon.style.height = '20px';
	this.element.style.marginLeft = '24px';

	if( this.elFull)
		this.element.removeChild( this.elFull);
	this.elFull = null;
}

JobBlock.prototype.update = function( i_displayFull)
{
	if( this.displayFull != i_displayFull )
	{
		if( i_displayFull )
			this.constructFull();
		else
			this.constructBrief();
	}
	this.displayFull = i_displayFull;

	if( this.params.name )
	{
		this.elName.innerHTML = this.params.name;

		if( this.service != this.params.service )
		{
			this.service = this.params.service;
			this.elIcon.src = 'icons/software/'+this.service+'.png';
		}
		this.elIcon.title = this.service;
//info(this.service);

		var deps = '';
		var deps_title = ''
		if( this.params.depend_mask )
		{
			deps += ' [' + this.params.depend_mask + ']';
			if( deps_title.length ) deps_title += '\n';
			deps_title += 'Depend mask = \"' + this.params.depend_mask + '\".'
		}
		if( this.params.tasks_depend_mask )
		{
			deps += ' T[' + this.params.tasks_depend_mask + ']';
			if( deps_title.length ) deps_title += '\n';
			deps_title += 'Tasks depend mask = \"' + this.params.tasks_depend_mask + '\".'
		}
		if( this.params.depend_sub_task )
		{
			deps += ' [SUB]';
			if( deps_title.length ) deps_title += '\n';
			deps_title += 'Subtasks depend.'
		}
		this.elDepends.innerHTML = deps;
		this.elDepends.title = deps_title;

		var props = '';
		props += '[' + this.params.capacity + ']';
		this.elProperties.innerHTML = props;
	}

	if( this.displayFull )
	{
		var percentage = 0;
		if( this.params.p_percentage ) percentage = this.params.p_percentage;
		this.elPercentage.innerHTML = percentage + '%';

		var tasks_done = 0;
		if( this.params.p_tasksdone ) tasks_done = this.params.p_tasksdone;

		if( this.params.p_taskssumruntime && tasks_done )
		{
			var rt = 'RT: S' + cm_TimeStringFromSeconds( this.params.p_taskssumruntime);
			var done
			var avg = cm_TimeStringFromSeconds( Math.round( this.params.p_taskssumruntime / tasks_done));
			rt += '/A' + avg;
			this.elRunTime.innerHTML = rt;
		}
		else
		{
			this.elRunTime.innerHTML = '';
			this.elRunTime.title = '';
		}
	}
}

