function JobNode() {}

Block_ProgressBarLength = 128;
Block_ProgressBarHeight =  10;

BarDONrgb = '#363';
BarSKPrgb = '#444';
BarDWRrgb = '#141';
BarWDPrgb = '#A2A';
BarRUNrgb = '#FF0';
BarRWRrgb = '#FA0';
BarERRrgb = '#F00';

JobNode.prototype.init = function() 
{
	this.element.classList.add('job');

	cm_CreateStart( this);

	this.elName = document.createElement('span');
	this.elName.classList.add('name');
	this.element.appendChild( this.elName);
	this.elName.classList.add('prestar');

	this.elUserName = cm_ElCreateFloatText( this.element,	'right', 'User Name');

	this.element.appendChild( document.createElement('br'));

	this.elState = document.createElement('span');
	this.element.appendChild( this.elState);
	this.elState.title = 'Job State';
	this.elState.classList.add('prestar');

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

	this.state = {};
	this.running_tasks = 0;
	this.percentage = 0;
}

JobNode.prototype.update = function( i_obj)
{
	if( i_obj ) this.params = i_obj;

	if( this.params.user_list_order != null )
		this.params.order = this.params.user_list_order;

	cm_GetState( this.params.state, this.state, this.element);
	if( this.params.state == null ) this.params.state = '';
	this.elState.textContent = this.params.state;

	var displayFull = false;
	if( this.state.ERR || this.state.RUN || this.state.SKP ||
	  ((this.state.DON == false) && (this.params.time_started > 0 )))
		displayFull = true;

	this.elName.textContent = this.params.name;
	this.elName.title = 'ID = '+this.params.id;
	this.elPriority.textContent = 'P' + this.params.priority;
	this.elUserName.textContent = this.params.user_name;

	if( this.params.time_life )
		this.elLifeTime.textContent = 'L' + cm_TimeStringFromSeconds( this.params.time_life);
	else this.elLifeTime.textContent = '';

	if( this.params.depend_mask )
		this.elDependMask.textContent = 'D(' + this.params.depend_mask + ')';
	else this.elDependMask.textContent = '';

	if( this.params.depend_mask_global )
		this.elDependMaskGlobal.textContent = 'G(' + this.params.depend_mask_global + ')';
	else this.elDependMaskGlobal.textContent = '';

	if( this.params.hosts_mask )
		this.elHostsMask.textContent = 'H(' + this.params.hosts_mask + ')';
	else this.elHostsMask.textContent = '';

	if( this.params.hosts_mask_exclude )
		this.elHostsMaskExclude.textContent = 'E(' + this.params.hosts_mask_exclude + ')';
	else this.elHostsMaskExclude.textContent = '';

	if( this.params.max_running_tasks != null )
		this.elMaxRunTasks.textContent = 'Max' + this.params.max_running_tasks;
	else this.elMaxRunTasks.textContent = '';

	if( this.params.max_running_tasks_per_host != null )
		this.elMaxRunTasksPH.textContent = 'MPH' + this.params.max_running_tasks_per_host;
	else this.elMaxRunTasksPH.textContent = '';

	if( this.params.need_properties )
		this.elNeedProperties.textContent = this.params.need_properties;
	else this.elNeedProperties.textContent = '';

	if( this.params.need_os)
		this.elNeedOS.textContent = this.params.need_os;
	else this.elNeedOS.textContent = '';

	if( this.params.annotation )
		this.elAnnotation.textContent = this.params.annotation;
	else this.elAnnotation.textContent = '';

	this.running_tasks = 0;
	this.percentage = 0;
	for( var b = 0; b < this.params.blocks.length; b++)
	{
		this.blocks[b].params = this.params.blocks[b];
		this.blocks[b].update( displayFull);
		if( this.blocks[b].params.running_tasks_counter )
			this.running_tasks += this.blocks[b].params.running_tasks_counter;
		if( this.blocks[b].params.p_percentage )
			this.percentage += Math.round( this.blocks[b].params.p_percentage/this.params.blocks.length);
	}

	if( this.running_tasks )
	{
		this.elStar.style.display = 'block';
		this.elStarCount.textContent = this.running_tasks;
	}
	else
		this.elStar.style.display = 'none';

	this.refresh();
}

JobNode.prototype.refresh = function()
{
	var time = this.params.time_wait;
	if( time && this.state.WTM )
	{
		time = cm_TimeStringInterval( new Date().getTime()/1000, time);
		this.elTime.textContent = time;
		return;
	}

	time = this.params.time_started;
	if( time )
	{
		if( this.state.DON == true )
			time = cm_TimeStringInterval( this.params.time_started, this.params.time_done )
		else
			time = cm_TimeStringInterval( time);
		this.elTime.textContent = time;
		return;
	}

	this.elTime.textContent = '';
}

JobNode.prototype.onDoubleClick = function() { g_OpenTasks( this.params.name, this.params.id );}
JobNode.prototype.menuHandleShowObj = function() { g_ShowObject( this.params );}
JobNode.prototype.menuHandleMove = function( i_name)
{
	if( g_uid < 1 )
	{
		g_Error('Can`t move nodes for uid < 1');
		return;
	}
	var operation = {};
	operation.type = i_name;
	operation.jids = this.monitor.getSelectedIds();
	nw_Action('users', [g_uid], operation);
	this.monitor.info('Moving Jobs');
}

function JobBlock( i_elParent, i_block)
{
	this.params = i_block;
	this.job = i_elParent.item;

	this.tasks_num = this.params.tasks_num;

	this.elRoot = document.createElement('div');
	i_elParent.appendChild( this.elRoot);
	this.elRoot.style.clear = 'both';
	this.elRoot.block = this;
	this.elRoot.oncontextmenu = function(e){ return e.currentTarget.block.onContextMenu(e);}

	this.service = this.params.service;
	this.elIcon = document.createElement('img');
	this.elRoot.appendChild( this.elIcon);
	this.elIcon.src = 'icons/software/'+this.service+'.png';
	this.elIcon.style.position = 'absolute';
//	this.elIcon.classList.add('icon');

	this.element = document.createElement('div');
	this.elRoot.appendChild( this.element);
	this.element.classList.add('jobblock');

	this.elTasks = cm_ElCreateText( this.element);
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
	tasks += ':';
	this.elTasks.textContent = tasks;
	this.elTasks.title = tasks_title;

	this.elNonSeq = cm_ElCreateText( this.element, 'Non-Sequential Tasks Running');
	this.elNonSeq.textContent = '(N-S)';

	this.elName = cm_ElCreateText( this.element, 'Block Name');
	this.elDepends = cm_ElCreateText( this.element);

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

JobBlock.prototype.onContextMenu = function( evt)
{
	evt.stopPropagation();

	g_cur_monitor = this.job.monitor;

	if( this.job.monitor.menu ) this.job.monitor.menu.destroy();
	this.element.classList.add('selected');

	var onlyBlockIsSelected = false;
	if(( this.job.element.selected == null ) || ( this.job.element.selected == false ))
	{
		this.job.monitor.selectAll( false);
		onlyBlockIsSelected = true;
	}

	var menu = new cgru_Menu( document, document.body, evt, this, 'jobblock_context', 'onContextMenuDestroy');
	this.job.monitor.menu = menu;

	if( onlyBlockIsSelected )
		menu.addItem( null, null, null, '<b>'+this.params.name+'</b>', false);
	else
		menu.addItem( null, null, null, '<b>All Blocks</b>', false);
	menu.addItem();

	var actions = JobBlock.actions;
	for( var i = 0; i < actions.length; i++)
		menu.addItem( actions[i][1], this, actions[i][3], actions[i][4]);
	menu.show();

	return false;
}
JobBlock.prototype.onContextMenuDestroy = function()
{
	this.element.classList.remove('selected');
	this.job.monitor.menu = null;
}
JobBlock.prototype.menuHandleDialog = function( i_parameter)
{
	var ptype = null;
	var actions = JobBlock.actions;
	for( var i = 0; i < actions.length; i++)
		if( i_parameter == actions[i][1])
			ptype = actions[i][2];
	new cgru_Dialog( this.job.monitor.window, this, 'setParameter', i_parameter, ptype, this.params[i_parameter], 'jobblock_parameter');
}
JobBlock.prototype.setParameter = function( i_parameter, i_value)
{
	var params = {};
	params[i_parameter] = i_value;
g_Info( this.job.params.name+'['+this.params.name+'].'+i_parameter+' = ' + i_value);
	this.action( null, params);
}
JobBlock.prototype.action = function( i_operation, i_params)
{
	var jids = this.job.monitor.getSelectedIds();
	var bids = [-1];
	if( jids.length == 0 )
	{
		jids = [this.job.params.id];
		bids = [this.params.block_num];
	}
	nw_Action('jobs', jids, i_operation, i_params, bids);
}

JobBlock.prototype.constructFull = function()
{
	this.elIcon.style.width = '48px';
	this.elIcon.style.height = '48px';
	this.elIcon.style.marginTop = '4px';
	this.element.style.marginLeft = '54px';

	this.elFull = document.createElement('div');
	this.element.appendChild( this.elFull);
	this.elFull.style.clear = 'both';

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
	this.elTasksErr.style.display = 'none';

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
		this.elName.textContent = this.params.name;

		this.elNonSeq.style.display = this.params.non_sequential ? 'inline':'none';
//		if( this.params.non_sequential ) this.elNonSeq.style.display = 'none';
//		else this.elNonSeq.textContent = '';

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
		this.elDepends.textContent = deps;
		this.elDepends.title = deps_title;

		this.elCapacity.textContent = '[' + this.params.capacity + ']';

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

			errTxt += ',' + eth + 'T';
			errTit += '\nAvoid Task Same Host: ' + eth;
			if( eth == 0 ) errTit = ' (unlimited)';
			else if( eth == -1 ) errTit = ' (user settings used)';

			errTxt += ',' + ert + 'R';
			errTit += '\nTask Errors Retries: ' + ert;
			if( ert == 0 ) errTit = ' (unlimited)';
			else if( ert == -1 ) errTit = ' (user settings used)';
		}
		this.elErrSolving.textContent = errTxt;
		this.elErrSolving.title = errTit;

		if(( this.params.errors_forgive_time != null ) && ( this.params.errors_forgive_time >= 0 ))
			this.elForgiveTime.textContent = 'F'+cm_TimeStringFromSeconds( this.params.errors_forgive_time);
		else this.elForgiveTime.textContent = '';

		if( this.params.tasks_max_run_time != null )
			this.elMaxRunTime.textContent = 'MRT'+cm_TimeStringFromSeconds( this.params.tasks_max_run_time);
		else this.elMaxRunTime.textContent = '';

		if( this.params.max_running_tasks != null )
			this.elMaxRunTasks.textContent = 'Max'+this.params.max_running_tasks;
		else this.elMaxRunTasks.textContent = '';

		if( this.params.max_running_tasks_per_host != null )
			this.elMaxRunTasksPH.textContent = 'MPH'+this.params.max_running_tasks_per_host;
		else this.elMaxRunTasksPH.textContent = '';

		if( this.params.hosts_mask)
			this.elHostsMask.textContent = 'H('+this.params.hosts_mask+')';
		else this.elHostsMask.textContent = '';

		if( this.params.hosts_mask_exclude)
			this.elHostsMaskExclude.textContent = 'E('+this.params.hosts_mask_exclude+')';
		else this.elHostsMaskExclude.textContent = '';

		if( this.params.need_memory)
			this.elNeedMem.textContent = 'RAM'+this.params.need_memory;
		else this.elNeedMem.textContent = '';

		if( this.params.need_hdd)
			this.elNeedHDD.textContent = 'HDD'+this.params.need_hdd;
		else this.elNeedHDD.textContent = '';

		if( this.params.need_power)
			this.elNeedPower.textContent = 'Pow'+this.params.need_power;
		else this.elNeedPower.textContent = '';

		if( this.params.need_properties)
			this.elNeedProperties.textContent = this.params.need_properties;
		else this.elNeedProperties.textContent = '';
	}

	if( this.displayFull )
	{
		var percentage = 0;
		if( this.params.p_percentage ) percentage = this.params.p_percentage;
		this.elPercentage.textContent = percentage + '%';

		var tasks_done = 0;
		if( this.params.p_tasks_done ) tasks_done = this.params.p_tasks_done;
		this.elTasksDon.textContent = 'don:'+tasks_done;

		var tasks_rdy = 0;
		if( this.params.p_tasks_ready ) tasks_rdy = this.params.p_tasks_ready;
		this.elTasksRdy.textContent = 'rdy:'+tasks_rdy;

		var tasks_run = 0;
		if( this.params.running_tasks_counter )
		{
			tasks_run = this.params.running_tasks_counter;
			this.elTasksRun.textContent = 'run:'+tasks_run;
		}
		else this.elTasksRun.textContent = '';

		var tasks_err = 0;
		if( this.params.p_tasks_error )
		{
			tasks_err = this.params.p_tasks_error;
			this.elTasksErr.textContent = 'err:'+tasks_err;
			this.elTasksErr.style.display = 'inline';
		}
		else
		{
			this.elTasksErr.textContent = '';
			this.elTasksErr.style.display = 'none';
		}

		var tasks_skp = 0;
		if( this.params.p_tasks_skipped )
		{
			tasks_skp = this.params.p_tasks_skipped;
			this.elTasksSkp.textContent = 'skp:'+tasks_skp;
		}
		else this.elTasksSkp.textContent = '';

		var tasks_wrn = 0;
		if( this.params.p_tasks_warning )
		{
			tasks_wrn = this.params.p_tasks_warning;
			this.elTasksWrn.textContent = 'wrn:'+tasks_wrn;
		}
		else this.elTasksWrn.textContent = '';

		if( this.params.p_tasks_run_time && tasks_done )
		{
			var sum = cm_TimeStringFromSeconds( this.params.p_tasks_run_time);
			var avg = cm_TimeStringFromSeconds( Math.round( this.params.p_tasks_run_time / tasks_done));
			this.elRunTime.textContent = sum +'/'+avg;
			this.elRunTime.title = 'Running Time:\nTotal: '+sum+'\nAverage per task: '+avg;
		}
		else
		{
			this.elRunTime.textContent = '';
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
		this.elErrHosts.textContent = he_txt;
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

JobNode.actions = [];

JobNode.actions.push(['context', 'log',               null, 'menuHandleGet',       'Show Log']);
JobNode.actions.push(['context', 'error_hosts',       null, 'menuHandleGet',       'Show Error Hosts']);
JobNode.actions.push(['context', 'show_obj',          null, 'menuHandleShowObj',   'Show Object']);
JobNode.actions.push(['context',  null,               null,  null,                  null]);
JobNode.actions.push(['context', 'reset_error_hosts', null, 'menuHandleOperation', 'Reset Error Hosts']);
JobNode.actions.push(['context', 'restart_errors',    null, 'menuHandleOperation', 'Restart Errors']);
JobNode.actions.push(['context', 'restart_running',   null, 'menuHandleOperation', 'Restart Running']);
JobNode.actions.push(['context',  null,               null,  null,                  null]);
JobNode.actions.push(['context', 'move_jobs_up',      null, 'menuHandleMove',      'Move Up',     'user']);
JobNode.actions.push(['context', 'move_jobs_down',    null, 'menuHandleMove',      'Move Down',   'user']);
JobNode.actions.push(['context', 'move_jobs_top',     null, 'menuHandleMove',      'Move Top',    'user']);
JobNode.actions.push(['context', 'move_jobs_bottom',  null, 'menuHandleMove',      'Move Bottom', 'user']);
JobNode.actions.push(['context',  null,               null,  null,                  null]);
JobNode.actions.push(['context', 'start',             null, 'menuHandleOperation', 'Start']);
JobNode.actions.push(['context', 'pause',             null, 'menuHandleOperation', 'Pause']);
JobNode.actions.push(['context', 'stop',              null, 'menuHandleOperation', 'Stop']);
JobNode.actions.push(['context', 'restart',           null, 'menuHandleOperation', 'Restart']);
JobNode.actions.push(['context', 'restart_pause',     null, 'menuHandleOperation', 'Restart&Pause']);
JobNode.actions.push(['context', 'delete',            null, 'menuHandleOperation', 'Delete']);

JobNode.actions.push(['set', 'annotation',                 'str', 'menuHandleDialog', 'Annotation']);
JobNode.actions.push(['set', 'depend_mask',                'reg', 'menuHandleDialog', 'Depend Mask']);
JobNode.actions.push(['set', 'depend_mask_global',         'reg', 'menuHandleDialog', 'Global Depend Mask']);
JobNode.actions.push(['set', 'max_running_tasks',          'num', 'menuHandleDialog', 'Max Runnig Tasks']);
JobNode.actions.push(['set', 'max_running_tasks_per_host', 'num', 'menuHandleDialog', 'Max Run Tasks Per Host']);
JobNode.actions.push(['set', 'hosts_mask',                 'reg', 'menuHandleDialog', 'Hosts Mask']);
JobNode.actions.push(['set', 'hosts_mask_exclude',         'reg', 'menuHandleDialog', 'Exclude Hosts Mask']);
JobNode.actions.push(['set', 'time_wait',                  'tim', 'menuHandleDialog', 'Time Wait']);
JobNode.actions.push(['set', 'priority',                   'num', 'menuHandleDialog', 'Priority']);
JobNode.actions.push(['set', 'need_os',                    'reg', 'menuHandleDialog', 'OS Needed']);
JobNode.actions.push(['set', 'need_properties',            'reg', 'menuHandleDialog', 'Need Properties']);
JobNode.actions.push(['set', 'time_life',                  'hrs', 'menuHandleDialog', 'Life Time']);
JobNode.actions.push(['set', 'user_name',                  'str', 'menuHandleDialog', 'Owner', 'visor']);
JobNode.actions.push(['set',  null,                         null,  null,               null]);
JobNode.actions.push(['set', 'hidden',                     'bl1', 'menuHandleDialog', 'Hidden']);

JobBlock.actions = [];
JobBlock.actions.push(['set', 'capacity',                   'num', 'menuHandleDialog', 'Capacity']);
JobBlock.actions.push(['set',  null,                         null,  null,               null]);
JobBlock.actions.push(['set', 'errors_retries',             'num', 'menuHandleDialog', 'Errors Retries']);
JobBlock.actions.push(['set', 'errors_avoid_host',          'num', 'menuHandleDialog', 'Errors Avoid Host']);
JobBlock.actions.push(['set', 'errors_task_same_host',      'num', 'menuHandleDialog', 'Errors Task Same Host']);
JobBlock.actions.push(['set', 'errors_forgive_time',        'hrs', 'menuHandleDialog', 'Errors Forgive Time']);
JobBlock.actions.push(['set', 'tasks_max_run_time',         'hrs', 'menuHandleDialog', 'Tasks Max Run Time']);
JobBlock.actions.push(['set',  null,                         null,  null,               null]);
JobBlock.actions.push(['set', 'non_sequential',             'bl1', 'menuHandleDialog', 'Non-Sequential']);
JobBlock.actions.push(['set',  null,                         null,  null,               null]);
JobBlock.actions.push(['set', 'max_running_tasks',          'num', 'menuHandleDialog', 'Max Runnig Tasks']);
JobBlock.actions.push(['set', 'max_running_tasks_per_host', 'num', 'menuHandleDialog', 'Max Run Tasks Per Host']);
JobBlock.actions.push(['set', 'hosts_mask',                 'reg', 'menuHandleDialog', 'Hosts Mask']);
JobBlock.actions.push(['set', 'hosts_mask_exclude',         'reg', 'menuHandleDialog', 'Exclude Hosts Mask']);
JobBlock.actions.push(['set', 'depend_mask',                'reg', 'menuHandleDialog', 'Depend Mask']);
JobBlock.actions.push(['set', 'tasks_depend_mask',          'reg', 'menuHandleDialog', 'Tasks Depend Mask']);
JobBlock.actions.push(['set', 'need_properties',            'reg', 'menuHandleDialog', 'Properties Needed']);

JobNode.sortVisor = 'time_creation';
JobNode.sort = ['order','time_creation','priority','user_name','name','host_name'];
JobNode.filter = ['name','host_name','user_name'];
JobNode.filterVisor = 'user_name';

