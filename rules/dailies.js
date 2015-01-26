d_moviemaker = '/cgru/utilities/moviemaker';
d_makemovie = d_moviemaker+'/makemovie.py';

d_params_types = {};
d_params_types.general  = {"label":'General',"tooltip":'General parameters.'};
d_params_types.settings = {"label":'Settings',"tooltip":'Other parameters.'};

d_params = {"general":{},"settings":{}};
d_params.general.project = {"width":'50%'};
d_params.general.shot = {"width":'50%',"lwidth":'70px'};
d_params.general.artist = {"width":'50%'};
d_params.general.activity = {"width":'25%',"lwidth":'70px'};
d_params.general.version = {"width":'25%',"lwidth":'70px'};
d_params.general.input = {};
d_params.general.output = {};
d_params.general.filename = {"width":'75%'}
d_params.general.fps = {"label":'FPS',"width":'25%',"lwidth":'70px'};

d_params.settings.af_depend_mask = {"label":'Depends',"tooltip":'Afanasy job depend mask'}
d_params.settings.fffirst = {"label":"F.F.First","tooltip":'First frame is "1"\nNo matter image file name number.'};
d_params.settings.aspect_in = {"label":'Aspect In'};
d_params.settings.gamma = {};

function d_Make( i_path, i_outfolder)
{
	c_Log('Make Dailies: '+i_path);

	var params = {};

	params.project = 'project';
	if( ASSETS.project ) params.project = ASSETS.project.name;
	params.shot = c_PathBase( i_path);
	if( ASSETS.shot ) params.shot = ASSETS.shot.name;

	params.version = i_path.split('/');
	params.version = params.version[params.version.length-1];
	var match = params.version.match(/v\d{2,}.*/gi);
	if( match )
		params.version = match[match.length-1];

	params.input = i_path;
	params.output = i_outfolder;
	params.activity = RULES.dailies.activity;

	d_params.general.artist = {"width":'50%'};
	params.artist = c_GetUserTitle();
	if( RULES.status && RULES.status.artists && RULES.status.artists.length )
	{
		artist = c_GetUserTitle( RULES.status.artists[0]);

		if( artist != c_GetUserTitle() || ( RULES.status.artists.length > 1 ))
		{
			params.artist = artist;
			var artists = [];
			for( var i = 0; i < RULES.status.artists.length; i++)
				artists.push( c_GetUserTitle( RULES.status.artists[i]));
			if( artists.indexOf( c_GetUserTitle()) == -1 )
				artists.push( c_GetUserTitle());
			d_params.general.artist.pulldown = artists;
		}
	}

	var dateObj = new Date();
	date = ''+dateObj.getFullYear();
	date = date.substr(2);
	date += (dateObj.getMonth()+1) < 10 ? '0'+(dateObj.getMonth()+1) : dateObj.getMonth()+1;
	date +=  dateObj.getDate()     < 10 ? '0'+ dateObj.getDate()     : dateObj.getDate();

	var naming = RULES.dailies.naming;
	var filename = RULES.dailies.naming;
	filename = filename.replace('(p)', params.project);
	filename = filename.replace('(P)', params.project.toUpperCase());
	filename = filename.replace('(s)', params.shot);
	filename = filename.replace('(S)', params.shot.toUpperCase());
	filename = filename.replace('(v)', params.version);
	filename = filename.replace('(V)', params.version.toUpperCase());
	filename = filename.replace('(d)', date);
	filename = filename.replace('(D)', date.toUpperCase());
	filename = filename.replace('(a)', params.activity);
	filename = filename.replace('(A)', params.activity.toUpperCase());
	filename = filename.replace('(c)', RULES.company);
	filename = filename.replace('(C)', RULES.company.toUpperCase());
	filename = filename.replace('(u)', params.artist);
	filename = filename.replace('(U)', params.artist.toUpperCase());
	params.filename = filename;

	var wnd = new cgru_Window({"name":'dailes',"title":'Make Dailies'});

	n_WalkDir({"paths":[i_path],"wfunc":d_DailiesWalkReceived,"info":'walk dailies',"d_params":params,"d_wnd":wnd});
}
function d_DailiesWalkReceived( i_data, i_args)
{
	var wnd = i_args.d_wnd;
	var params = i_args.d_params;
	var walk = i_data[0];

	if( walk && walk.files && walk.files.length )
	for( var f = 0; f < walk.files.length; f++ )
	{
		var file = walk.files[f].name;
		var match = file.match(/\d+\./g);
		if( match )
		{
			match = match[match.length-1];
			var pos = file.lastIndexOf( match);
			var pattern = file.substr( 0, pos);
			for( var d = 0; d < match.length-1; d++ ) pattern += '#';
			pattern += file.substr( pos-1 + match.length);
			params.input += '/' + pattern;
			break;
		}
//window.console.log( match);
	}

	wnd.elTabs = gui_CreateTabs({"tabs":d_params_types,"elParent":wnd.elContent,"name":'d_params_types'});

	for( var type in d_params_types )
		gui_Create( wnd.elTabs[type], d_params[type], [params, RULES.dailies]);

	gui_CreateChoises({"wnd":wnd.elTabs.general,"name":'colorspace',"value":RULES.dailies.colorspace,"label":'Colorspace:',"keys":RULES.dailies.colorspaces});
	gui_CreateChoises({"wnd":wnd.elTabs.general,"name":'format',"value":RULES.dailies.format,"label":'Formats:',"keys":RULES.dailies.formats});
	gui_CreateChoises({"wnd":wnd.elTabs.general,"name":'codec',"value":RULES.dailies.codec,"label":'Codec:',"keys":RULES.dailies.codecs});
	gui_CreateChoises({"wnd":wnd.elTabs.general,"name":'container',"value":RULES.dailies.container,"label":'Container:',"keys":RULES.dailies.containers});

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);
	elBtns.style.clear = 'both';

	var elAfDiv = document.createElement('div');
	elBtns.appendChild( elAfDiv);
	elAfDiv.classList.add('param');

	var elLabel = document.createElement('div');
	elAfDiv.appendChild( elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';

	var elSend = document.createElement('div');
	elAfDiv.appendChild( elSend);
	elSend.textContent = 'Send Job';
	elSend.classList.add('button');
	elSend.onclick = function(e){ d_ProcessGUI( e.currentTarget.m_wnd);}
	elSend.m_wnd = wnd;

	wnd.elContent.focus();
	wnd.elContent.m_wnd = wnd;
	wnd.elContent.onkeydown = function(e)
	{
//		console.log( e.keyCode);
		if( e.keyCode == 13 ) d_ProcessGUI( e.currentTarget.m_wnd);
	}
}

function d_ProcessGUI( i_wnd)
{
	var params = {};
	for( var type in d_params_types )
		gui_GetParams( i_wnd.elTabs[type], d_params[type], params);
//console.log( JSON.stringify( params)); return;

	for( key in i_wnd.elTabs.general.m_choises )
		params[key] = i_wnd.elTabs.general.m_choises[key].value;

	i_wnd.destroy();

	if( g_auth_user == null )
	{
		c_Error("Guests can't generate dailies.");
		return;
	}

	var job = {};
	//job.offline = true;
	job.name = params.filename;
	if( params.af_depend_mask.length )
		job.depend_mask = params.af_depend_mask;

	var block = {};
	block.name = 'Dailies';
	block.service = 'movgen';
	block.parser = 'generic';
	if( RULES.dailies.af_capacity ) block.capacity = RULES.dailies.af_capacity;
	block.working_directory = cgru_PM('/' + RULES.root+params.output, true);
	job.blocks = [block];

	var task = {}
	task.name = params.filename;
	task.command = d_MakeCmd( params);
	block.tasks = [task];

//console.log( task.command);
//console.log( JSON.stringify(job));
	n_SendJob( job);

	nw_MakeNews({"title":'dailies'});
}

function d_MakeCmd( i_params)
{
	var params = c_CloneObj( RULES.dailies);
	for( var parm in i_params )
		params[parm] = i_params[parm];

	var input = cgru_PM('/' + RULES.root+params.input, true);
	var output = cgru_PM('/' + RULES.root+params.output + '/' + params.filename, true);

	var cmd = 'python';

	cmd += ' "' + cgru_PM( d_makemovie, true) + '"';

	cmd += ' -a "' + RULES.avcmd + '"';
	cmd += ' -c "'+params.codec+'"';
	cmd += ' -f '+params.fps;
	cmd += ' -r '+params.format;
	cmd += ' -s '+params.slate;
	cmd += ' -t '+params.template;

	if( params.container != 'DEFAULT' )
		cmd += ' -n ' + params.container;

	cmd += ' --colorspace "'+params.colorspace+'"';

	if( params.gamma != '' )
		cmd += ' -g ' + params.gamma;
	if( params.fffirst != '' )
		cmd += ' --fffirst';

	cmd += ' --lgspath "'+params.logo_slate_path+'"';
	cmd += ' --lgssize '+params.logo_slate_size;
	cmd += ' --lgsgrav '+params.logo_slate_grav;
	cmd += ' --lgfpath "'+params.logo_frame_path+'"';
	cmd += ' --lgfsize '+params.logo_frame_size;
	cmd += ' --lgfgrav '+params.logo_frame_grav;

	cmd += ' --project "'+params.project+'"';
	cmd += ' --shot "'+params.shot+'"';

	cmd += ' --ver "'+params.version+'"';
	cmd += ' --artist "'+params.artist+'"';
	cmd += ' --activity "'+params.activity+'"';

	if( RULES.dailies.preview )
	{
		cmd += ' --pcodec "' + RULES.dailies.preview.codec + '"';
		cmd += ' --pargs "' + RULES.dailies.preview.args + '"';
		cmd += ' --pdir "' + RULES.rufolder + '"';
	}

	if(( params.aspect_in != null ) && ( params.aspect_in != '' ))
		cmd += ' --aspect_in '+params.aspect_in;

	cmd += ' --createoutdir';

	cmd += ' "'+input+'"';
	cmd += ' "'+output+'"';

//python "/cgru/utilities/moviemaker/makemovie.py" -c "/cgru/utilities/moviemaker/codecs/photojpg_best.ffmpeg" -f 25 -n mov --fs 1 --fe 20 -r 720x576x1.09 -g 1.00 -s "dailies_slate" -t "dailies_withlogo" --project "ENCODE" --shot "preview" --ver "preview" --artist "Timurhai" --activity "comp" --tmpformat tga --lgspath "logo.png" --lgssize 25 --lgsgrav SouthEast --lgfpath "logo.png" --lgfsize 10 --lgfgrav North "/data/tools/encode/preview/preview.####.jpg" "/data/tools/encode/preview_preview_121226"

	return cmd;
}


//##############################################################################
//###############################   CONVERT   ##################################
//##############################################################################


d_cvtguiparams = {};
d_cvtguiparams.cvtres = {"label":'Resolution',"info":'WIDTH or WIDTHxHEIGHT ( e.g. 1280x720 ). On empty no changes.',"iwidth":"50%"};
d_cvtguiparams.fps = {"label":'FPS'};
d_cvtguiparams.time_start = {"default":'00:00:00',"width":'50%'};
d_cvtguiparams.duration   = {"default":'00:00:00',"width":'50%'};
d_cvtguiparams.quality = {"label":'JPEG Quality',"default":'100'};
d_cvtguiparams.afmaxtasks = {"width":"50%","label":'Max Tasks',"default":'-1',"tooltip":'Maximum running tasks for Afanasy job.'};
d_cvtguiparams.afperhost = {"width":"50%","label":'Per Host',"default":'-1',"tooltip":'Maximum running tasks per host for Afanasy job.'};

d_cvtmulti_params = {};
d_cvtmulti_params.input = {"label":'Result Paths'};
d_cvtmulti_params.skipexisting = {"label":'Skip Existing', "bool":true,"width":'50%'};
d_cvtmulti_params.skiperrors = {"label":'Skip Errors', "bool":false,"width":'50%'};
d_cvtmulti_params.dest = {"label":'Destination'};

function d_Convert( i_args)
{
	var params = {};
//	params.quality = 80;
//	params.cvtres = '';

	var title = 'Convert ';
	if( i_args.images ) title += ' Images';
	else if( i_args.folders ) title += ' Sequences';
	else if( i_args.movies) title += ' Movies';
	else if( i_args.results) title += ' Results';
	var wnd = new cgru_Window({"name":'dailes',"title":title});
	wnd.m_args = i_args;

	var img_types = {};
	img_types.jpg = {"name":'JPG'};
	img_types.png = {"name":'PNG'};
	img_types.dpx = {"name":'DPX'};
	img_types.tif8  = {"name":'TIF8' ,"tooltip":'8  bits TIF'};
	img_types.tif16 = {"name":'TIF16',"tooltip":'16 bits TIF'};
	if( i_args.movies !== true )
		img_types.exr = {"name":'EXR'};

	gui_Create( wnd.elContent, d_cvtguiparams, [params, RULES.dailies]);
	gui_CreateChoises({"wnd":wnd.elContent,"name":'imgtype',"value":'jpg',"label":'Image Type:',"keys":img_types});
	if( i_args.movies == false )
		gui_CreateChoises({"wnd":wnd.elContent,"name":'colorspace',"value":RULES.dailies.colorspace,"label":'Colorspace:',"keys":RULES.dailies.colorspaces});
	gui_CreateChoises({"wnd":wnd.elContent,"name":'codec',"value":RULES.dailies.codec,"label":'Codec:',"keys":RULES.dailies.codecs});
	gui_CreateChoises({"wnd":wnd.elContent,"name":'container',"value":RULES.dailies.container,"label":'Container:',"keys":RULES.dailies.containers});

	if( i_args.results )
	{
		if( RULES.put.dest.indexOf('/') !== 0 )
			if( ASSETS.project )
				params.dest = cgru_PM('/' + RULES.root + ASSETS.project.path + '/' + RULES.put.dest, true);
		gui_Create( wnd.elContent, d_cvtmulti_params, [RULES.put,params]);
	}

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);
	elBtns.style.clear = 'both';
	elBtns.classList.add('buttons');
	elBtns.classList.add('param');

	var elLabel = document.createElement('div');
	elBtns.appendChild( elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';

	if( i_args.results )
	{
		var el = document.createElement('div');
		elBtns.appendChild( el);
		el.classList.add('button');
		el.style.cssFloat = 'right';
		el.textContent = 'Find Results';
		el.m_wnd = wnd;
		el.onclick = function(e){ fu_ResultsFind( e.currentTarget.m_wnd);}
	}

	wnd.m_res_btns_show = [];

	var elCvtBtn = document.createElement('div');
	elBtns.appendChild( elCvtBtn);
	elCvtBtn.textContent = title + ' To Movies';
	elCvtBtn.classList.add('button');
	elCvtBtn.onclick = function(e){ d_CvtProcessGUI( e.currentTarget.m_wnd, false);}
	elCvtBtn.m_wnd = wnd;
	wnd.m_res_btns_show.push( elCvtBtn);
	if( i_args.results ) elCvtBtn.style.display = 'none';

	var elExpBtn = document.createElement('div');
	elBtns.appendChild( elExpBtn);
	elExpBtn.textContent = title + ' To Sequences';
	elExpBtn.classList.add('button');
	elExpBtn.onclick = function(e){ d_CvtProcessGUI( e.currentTarget.m_wnd, true);}
	elExpBtn.m_wnd = wnd;
	wnd.m_res_btns_show.push( elExpBtn);
	if( i_args.results ) elExpBtn.style.display = 'none';

	wnd.m_elResults = document.createElement('div');
	wnd.elContent.appendChild( wnd.m_elResults);
	if( i_args.results )
		wnd.m_elResults.classList.add('output');
	else
		wnd.m_elResults.classList.add('source');
	for( var i = 0; i < i_args.paths.length; i++)
	{
		var el = document.createElement('div');
		wnd.m_elResults.appendChild( el);
		el.textContent = i_args.paths[i];
	}
}

function d_CvtProcessGUI( i_wnd, i_to_sequence)
{
	// Get GUI parameters:
	var params = gui_GetParams( i_wnd.elContent, d_cvtguiparams);

	if( i_wnd.m_args.results )
		gui_GetParams( i_wnd.elContent, d_cvtmulti_params, params);

	for( key in i_wnd.elContent.m_choises )
		params[key] = i_wnd.elContent.m_choises[key].value;

	// Process paths:
	var paths = i_wnd.m_args.paths;
	if( i_wnd.m_args.results )
	{
		// Paths are founded with find_results.py:
		var results = i_wnd.m_result.results;
		var res_skipped = [];
		paths = [];
		for( var i = 0; i < results.length; i++ )
		{
			if( results[i].error )
				continue;

			if( results[i].exist && params.skipexisting )
				continue;

			paths.push( results[i].src);
			res_skipped.push( results[i]);
		}

		i_wnd.m_result.results = res_skipped;
	}
	else
	{
		// Paths are just selected in filesview:
		for( var i = 0; i < paths.length; i++ )
			paths[i] = cgru_PM('/' + RULES.root + paths[i], true);
	}

	if( paths.length == 0 )
	{
		c_Error('Nothing to convert founded.');
		return;
	}

	i_wnd.m_args.cvt_paths = paths;

	// Run specific funtions:
	if( i_wnd.m_args.movies || ( i_to_sequence == false ))
	{
		d_CvtMovies( i_wnd, params, i_to_sequence );
	}
	else
	{
		d_CvtImages( i_wnd, params );
	}
}

function d_CvtImages( i_wnd, i_params)
{
//console.log( JSON.stringify( i_wnd.m_args));
//console.log( JSON.stringify( i_params));
	var paths = i_wnd.m_args.cvt_paths;

	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild( elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var cmd = 'rules/bin/convert.sh -J';

	cmd += ' -t ' + i_params.imgtype;
	cmd += ' -c ' + i_params.colorspace;
	cmd += ' -q ' + i_params.quality;
	if( i_params.cvtres != '' ) cmd += ' -r ' + i_params.cvtres;

	var afanasy = false;
	if( i_wnd.m_args.folders || ( paths.length > 1 ))
	{
		afanasy = true;
		cmd += ' -A';
		cmd += ' --afuser "' + g_auth_user.id + '"';
		cmd += ' --afmax ' + i_params.afmaxtasks;
		cmd += ' --afmph ' + i_params.afperhost;

		if( i_wnd.m_args.results )
			cmd += ' -o "' + i_wnd.m_result.dest + '"';
	}

	for( var i = 0; i < paths.length; i++)
		cmd += ' "' + paths[i] + '"'

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":d_CvtImagesFinished,"wnd":i_wnd,"afanasy":afanasy});
}
function d_CvtImagesFinished( i_data, i_args)
{
//console.log( JSON.stringify( i_data));
//console.log( JSON.stringify( i_args));
	if( i_args.afanasy == false )
	{
		i_args.wnd.destroy();
		i_args.wnd.m_args.filesview.refresh();
//		fv_ReloadAll();
		return;
	}

	i_args.wnd.elContent.removeChild( i_args.wnd.m_elWait);

	var elOut = i_args.wnd.m_elResults;
	elOut.textContent = '';
	elOut.classList.remove('source');
	elOut.classList.add('output');

	if(( i_data.cmdexec == null ) || ( i_data.cmdexec.length == 0 ) || ( i_data.cmdexec[0].convert == null ))
	{
		elOut.textContent = JSON.stringify( i_data);
		return;
	}

	if( i_data.cmdexec[0].error ) c_Error( i_data.cmdexec[0].error );

	var convert = i_data.cmdexec[0].convert;	

	if( convert.error ) c_Error( convert.error );

	for( var i = 0; i < convert.length; i++ )
	{
		var c = convert[i];
		var el = document.createElement('div');
		elOut.appendChild( el);
		var text =  JSON.stringify( c);
		if( c.input )
		{
			text = 'File: ';
			if( c.type == 'folder') text = 'Folder: ';
			text += c_PathBase( c.input );
			if( c.files_num ) text += '[' + c.files_num + ']';
			if( c.warning )
			{
				text += ' ' + c.warning;
				el.style.color = '#F82';
			}
			else
			{
				text += ' -> ';
				text += c_PathBase( c.output );
			}
		}
		el.textContent = text;
	}
}

function d_CvtMovies( i_wnd, i_params, i_to_sequence )
{
	var paths = i_wnd.m_args.cvt_paths;

	var job = {};
	job.name = c_PathBase( paths[0]);

	if( i_params.cvtres.length ) job.name += '-' + i_params.cvtres;

	if( i_to_sequence )
		job.name = 'Explode ' + job.name;
	else
		job.name = 'Convert ' + job.name;

	if( paths.length > 1 )
		job.name = c_PathDir( paths[0]) + ' x' + paths.length;

	job.max_running_tasks = parseInt( i_params.afmaxtasks);
	if( isNaN( job.max_running_tasks ))
	{
		c_Error('Invalid "Max Tasks" value: "' + i_params.afmaxtasks + '"');
		job.max_running_tasks = -1;
	}
	job.max_running_tasks_per_host = parseInt( i_params.afperhost);
	if( isNaN( job.max_running_tasks_per_host ))
	{
		c_Error('Invalid "Per Host" value: "' + i_params.afperhost + '"');
		job.max_running_tasks_per_host = -1;
	}

	var block = {};
	block.service = 'movgen';
	block.parser = 'generic';
	if( RULES.dailies.af_capacity ) block.capacity = RULES.dailies.af_capacity;
	block.working_directory = c_PathDir( paths[0]);
	block.tasks = [];
	job.blocks = [block];

	var cmd = 'movconvert';
	cmd += ' -a ' + RULES.avcmd;
	if( i_params.cvtres.length ) cmd += ' -r ' + i_params.cvtres;
	if( i_params.time_start != d_cvtguiparams.time_start.default )
		cmd += ' -s ' + i_params.time_start;
	if( i_params.duration != d_cvtguiparams.duration.default )
		cmd += ' -d ' + i_params.duration;

	if( i_to_sequence )
	{
		block.name = 'Explode to ' + i_params.imgtype.toUpperCase();
		cmd += ' -t ' + i_params.imgtype;
		var q = parseInt( i_params.quality);
		q = Math.round( 10 - ( q / 10 ));
		if( q < 1 ) q = 1;
		cmd += ' -q ' + q;
	}
	else
	{
		job.name += '.' + i_params.codec.toUpperCase();
		block.name = 'Convert to ' + i_params.codec.toUpperCase();
		cmd += ' -c "' + i_params.codec + '"';
		if( i_params.container != 'DEFAULT' )
		{
			cmd += ' -n ' + i_params.container;
			job.name += '.' + i_params.container.toUpperCase();
			block.name += '.' + i_params.container.toUpperCase();
		}
		cmd += ' -f ' + i_params.fps;
	}

	for( var i = 0; i < paths.length; i++)
	{
		var path = paths[i];
		var task = {};
		block.tasks.push( task);
		task.name = c_PathBase( path);
		task.command = cmd + ' "' + path + '"';

		if( i_wnd.m_args.results )
			task.command += ' -o "' + i_wnd.m_result.dest + '/' + i_wnd.m_result.results[i].name + '"';
	}

	n_SendJob( job);

	i_wnd.destroy();
//console.log(JSON.stringify(job));
}

//##############################################################################
//###############################   CUT   ######################################
//##############################################################################

d_cutparams = {};
d_cutparams.cut_name = {};
d_cutparams.input = {};
d_cutparams.fps = {"label":'FPS'};
d_cutparams.output = {};

function d_MakeCut( i_args)
{
//console.log( JSON.stringify( i_args));
	var wnd = new cgru_Window({"name":'cut',"title":'Make Cut'});
	wnd.m_args = i_args;

	var params = {};
	params.cut_name = i_args.cut_name;
	params.output = i_args.output;
	params.input = RULES.assets.shot.result.path.join(',');
	if( RULES.cut.input ) params.input = RULES.cut.input;

	gui_Create( wnd.elContent, d_cutparams, [RULES.dailies, RULES.cut, params]);
	gui_CreateChoises({"wnd":wnd.elContent,"name":'format',"value":RULES.dailies.format,"label":'Formats:',"keys":RULES.dailies.formats});
	gui_CreateChoises({"wnd":wnd.elContent,"name":'colorspace',"value":RULES.dailies.colorspace,"label":'Colorspace:',"keys":RULES.dailies.colorspaces});
	gui_CreateChoises({"wnd":wnd.elContent,"name":'codec',"value":RULES.dailies.codec,"label":'Codecs:',"keys":RULES.dailies.codecs});

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);
	elBtns.style.clear = 'both';
	elBtns.classList.add('buttons');

	var elAfDiv = document.createElement('div');
	elBtns.appendChild( elAfDiv);
	elAfDiv.classList.add('param');

	var elLabel = document.createElement('div');
	elAfDiv.appendChild( elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';

	var elSend = document.createElement('div');
	elAfDiv.appendChild( elSend);
	elSend.textContent = 'Send Job';
	elSend.classList.add('button');
	elSend.m_wnd = wnd;
	elSend.onclick = function(e){ d_CutProcessGUI( e.currentTarget.m_wnd, false);}

	var elTest = document.createElement('div');
	elAfDiv.appendChild( elTest);
	elTest.textContent = 'Test Inputs';
	elTest.classList.add('button');
	elTest.m_wnd = wnd;
	elTest.onclick = function(e){ d_CutProcessGUI( e.currentTarget.m_wnd, true);}

	var elResults = document.createElement('div');
	wnd.elContent.appendChild( elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');

	for( var i = 0; i < i_args.shots.length; i++)
	{
		el = document.createElement('div');
		elResults.appendChild( el);
		el.textContent = i_args.shots[i];
	}
}

function d_CutProcessGUI( i_wnd, i_test)
{
	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild( elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var shots = i_wnd.m_args.shots;
	var params = gui_GetParams( i_wnd.elContent, d_cutparams);
	for( key in i_wnd.elContent.m_choises )
		params[key] = i_wnd.elContent.m_choises[key].value;

	var cmd = 'rules/bin/makecut.sh';

	cmd += ' -i "' + params.input + '"';
	cmd += ' -n "' + params.cut_name + '"';
	cmd += ' -u "' + g_auth_user.id + '"';
	cmd += ' -f "' + params.fps + '"';
	cmd += ' -r "' + params.format + '"';
	cmd += ' -c "' + params.codec + '"';
	cmd += ' --colorspace "' + params.colorspace + '"';
	cmd += ' --afcapacity ' + RULES.cut.af_capacity;
	cmd += ' -o "' + cgru_PM('/' + RULES.root + params.output, true) + '"';
	if( i_test ) cmd += ' -t';

	for( var i = 0; i < shots.length; i++)
		cmd += ' "' + cgru_PM('/' + RULES.root + shots[i], true) + '"'

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":d_CutFinished,"wnd":i_wnd});
}
function d_CutFinished( i_data, i_args)
{
//console.log( JSON.stringify( i_data));
//console.log( JSON.stringify( i_args));
	i_args.wnd.elContent.removeChild( i_args.wnd.m_elWait);
	var elResults = i_args.wnd.m_elResults;
	elResults.textContent = '';

	if(( i_data.cmdexec == null ) || ( ! i_data.cmdexec.length ) || ( i_data.cmdexec[0].cut == null ))
	{
		elResults.textContent = ( JSON.stringify( i_data));
		return;
	}

	var cut = i_data.cmdexec[0].cut;

	for( var i = cut.length - 1; i >= 0; i--)
	{
		var el = document.createElement('div');
		elResults.appendChild( el);
		for( var msg in cut[i])
		{
			el.textContent = msg + ': ' + cut[i][msg];
			if(( msg == 'error' ) || ( cut[i][msg].indexOf('error') != -1 ))
				el.style.color = '#F42';
		}
	}

//	i_wnd.destroy();
}

