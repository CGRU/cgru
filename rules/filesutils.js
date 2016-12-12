//========================== Put: ============================
fu_put_params = {};
//fu_put_params.src = {"label":'Source', "disabled":true};
fu_put_params.dest = {"label":'Destination'};
//fu_put_params.name = {};

fu_putftp_params = {};
fu_putftp_params.host = {"label":'FTP Server'};
fu_putftp_params.user = {"label":'FTP User',"width":'50%'};
fu_putftp_params.pass = {"lwidth":'150px',"label":'FTP Password',"width":'50%'};

function fu_Put( i_args)
{
	var wnd = new cgru_Window({"name":'put',"title":'Put Folder'});
	wnd.m_args = i_args;
	i_args.names = [];
	var params = {};

	params.dest = RULES.put.dest;
	if( params.dest.indexOf('/')  != 0 )
	{
		if( ASSETS.project )
			params.dest = ASSETS.project.path + '/' + params.dest;
		else
			params.dest = '/' + params.dest;

		params.dest = c_PathPM_Rules2Client( params.dest);
	}
	else
		params.dest = c_PathPM_Server2Client( params.dest);

	gui_Create( wnd.elContent, fu_put_params, [RULES.put, params]);
	if( RULES.put.ftp )
		gui_Create( wnd.elContent, fu_putftp_params, [RULES.put.ftp, params]);

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
	elSend.onclick = function(e){ fu_PutDo( e.currentTarget.m_wnd);}
	elSend.m_wnd = wnd;

	var elResults = document.createElement('div');
	wnd.elContent.appendChild( elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');
	for( var i = 0; i < i_args.paths.length; i++)
	{
        i_args.paths[i] = c_PathPM_Rules2Client(i_args.paths[i]);

		var name = c_PathBase( i_args.paths[i]);
		if( ASSETS.shot )
		{
			name = ASSET.name;
			var version = i_args.paths[i].split('/');
			version = version[version.length-1];
			var match = version.match(/v\d{2,}.*/gi);
			if( match )
				name += '_' + match[match.length-1];
		}
		i_args.names.push( name);

		el = document.createElement('div');
		elResults.appendChild( el);
		el.textContent = i_args.paths[i] + ' -> ' + name;
	}

	var elRules = document.createElement('div');
	wnd.elContent.appendChild( elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.put='+JSON.stringify(RULES.put).replace(/,/g,', ');
}

function fu_PutDo( i_wnd)
{
	var params = gui_GetParams( i_wnd.elContent, fu_put_params);
	if( RULES.put.ftp )
		gui_GetParams( i_wnd.elContent, fu_putftp_params, params);

	for( var i = 0; i < i_wnd.m_args.paths.length; i++)
	{
	var source = c_PathPM_Client2Server( i_wnd.m_args.paths[i]);
	params.dest = c_PathPM_Client2Server( params.dest);
	var name   = i_wnd.m_args.names[i];

	var job = {};
	job.folders = {};
	job.folders.source = source;

	var block = {};
	block.name = 'put';
	block.service = RULES.put.af_service;
	block.parser  = RULES.put.af_parser;
	if( RULES.put.af_capacity ) block.capacity = RULES.put.af_capacity;
	job.blocks = [block];

	var task = {}
	task.name = name;
	block.tasks = [task];

	var cmd = c_PathPM_Client2Server( RULES.put.cmd);
	cmd += ' -s "' + source + '"';
	if( RULES.put.ftp )
	{
		job.name = 'FTP ' + name;
		cmd += ' --ftp ' + params.host;
		if( params.user.length ) cmd += ' --ftpuser ' + params.user;
		if( params.pass.length ) cmd += ' --ftppass ' + params.pass;
		cmd += ' -d "' + params.dest + '"';
	}
	else
	{
		job.name = 'PUT ' + name;
		cmd += ' -d "' + params.dest + '"';
		cmd += ' -n "' + name + '"';
		if( RULES.put.post_delete )
			job.command_post = 'rm -rf "' + source + '"';
	}

	task.command = cmd;


//job.offline = true;
	n_SendJob( job);
	}
//console.log( task.command);
//console.log( JSON.stringify( params));
//console.log( JSON.stringify( job));
	i_wnd.destroy();
}


//========================== Checksum: ============================
fu_sum_params = [];
fu_sum_params.path = {"label":'Path', "disabled":true};
fu_sum_params.type = {"label":'Type', "disabled":true};
fu_sum_params.update_all = {"label":'Update All','type':"bool",'default':false};

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
	elAfDiv.classList.add('param');

	var elLabel = document.createElement('div');
	elAfDiv.appendChild( elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';

	var elSend = document.createElement('div');
	elAfDiv.appendChild( elSend);
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

		var cmd = cgru_PM( RULES.checksum[params.type].cmd, true);

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

//
// ########################### Multi Put: ################################# //
//

fu_putmulti_params = {};
fu_putmulti_params.input        = {"label":'Result Paths'};
fu_putmulti_params.skipexisting = {"label":'Skip Existing','type':"bool",'default':true, "width":'33%'};
fu_putmulti_params.skiperrors   = {"label":'Skip Errors',  'type':"bool",'default':false,"width":'33%'};
fu_putmulti_params.skipcheck    = {"label":'Skip Check',   'type':"bool",'default':false,"width":'33%'};
fu_putmulti_params.dest         = {"label":'Destination'};
fu_putmulti_params.af_capacity  = {'label':'Capacity',  'width':'20%','type':'int',};
fu_putmulti_params.af_maxtasks  = {'label':'Max Tasks', 'width':'15%','lwidth':'80px','type':'int'};
fu_putmulti_params.af_perhost   = {'label':'Per Host',  'width':'15%','lwidth':'80px','type':'int'};
fu_putmulti_params.af_hostsmask = {'label':'Hosts Mask','width':'35%','lwidth':'100px'};
fu_putmulti_params.af_paused    = {'label':'Paused',    'width':'15%','lwidth':'50px','type':'bool'};

fu_findres_params = {};
fu_findres_params.input = {};
fu_findres_params.dest = {};
fu_findres_params.skiperrors = {'type':"bool",'default':false};
fu_findres_params.skipcheck = {'type':"bool",'default':false};

function fu_PutMultiDialog( i_args)
{
//console.log( JSON.stringify( i_args));
	var wnd = new cgru_Window({"name":'put',"title":'Put Results'});
	wnd.m_args = i_args;

	var params = {};

	params.input = RULES.assets.shot.result.path.join(',');
	if( RULES.put.input ) params.input = RULES.put.input;

	if( RULES.put.dest.indexOf('/') !== 0 )
		if( ASSETS.project )
			params.dest = c_PathPM_Rules2Client( ASSETS.project.path + '/' + RULES.put.dest);

	gui_Create( wnd.elContent, fu_putmulti_params, [RULES.put, params]);
	if( RULES.put.ftp )
		gui_Create( wnd.elContent, fu_putftp_params, [RULES.put.ftp, params]);

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

	var elSendJob = document.createElement('div');
	elAfDiv.appendChild( elSendJob);
	elSendJob.textContent = 'Send Job';
	elSendJob.classList.add('button');
	elSendJob.style.display = 'none';
	elSendJob.m_wnd = wnd;
	elSendJob.onclick = function(e){ fu_PutMultiDo( e.currentTarget.m_wnd);}
	wnd.m_res_btns_show = [elSendJob];

	var elFind = document.createElement('div');
	elAfDiv.appendChild( elFind);
	elFind.textContent = 'Find Results';
	elFind.classList.add('button');
	elFind.style.cssFloat = 'right';
	elFind.m_wnd = wnd;
	elFind.onclick = function(e){ fu_ResultsFind( e.currentTarget.m_wnd);}

	var elResults = document.createElement('div');
	wnd.elContent.appendChild( elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');

	for( var i = 0; i < i_args.paths.length; i++)
	{
		i_args.paths[i] = c_PathPM_Rules2Client( i_args.paths[i]);
		el = document.createElement('div');
		elResults.appendChild( el);
		el.textContent = i_args.paths[i];
	}
}
function fu_ResultsFind( i_wnd)
{
	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild( elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var paths = i_wnd.m_args.paths;
	var params = gui_GetParams( i_wnd.elContent, fu_findres_params);

	var cmd = 'rules/bin/find_results.py';
	cmd += ' -r "' + params.input + '"';
	cmd += ' -d "' + c_PathPM_Client2Server( params.dest) + '"';
	if( params.skipcheck  ) cmd += ' --skipcheck';
	if( params.skiperrors ) cmd += ' --skiperrors';

	for( var i = 0; i < paths.length; i++)
		cmd += ' "' + c_PathPM_Client2Server( paths[i]) + '"';

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":fu_ResultsReceived,"wnd":i_wnd});
}
function fu_ResultsReceived( i_data, i_args)
{
//console.log( JSON.stringify( i_data));
//console.log( JSON.stringify( i_args));
	i_args.wnd.elContent.removeChild( i_args.wnd.m_elWait);

	var elResults = i_args.wnd.m_elResults;
	elResults.textContent = '';
	for( var i = 0; i < i_args.wnd.m_res_btns_show.length; i++)
		i_args.wnd.m_res_btns_show[i].style.display = 'none';

	if(( i_data.cmdexec == null ) || ( ! i_data.cmdexec.length ) || ( i_data.cmdexec[0].find_results == null ))
	{
		c_Error('Invalid results data received.');
		elResults.textContent = ( JSON.stringify( i_data));
		return;
	}

	var result = i_data.cmdexec[0].find_results;

	if( result.error )
	{
		var el = document.createElement('div');
		elResults.appendChild( el);
		el.textContent = result.error;
		el.style.color = '#F42';
	}

	if( result.info )
	{
		var el = document.createElement('div');
		elResults.appendChild( el);
		el.textContent = result.info;
	}

	if( result.results == null )
		return;

	if( result.results.length == 0 )
	{
		var el = document.createElement('div');
		elResults.appendChild( el);
		el.textContent = 'No results founded.';
		el.style.color = '#F42';
		return;
	}

	var elTable = document.createElement('table');
	elResults.appendChild( elTable);

	var founded = false;
	for( var i = 0; i < result.results.length; i++)
	{
		var res = result.results[i];
		//console.log( JSON.stringify( res));

		var elTr = document.createElement('tr');
		elTable.appendChild(elTr);

		var el = document.createElement('td');
		elTr.appendChild( el);
		el.textContent = res.asset;

		var el = document.createElement('td');
		elTr.appendChild( el);
		el.textContent = res.respath;

		var el = document.createElement('td');
		elTr.appendChild( el);
		el.textContent = res.name;

		var el = document.createElement('td');
		elTr.appendChild( el);
		el.textContent = res.version;

		var el = document.createElement('td');
		elTr.appendChild( el);

		var msg = '';
		if( res.exist )
		{
			msg += ' EXIST';
			elTr.style.color = '#888';
		}

		el.textContent = msg;

		if( res.error )
		{
			el.textContent = res.error;
			elTr.style.color = '#F42';
		}
		else
			founded = true;
	}

	if( founded )
	{
		i_args.wnd.m_result = result;
		for( var i = 0; i < i_args.wnd.m_res_btns_show.length; i++)
			i_args.wnd.m_res_btns_show[i].style.display = 'block';
	}
//console.log(JSON.stringify(result));
}
function fu_PutMultiDo( i_wnd)
{
	var params = gui_GetParams( i_wnd.elContent, fu_putmulti_params);
	if( RULES.put.ftp )
		gui_GetParams( i_wnd.elContent, fu_putftp_params, params);

	var result = i_wnd.m_result;

	var job = {};
	job.name = 'PUT ' + g_CurPath();
	job.max_running_tasks = params.af_maxtasks;
	job.max_running_tasks_per_host = params.af_perhost;
	job.hosts_mask = params.af_hostsmask;
	job.offline = params.af_paused;

	var block = {};
	job.blocks = [block];
	block.name = 'put';
	block.service = RULES.put.af_service;
	block.capacity = params.af_capacity;
	block.parser = 'generic';
	block.tasks = [];

	var put = c_PathPM_Client2Server( RULES.put.cmd);
	if( RULES.put.ftp	)
	{
		put += ' --ftp ' + params.host;
		if( params.user.length ) put += ' --ftpuser ' + params.user;
		if( params.pass.length ) put += ' --ftppass ' + params.pass;
	}
	put += ' -d "' + result.dest + '"';

	for( var i = 0; i < result.results.length; i++)
	{
		var res = result.results[i];

		if( res.error )
			continue;

		if( res.exist && params.skipexisting )
			continue;

		var cmd = put;
		cmd += ' -s "' + res.src + '"';
		cmd += ' -n "' + res.name + '"';

		var task = {};
		task.name = res.name;
		task.command = cmd;
		block.tasks.push( task);
	}

	if( block.tasks.length == 0 )
		c_Error('No results to put.');
	else
	{
		n_SendJob( job);
		for( var i = 0; i < i_wnd.m_res_btns_show.length; i++)
			i_wnd.m_res_btns_show[i].style.display = 'none';
	}
//console.log(JSON.stringify(job));
}
//
// ########################### Archive: ################################# //
//
fu_arch_params = {};
fu_arch_params.dest = {'label':'Destination'};
fu_arch_params.split = {'tooltip':'Split archive size.'};
fu_arch_params.af_capacity = {'label':'Capacity','tooltip':'Afanasy tasks capacity.','width':'25%'};
fu_arch_params.af_maxtasks = {'label':'Max Run Tasks','tooltip':'Maximum running tasks.','width':'25%','lwidth':'150px'};
fu_arch_params.af_perhost = {'label':'Max Per Host','tooltip':'Maximum running tasks per host.','default':-1,'width':'25%'};
fu_arch_params.af_hostsmaks = {'label':'Hosts Mask','tooltip':'Hosts Mask.','width':'25%'};
function fu_Archive( i_args)
{
//console.log( JSON.stringify( i_args));
	var title = i_args.archive ? 'Archive' : 'Extract Archive';
	var wnd = new cgru_Window({"name":'archive',"title":title});
	wnd.m_args = i_args;

	var params = {};

	gui_Create( wnd.elContent, fu_arch_params,[RULES.archive]);
	if( i_args.archive )
		gui_CreateChoises({"wnd":wnd.elContent,"name":'type',"value":RULES.archive.default,"label":'Type:',"keys":RULES.archive.types});

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
	elSend.onclick = function(e){ fu_ArchivateProcessGUI( e.currentTarget.m_wnd);}

	var elResults = document.createElement('div');
	wnd.elContent.appendChild( elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');

	for( var i = 0; i < i_args.paths.length; i++)
	{
		i_args.paths[i] = c_PathPM_Rules2Client( i_args.paths[i]);
		el = document.createElement('div');
		elResults.appendChild( el);
		el.textContent = i_args.paths[i];
	}
}
function fu_ArchivateProcessGUI( i_wnd)
{
	var paths = i_wnd.m_args.paths;
	for( var i = 0; i < paths.length; i++)
		paths[i] = c_PathPM_Client2Server( paths[i]);

	var params = gui_GetParams( i_wnd.elContent, fu_arch_params);
	if( i_wnd.elContent.m_choises )
		for( key in i_wnd.elContent.m_choises )
			params[key] = i_wnd.elContent.m_choises[key].value;

	var job = {};
	job.folders = {};
	job.folders.input = c_PathDir( paths[0]);

	var arch_cmd = null;

	if( i_wnd.m_args.archive )
	{
		job.name = 'Archive ' + params.type;
		arch_cmd = c_PathPM_Client2Server('/cgru/utilities/arch.py', true);
		arch_cmd += ' -t ' + params.type;
		if( params.split != '' )
			arch_cmd += ' -s ' + params.split;
	}
	else
	{
		job.name = 'Extract';
		arch_cmd = c_PathPM_Client2Server('/cgru/utilities/arch_x.py', true);
	}

	// Output path for thumbnails frequency:
	arch_cmd += ' --thumbsec 5';

	job.name += ' ' + c_PathBase( c_PathDir( paths[0])) + ' x' + paths.length;
	job.max_running_tasks = parseInt( params.af_maxtasks);
	job.max_running_tasks_per_host = parseInt( params.af_perhost);
	job.hosts_mask = params.af_hostsmaks;

	var block = {};
	block.name = c_PathDir( paths[0]);
	block.service = RULES.archive.af_service;
	block.parser = RULES.archive.af_parser;
	block.capacity = parseInt( params.af_capacity);
	block.tasks = [];
	block.working_directory = c_PathDir(paths[0]);
	job.blocks = [block];

	for( var i = 0; i < paths.length; i++)
	{
		var cmd = arch_cmd;
		var task = {};

		var input = c_PathBase( paths[i]);
		cmd += ' -i "' + input + '"';

		var output = '';
		if( params.dest.length )
		{
			output = c_PathPM_Client2Server( params.dest) + '/';
			if( i_wnd.m_args.extract )
			{
				cmd += ' -o "' + output + '"';
			}
		}

		if( i_wnd.m_args.archive )
		{
			output += c_PathBase( input);
			cmd += ' -o "' + output + '"';
			task.name = c_PathBase( output);
			if( paths.length == 1 )
				job.name = c_PathBase( output);
		}
		else
		{
			task.name = c_PathBase( input);
			if( paths.length == 1 )
				job.name = c_PathBase( input);
		}

		task.command = cmd;
		block.tasks.push(task);
	}

	n_SendJob( job);

	i_wnd.destroy();
}

//
// ########################### Walk: ################################# //
//
fu_walk_params = {};
fu_walk_params.path = {};
fu_walk_params.verbose   = {"label":'Verbose Level',"default":2,"lwidth":'170px',"width":'33%'};
fu_walk_params.upparents = {"label":'Update Parent Folders','type':"bool",'default':true,"lwidth":'200px',"width":'33%'};
fu_walk_params.mediainfo = {"label":'Get Media Info','type':"bool",'default':false,"lwidth":'180px',"width":'33%'};
function fu_Walk( i_args)
{
	var wnd = new cgru_Window({"name":'walk',"title":'Send Walk Job'});
	wnd.m_args = i_args;
	var params = {};
	params.path = c_PathPM_Rules2Client( i_args.path);

	gui_Create( wnd.elContent, fu_walk_params, [RULES.walk, params]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);
	elBtns.style.clear = 'both';
//	elBtns.classList.add('buttons');

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
	elSend.onclick = function(e){ fu_WalkProcessGUI( e.currentTarget.m_wnd);}

	var elRules = document.createElement('div');
	wnd.elContent.appendChild( elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.walk='+JSON.stringify(RULES.walk).replace(/,/g,', ');
}

function fu_WalkProcessGUI( i_wnd)
{
	var params = gui_GetParams( i_wnd.elContent, fu_walk_params);

	var job = {};
	job.name = 'Walk ' + params.path;

	var block = {};
	block.name = 'walk';
	block.service  = RULES.walk.af_service;
	block.parser   = RULES.walk.af_parser;
	block.capacity = RULES.walk.af_capacity;
	job.blocks = [block];

	var task = {}
	task.name = params.path;
	block.tasks = [task];

	var cmd = c_PathPM_Client2Server( RULES.walk.cmd);
	cmd += ' --thumb 128';
	cmd += ' --report 256';
	if( params.upparents == false ) cmd += ' -n';
	if( params.mediainfo ) cmd += ' -m';
	cmd += ' -V ' + params.verbose;
	cmd += ' "' + c_PathPM_Client2Server( params.path) + '"';
	task.command = cmd;
//console.log( cmd);

	n_SendJob( job);

	i_wnd.destroy();
}


// ############################# TMPFIO: ##################################//

function fu_TmpFio( i_args)
{
console.log(JSON.stringify(i_args.fview.path));

	var args = {};

	args.template = RULES.tmpfio.template;
	args.destination = i_args.fview.path;
	args.name = 'tmpfio_' + Math.random().toString(36).substring(2);

	a_Copy( args);
}

// ############################### BUFFER: ################################ //

fu_bufferItems = [];

function fu_BufferAdd( i_path)
{
	$('buffer_div').style.display = 'block';

	for( var i = 0; i < fu_bufferItems.length; i++)
		if( fu_bufferItems[i].m_path == i_path )
			return;

	var elItem = document.createElement('div');
	$('buffer').appendChild( elItem);
	fu_bufferItems.push( elItem);
	elItem.classList.add('item');
	elItem.m_path = i_path;

	var el = document.createElement('div');
	elItem.appendChild( el);
	el.classList.add('button');
	el.m_elItem = elItem;
	el.ondblclick = function(e){fu_BufferRemove( e.currentTarget.m_elItem)};
	el.title = 'Double click to delete an item.';

	var el = document.createElement('div');
	elItem.appendChild( el);
	el.classList.add('name');
	el.textContent = i_path;
	el.m_elItem = elItem;
	el.onclick = function(e){fu_BufferClick( e.currentTarget.m_elItem)};

	fv_BufferAdded();
}

function fu_BufferClick( i_elItem) { fu_BufferSelect( i_elItem, null); }
function fu_BufferSelect( i_elItem, i_sel)
{
	if( i_sel === null )
	{
		if( i_elItem.selected ) i_sel = false;
		else i_sel = true;
	}

	if( i_sel )
	{
		i_elItem.selected = true;
		i_elItem.classList.add('selected');
	}
	else
	{
		i_elItem.selected = false;
		i_elItem.classList.remove('selected');
	}
}
function fu_BufferSelectAll( i_sel)
{
	if( i_sel !== false ) i_sel = true;
	for( var i = 0; i < fu_bufferItems.length; i++)
		fu_BufferSelect( fu_bufferItems[i], i_sel);
}
function fu_BufferSelectNone() { fu_BufferSelectAll( false);}
function fu_BufferRemove( i_elItem)
{
	for( var i = 0; i < fu_bufferItems.length; i++)
		if( fu_bufferItems[i] == i_elItem )
		{
			fu_bufferItems.splice( i, 1);
			break;
		}

	$('buffer').removeChild( i_elItem);

	if( fu_bufferItems.length == 0 )
	{
		$('buffer_div').style.display = 'none';
		fv_BufferEmpty();
	}
}
function fu_BufferClear() { while( fu_bufferItems.length ) fu_BufferRemove( fu_bufferItems[0]); }
function fu_BufferExists() { return fu_bufferItems.length != 0; }

function fu_BufferTakeSelected()
{
	var items = [];
	for( var i = 0; i < fu_bufferItems.length; i++)
		if( fu_bufferItems[i].selected )
			items.push( fu_bufferItems[i]);

	var paths = [];
	for( var i = 0; i < items.length; i++)
	{
		paths.push( items[i].m_path);
		fu_BufferRemove( items[i]);
	}

	return paths;
}

