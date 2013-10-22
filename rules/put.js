pu_params = [];
pu_params.push({"name":'src', "label":'Source', "disabled":true});
pu_params.push({"name":'dest',"label":'Destination'});
pu_params.push({"name":'name'});

function pu_Put( i_path)
{
	var wnd = new cgru_Window('put','Put Folder');
	wnd.elContent.classList.add('dialog');

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

	c_CreateGUI( wnd, pu_params, [params, RULES.put]);

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
	elSend.onclick = function(e){ pu_ProcessGUI( e.currentTarget.m_wnd);}
	elSend.m_wnd = wnd;

	var elRules = document.createElement('div');
	wnd.elContent.appendChild( elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.put='+JSON.stringify(RULES.put).replace(/,/g,', ');
}

function pu_ProcessGUI( i_wnd)
{
	var params = {};
	for( var p = 0; p < pu_params.length; p++)
		params[pu_params[p].name] = i_wnd.m_elements[pu_params[p].name].textContent;

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

	var cmd = RULES.put.cmd;
	cmd += ' -s "' + source + '"';
	cmd += ' -d "' + dest + '"';
	cmd += ' -n "' + params.name + '"';
	task.command = cmd;

	n_SendJob( job);

//console.log( task.command);
//console.log( JSON.stringify( params));
//console.log( JSON.stringify( job));
}

