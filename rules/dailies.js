d_moviemaker = '/cgru/utilities/moviemaker';
d_makemovie = d_moviemaker+'/makemovie.py';
d_guiparams = [];
d_guiparams.push({"name":'project',"width":'50%'});
d_guiparams.push({"name":'shot',"width":'50%',"lwidth":'70px'});
d_guiparams.push({"name":'artist',"width":'50%'});
d_guiparams.push({"name":'activity',"width":'25%',"lwidth":'70px'});
d_guiparams.push({"name":'version',"width":'25%',"lwidth":'70px'});
d_guiparams.push({"name":'input'});
d_guiparams.push({"name":'output'});
d_guiparams.push({"name":'filename'});
//d_guiparams.push({"name":'format',"width":'32%'});
d_guiparams.push({"name":'fps',"label":'FPS',"width":'25%'});
d_guiparams.push({"name":'fffirst',"label":"F.F.First","width":'25%',"lwidth":'70px'});
d_guiparams.push({"name":'aspect_in',"label":'Aspect In',"width":'50%',"lwidth":'170px'});
d_guiparams.push({"name":'gamma',"width":'50%'});
d_guiparams.push({"name":'no_auto_colospace',"label":'No Auto Color Space',"width":'50%',"lwidth":'170px'});
//d_guiparams.push({"name":'codec'});

d_cvtguiparams = [];
d_cvtguiparams.push({"name":'cvtres',"label":'Resolution',"info":'on empty no changes'});
d_cvtguiparams.push({"name":'fps',"label":'FPS'});

d_expguiparams = [];
d_expguiparams.push({"name":'quality',"label":'JPEG Images Compression Rate',"lwidth":'250px',"info":'1 is the best quality'});

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
	wnd.elContent.classList.add('dialog');

	c_CreateGUI( wnd, d_guiparams, [params, RULES.dailies]);
	d_CreateGUI_Choises({"wnd":wnd,"name":'format',"value":RULES.dailies.format,"label":'Formats:',"keys":RULES.dailies.formats});
	d_CreateGUI_Choises({"wnd":wnd,"name":'codec',"value":RULES.dailies.codec,"label":'Codecs:',"keys":RULES.dailies.codecs});

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);
	elBtns.style.clear = 'both';

	var elAfDiv = document.createElement('div');
	elBtns.appendChild( elAfDiv);
	elAfDiv.classList.add('label');
	elAfDiv.innerHTML = '<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';

	var elSend = document.createElement('div');
	elBtns.appendChild( elSend);
	elSend.textContent = 'Send Job';
	elSend.classList.add('button');
	elSend.onclick = function(e){ d_ProcessGUI( e.currentTarget.m_wnd);}
	elSend.m_wnd = wnd;

	var elRules = document.createElement('div');
	wnd.elContent.appendChild( elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.dailies='+JSON.stringify(RULES.dailies).replace(/,/g,', ');
}

function d_CreateGUI_Choises( i_args)
{
	var wnd = i_args.wnd;
	var name = i_args.name;
	var def_val = i_args.value;
	var label = i_args.label;
	var keys = i_args.keys;

	if( wnd.m_choises == null )
		wnd.m_choises = {};

	wnd.m_choises[name] = {};
	wnd.m_choises[name].value = def_val;
	wnd.m_choises[name].elements = [];
	
	var elDiv = document.createElement('div');
	wnd.elContent.appendChild( elDiv);
	elDiv.style.clear = 'both';
	var elLabel = document.createElement('div');
	elDiv.appendChild( elLabel);
	elLabel.textContent = label;
	elLabel.classList.add('label');
	for( var key in keys)
	{
		var el = document.createElement('div');
		elDiv.appendChild( el);
		el.classList.add('choise');
		el.classList.add('button');
		el.textContent = keys[key].name;
		el.onclick = d_ChoiseOnClick;
		var value = key;
		if( keys[key].value ) value = keys[key].value;
		if( value == def_val ) el.classList.add('selected');

		el.m_value = value;
		el.m_wnd = wnd;
		el.m_name = name;

		wnd.m_choises[name].elements.push( el);
	}
}
function d_ChoiseOnClick( i_evt)
{
	var el = i_evt.currentTarget;
	var wnd = el.m_wnd;
	var name = el.m_name;
	var elements = wnd.m_choises[name].elements;

	for( var i = 0; i < elements.length; i++)
		elements[i].classList.remove('selected');
	el.classList.add('selected');
	wnd.m_choises[name].value = el.m_value;
}

function d_ProcessGUI( i_wnd)
{
	var params = {};
	for( var p = 0; p < d_guiparams.length; p++)
		params[d_guiparams[p].name] = i_wnd.m_elements[d_guiparams[p].name].textContent;

	for( key in i_wnd.m_choises )
		params[key] = i_wnd.m_choises[key].value;

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

	nw_MakeNews('<i>dailies</i>');
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

	cmd += ' -c "'+params.codec+'"';
	cmd += ' -f '+params.fps;
	cmd += ' -r '+params.format;
	cmd += ' -s '+params.slate;
	cmd += ' -t '+params.template;

	if( params.gamma != '' )
		cmd += ' -g ' + params.gamma;
	if( params.no_auto_colospace != '' )
		cmd += ' --noautocorr';
	if( params.no_auto_colospace != '' )
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

	cmd += ' "'+input+'"';
	cmd += ' "'+output+'"';

//python "/cgru/utilities/moviemaker/makemovie.py" -c "/cgru/utilities/moviemaker/codecs/photojpg_best.ffmpeg" -f 25 -n mov --fs 1 --fe 20 -r 720x576x1.09 -g 1.00 -s "dailies_slate" -t "dailies_withlogo" --project "ENCODE" --shot "preview" --ver "preview" --artist "Timurhai" --activity "comp" --tmpformat tga --lgspath "logo.png" --lgssize 25 --lgsgrav SouthEast --lgfpath "logo.png" --lgfsize 10 --lgfgrav North "/data/tools/encode/preview/preview.####.jpg" "/data/tools/encode/preview_preview_121226"

	return cmd;
}

function d_Convert( i_path)
{
	var params = {};
	params.quality = 5;
	params.cvtres = '';

	var wnd = new cgru_Window({"name":'dailes',"title":'Convert Movie'});
	wnd.elContent.classList.add('dialog');

	var elSrc = document.createElement('div');
	wnd.elContent.appendChild( elSrc);
	elSrc.classList.add('source');
	elSrc.textContent = i_path;

	c_CreateGUI( wnd, d_cvtguiparams, [params, RULES.dailies]);
	d_CreateGUI_Choises({"wnd":wnd,"name":'codec',"value":RULES.dailies.codec,"label":'Codecs:',"keys":RULES.dailies.codecs});

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);

	var elCvtBtn = document.createElement('div');
	elBtns.appendChild( elCvtBtn);
	elCvtBtn.textContent = 'Convert Movie To Another Movie';
	elCvtBtn.classList.add('button');
	elCvtBtn.onclick = function(e){ d_CvtProcessGUI( e.currentTarget.m_wnd);}
	elCvtBtn.m_wnd = wnd;

	var div = document.createElement('div');
	wnd.elContent.appendChild(div );
	div.style.margin = '10px';

	c_CreateGUI( wnd, d_expguiparams, [params]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);
	elBtns.classList.add('buttons');

	var elExpBtn = document.createElement('div');
	elBtns.appendChild( elExpBtn);
	elExpBtn.textContent = 'Explode Movie To Images Sequence';
	elExpBtn.classList.add('button');
	elExpBtn.onclick = function(e){ d_ExpProcessGUI( e.currentTarget.m_wnd, false);}
	elExpBtn.m_wnd = wnd;

	var elExpBtn = document.createElement('div');
	elBtns.appendChild( elExpBtn);
	elExpBtn.textContent = 'Send Job to AFANASY';
	elExpBtn.classList.add('button');
	elExpBtn.onclick = function(e){ d_ExpProcessGUI( e.currentTarget.m_wnd, true);}
	elExpBtn.m_wnd = wnd;

	wnd.m_path = i_path;
}

function d_CvtProcessGUI( i_wnd)
{
	var params = {};
	for( var p = 0; p < d_expguiparams.length; p++)
		params[d_expguiparams[p].name] = i_wnd.m_elements[d_expguiparams[p].name].textContent;
	for( var p = 0; p < d_cvtguiparams.length; p++)
		params[d_cvtguiparams[p].name] = i_wnd.m_elements[d_cvtguiparams[p].name].textContent;
	for( key in i_wnd.m_choises )
		params[key] = i_wnd.m_choises[key].value;

	var cmd = 'movconvert';
	cmd += ' -a ' + RULES.avconv;
	cmd += ' -c "' + params.codec + '"';
	cmd += ' -f ' + params.fps;
	if( params.cvtres.length ) cmd += ' -x ' + params.cvtres;
	cmd += ' "' + cgru_PM('/' + RULES.root + i_wnd.m_path, true) + '"';

	var job = {};
	//job.offline = true;
	job.name = c_PathBase( i_wnd.m_path) + '-' + params.codec;
	if( params.cvtres.length ) job.name += '-' + params.cvtres;

	var block = {};
	block.name = 'Convert';
	block.service = 'movgen';
	block.parser = 'generic';
	if( RULES.dailies.af_capacity ) block.capacity = RULES.dailies.af_capacity;
	block.working_directory = cgru_PM('/' + RULES.root + c_PathDir(i_wnd.m_path), true);
	job.blocks = [block];

	var task = {}
	task.name = c_PathBase( i_wnd.m_path);
	task.command = cmd;
	block.tasks = [task];

	n_SendJob( job);
	i_wnd.destroy();
}

function d_ExpProcessGUI( i_wnd, i_afanasy)
{
	var params = {};
	for( var p = 0; p < d_expguiparams.length; p++)
		params[d_expguiparams[p].name] = i_wnd.m_elements[d_expguiparams[p].name].textContent;
	for( var p = 0; p < d_cvtguiparams.length; p++)
		params[d_cvtguiparams[p].name] = i_wnd.m_elements[d_cvtguiparams[p].name].textContent;

	var cmd = 'utilities/moviemaker/movconvert.py';
	if( i_afanasy ) cmd = cgru_PM('/cgru/' + cmd, true);
	cmd += ' -t jpg';
	cmd += ' -a ' + RULES.avconv;
	cmd += ' -q ' + params.quality;
	if( params.cvtres.length ) cmd += ' -x ' + params.cvtres;
	cmd += ' "' + cgru_PM('/' + RULES.root + i_wnd.m_path, true) + '"';

	if( i_afanasy !== true )
	{
		i_wnd.elContent.classList.add('waiting');
		n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":d_ExpFinished,"wnd":i_wnd});
		return;
	}

	var job = {};
	//job.offline = true;
	job.name = c_PathBase( i_wnd.m_path);

	var block = {};
	block.name = 'Explode';
	block.service = 'movgen';
	block.parser = 'generic';
	if( RULES.dailies.af_capacity ) block.capacity = RULES.dailies.af_capacity;
	block.working_directory = cgru_PM('/' + RULES.root + c_PathDir(i_wnd.m_path), true);
	job.blocks = [block];

	var task = {}
	task.name = c_PathBase( i_wnd.m_path);
	task.command = cmd;
	block.tasks = [task];

	n_SendJob( job);

	i_wnd.destroy();
}
function d_ExpFinished( i_data, i_args)
{
	i_args.wnd.destroy();
	fv_ReloadAll();
}

