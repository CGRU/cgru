d_moviemaker = '/cgru/utilities/moviemaker';
d_makemovie = d_moviemaker+'/makemovie.py';
d_guiparams = [];
d_guiparams.push({"name":'artist',"width":'50%'});
d_guiparams.push({"name":'activity',"width":'25%',"lwidth":'70px'});
d_guiparams.push({"name":'version',"width":'25%',"lwidth":'70px'});
d_guiparams.push({"name":'input'});
d_guiparams.push({"name":'output'});
d_guiparams.push({"name":'filename'});
d_guiparams.push({"name":'resolution',"width":'32%'});
d_guiparams.push({"name":'fps',"label":'FPS',"width":'18%',"lwidth":'30px'});
d_guiparams.push({"name":'aspect_in',"label":'Aspect In',"width":'50%',"lwidth":'70px'});
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
	filename = filename.replace('(p)', ASSETS.project.name);
	filename = filename.replace('(P)', ASSETS.project.name.toUpperCase());
	filename = filename.replace('(s)', ASSETS.shot.name);
	filename = filename.replace('(S)', ASSETS.shot.name.toUpperCase());
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

	var wnd = new cgru_Window('dailes','Make Dailies');
	wnd.elContent.classList.add('dailies');

	d_CreateGUI( wnd, d_guiparams, [params, RULES.dailies]);
	d_CreateGUI_Codecs( wnd);

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

function d_CreateGUI_Codecs( i_wnd)
{
	i_wnd.m_codec = RULES.dailies.codec;
	i_wnd.m_elCodecs = [];
	var elCodecDiv = document.createElement('div');
	i_wnd.elContent.appendChild( elCodecDiv);
	elCodecDiv.style.clear = 'both';
	var elCodecLabel = document.createElement('div');
	elCodecDiv.appendChild( elCodecLabel);
	elCodecLabel.textContent = 'Codec:';
	elCodecLabel.classList.add('label');
	for( var codec in RULES.dailies.codecs)
	{
		var el = document.createElement('div');
		elCodecDiv.appendChild( el);
		el.classList.add('choise');
		el.classList.add('button');
		el.textContent = RULES.dailies.codecs[codec];
		el.onclick = d_CodecOnClick;
		if( codec == i_wnd.m_codec )
			el.classList.add('selected');

		el.m_codec = codec;
		el.m_wnd = i_wnd;

		i_wnd.m_elCodecs.push( el);
	}
}
function d_CodecOnClick( i_evt)
{
	var el = i_evt.currentTarget;
	for( var i = 0; i < el.m_wnd.m_elCodecs.length; i++)
		el.m_wnd.m_elCodecs[i].classList.remove('selected');
	el.classList.add('selected');
	el.m_wnd.m_codec = el.m_codec;
}

function d_ProcessGUI( i_wnd)
{
	var params = {};
	for( var p = 0; p < d_guiparams.length; p++)
		params[d_guiparams[p].name] = i_wnd.m_elements[d_guiparams[p].name].textContent;
	params['codec'] = i_wnd.m_codec;

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
	block.working_directory = cgru_PM('/'+RULES.root+params.output);
	job.blocks = [block];

	var task = {}
	task.name = params.filename;
	task.command = d_MakeCmd( params);
	block.tasks = [task];

	n_SendJob( job);

	nw_MakeNews('<i>dailies</i>');
}

function d_MakeCmd( i_params)
{
	var params = c_CloneObj( RULES.dailies);
	for( var parm in i_params )
		params[parm] = i_params[parm];

	var input = cgru_PM('/'+RULES.root+params.input);
	var output = cgru_PM('/'+RULES.root+params.output+'/'+params.filename);

	var cmd = 'python';

	cmd += ' "'+cgru_PM( d_makemovie)+'"';

	cmd += ' -c "'+params.codec+'"';
	cmd += ' -f '+params.fps;
	cmd += ' -r '+params.resolution;
	cmd += ' -s '+params.slate;
	cmd += ' -t '+params.template;
	cmd += ' --lgspath "'+params.logo_slate_path+'"';
	cmd += ' --lgssize '+params.logo_slate_size;
	cmd += ' --lgsgrav '+params.logo_slate_grav;
	cmd += ' --lgfpath "'+params.logo_frame_path+'"';
	cmd += ' --lgfsize '+params.logo_frame_size;
	cmd += ' --lgfgrav '+params.logo_frame_grav;

	cmd += ' --project "'+ASSETS.project.name+'"';
	cmd += ' --shot "'+ASSETS.shot.name+'"';

	cmd += ' --ver "'+params.version+'"';
	cmd += ' --artist "'+params.artist+'"';
	cmd += ' --activity "'+params.activity+'"';

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

	var wnd = new cgru_Window('dailes','Convert Movie');
	wnd.elContent.classList.add('dailies');

	var elSrc = document.createElement('div');
	wnd.elContent.appendChild( elSrc);
	elSrc.classList.add('source');
	elSrc.textContent = i_path;

	d_CreateGUI( wnd, d_cvtguiparams, [params, RULES.dailies]);
	d_CreateGUI_Codecs( wnd);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);

	var elCvtBtn = document.createElement('div');
	elBtns.appendChild( elCvtBtn);
	elCvtBtn.textContent = 'Convert Movie To Another Movie';
	elCvtBtn.classList.add('button');
	elCvtBtn.onclick = function(e){ d_CvtProcessGUI( e.currentTarget.m_wnd);}
	elCvtBtn.m_wnd = wnd;

	d_CreateGUI( wnd, d_expguiparams, [params]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);

	var elExpBtn = document.createElement('div');
	elBtns.appendChild( elExpBtn);
	elExpBtn.textContent = 'Explode Movie To Images Sequence';
	elExpBtn.classList.add('button');
	elExpBtn.onclick = function(e){ d_ExpProcessGUI( e.currentTarget.m_wnd);}
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
	params['codec'] = i_wnd.m_codec;

	var cmd = 'movconvert';
	cmd += ' -a ' + RULES.avconv;
	cmd += ' -c "' + params.codec + '"';
	cmd += ' -f ' + params.fps;
	if( params.cvtres.length ) cmd += ' -x ' + params.cvtres;
	cmd += ' "' + cgru_PM('/' + RULES.root + i_wnd.m_path) + '"';

	var job = {};
	//job.offline = true;
	job.name = c_PathBase( i_wnd.m_path) + '-' + params.codec;
	if( params.cvtres.length ) job.name += '-' + params.cvtres;

	var block = {};
	block.name = 'Dailies';
	block.service = 'movgen';
	block.parser = 'generic';
	if( RULES.dailies.af_capacity ) block.capacity = RULES.dailies.af_capacity;
	block.working_directory = cgru_PM('/' + RULES.root + c_PathDir(i_wnd.m_path));
	job.blocks = [block];

	var task = {}
	task.name = c_PathBase( i_wnd.m_path);
	task.command = cmd;
	block.tasks = [task];

	n_SendJob( job);
	i_wnd.destroy();
}

function d_ExpProcessGUI( i_wnd)
{
	var params = {};
	for( var p = 0; p < d_expguiparams.length; p++)
		params[d_expguiparams[p].name] = i_wnd.m_elements[d_expguiparams[p].name].textContent;
	for( var p = 0; p < d_cvtguiparams.length; p++)
		params[d_cvtguiparams[p].name] = i_wnd.m_elements[d_cvtguiparams[p].name].textContent;

	var cmd = 'utilities/moviemaker/movconvert.py -t jpg';
	cmd += ' -a ' + RULES.avconv;
	cmd += ' -q ' + params.quality;
	if( params.cvtres.length )
		cmd += ' -x ' + params.cvtres;
	cmd += ' "' + cgru_PM('/' + RULES.root + i_wnd.m_path) + '"';

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"wait":true});
	i_wnd.destroy();
	fv_ReloadAll();
}

function d_CreateGUI( i_wnd, i_params, i_defaults)
{
	if( i_wnd.m_elements == null ) i_wnd.m_elements = {};
	for( var p = 0; p < i_params.length; p++)
	{
		var elDiv = document.createElement('div');
		i_wnd.elContent.appendChild( elDiv);

		if( i_params[p].width != null )
		{
			elDiv.style.cssFloat = 'left';
			elDiv.style.width = i_params[p].width;
		}
		else
			elDiv.style.clear = 'both';

		var elLabel = document.createElement('div');
		elDiv.appendChild( elLabel);
		elLabel.classList.add('label');
		if( i_params[p].label )
			elLabel.textContent = i_params[p].label+':';
		else
		{
			elLabel.textContent = i_params[p].name+':';
			elLabel.style.textTransform = 'capitalize';
		}
		if( i_params[p].lwidth )
			elLabel.style.width = i_params[p].lwidth;

		if( i_params[p].info )
		{	
			var elInfo = document.createElement('div');
			elDiv.appendChild( elInfo);
			elInfo.classList.add('info');
			elInfo.textContent = i_params[p].info;
			if( i_params[p].iwidth )
				elInfo.style.width = i_params[p].iwidth;
		}

		var elParam = document.createElement('div');
		elDiv.appendChild( elParam);
		elParam.classList.add('param');
		elParam.classList.add('editing');
		if( i_defaults )
			for( var d = 0; d < i_defaults.length; d++)
				if( i_defaults[d][i_params[p].name] )
				{
					elParam.textContent = i_defaults[d][i_params[p].name];
					break;
				}
		elParam.contentEditable = 'true';

		i_wnd.m_elements[i_params[p].name] = elParam;
	}
}

