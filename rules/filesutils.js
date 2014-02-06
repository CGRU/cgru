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

// ########################### Multi Put: #################################
fu_putmulti_params = {};
fu_putmulti_params.input = {};
fu_putmulti_params.dest = {"label":'Destination'};
function fu_PutMultiDialog( i_args)
{
//console.log( JSON.stringify( i_args));
	var wnd = new cgru_Window({"name":'put',"title":'Multi Put'});
	wnd.m_args = i_args;

	var params = {};
	if( RULES.put.dest.indexOf('/') !== 0 )
		if( ASSETS.project )
			params.dest = ASSETS.project.path + '/' + RULES.put.dest;

	gui_Create( wnd.elContent, fu_putmulti_params, [RULES.put, params]);

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
	elSend.onclick = function(e){ fu_PutMultiProcessGUI( e.currentTarget.m_wnd, false);}

	var elTest = document.createElement('div');
	elAfDiv.appendChild( elTest);
	elTest.textContent = 'Test Inputs';
	elTest.classList.add('button');
	elTest.m_wnd = wnd;
	elTest.onclick = function(e){ fu_PutMultiProcessGUI( e.currentTarget.m_wnd, true);}

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

function fu_PutMultiProcessGUI( i_wnd, i_test)
{
	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild( elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var shots = i_wnd.m_args.shots;
	var params = gui_GetParams( i_wnd.elContent, fu_putmulti_params);
	for( key in i_wnd.elContent.m_choises )
		params[key] = i_wnd.elContent.m_choises[key].value;

	var cmd = 'rules/bin/putmulti.sh';

	cmd += ' -i "' + params.input + '"';
	cmd += ' -u "' + g_auth_user.id + '"';
	cmd += ' -c ' + RULES.put.af_capacity;
	cmd += ' -m ' + RULES.put.af_maxtasks;
	cmd += ' -d "' + cgru_PM('/' + RULES.root + params.dest, true) + '"';
	if( i_test ) cmd += ' -t';

	for( var i = 0; i < shots.length; i++)
		cmd += ' "' + cgru_PM('/' + RULES.root + shots[i], true) + '"'

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":fu_PutMultiFinished,"wnd":i_wnd});

}
function fu_PutMultiFinished( i_data, i_args)
{
//console.log( JSON.stringify( i_data));
//console.log( JSON.stringify( i_args));
	i_args.wnd.elContent.removeChild( i_args.wnd.m_elWait);
	var elResults = i_args.wnd.m_elResults;
	elResults.textContent = '';

	if(( i_data.cmdexec == null ) || ( ! i_data.cmdexec.length ) || ( i_data.cmdexec[0].put == null ))
	{
		elResults.textContent = ( JSON.stringify( i_data));
		return;
	}

	var put = i_data.cmdexec[0].put;

	for( var i = put.length - 1; i >= 0; i--)
	{
		var el = document.createElement('div');
		elResults.appendChild( el);
		for( var msg in put[i])
		{
			if( msg == 'src' )
			{
				msg = 'put:';
				msg += ' ' + put[i].src;
				msg += ' -> ' + put[i].name;
				//msg += ' ' + put[i].version;
				//msg += ' ' + put[i].dst;
				el.textContent = msg;
				break;
			}
			el.textContent = msg + ': ' + put[i][msg];
			if(( msg == 'error' ) || ( put[i][msg].indexOf('error') != -1 ))
				el.style.color = '#F42';
		}
	}

//	i_wnd.destroy();
}

