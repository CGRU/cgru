d_moviemaker = '/cgru/utilities/moviemaker';
d_makemovie = d_moviemaker+'/makemovie.py';
d_guiparams = {};
d_guiparams.project = {"width":'50%'};
d_guiparams.shot = {"width":'50%',"lwidth":'70px'};
d_guiparams.artist = {"width":'50%'};
d_guiparams.activity = {"width":'25%',"lwidth":'70px'};
d_guiparams.version = {"width":'25%',"lwidth":'70px'};
d_guiparams.input = {};
d_guiparams.output = {};
d_guiparams.filename = {}
d_guiparams.fps = {"label":'FPS',"width":'25%'};
d_guiparams.fffirst = {"label":"F.F.First","width":'25%',"lwidth":'70px',"tooltip":'First frame is "1"\nNo matter image file name number.'};
d_guiparams.aspect_in = {"label":'Aspect In',"width":'25%',"lwidth":'70px'};
d_guiparams.gamma = {"width":'25%',"lwidth":'70px'};

d_cvtguiparams = {};
d_cvtguiparams.cvtres = {"label":'Resolution',"info":'WIDTH or WIDTHxHEIGHT ( e.g. 1280x720 ). On empty no changes.',"iwidth":"50%"};
d_cvtguiparams.fps = {"label":'FPS'};
d_cvtguiparams.time_start = {"default":'00:00:00',"width":'50%'};
d_cvtguiparams.duration   = {"default":'00:00:00',"width":'50%'};
d_cvtguiparams.quality = {"label":'JPEG Quality',"default":'100'};

d_cutparams = {};
d_cutparams.cut_name = {};
d_cutparams.input = {};
d_cutparams.fps = {"label":'FPS'};
d_cutparams.output = {};

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

	var readir = n_WalkDir({"paths":[i_path]})[0];
	if( readir && readir.files && readir.files.length )
	for( var f = 0; f < readir.files.length; f++ )
	{
		var file = readir.files[f].name;
		var match = file.match(/\d{1,}\./g);
		if( match )
		{
			match = match[match.length-1];
			var pos = file.lastIndexOf( match);
			var pattern = file.substr( 0, pos);
			for( var d = 0; d < match.length-1; d++ ) pattern += '#';
			pattern += file.substr( pos-1 + match.length);
			i_path += '/' + pattern;
			break;
		}
//window.console.log( match);
	}

	params.input = i_path;
	params.output = i_outfolder;

	params.artist = c_GetUserTitle();
	params.activity = RULES.dailies.activity;

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

	gui_Create( wnd.elContent, d_guiparams, [params, RULES.dailies]);
	gui_CreateChoises({"wnd":wnd.elContent,"name":'colorspace',"value":RULES.dailies.colorspace,"label":'Colorspace:',"keys":RULES.dailies.colorspaces});
	gui_CreateChoises({"wnd":wnd.elContent,"name":'format',"value":RULES.dailies.format,"label":'Formats:',"keys":RULES.dailies.formats});
	gui_CreateChoises({"wnd":wnd.elContent,"name":'codec',"value":RULES.dailies.codec,"label":'Codecs:',"keys":RULES.dailies.codecs});

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

	var elRules = document.createElement('div');
	wnd.elContent.appendChild( elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.dailies='+JSON.stringify(RULES.dailies).replace(/,/g,', ');
}

function d_ProcessGUI( i_wnd)
{
	var params = gui_GetParams( i_wnd.elContent, d_guiparams);

	for( key in i_wnd.elContent.m_choises )
		params[key] = i_wnd.elContent.m_choises[key].value;

	i_wnd.destroy();

	if( g_auth_user == null )
	{
		c_Error("Guests can't generate dailies.");
		return;
	}

	var job = {};
	//job.offline = true;
	job.name = params.filename;

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

function d_Convert( i_args)
{
	var params = {};
//	params.quality = 80;
//	params.cvtres = '';

	var title = 'Convert ';
	if( i_args.images ) title += ' Images';
	else if( i_args.folders ) title += ' Sequences';
	else if( i_args.movies) title += ' Movies';
	var wnd = new cgru_Window({"name":'dailes',"title":title});
	wnd.m_args = i_args;

	gui_Create( wnd.elContent, d_cvtguiparams, [params, RULES.dailies]);
	gui_CreateChoises({"wnd":wnd.elContent,"name":'codec',"value":RULES.dailies.codec,"label":'Codecs:',"keys":RULES.dailies.codecs});
	if( i_args.movies == false )
		gui_CreateChoises({"wnd":wnd.elContent,"name":'colorspace',"value":RULES.dailies.colorspace,"label":'Colorspace:',"keys":RULES.dailies.colorspaces});

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);
	elBtns.style.clear = 'both';
	elBtns.classList.add('buttons');
	elBtns.classList.add('param');

	var elLabel = document.createElement('div');
	elBtns.appendChild( elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';

	var elCvtBtn = document.createElement('div');
	elBtns.appendChild( elCvtBtn);
	elCvtBtn.textContent = title + ' To Movies';
	elCvtBtn.classList.add('button');
	elCvtBtn.onclick = function(e){ d_CvtProcessGUI( e.currentTarget.m_wnd, false);}
	elCvtBtn.m_wnd = wnd;

	var elExpBtn = document.createElement('div');
	elBtns.appendChild( elExpBtn);
	elExpBtn.textContent = title + ' To Sequences';
	elExpBtn.classList.add('button');
	elExpBtn.onclick = function(e){ d_CvtProcessGUI( e.currentTarget.m_wnd, true);}
	elExpBtn.m_wnd = wnd;

	var elSrc = document.createElement('div');
	wnd.elContent.appendChild( elSrc);
	elSrc.classList.add('source');
	wnd.m_elSrc = elSrc;
	for( var i = 0; i < i_args.paths.length; i++)
	{
		var el = document.createElement('div');
		elSrc.appendChild( el);
		el.textContent = i_args.paths[i];
	}
}

function d_CvtProcessGUI( i_wnd, i_to_sequence)
{
	var params = gui_GetParams( i_wnd.elContent, d_cvtguiparams);
	for( key in i_wnd.elContent.m_choises )
		params[key] = i_wnd.elContent.m_choises[key].value;

	if( i_wnd.m_args.movies || ( i_to_sequence == false ))
	{
		d_CvtMovies( i_wnd.m_args, params, i_to_sequence );
		i_wnd.destroy();
	}
	else
	{
		d_CvtImages( i_wnd, i_wnd.m_args, params );
	}
}

function d_CvtImages( i_wnd, i_args, i_params)
{
//console.log( JSON.stringify( i_args));
//console.log( JSON.stringify( i_params));
	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild( elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var cmd = 'rules/bin/convert.sh -J';

	cmd += ' -c ' + i_params.colorspace;
	cmd += ' -q ' + i_params.quality;
	if( i_params.cvtres != '' ) cmd += ' -r ' + i_params.cvtres;

	var afanasy = false;
	if( i_args.folders || ( i_args.paths.length > 1 ))
	{
		afanasy = true;
		cmd += ' -A';
		cmd += ' --afuser "' + g_auth_user.id + '"';
	}

	for( var i = 0; i < i_args.paths.length; i++)
		cmd += ' "' + cgru_PM('/' + RULES.root + i_args.paths[i], true) + '"'

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

	var elOut = i_args.wnd.m_elSrc;
	elOut.textContent = '';
	elOut.classList.remove('source');
	elOut.classList.add('output');

	if(( i_data.cmdexec == null ) || ( i_data.cmdexec.length == 0 ) || ( i_data.cmdexec[0].convert == null ))
	{
		elOut.textContent = JSON.stringify( i_data);
		return;
	}

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

function d_CvtMovies( i_args, i_params, i_to_sequence )
{
	var job = {};
	job.name = c_PathBase( i_args.paths[0]);
	if( i_args.paths.length > 1 )
		job.name = c_PathDir( i_args.paths[0]) + ' x' + i_args.paths.length;
	if( i_params.cvtres.length ) job.name += '-' + i_params.cvtres;
	//job.offline = true;

	var block = {};
	block.service = 'movgen';
	block.parser = 'generic';
	if( RULES.dailies.af_capacity ) block.capacity = RULES.dailies.af_capacity;
	block.working_directory = cgru_PM('/' + RULES.root + c_PathDir( i_args.paths[0]), true);
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
		job.name = 'Explode ' + job.name;
		block.name = 'Explode';
		cmd += ' -t jpg';
		var q = parseInt( i_params.quality);
		q = Math.round( 10 - ( q / 10 ));
		if( q < 1 ) q = 1;
		cmd += ' -q ' + q;
	}
	else
	{
		job.name = 'Convert ' + job.name;
		block.name = 'Convert';
		cmd += ' -c "' + i_params.codec + '"';
		cmd += ' -f ' + i_params.fps;
	}

	for( var i = 0; i < i_args.paths.length; i++)
	{
		var path = i_args.paths[i];
		var task = {};
		task.name = c_PathBase( path);
		task.command = cmd + ' "' + cgru_PM('/' + RULES.root + path, true) + '"';
		block.tasks.push( task);
	}

	n_SendJob( job);
}

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
	gui_CreateChoises({"wnd":wnd.elContent,"name":'codec',"value":RULES.dailies.codec,"label":'Codecs:',"keys":RULES.dailies.codecs});
	gui_CreateChoises({"wnd":wnd.elContent,"name":'format',"value":RULES.dailies.format,"label":'Formats:',"keys":RULES.dailies.formats});

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

