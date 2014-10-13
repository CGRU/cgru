// Submit Adobe After Effects to Afanasy render manager

if( app.project.af == null )
{
	af = {};
	af.frame_start = 1;
	af.frame_end = 100;
	af.frame_by = 1;
	af.frame_per_task = 5;
	af.paused = false;
	af.comp_name = '';
	af.hosts_mask = '';
	af.exclude_hosts = '';
	af.mov_enable = false;
	af.mov_name = '';
	af.mov_codec = 'h264_good';
	af.mov_res = '1280x720';
	app.project.af = af;
}
else
	af = app.project.af;

function af_StartRender( i_execute)
{

	// Collect all the parameters required to render the comps in the Render Queue

	var comp_num = 0;
	var output_path;  // Full path to output rendered comp
	var frame_startL; // Frame to start rendering at
	var frame_endL;   // Frame to stop rendering at
	var fps = 25;

	// file paths
	var project_name = app.project.file.name;
	var project_path = app.project.file.fsName;

	if( i_execute)
	{
		app.project.save( app.project.file );
	}

	// Get last comp name
	for( var i = 1; i <= app.project.renderQueue.numItems; i++ )
	{
		if( app.project.renderQueue.item( i ).status != RQItemStatus.QUEUED )
		{
			continue;
		}

		af.comp_name = app.project.renderQueue.item( i ).comp.name;
	}

	// Cycle through all the comps in the Render Queue
	for( var i = 1; i <= app.project.renderQueue.numItems; i++ )
	{
		if( app.project.renderQueue.item( i ).status != RQItemStatus.QUEUED )
		{
			continue;
		}

		if( af.comp_name != app.project.renderQueue.item( i ).comp.name )
		{
			continue;
		}

		for( comp_num = 1; comp_num <= app.project.numItems; comp_num++ )
		{
			if( app.project.item( comp_num).name == af.comp_name)
			{
				break;
			}
		}

		output_path = app.project.renderQueue.item( i ).outputModule( 1 ).file.fsName;

		fps = 1 / app.project.item(comp_num).frameDuration;

		frame_startL = app.project.item(comp_num).workAreaStart*fps + app.project.item(comp_num).displayStartTime*fps;
		frame_endL = frame_startL + app.project.renderQueue.item( i ).timeSpanDuration * fps - 1;

		if( !i_execute )
		{
			af.frame_start = frame_startL;
			af.frame_end = frame_endL;
			af.mov_name = af.comp_name;
			return;
		}
		
		if ( system.osName == "MacOS") // Macosx
		{
			var cmd = 'python';
			cmd += ' ' + $.getenv('AF_ROOT') + "/python/afjob.py"
		}
		else // Windows
		{
			var cmd = 'pythonw.exe';
			cmd += ' "%AF_ROOT%\\python\\afjob.py"';
		}

		cmd += ' "' + project_path + '"';
		cmd += ' ' + af.frame_start;
		cmd += ' ' + af.frame_end;
		cmd += ' -by ' + af.frame_by;
		cmd += ' -fpt ' + af.frame_per_task;
		cmd += ' -node "' + af.comp_name + '"';
		cmd += ' -name "' + project_name + '"';
		cmd += ' -images "' + output_path + '"';
		cmd += ' -hostsmask "' + af.hosts_mask + '"';
		cmd += ' -hostsexcl "' + af.exclude_hosts + '"';
		cmd += ' -tempscene';
		cmd += ' -deletescene';
		if( af.paused )
			cmd += ' -pause';

		if( af.mov_enable )
		{
			cmd += ' -mname "' + af.mov_name + '"';
			if( af.mov_codec != '' )
				cmd += ' -mcodec "' + af.mov_codec + '"';
			if( af.mov_res != '' )
				cmd += ' -mres "' + af.mov_res + '"';
		}
		
		if ( system.osName == "MacOS")
		{
			var cmd_output = system.callSystem(cmd);
		}
		else
			var cmd_output = system.callSystem('cmd /c "' + cmd + '"');

//alert( cmd_output);
//alert( system.callSystem('cmd /c \"pythonw.exe "c:\\cg\\tools\\cgru\\afanasy\\python\\afjob.py" "Z:\\PROJECTS\\Zona\\test\\station\\station_test_afo.aep" 0 2310 -by 1 -fpt 77 -node "station_anim" -os any -pwd "Z:\\PROJECTS\\Zona\\test\\station" -pause -name "station_test_afo.aep\""'));
	}
}

function af_CheckScene()
{
	// Check 1 - Ensure a project is open
	if( ! app.project )
	{
		alert('A project must be open to run this script.');
		return false;
	}

	// Check 2 - Ensure the project has been saved in the past
	if( ! app.project.file )
	{
		alert('This project must be saved before running this script.');
		return false;
	}

	// Check 3 - Ensure that at least 1 comp is queued
	var found = false;
	for( i = 1; i <= app.project.renderQueue.numItems; i++ )
	{
		if( app.project.renderQueue.item( i ).status == RQItemStatus.QUEUED )
		{
			found = true;
			break;
		}
	}
	if( ! found )
	{
		alert('You do not have any items set to render.');
		return false;
	}

	// Check 4 - Ensure that no 2 comps in the Render Queue have the same name
	var compItem1;
	var compItem2;
	for( var i = 1; i < app.project.renderQueue.numItems; i++ )
	{

		if( app.project.renderQueue.item( i ).status != RQItemStatus.QUEUED )
		{
			continue;
		}

		compItem1 = app.project.renderQueue.item( i ).comp;

		for( var j = i + 1; j <= app.project.renderQueue.numItems; j++ )
		{
			if( app.project.renderQueue.item( j ).status != RQItemStatus.QUEUED )
			{
				continue;
			}

			compItem2 = app.project.renderQueue.item( j ).comp;
			if( compItem1.name == compItem2.name )
			{
				af.safe_to_run = false;
				alert('At least 2 of your active items in the Render Queue have the same name. Please ensure that all of your items have unique names.' );
				return false;
			}
		}
	}

	return true;
}

function af_DrawWindow()
{
	var af_Window = new Window('palette','Submit Job To Afanasy', undefined, {resizeable:true});
	//af_Window.margins = [0,0,0,0];
	af_Window.alignChildren = ['center','top'];

	var tabPanel = af_Window.add( 'tabbedpanel', undefined,'', {resizeable:true});
    //tabPanel.margins = [0,0,0,0];
    tabPanel.alignment = ['fill','fill'];
    tabPanel.alignChildren = ['left','top'];

    var generalTab = tabPanel.add( 'tab', undefined, 'General', {resizeable:true});
    //generalTab.spacing= 20;
    //generalTab.margins = [0,0,0,0];
    generalTab.alignment = ['fill','fill'];
    generalTab.alignChildren = ['left','top'];

	generalTab.add('statictext', undefined, 'Frame Start');
	var elStart = generalTab.add('edittext', undefined, af.frame_start);
    elStart.alignment = ['fill','center'];

    generalTab.add('statictext', undefined, 'Frame Finish');
	var elEnd = generalTab.add('edittext', undefined, af.frame_end);
    elEnd.alignment = ['fill','center'];

	generalTab.add('statictext', undefined, 'Frame Increment');
	var elBy = generalTab.add('edittext', undefined, af.frame_by);
    elBy.alignment = ['fill','center'];

	generalTab.add('statictext', undefined, 'Frames Per Task');
	var elFpt = generalTab.add('edittext', undefined, af.frame_per_task);
    elFpt.alignment = ['fill','center'];

	elStart.onChange = function(){ if( isNaN(this.text)) this.text = af.frame_start;    af.frame_start    = parseInt(this.text);};
	elEnd.onChange   = function(){ if( isNaN(this.text)) this.text = af.frame_end;      af.frame_end      = parseInt(this.text);};
	elFpt.onChange   = function(){ if( isNaN(this.text)) this.text = af.frame_per_task; af.frame_per_task = parseInt(this.text);};
	elBy.onChange    = function(){ if( isNaN(this.text)) this.text = af.frame_by;       af.frame_by       = parseInt(this.text);};

	generalTab.add('statictext', undefined, 'Comp Name:');
	generalTab.add('statictext', undefined, af.comp_name);

	var elPaused = generalTab.add('checkbox', undefined, 'Start Paused');
	elPaused.helpTip = 'If enabled, the job will submit in the offline state.';
	elPaused.value = af.paused;
	elPaused.onClick = function(){ af.paused = this.value;};


	generalTab.add('statictext', undefined, 'Hosts Mask');
	var elHostsMask = generalTab.add('edittext', undefined, af.hosts_mask);
	elHostsMask.onChange = function(){ af.hosts_mask = this.text;};
	elHostsMask.alignment = ['fill','center'];

	generalTab.add('statictext', undefined, 'Exclude Hosts Mask');
	var elExcludeHosts = generalTab.add('edittext', undefined, af.exclude_hosts);
	elExcludeHosts.onChange = function(){ af.exclude_hosts = this.text;};
	elExcludeHosts.alignment = ['fill', 'center'];

	// Movie:
	var movieTab = tabPanel.add( 'tab', undefined, 'Movie', {resizeable:true});
    movieTab.alignment = ['fill','fill'];
    movieTab.alignChildren = ['left','top'];

	var elMovEnable = movieTab.add('checkbox', undefined, 'Encode Movie');
	elMovEnable.helpTip = 'If enabled, a movie will be encoded from sequence.';
	elMovEnable.value = af.mov_enable;
	elMovEnable.onClick = function(){ af.mov_enable = this.value;};

	movieTab.add('statictext', undefined, 'Name:');
	elMovName = movieTab.add('edittext', undefined, af.mov_name);
    elMovName.onChange = function(){ af.mov_name = this.text;};
    elMovName.alignment = ['fill','center'];

	movieTab.add('statictext', undefined, 'Codec:');
	elMovCodec = movieTab.add('edittext', undefined, af.mov_codec);
    elMovCodec.onChange = function(){ af.mov_codec = this.text;};
    elMovCodec.alignment = ['fill','center'];

	movieTab.add('statictext', undefined, 'Resolution:');
	elMovRes = movieTab.add('edittext', undefined, af.mov_res)
    elMovRes.onChange = function(){ af.mov_res = this.text;};
    elMovRes.alignment = ['fill','center'];

	// Submit and close buttons:
	var startRenderBtn = af_Window.add ('button', undefined, 'Render');
	startRenderBtn.onClick = function() { af_StartRender(1); };
    startRenderBtn.preferredSize = [100,20];

	var closeBtn = af_Window.add('button', undefined, 'Render & Close');
	closeBtn.onClick = function() { af_StartRender(1); af_Window.hide(); };
    closeBtn.preferredSize = [100,20];

	af_Window.onResizing = af_Window.onResize = function () {this.layout.resize();}
		
    af_Window.layout.resize();
    af_Window.center();
	af_Window.show()
}

if( af_CheckScene())
{
	af_StartRender(0);
	af_DrawWindow();
}
