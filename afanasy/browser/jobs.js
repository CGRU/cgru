function JobNode() {}

Block_ProgressBarLength = 128;
Block_ProgressBarHeight =  10;

BarDONrgb = '#363';
BarSKPrgb = '#266';
BarDWRrgb = '#141';
BarWDPrgb = '#A2A';
BarRUNrgb = '#FF0';
BarRWRrgb = '#FA0';
BarERRrgb = '#F00';

JobNode.prototype.init = function() 
{
	this.element.classList.add('job');

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild( this.elName);
	this.elName.title = 'Job Name';

	this.elUserName = cm_ElCreateFloatText( this.element,	'right', 'User Name');

	this.element.appendChild( document.createElement('br'));

	this.elState = document.createElement('span');
	this.element.appendChild( this.elState);
	this.elState.title = 'Job State';

	this.elTime = cm_ElCreateFloatText( this.element, 'right', 'Running Time');
	this.elLifeTime = cm_ElCreateFloatText( this.element, 'right', 'Life Time');
	this.elPriority = cm_ElCreateFloatText( this.element, 'right', 'Priority');
	this.elDependMask = cm_ElCreateFloatText( this.element, 'right', 'Depend Mask');
	this.elDependMaskGlobal = cm_ElCreateFloatText( this.element, 'right', 'Global Depend Mask');
	this.elHostsMask = cm_ElCreateFloatText( this.element, 'right', 'Hosts Mask');
	this.elHostsMaskExclude = cm_ElCreateFloatText( this.element, 'right', 'Exclude Hosts Mask');
	this.elMaxRunTasks = cm_ElCreateFloatText( this.element, 'right', 'Maximum Running Tasks');
	this.elMaxRunTasksPH = cm_ElCreateFloatText( this.element, 'right', 'Maximum Running Tasks Per Host');
	this.elNeedProperties = cm_ElCreateFloatText( this.element, 'right', 'Properties');
	this.elNeedOS = cm_ElCreateFloatText( this.element, 'right', 'OS Needed');
	

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

	if( this.params.time_life )
		this.elLifeTime.innerHTML = 'L' + cm_TimeStringFromSeconds( this.params.time_life);
	else this.elLifeTime.innerHTML = '';

	if( this.params.depend_mask )
		this.elDependMask.innerHTML = 'D(' + this.params.depend_mask + ')';
	else this.elDependMask.innerHTML = '';

	if( this.params.depend_mask_global )
		this.elDependMaskGlobal.innerHTML = 'G(' + this.params.depend_mask_global + ')';
	else this.elDependMaskGlobal.innerHTML = '';

	if( this.params.hosts_mask )
		this.elHostsMask.innerHTML = 'H(' + this.params.hosts_mask + ')';
	else this.elHostsMask.innerHTML = '';

	if( this.params.hosts_mask_exclude )
		this.elHostsMaskExclude.innerHTML = 'E(' + this.params.hosts_mask_exclude + ')';
	else this.elHostsMaskExclude.innerHTML = '';

	if( this.params.max_running_tasks != null )
		this.elMaxRunTasks.innerHTML = 'Max' + this.params.max_running_tasks;
	else this.elMaxRunTasks.innerHTML = '';

	if( this.params.max_running_tasks_per_host != null )
		this.elMaxRunTasksPH.innerHTML = 'MPH' + this.params.max_running_tasks_per_host;
	else this.elMaxRunTasksPH.innerHTML = '';

	if( this.params.need_properties )
		this.elNeedProperties.innerHTML = this.params.need_properties;
	else this.elNeedProperties.innerHTML = '';

	if( this.params.need_os)
		this.elNeedOS.innerHTML = this.params.need_os;
	else this.elNeedOS.innerHTML = '';

	if( this.params.annotation )
		this.elAnnotation.innerHTML = this.params.annotation;
	else this.elAnnotation.innerHTML = '';

	for( var b = 0; b < this.params.blocks.length; b++)
	{
		this.blocks[b].params = this.params.blocks[b];
		this.blocks[b].update( displayFull);
	}

	this.refresh();
}

JobNode.prototype.refresh = function()
{
	var time = this.params.time_wait;
	if( time && this.elState.WTM )
	{
		time = cm_TimeStringInterval( new Date().getTime()/1000, time);
		this.elTime.innerHTML = time;
		return;
	}

	time = this.params.time_started;
	if( time )
	{
		if( this.elState.DON == true )
			time = cm_TimeStringInterval( this.params.time_started, this.params.time_done )
		else
			time = cm_TimeStringInterval( time);
		this.elTime.innerHTML = time;
		return;
	}

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

	this.elCapacity = cm_ElCreateFloatText( this.element, 'right', 'Tasks Capacity');
	this.elErrSolving = cm_ElCreateFloatText( this.element, 'right');
	this.elForgiveTime = cm_ElCreateFloatText( this.element, 'right', 'Errors Forgive Time');
	this.elMaxRunTime = cm_ElCreateFloatText( this.element, 'right', 'Task Maximum Run Time');
	this.elMaxRunTasks = cm_ElCreateFloatText( this.element, 'right', 'Maximum Running Tasks');
	this.elMaxRunTasksPH = cm_ElCreateFloatText( this.element, 'right', 'Maximum Running Tasks Per Host');
	this.elHostsMask = cm_ElCreateFloatText( this.element, 'right', 'Hosts Mask');
	this.elHostsMaskExclude = cm_ElCreateFloatText( this.element, 'right', 'Exclude Hosts Mask');
	this.elNeedMem = cm_ElCreateFloatText( this.element, 'right', 'Required Memory');
	this.elNeedHDD = cm_ElCreateFloatText( this.element, 'right', 'Required HDD Space');
	this.elNeedPower = cm_ElCreateFloatText( this.element, 'right', 'Needed Power');
	this.elNeedProperties = cm_ElCreateFloatText( this.element, 'right', 'Needed Properties');
}


JobBlock.prototype.constructFull = function()
{
	this.elIcon.style.width = '48px';
	this.elIcon.style.height = '48px';
	this.elIcon.style.marginTop = '4px';
	this.element.style.marginLeft = '54px';

	this.elFull = document.createElement('div');
	this.element.appendChild( this.elFull);

	this.elProgress = document.createElement('div');
	this.elFull.appendChild( this.elProgress);
	this.elProgress.classList.add('progress');

	this.elBarDone = document.createElement('div');
	this.elProgress.appendChild( this.elBarDone);
	this.elBarDone.classList.add('bar');
	this.elBarDone.classList.add('DON');
	this.elBarDone.style.height = '4px';
	this.elBarDone.style.cssFloat = 'left';

	this.elBarErr = document.createElement('div');
	this.elProgress.appendChild( this.elBarErr);
	this.elBarErr.classList.add('bar');
	this.elBarErr.classList.add('ERR');
	this.elBarErr.style.height = '4px';
	this.elBarErr.style.cssFloat = 'left';

	this.elBarRun = document.createElement('div');
	this.elProgress.appendChild( this.elBarRun);
	this.elBarRun.classList.add('bar');
	this.elBarRun.classList.add('RUN');
	this.elBarRun.style.height = '4px';
//	this.elBarRun.style.cssFloat = 'left';

	this.elBarPercentage = document.createElement('div');
	this.elProgress.appendChild( this.elBarPercentage);
	this.elBarPercentage.classList.add('bar');
	this.elBarPercentage.classList.add('DON');
	this.elBarPercentage.style.height = '4px';

	this.elCanvas = document.createElement('div');
	this.elProgress.appendChild( this.elCanvas);
	this.elCanvas.style.height = Block_ProgressBarHeight + 'px';
	this.elCanvas.style.width = '100%';

	this.canvas = document.createElement('canvas');
	this.elCanvas.appendChild( this.canvas);
	this.canvas.width  = Block_ProgressBarLength;
	this.canvas.height = Block_ProgressBarHeight;
	this.canvas.style.height = Block_ProgressBarHeight + 'px';
	this.canvas.style.width = '100%';

	this.elPercentage = cm_ElCreateText( this.elFull, 'Block Done Percentage');
	this.elTasksDon = cm_ElCreateText( this.elFull, 'Done Tasks Counter');
	this.elTasksRdy = cm_ElCreateText( this.elFull, 'Ready Tasks Counter');
	this.elTasksRun = cm_ElCreateText( this.elFull, 'Running Tasks Counter');
	this.elTasksSkp = cm_ElCreateText( this.elFull, 'Skipped Tasks Counter');
	this.elTasksWrn = cm_ElCreateText( this.elFull, 'Warning Tasks Counter');
	this.elTasksErr = cm_ElCreateText( this.elFull, 'Error Tasks Counter');

//	this.elTasksDon.classList.add('font-done');
//	this.elTasksRdy.classList.add('font-ready');
//	this.elTasksRun.classList.add('font-run');
//	this.elTasksErr.classList.add('font-error');
	this.elTasksErr.classList.add('ERR');

	this.elRunTime = cm_ElCreateFloatText( this.elFull, 'right');

	this.elErrHosts = cm_ElCreateFloatText( this.elFull, 'right');
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

		this.elCapacity.innerHTML = '[' + this.params.capacity + ']';

		var errTxt = '';
		var errTit = '';
		var eah = -1, eth = -1, ert = -1;
		if( this.params.errors_avoid_host ) eah = this.params.errors_avoid_host;
		if( this.params.errors_task_same_host) eth = this.params.errors_task_same_host;
		if( this.params.errors_retries) ert = this.params.errors_retries;
		if(( eah != -1 ) || ( eth != -1 ) || ( ert != -1 ))
		{
			errTxt = 'Err:';
			errTit = 'Errors Solving:';

			errTxt += eah + 'J';
			errTit += '\nAvoid Job Block: ' + eah;
			if( eah == 0 ) errTit = ' (unlimited)';
			else if( eah == -1 ) errTit = ' (user settings used)';

			errTxt += '-' + eth + 'T';
			errTit += '\nAvoid Task Same Host: ' + eth;
			if( eth == 0 ) errTit = ' (unlimited)';
			else if( eth == -1 ) errTit = ' (user settings used)';

			errTxt += '-' + ert + 'R';
			errTit += '\nTask Errors Retries: ' + ert;
			if( ert == 0 ) errTit = ' (unlimited)';
			else if( ert == -1 ) errTit = ' (user settings used)';
		}
		this.elErrSolving.innerHTML = errTxt;
		this.elErrSolving.title = errTit;

		if(( this.params.errors_forgive_time != null ) && ( this.params.errors_forgive_time >= 0 ))
			this.elForgiveTime.innerHTML = 'F'+cm_TimeStringFromSeconds( this.params.errors_forgive_time);
		else this.elForgiveTime.innerHTML = '';

		if( this.params.tasks_max_run_time != null )
			this.elMaxRunTime.innerHTML = 'MRT'+cm_TimeStringFromSeconds( this.params.tasks_max_run_time);
		else this.elMaxRunTime.innerHTML = '';

		if( this.params.max_running_tasks != null )
			this.elMaxRunTasks.innerHTML = 'Max'+this.params.max_running_tasks;
		else this.elMaxRunTasks.innerHTML = '';

		if( this.params.max_running_tasks_per_host != null )
			this.elMaxRunTasksPH.innerHTML = 'MPH'+this.params.max_running_tasks_per_host;
		else this.elMaxRunTasksPH.innerHTML = '';

		if( this.params.hosts_mask)
			this.elHostsMask.innerHTML = 'H('+this.params.hosts_mask+')';
		else this.elHostsMask.innerHTML = '';

		if( this.params.hosts_mask_exclude)
			this.elHostsMaskExclude.innerHTML = 'E('+this.params.hosts_mask_exclude+')';
		else this.elHostsMaskExclude.innerHTML = '';

		if( this.params.need_memory)
			this.elNeedMem.innerHTML = 'RAM'+this.params.need_memory;
		else this.elNeedMem.innerHTML = '';

		if( this.params.need_hdd)
			this.elNeedHDD.innerHTML = 'HDD'+this.params.need_hdd;
		else this.elNeedHDD.innerHTML = '';

		if( this.params.need_power)
			this.elNeedPower.innerHTML = 'Pow'+this.params.need_power;
		else this.elNeedPower.innerHTML = '';

		if( this.params.need_properties)
			this.elNeedProperties.innerHTML = this.params.need_properties;
		else this.elNeedProperties.innerHTML = '';
	}

	if( this.displayFull )
	{
		var percentage = 0;
		if( this.params.p_percentage ) percentage = this.params.p_percentage;
		this.elPercentage.innerHTML = percentage + '%';

		var tasks_done = 0;
		if( this.params.p_tasks_done ) tasks_done = this.params.p_tasks_done;
		this.elTasksDon.innerHTML = 'don:'+tasks_done;

		var tasks_rdy = 0;
		if( this.params.p_tasks_ready ) tasks_rdy = this.params.p_tasks_ready;
		this.elTasksRdy.innerHTML = 'rdy:'+tasks_rdy;

		var tasks_run = 0;
		if( this.params.running_tasks_counter )
		{
			tasks_run = this.params.running_tasks_counter;
			this.elTasksRun.innerHTML = 'run:'+tasks_run;
		}
		else this.elTasksRun.innerHTML = '';

		var tasks_err = 0;
		if( this.params.p_tasks_error )
		{
			tasks_err = this.params.p_tasks_error;
			this.elTasksErr.innerHTML = 'err:'+tasks_err;
		}
		else this.elTasksErr.innerHTML = '';

		var tasks_skp = 0;
		if( this.params.p_tasks_skipped )
		{
			tasks_skp = this.params.p_tasks_skipped;
			this.elTasksSkp.innerHTML = 'skp:'+tasks_skp;
		}
		else this.elTasksSkp.innerHTML = '';

		var tasks_wrn = 0;
		if( this.params.p_tasks_warning )
		{
			tasks_wrn = this.params.p_tasks_warning;
			this.elTasksWrn.innerHTML = 'wrn:'+tasks_wrn;
		}
		else this.elTasksWrn.innerHTML = '';

		if( this.params.p_tasks_run_time && tasks_done )
		{
			var sum = cm_TimeStringFromSeconds( this.params.p_tasks_run_time);
			var avg = cm_TimeStringFromSeconds( Math.round( this.params.p_tasks_run_time / tasks_done));
			this.elRunTime.innerHTML = sum +'/'+avg;
			this.elRunTime.title = 'Running Time:\nTotal: '+sum+'\nAverage per task: '+avg;
		}
		else
		{
			this.elRunTime.innerHTML = '';
			this.elRunTime.title = '';
		}

		var he_txt = '', he_tit = '';
		this.elErrHosts.classList.remove('ERR');
		if( this.params.p_error_hosts )
		{
			he_txt = 'Eh' + this.params.p_error_hosts;
			he_tit = 'Error Hosts: ' + this.params.p_error_hosts;
			if( this.params.p_avoid_hosts )
			{
				he_txt += ': ' + this.params.p_avoid_hosts + ' Avoid';
				he_tit += '\nAvoiding Hosts: ' + this.params.p_avoid_hosts;
				this.elErrHosts.classList.add('ERR');
			}
		}
		this.elErrHosts.innerHTML = he_txt;
		this.elErrHosts.title = he_tit;

		this.elBarPercentage.style.width = percentage + '%';
		this.elBarDone.style.width = Math.floor( 100 * tasks_done / this.tasks_num ) + '%';
		this.elBarErr.style.width = Math.ceil( 100 * tasks_err / this.tasks_num ) + '%';
		this.elBarRun.style.width = Math.ceil( 100 * tasks_run / this.tasks_num ) + '%';

		if( this.params.p_progressbar )
		{
			var ctx = this.canvas.getContext('2d');
			ctx.clearRect( 0, 0, Block_ProgressBarLength, Block_ProgressBarHeight);
			ctx.lineWidth = 1;
			ctx.lineCap = 'square';
			for( var i = 0; i < Block_ProgressBarLength; i++ )
			{
				var rgb = '#000';
				switch( this.params.p_progressbar.charAt(i) )
				{
					case 'r': continue; // RDY
					case 'D': rgb = BarDONrgb; break;
					case 'S': rgb = BarSKPrgb; break;// SKP
					case 'G': rgb = BarDWRrgb; break;// DON | WRN
					case 'W': rgb = BarWDPrgb; break;// WDP
					case 'R': rgb = BarRUNrgb; break;// RUN
					case 'N': rgb = BarRWRrgb; break;// RUN | WRN
					case 'E': rgb = BarERRrgb; break;// ERR
				}
				ctx.strokeStyle = rgb;
				ctx.beginPath();
				ctx.moveTo( i+.5, 0);
				ctx.lineTo( i+.5, Block_ProgressBarHeight);
				ctx.stroke();
			}
		}
	}
}

JobNode.prototype.onContexMenu = function( i_evt)
{
	var menu = new cgru_Menu( document.body, i_evt, this, 'jobitem_context');
	menu.addItem('Show Log...', 'log');
	menu.addItem('Reset Error Hosts', 'reset_error_hosts');
	menu.addItem('Restart Errors', 'restart_errors', this.elState.ERR);
	menu.addSeparator();
	menu.addItem('Start', 'start');
	menu.addItem('Pause', 'pause');
	menu.addItem('Stop', 'stop');
	menu.addItem('Restart', 'restart');
	menu.addItem('Restart&Pause', 'restart_pause');
	menu.addItem('Delete', 'delete');
	menu.show();
}

JobNode.prototype.openMenuSet = function( i_evt)
{
	var menu = new cgru_Menu( document.body, i_evt, this, 'jobs_menuset');
	menu.addItem('Annotation','annotation');
	menu.show();
}

JobNode.prototype.action = function( i_name)
{
	g_Info('action = ' + i_name);	
	if( i_name == 'annotation')
	{
		var dialog = new cgru_Dialog( document, document.body, this, 'jobs_parameter', 'Set Parameter', 'Enter New Value');
	}
	else if( i_name == 'log')
	{
		nw_GetNodes('jobs', [this.monitor.cur_item.params.id], i_name);
	}
	else
		this.monitor.action( i_name);
}

