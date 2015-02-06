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

	this.elUserName = cm_ElCreateFloatText( this.element,'right','User Name');
	this.elETA = cm_ElCreateFloatText( this.element,'right','ETA:');

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

	this.elThumbs = document.createElement('div');
	this.element.appendChild( this.elThumbs);
	this.elThumbs.classList.add('thumbnails');
	this.elThumbs.style.display = 'none';

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

	if( this.params.thumb_path )
		this.showThumb( this.params.thumb_path );

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
	var time_txt = '';
	var time_tip = '';
	var eta = '';

	if( this.params.time_wait && this.state.WTM )
	{
		time_txt = cm_TimeStringInterval( new Date().getTime()/1000, this.params.time_wait);
		time_tip = 'Waiting for: ' + cm_DateTimeStrFromSec( this.params.time_wait);
	}
	else if( this.params.time_started )
	{
		if( this.state.DON == true )
		{
			time_txt = cm_TimeStringInterval( this.params.time_started, this.params.time_done )
			time_tip = 'Done at: ' + cm_DateTimeStrFromSec( this.params.time_done);
			time_tip += '\nRunning time: ' + time_txt;
		}
		else
		{
			time_txt = cm_TimeStringInterval( this.params.time_started);

			// ETA (but not for the system job which id == 1):
			if( this.params.id > 1 )
			{
				var percentage = 0;
				for( var b = 0; b < this.blocks.length; b++)
				{
					if( this.blocks[b].params.p_percentage )
						percentage += this.blocks[b].params.p_percentage;
				}
				percentage /= this.blocks.length;
				if(( percentage > 0 ) && ( percentage < 100 ))
				{
					var sec_now = (new Date()).valueOf() / 1000;
					var sec_run = sec_now - this.params.time_started;
					var sec_all = sec_run * 100.0 / percentage;
					eta = sec_all - sec_run;
					if( eta > 0 )
					{
						eta = cm_TimeStringInterval( 0, eta);
						eta = 'ETA≈' + eta;
					}
				}
			}
		}
		time_tip = 'Started at: ' + cm_DateTimeStrFromSec( this.params.time_started) + '\n' + time_tip;
	}

	time_tip = 'Created at: ' + cm_DateTimeStrFromSec( this.params.time_creation) + '\n' + time_tip;

	this.elTime.textContent = time_txt;
	this.elTime.title = time_tip;
	this.elETA.textContent = eta;
}

JobNode.prototype.onDoubleClick = function( i_evt)
{
	g_OpenMonitor({"type":'tasks',"evt":i_evt,"id":this.params.id,"name":this.params.name,"wnd":this.monitor.window});
}

JobNode.prototype.mh_Show = function( i_param, i_evt )
{
	g_ShowObject({"object":this.params},{"evt":i_evt,"wnd":this.monitor.window});
}
JobNode.prototype.mh_Move = function( i_param)
{
	if( g_uid < 1 )
	{
		g_Error('Can`t move nodes for uid < 1');
		return;
	}
	var operation = {};
	operation.type = i_param.name;
	operation.jids = this.monitor.getSelectedIds();
	nw_Action('users', [g_uid], operation);
	this.monitor.info('Moving Jobs');
}

JobNode.prototype.showThumb = function( i_path)
{
//console.log('JobNode.prototype.showThumb = '+i_path);
	if( this.elThumbs.m_divs == null )
	{
		this.elThumbs.style.display = 'block';
		this.elThumbs.m_divs = [];
	}

	// Do nothing if the last image is the same
	if( this.elThumbs.m_divs.length )
		if( this.elThumbs.m_divs[this.elThumbs.m_divs.length-1].m_path == i_path )
			return;

	var label = cm_PathBase( i_path).replace(/\.jpg$/,'');

	var thumb = document.createElement('div')
	this.elThumbs.appendChild( thumb);
	this.elThumbs.m_divs.push( thumb);
	thumb.m_path = i_path;
	thumb.title = label;

	var name = document.createElement('div');
	name.textContent = label;
	thumb.appendChild( name);

	if( i_path.lastIndexOf('.jpg') == (i_path.length - 4))
	{
		var img = document.createElement('img');
		thumb.appendChild( img);
		img.src = '@TMP@' + i_path;
		img.style.display = 'none';
		img.m_height = this.monitor.options.jobs_thumbs_height;
		img.onload = function( e) {
			var img = e.currentTarget;
			if( img.height != img.m_height )
			{
				img.width = img.m_height * img.width / img.height;
				img.height = img.m_height;
			}
			img.style.display = 'block';
		}
	}
	else
		name.style.position = 'relative';

	if( this.elThumbs.m_divs.length > this.monitor.options.jobs_thumbs_num )
	{
		this.elThumbs.removeChild( this.elThumbs.m_divs[0]);
		this.elThumbs.m_divs.splice( 0, 1);
	}
}

// ###################### BLOCK ###################################

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
	this.elRunTime = cm_ElCreateFloatText( this.element, 'right');
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

	var menu = new cgru_Menu({"parent":document.body,"evt":evt,"name":'jobblock_context',"receiver":this,"destroy":'onContextMenuDestroy'});
	this.job.monitor.menu = menu;

	if( onlyBlockIsSelected )
		menu.addItem({"label":'<b>'+this.params.name+'</b>',"enabled":false});
	else
		menu.addItem({"label":'<b>All Blocks</b>',"enabled":false});
	menu.addItem();

	var actions = JobBlock.actions;
	for( var i = 0; i < actions.length; i++)
	{
		var item = {};
		for( var key in actions[i] ) item[key] = actions[i][key];
		item.receiver = this;
		item.param = actions[i];
		menu.addItem( item);
	}
	menu.show();

	return false;
}
JobBlock.prototype.onContextMenuDestroy = function()
{
	this.element.classList.remove('selected');
	this.job.monitor.menu = null;
}
JobBlock.prototype.mh_Dialog = function( i_parameter)
{
	new cgru_Dialog({"wnd":this.job.monitor.window,"receiver":this,"handle":'setParameter',
		"param":i_parameter.name,"type":i_parameter.type,"value":this.params[i_parameter.name],
		"name":'jobblock_parameter'});
}
JobBlock.prototype.setParameter = function( i_value, i_parameter)
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

		if( this.params.p_tasks_run_time && this.params.p_tasks_done )
		{
			var sum = cm_TimeStringFromSeconds( this.params.p_tasks_run_time);
			var avg = cm_TimeStringFromSeconds( Math.round( this.params.p_tasks_run_time / this.params.p_tasks_done));
			this.elRunTime.textContent = sum +'/'+avg;
			this.elRunTime.title = 'Running Time:\nTotal: '+sum+'\nAverage per task: '+avg;
		}
		else
		{
			this.elRunTime.textContent = '';
			this.elRunTime.title = '';
		}
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

JobNode.resetPanels = function( i_monitor)
{
	if( i_monitor.panel_item == null ) return;

	var elPanelL = i_monitor.elPanelL;
	elPanelL.m_elLog.classList.remove('active');
	elPanelL.m_elRes.classList.remove('active');
	elPanelL.m_elObj.classList.remove('active');
	elPanelL.m_elMov.classList.remove('active');
	elPanelL.m_elErr.classList.remove('errors');
	elPanelL.m_elErr.classList.add('hide_childs');
	elPanelL.m_elDel.classList.remove('active');

	var elPanelR = i_monitor.elPanelR;
	elPanelR.m_elName.style.display = 'none';

	elPanelR.m_elFolders.classList.remove('active');
	var elFolders = elPanelR.m_elFolders;
	if( elFolders.m_elFolders )
		for( var i = 0; i < elFolders.m_elFolders.length; i++)
			elFolders.removeChild( elFolders.m_elFolders[i]);
	elFolders.m_elFolders = [];

	elPanelR.m_elActions.classList.remove('active');
	var elParams = elPanelR.m_elActions.m_elParams;
	for( var p in elParams )
		elParams[p].style.display = 'none';

	i_monitor.panel_item = null;
}
JobNode.prototype.updatePanels = function()
{
	JobNode.resetPanels( this.monitor);
	this.monitor.panel_item = this;

	var elPanelL = this.monitor.elPanelL;
	elPanelL.m_elLog.classList.add('active');
	elPanelL.m_elRes.classList.add('active');
	elPanelL.m_elObj.classList.add('active');
	elPanelL.m_elMov.classList.add('active');
	elPanelL.m_elDel.classList.add('active');

	var errors = 0;
	var avoids = 0;
	for( var b = 0; b < this.params.blocks.length; b++)
	{
		var block = this.params.blocks[b];
		if( block.p_error_hosts )
			errors += block.p_error_hosts;
		if( block.p_avoid_hosts )
			avoids += block.p_avoid_hosts;
	}
	if( errors || avoids || this.state.ERR )
	{
		elPanelL.m_elErr.classList.add('errors');
		elPanelL.m_elErr.classList.remove('hide_childs');
	}

	var elPanelR = this.monitor.elPanelR;
	elPanelR.m_elName.textContent = this.params.name;
	elPanelR.m_elName.title = 'Current job name:\n' + this.params.name;
	elPanelR.m_elName.style.display = 'block';

	var elFolders = elPanelR.m_elFolders;
	elFolders.classList.add('active');
	elFolders.m_elFolders = [];
	var folders = this.params.folders;
//console.log(JSON.stringify( folders));
	for( var name in folders )
	{
		var elDiv = document.createElement('div');
		elDiv.classList.add('param');
		elDiv.classList.add('folder');
		elFolders.appendChild( elDiv);
		elFolders.m_elFolders.push( elDiv);

		var elLabel = document.createElement('div');
		elDiv.appendChild( elLabel);
		elLabel.textContent = name;
		elLabel.classList.add('label');

		var elValue = document.createElement('div');
		elDiv.appendChild( elValue);
		elValue.textContent = name;
		elValue.classList.add('value');
		elValue.textContent = folders[name]
		elValue.title = folders[name]
	}

	elPanelR.m_elActions.classList.add('active');
	var elParams = elPanelR.m_elActions.m_elParams;
	for( var p in elParams )
	{
		if( this.params[p] == null )
		{
			elParams[p].style.display = 'none';
			elParams[p].m_elValue.textContent = '';
			continue;
		}

		var value = this.params[p];
		if(( typeof value ) == 'string' )
		{
			// word-wrap long regular expressions:
			value = value.replace(/\./g,'.&shy;');
			value = value.replace(/\|/g,'|&shy;');
			value = value.replace(/\)/g,')&shy;');
		}
		elParams[p].m_elValue.innerHTML = value;
		elParams[p].style.display = 'block';
	}
}

JobNode.createPanelL = function( i_monitor)
{
	// Errors:
	var elBtn = document.createElement('div');
	i_monitor.elPanelL.appendChild( elBtn);
	i_monitor.elPanelL.m_elErr = elBtn;
	elBtn.classList.add('ctrl_button');
	elBtn.classList.add('hide_childs');
	elBtn.textContent = 'ERR';
	elBtn.title = 'Error tasks and hosts.';
	elBtn.monitor = i_monitor;
	elBtn.oncontextmenu = elBtn.onclick;

	var acts = {};
	acts.error_hosts       = {'label':'GEH', "handle":'mh_Get',  'tooltip':'Show error hosts.'};
	acts.reset_error_hosts = {'label':'REH', 'handle':'mh_Oper', 'tooltip':'Reset error hosts.'};
	acts.restart_errors    = {'label':'RET', 'handle':'mh_Oper', 'tooltip':'Restart error tasks.'};

	for( var a in acts )
	{
		var el = document.createElement('div');
		elBtn.appendChild( el);
		el.classList.add('sub_button');
		el.textContent = acts[a].label;
		el.title = acts[a].tooltip;
		el.m_action = a;
		el.m_handle = acts[a].handle;
		el.m_monitor = i_monitor;
		el.onclick = function(e){
			e.stopPropagation();
			var el = e.currentTarget;
			el.m_monitor[el.m_handle]({'name':el.m_action}, e);
			return false;
		}
		el.oncontextmenu = el.onclick;
	}

	// Restart:
	var elBtn = document.createElement('div');
	i_monitor.elPanelL.appendChild( elBtn);
	i_monitor.elPanelL.m_elRes = elBtn;
	elBtn.classList.add('ctrl_button');
	elBtn.classList.add('hide_childs');
	elBtn.textContent = 'RES';
	elBtn.title = 'Restart job tasks.';
	elBtn.monitor = i_monitor;
	elBtn.onclick = function(e){ e.currentTarget.classList.toggle('hide_childs'); return false; }
	elBtn.oncontextmenu = elBtn.onclick;

	var acts = {};
	acts.restart         = {'label':'ALL', 'tooltip':'Restart all tasks.'};
	acts.restart_pause   = {'label':'A&P', 'tooltip':'Restart all and pause.'};
	acts.restart_errors  = {'label':'ERR', 'tooltip':'Restart error tasks.'};
	acts.restart_running = {'label':'RUN', 'tooltip':'Restart running tasks.'};
	acts.restart_skipped = {'label':'SKP', 'tooltip':'Restart skipped tasks.'};
	acts.restart_done    = {'label':'DON', 'tooltip':'Restart done task.'};

	for( var a in acts )
	{
		var el = document.createElement('div');
		elBtn.appendChild( el);
		el.classList.add('sub_button');
		el.textContent = acts[a].label;
		el.title = acts[a].tooltip;
		el.m_action = a;
		el.m_monitor = i_monitor;
		el.onclick = function(e){
			e.stopPropagation();
			var el = e.currentTarget;
			el.m_monitor.mh_Oper({'name':el.m_action});
			return false;
		}
		el.oncontextmenu = el.onclick;
	}

	// Move:
	var elBtn = document.createElement('div');
	i_monitor.elPanelL.appendChild( elBtn);
	i_monitor.elPanelL.m_elMov = elBtn;
	elBtn.classList.add('ctrl_button');
	elBtn.classList.add('hide_childs');
	elBtn.textContent = 'MOV';
	elBtn.title = 'Change jobs order.';
	elBtn.monitor = i_monitor;
	elBtn.onclick = function(e){ e.currentTarget.classList.toggle('hide_childs'); return false; }
	elBtn.oncontextmenu = elBtn.onclick;

	var acts = {};
	acts.move_jobs_top    = {'label':'TOP', 'tooltip':'Move jobs top.'};
	acts.move_jobs_up     = {'label':'UP',  'tooltip':'Move jobs up.'};
	acts.move_jobs_down   = {'label':'DWN', 'tooltip':'Move jobs down.'};
	acts.move_jobs_bottom = {'label':'BOT', 'tooltip':'Move jobs bottom.'};

	for( var a in acts )
	{
		var el = document.createElement('div');
		elBtn.appendChild( el);
		el.classList.add('sub_button');
		el.textContent = acts[a].label;
		el.title = acts[a].tooltip;
		el.m_action = a;
		el.m_monitor = i_monitor;
		el.onclick = function(e){
			e.stopPropagation();
			var el = e.currentTarget;
			nw_Action('users', [g_uid], {'type':el.m_action,'jids':el.m_monitor.getSelectedIds()});
			el.m_monitor.info('Moving Jobs');
			return false;
		}
		el.oncontextmenu = el.onclick;
	}

	// Delete:
	var el = document.createElement('div');
	i_monitor.elPanelL.appendChild( el);
	i_monitor.elPanelL.m_elDel = el;
	el.classList.add('ctrl_button');
	el.textContent = 'DEL';
	el.title = 'Double click to delete job(s).';
	el.monitor = i_monitor;
	el.ondblclick = function(e){ e.currentTarget.monitor.mh_Oper({"name":'delete'});}
	el.oncontextmenu = function(e){ return false;}
}

JobNode.createPanelR = function( i_monitor)
{
	var elPanelR = i_monitor.elPanelR;

	// Job name:
	var el = document.createElement('div');
	elPanelR.appendChild( el);
	el.classList.add('name');
	elPanelR.m_elName = el;

	// Folders:
	var el = document.createElement('div');
	elPanelR.appendChild( el);
	el.classList.add('section');
	elPanelR.m_elFolders = el;
	var el = document.createElement('div');
	elPanelR.m_elFolders.appendChild( el);
	el.textContent = 'Folders';
	el.classList.add('caption');

	// Parameters:
	var el = document.createElement('div');
	elPanelR.appendChild( el);
	el.classList.add('section');
	elPanelR.m_elActions = el;
	var el = document.createElement('div');
	elPanelR.m_elActions.appendChild( el);
	el.textContent = 'Parameters';
	el.classList.add('caption');
	el.title = 'Click to edit all paramters.';
	el.m_elActions = elPanelR.m_elActions;
	el.onclick = function(e){
		var el = e.currentTarget;
		if( el.m_elActions.classList.contains('active') != true ) return false;
		var elParams = el.m_elActions.m_elParams;
		for( var p in elParams )
			elParams[p].style.display = 'block';
		return false;
	}
	el.oncontextmenu = el.onclick;

	elPanelR.m_elActions.m_elParams = {};
	for( var i = 0; i < JobNode.actions.length; i++ )
	{
		var act = JobNode.actions[i];
		if( act.mode != 'set' ) continue;
		if( false == cm_CheckPermissions( act.permissions )) continue;

		var elDiv = document.createElement('div');
		elPanelR.m_elActions.appendChild( elDiv);
		elDiv.classList.add('param');
		elDiv.style.display = 'none';

		var elLabel = document.createElement('div');
		elDiv.appendChild( elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = act.label;

		var elValue = document.createElement('div');
		elDiv.appendChild( elValue);
		elValue.classList.add('value');
		elDiv.m_elValue = elValue;

		elPanelR.m_elActions.m_elParams[act.name] = elDiv;

		var el = elDiv;
		el.title = 'Double click to edit.'
		el.monitor = i_monitor;
		el.action = act;
		el.ondblclick = function(e){e.currentTarget.monitor.mh_Dialog( e.currentTarget.action);}
	}
}


JobNode.actions = [];

JobNode.actions.push({"mode":'option', "name":'jobs_thumbs_num',    "type":'num', "handle":'mh_Opt', "label":'Thumbnails Quantity', "default":10  });
JobNode.actions.push({"mode":'option', "name":'jobs_thumbs_height', "type":'num', "handle":'mh_Opt', "label":'Thumbnails Height',   "default":50 });

JobNode.actions.push({"mode":'context', "name":'start',             "handle":'mh_Oper', "label":'Start'});
JobNode.actions.push({"mode":'context', "name":'pause',             "handle":'mh_Oper', "label":'Pause'});
JobNode.actions.push({"mode":'context', "name":'stop',              "handle":'mh_Oper', "label":'Stop'});

JobNode.actions.push({"mode":'set', "name":'annotation',                 "type":'str', "handle":'mh_Dialog', "label":'Annotation'});
JobNode.actions.push({"mode":'set', "name":'depend_mask',                "type":'reg', "handle":'mh_Dialog', "label":'Depend Mask'});
JobNode.actions.push({"mode":'set', "name":'depend_mask_global',         "type":'reg', "handle":'mh_Dialog', "label":'Global Depend Mask'});
JobNode.actions.push({"mode":'set', "name":'max_running_tasks',          "type":'num', "handle":'mh_Dialog', "label":'Max Runnig Tasks'});
JobNode.actions.push({"mode":'set', "name":'max_running_tasks_per_host', "type":'num', "handle":'mh_Dialog', "label":'Max Run Tasks Per Host'});
JobNode.actions.push({"mode":'set', "name":'hosts_mask',                 "type":'reg', "handle":'mh_Dialog', "label":'Hosts Mask'});
JobNode.actions.push({"mode":'set', "name":'hosts_mask_exclude',         "type":'reg', "handle":'mh_Dialog', "label":'Exclude Hosts Mask'});
JobNode.actions.push({"mode":'set', "name":'time_wait',                  "type":'tim', "handle":'mh_Dialog', "label":'Time Wait'});
JobNode.actions.push({"mode":'set', "name":'priority',                   "type":'num', "handle":'mh_Dialog', "label":'Priority'});
JobNode.actions.push({"mode":'set', "name":'need_os',                    "type":'reg', "handle":'mh_Dialog', "label":'OS Needed'});
JobNode.actions.push({"mode":'set', "name":'need_properties',            "type":'reg', "handle":'mh_Dialog', "label":'Need Properties'});
JobNode.actions.push({"mode":'set', "name":'time_life',                  "type":'hrs', "handle":'mh_Dialog', "label":'Life Time'});
JobNode.actions.push({"mode":'set', "name":'user_name',                  "type":'str', "handle":'mh_Dialog', "label":'Owner',"permissions":'visor'});
JobNode.actions.push({"mode":'set'});
JobNode.actions.push({"mode":'set', "name":'hidden',                     "type":'bl1', "handle":'mh_Dialog', "label":'Hidden'});

JobBlock.actions = [];
JobBlock.actions.push({"mode":'set', "name":'capacity',                   "type":'num', "handle":'mh_Dialog', "label":'Capacity'});
JobBlock.actions.push({"mode":'set'});
JobBlock.actions.push({"mode":'set', "name":'errors_retries',             "type":'num', "handle":'mh_Dialog', "label":'Errors Retries'});
JobBlock.actions.push({"mode":'set', "name":'errors_avoid_host',          "type":'num', "handle":'mh_Dialog', "label":'Errors Avoid Host'});
JobBlock.actions.push({"mode":'set', "name":'errors_task_same_host',      "type":'num', "handle":'mh_Dialog', "label":'Errors Task Same Host'});
JobBlock.actions.push({"mode":'set', "name":'errors_forgive_time',        "type":'hrs', "handle":'mh_Dialog', "label":'Errors Forgive Time'});
JobBlock.actions.push({"mode":'set', "name":'tasks_max_run_time',         "type":'hrs', "handle":'mh_Dialog', "label":'Tasks Max Run Time'});
JobBlock.actions.push({"mode":'set'});
JobBlock.actions.push({"mode":'set', "name":'non_sequential',             "type":'bl1', "handle":'mh_Dialog', "label":'Non-Sequential'});
JobBlock.actions.push({"mode":'set'});
JobBlock.actions.push({"mode":'set', "name":'max_running_tasks',          "type":'num', "handle":'mh_Dialog', "label":'Max Runnig Tasks'});
JobBlock.actions.push({"mode":'set', "name":'max_running_tasks_per_host', "type":'num', "handle":'mh_Dialog', "label":'Max Run Tasks Per Host'});
JobBlock.actions.push({"mode":'set', "name":'hosts_mask',                 "type":'reg', "handle":'mh_Dialog', "label":'Hosts Mask'});
JobBlock.actions.push({"mode":'set', "name":'hosts_mask_exclude',         "type":'reg', "handle":'mh_Dialog', "label":'Exclude Hosts Mask'});
JobBlock.actions.push({"mode":'set', "name":'depend_mask',                "type":'reg', "handle":'mh_Dialog', "label":'Depend Mask'});
JobBlock.actions.push({"mode":'set', "name":'tasks_depend_mask',          "type":'reg', "handle":'mh_Dialog', "label":'Tasks Depend Mask'});
JobBlock.actions.push({"mode":'set', "name":'need_properties',            "type":'reg', "handle":'mh_Dialog', "label":'Properties Needed'});

// First array item will be used by default (on load)
JobNode.sort = ['order','time_creation','priority','user_name','name','host_name'];
JobNode.sortVisor = 'time_creation';
// If user is visor, special parameter will be used as the default
JobNode.filter = ['name','host_name','user_name'];
JobNode.filterVisor = 'user_name';

