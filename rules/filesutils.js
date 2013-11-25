//========================== Put: ============================
fu_put_params = [];
fu_put_params.src = {"label":'Source', "disabled":true};
fu_put_params.dest = {"label":'Destination'};
fu_put_params.name = {};

function fu_Put( i_path)
{
	var wnd = new cgru_Window({"name":'put',"title":'Put Folder'});

	var params = {};

	params.src = i_path;

	params.dest = RULES.put.dest;
	if( params.dest.indexOf('/')  != 0 )
	{
		if( ASSETS.project )
			params.dest = ASSETS.project.path + '/' + params.dest;
		else
			params.dest = '/' + params.dest;
	}

	params.name = c_PathBase( i_path);
	if( ASSETS.shot )
	{
		params.name = ASSET.name;

		var version = i_path.split('/');
		version = version[version.length-1];
		var match = version.match(/v\d{2,}.*/gi);
		if( match )
			params.name += '_' + match[match.length-1];
	}

	gui_Create( wnd.elContent, fu_put_params, [params, RULES.put]);

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
	elSend.onclick = function(e){ fu_PutDo( e.currentTarget.m_wnd);}
	elSend.m_wnd = wnd;

	var elRules = document.createElement('div');
	wnd.elContent.appendChild( elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.put='+JSON.stringify(RULES.put).replace(/,/g,', ');
}

function fu_PutDo( i_wnd)
{
	var params = gui_GetParams( i_wnd.elContent, fu_put_params);
//	for( var p = 0; p < fu_put_params.length; p++)
//		params[fu_put_params[p].name] = i_wnd.m_elements[fu_put_params[p].name].textContent;

	i_wnd.destroy();

	var source = cgru_PM('/' + RULES.root+params.src,  true);
	var dest   = cgru_PM('/' + RULES.root+params.dest, true);

	var job = {};
	//job.offline = true;
	job.name = 'PUT ' + params.name;

	var block = {};
	block.name = 'put';
	block.service = RULES.put.af_service;
	block.parser  = RULES.put.af_parser;
	if( RULES.put.af_capacity ) block.capacity = RULES.put.af_capacity;
	job.blocks = [block];

	var task = {}
	task.name = params.name;
	block.tasks = [task];

	var cmd = cgru_PM( RULES.put.cmd, true);
	cmd += ' -s "' + source + '"';
	cmd += ' -d "' + dest + '"';
	cmd += ' -n "' + params.name + '"';
	task.command = cmd;

	if( RULES.put.post_delete )
		job.command_post = 'rm -rf "' + source + '"';

//job.offline = true;
	n_SendJob( job);

//console.log( task.command);
//console.log( JSON.stringify( params));
//console.log( JSON.stringify( job));
}


//========================== Checksum: ============================
fu_sum_params = [];
fu_sum_params.path = {"label":'Path', "disabled":true};
fu_sum_params.type = {"label":'Type', "disabled":true};
fu_sum_params.update_all = {"label":'Update All', "bool":false};

function fu_Checksum( i_args)
{
//console.log( JSON.stringify( i_args));
	if(( i_args.walk.files == null ) || ( i_args.walk.files.length == 0 ))
	{
		c_Error('Location has no files.');
		return;
	}

	var wnd = new cgru_Window({"name":'put',"title":'Put Folder'});
	wnd.i_walk = i_args.walk;

	var params = i_args;

	gui_Create( wnd.elContent, fu_sum_params, [params, RULES.checksum[i_args.type]]);

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
	elSend.onclick = function(e){ fu_ChecksumDo( e.currentTarget.m_wnd);}
	elSend.m_wnd = wnd;

	var elRules = document.createElement('div');
	wnd.elContent.appendChild( elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.checksum.'+i_args.type+'='+JSON.stringify(RULES.checksum[i_args.type]).replace(/,/g,', ');
}

function fu_ChecksumDo( i_wnd)
{
	var params = gui_GetParams( i_wnd.elContent, fu_sum_params);
	var walk = i_wnd.i_walk;
	i_wnd.destroy();

//console.log( JSON.stringify( params));
//return;

	var job = {};
	job.name = params.type + ':' + params.path;

	var block = {};
	block.name = params.type;
	block.service = RULES.checksum[params.type].af_service;
	block.parser  = RULES.checksum[params.type].af_parser;
	if( RULES.checksum[params.type].af_capacity)
		block.capacity = RULES.checksum[params.type].af_capacity;
	block.tasks = [];
	job.blocks = [block];

	for( var i = 0; i < walk.files.length; i++)
	{
		var file = walk.files[i];
		var task = {};

		var filepath = params.path + '/' + file.name;
		filepath = '/' + RULES.root + filepath;
		filepath = cgru_PM( filepath,  true);

		var cmd = 'python /cgru/utilities/checksum.py';

		cmd += ' -i "' + filepath + '"';
		cmd += ' -t "' + params.type + '"';
		task.command = cmd;
//console.log(cmd);
		task.name = file.name;

		block.tasks.push( task);
	}

//job.offline = true;
	n_SendJob( job);
}

