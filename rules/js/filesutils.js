/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	filesutils.js - TODO: description
*/

'use strict';

/* ---------------- [ Put structs and functions ] -------------------------------------------------------- */
var fu_put_params = {
	// src : {"label":'Source', "disabled":true},
	dest: {'label': 'Destination'}
	// name : {}
};
var fu_putftp_params = {
	host: {'label': 'FTP Server'},
	user: {'label': 'FTP User', 'width': '50%'},
	pass: {'lwidth': '150px', 'label': 'FTP Password', 'width': '50%'}
};

function fu_Put(i_args)
{
	var wnd = new cgru_Window({'name': 'put', 'title': 'Put Folder'});
	wnd.m_args = i_args;
	i_args.names = [];
	var params = {};

	params.dest = RULES.put.dest;
	if (params.dest.indexOf('/') != 0)
	{
		if (ASSETS.project)
			params.dest = ASSETS.project.path + '/' + params.dest;
		else
			params.dest = '/' + params.dest;

		params.dest = c_PathPM_Rules2Client(params.dest);
	}
	else
		params.dest = c_PathPM_Server2Client(params.dest);

	gui_Create(wnd.elContent, fu_put_params, [RULES.put, params]);
	if (RULES.put.ftp)
		gui_Create(wnd.elContent, fu_putftp_params, [RULES.put.ftp, params]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.style.clear = 'both';

	var elAfDiv = document.createElement('div');
	elBtns.appendChild(elAfDiv);
	elAfDiv.classList.add('param');

	var elLabel = document.createElement('div');
	elAfDiv.appendChild(elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://' + cgru_Config.af_servername + ':' + cgru_Config.af_serverport +
		'" target="_blank">AFANASY</a>';

	var elSend = document.createElement('div');
	elAfDiv.appendChild(elSend);
	elSend.textContent = 'Send Job';
	elSend.classList.add('button');
	elSend.onclick = function(e) {
		fu_PutDo(e.currentTarget.m_wnd);
	};
	elSend.m_wnd = wnd;

	var elResults = document.createElement('div');
	wnd.elContent.appendChild(elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');
	for (var i = 0; i < i_args.paths.length; i++)
	{
		i_args.paths[i] = c_PathPM_Rules2Client(i_args.paths[i]);

		var name = c_PathBase(i_args.paths[i]);
		if (ASSETS.shot)
		{
			name = ASSET.name;
			var version = i_args.paths[i].split('/');
			version = version[version.length - 1];
			var match = version.match(/v\d{2,}.*/gi);
			if (match)
				name += '_' + match[match.length - 1];
		}
		i_args.names.push(name);

		var el = document.createElement('div');
		elResults.appendChild(el);
		el.textContent = i_args.paths[i] + ' -> ' + name;
	}

	var elRules = document.createElement('div');
	wnd.elContent.appendChild(elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.put=' + JSON.stringify(RULES.put).replace(/,/g, ', ');
}

function fu_PutDo(i_wnd)
{
	var params = gui_GetParams(i_wnd.elContent, fu_put_params);
	if (RULES.put.ftp)
		gui_GetParams(i_wnd.elContent, fu_putftp_params, params);

	for (var i = 0; i < i_wnd.m_args.paths.length; i++)
	{
		var source = c_PathPM_Client2Server(i_wnd.m_args.paths[i]);
		params.dest = c_PathPM_Client2Server(params.dest);
		var name = i_wnd.m_args.names[i];

		var job = {};
		job.folders = {};
		job.folders.source = source;

		var block = {};
		block.name = 'put';
		block.service = RULES.put.af_service;
		block.parser = RULES.put.af_parser;
		if (RULES.put.af_capacity)
			block.capacity = RULES.put.af_capacity;
		job.blocks = [block];

		var task = {};
		task.name = name;
		block.tasks = [task];

		var cmd = c_PathPM_Client2Server(RULES.put.cmd);
		cmd += ' -s "' + source + '"';
		if (RULES.put.ftp)
		{
			job.name = 'FTP ' + name;
			cmd += ' --ftp ' + params.host;
			if (params.user.length)
				cmd += ' --ftpuser ' + params.user;
			if (params.pass.length)
				cmd += ' --ftppass ' + params.pass;
			cmd += ' -d "' + params.dest + '"';
		}
		else
		{
			job.name = 'PUT ' + name;
			cmd += ' -d "' + params.dest + '"';
			cmd += ' -n "' + name + '"';
			if (RULES.put.post_delete)
				job.command_post = 'rm -rf "' + source + '"';
		}

		task.command = cmd;


		// job.offline = true;
		n_SendJob(job);
	}
	// console.log( task.command);
	// console.log( JSON.stringify( params));
	// console.log( JSON.stringify( job));
	i_wnd.destroy();
}


/* ---------------- [ Checksum structs and functions ] --------------------------------------------------- */
var fu_sum_params = {
	path /********/: {'label': 'Path', 'disabled': true},
	type /********/: {'label': 'Type', 'disabled': true},
	update_all /**/: {'label': 'Update All', 'type': 'bool', 'default': false}
};

function fu_Checksum(i_args)
{
	// console.log( JSON.stringify( i_args));
	if ((i_args.walk.files == null) || (i_args.walk.files.length == 0))
	{
		c_Error('Location has no files.');
		return;
	}

	var wnd = new cgru_Window({'name': 'put', 'title': 'Put Folder'});
	wnd.i_walk = i_args.walk;

	var params = i_args;

	gui_Create(wnd.elContent, fu_sum_params, [params, RULES.checksum[i_args.type]]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.style.clear = 'both';

	var elAfDiv = document.createElement('div');
	elBtns.appendChild(elAfDiv);
	elAfDiv.classList.add('param');

	var elLabel = document.createElement('div');
	elAfDiv.appendChild(elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://' + cgru_Config.af_servername + ':' + cgru_Config.af_serverport +
		'" target="_blank">AFANASY</a>';

	var elSend = document.createElement('div');
	elAfDiv.appendChild(elSend);
	elSend.textContent = 'Send Job';
	elSend.classList.add('button');
	elSend.onclick = function(e) {
		fu_ChecksumDo(e.currentTarget.m_wnd);
	};
	elSend.m_wnd = wnd;

	var elRules = document.createElement('div');
	wnd.elContent.appendChild(elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.checksum.' + i_args.type + '=' +
		JSON.stringify(RULES.checksum[i_args.type]).replace(/,/g, ', ');
}

function fu_ChecksumDo(i_wnd)
{
	var params = gui_GetParams(i_wnd.elContent, fu_sum_params);
	var walk = i_wnd.i_walk;
	i_wnd.destroy();

	// console.log( JSON.stringify( params));
	// return;

	var job = {};
	job.name = params.type + ':' + params.path;

	var block = {};
	block.name = params.type;
	block.service = RULES.checksum[params.type].af_service;
	block.parser = RULES.checksum[params.type].af_parser;
	if (RULES.checksum[params.type].af_capacity)
		block.capacity = RULES.checksum[params.type].af_capacity;
	block.tasks = [];
	job.blocks = [block];

	for (var i = 0; i < walk.files.length; i++)
	{
		var file = walk.files[i];
		var task = {};

		var filepath = params.path + '/' + file.name;
		filepath = '/' + RULES.root + filepath;
		filepath = cgru_PM(filepath, true);

		var cmd = cgru_PM(RULES.checksum[params.type].cmd, true);

		cmd += ' -i "' + filepath + '"';
		cmd += ' -t "' + params.type + '"';
		task.command = cmd;
		// console.log(cmd);
		task.name = file.name;

		block.tasks.push(task);
	}

	// job.offline = true;
	n_SendJob(job);
}

/* ---------------- [ Multi Put structs and functions ] -------------------------------------------------- */
var fu_putmulti_params = {
	input /*********/: {'label': 'Result Paths', 'width': '50%'},
	activity: {'label': 'Activity', 'width': '25%'},
	filesext: {'label': 'Files Extensions', 'default': 'mp4,mov', 'width': '25%', 'lwidth': '150px'},
	skipexisting /**/: {'label': 'Skip Existing', 'type': 'bool', 'default': true, 'width': '33%'},
	skiperrors /****/: {'label': 'Skip Errors', 'type': 'bool', 'default': true, 'width': '33%'},
	skipcheck /*****/: {'label': 'Skip Check', 'type': 'bool', 'default': false, 'width': '33%'},
	dest /**********/: {'label': 'Destination'},
	af_capacity /***/: {'label': 'Capacity', 'width': '20%', 'type': 'int'},
	af_maxtasks /***/: {'label': 'Max Tasks', 'width': '15%', 'lwidth': '80px', 'type': 'int'},
	af_perhost /****/: {'label': 'Per Host', 'width': '15%', 'lwidth': '80px', 'type': 'int'},
	af_hostsmask /**/: {'label': 'Hosts Mask', 'width': '35%', 'lwidth': '100px'},
	af_paused /*****/: {'label': 'Paused', 'width': '15%', 'lwidth': '50px', 'type': 'bool'}
};
var fu_findres_params = {
	activity: {},
	filesext: {},
	input: {},
	dest: {},
	skiperrors: {'type': 'bool', 'default': true},
	skipcheck: {'type': 'bool', 'default': false}
};

function fu_PutMultiDialog(i_args)
{
	// console.log( JSON.stringify( i_args));
	var wnd = new cgru_Window({'name': 'put', 'title': 'Put Results'});
	wnd.m_args = i_args;

	var params = {};

	params.input = RULES.assets.shot.result.path.join(',');
	if (RULES.put.input)
		params.input = RULES.put.input;

	if (RULES.put.dest.indexOf('/') !== 0)
		if (ASSETS.project)
			params.dest = c_PathPM_Rules2Client(ASSETS.project.path + '/' + RULES.put.dest);

	gui_Create(wnd.elContent, fu_putmulti_params, [RULES.put, params]);
	if (RULES.put.ftp)
		gui_Create(wnd.elContent, fu_putftp_params, [RULES.put.ftp, params]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.style.clear = 'both';
	elBtns.classList.add('buttons');

	var elAfDiv = document.createElement('div');
	elBtns.appendChild(elAfDiv);
	elAfDiv.classList.add('param');

	var elLabel = document.createElement('div');
	elAfDiv.appendChild(elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://' + cgru_Config.af_servername + ':' + cgru_Config.af_serverport +
		'" target="_blank">AFANASY</a>';

	var elSendJob = document.createElement('div');
	elAfDiv.appendChild(elSendJob);
	elSendJob.textContent = 'Send Job';
	elSendJob.classList.add('button');
	elSendJob.style.display = 'none';
	elSendJob.m_wnd = wnd;
	elSendJob.onclick = function(e) {
		fu_PutMultiDo(e.currentTarget.m_wnd);
	};
	wnd.m_res_btns_show = [elSendJob];

	var elFind = document.createElement('div');
	elAfDiv.appendChild(elFind);
	elFind.textContent = 'Find Results';
	elFind.classList.add('button');
	elFind.style.cssFloat = 'right';
	elFind.m_wnd = wnd;
	elFind.onclick = function(e) {
		fu_ResultsFind(e.currentTarget.m_wnd);
	};

	var elResults = document.createElement('div');
	wnd.elContent.appendChild(elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');

	for (var i = 0; i < i_args.paths.length; i++)
	{
		i_args.paths[i] = c_PathPM_Rules2Client(i_args.paths[i]);
		var el = document.createElement('div');
		elResults.appendChild(el);
		el.textContent = i_args.paths[i];
	}
}

function fu_ResultsFind(i_wnd)
{
	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild(elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var paths = i_wnd.m_args.paths;
	var params = gui_GetParams(i_wnd.elContent, fu_findres_params);

	var cmd = 'rules/bin/find_results.py';
	cmd += ' -r "' + params.input + '"';
	if (params.activity.length)
		cmd += ' --activity "' + params.activity + '"';
	if (params.filesext.length)
		cmd += ' --filesext "' + params.filesext + '"';
	cmd += ' -d "' + c_PathPM_Client2Server(params.dest) + '"';
	if (params.skipcheck)
		cmd += ' --skipcheck';
	if (params.skiperrors)
		cmd += ' --skiperrors';

	for (var i = 0; i < paths.length; i++)
		cmd += ' "' + c_PathPM_Client2Server(paths[i]) + '"';

	n_Request({
		'send': {'cmdexec': {'cmds': [cmd], 'ignore_errors': true}},
		'func': fu_ResultsReceived,
		'wnd': i_wnd
	});
}

function fu_ResultsReceived(i_data, i_args)
{
	// console.log( JSON.stringify( i_data));
	// console.log( JSON.stringify( i_args));
	i_args.wnd.elContent.removeChild(i_args.wnd.m_elWait);

	var elResults = i_args.wnd.m_elResults;
	elResults.textContent = '';
	for (var i = 0; i < i_args.wnd.m_res_btns_show.length; i++)
		i_args.wnd.m_res_btns_show[i].style.display = 'none';

	if ((i_data.cmdexec == null) || (!i_data.cmdexec.length) || (i_data.cmdexec[0].find_results == null))
	{
		c_Error('Invalid results data received.');
		elResults.textContent = (JSON.stringify(i_data));
		return;
	}

	var result = i_data.cmdexec[0].find_results;

	if (result.error)
	{
		var el = document.createElement('div');
		elResults.appendChild(el);
		el.textContent = result.error;
		el.style.color = '#F42';
	}

	if (result.info)
	{
		var el = document.createElement('div');
		elResults.appendChild(el);
		el.textContent = result.info;
	}

	if (result.results == null)
		return;

	if (result.results.length == 0)
	{
		var el = document.createElement('div');
		elResults.appendChild(el);
		el.textContent = 'No results found.';
		el.style.color = '#F42';
		return;
	}

	var elTable = document.createElement('table');
	elResults.appendChild(elTable);

	var found = false;
	for (var i = 0; i < result.results.length; i++)
	{
		var res = result.results[i];
		// console.log( JSON.stringify( res));

		var elTr = document.createElement('tr');
		elTable.appendChild(elTr);

		var el = document.createElement('td');
		elTr.appendChild(el);
		el.textContent = res.asset;

		var el = document.createElement('td');
		elTr.appendChild(el);
		el.textContent = res.respath;

		var el = document.createElement('td');
		elTr.appendChild(el);
		if (res.file)
		{
			el.textContent = res.file;
			elTr.style.color = '#4DD';
		}
		else
			el.textContent = res.name;

		var el = document.createElement('td');
		elTr.appendChild(el);
		el.textContent = res.version;

		var el = document.createElement('td');
		elTr.appendChild(el);

		var msg = '';
		if (res.exist)
		{
			msg += ' EXIST';
			elTr.style.color = '#888';
		}

		el.textContent = msg;

		if (res.error)
		{
			el.textContent = res.error;
			elTr.style.color = '#F42';
		}
		else
			found = true;
	}

	if (found)
	{
		i_args.wnd.m_result = result;
		for (var i = 0; i < i_args.wnd.m_res_btns_show.length; i++)
			i_args.wnd.m_res_btns_show[i].style.display = 'block';
	}
	// console.log(JSON.stringify(result));
}

function fu_PutMultiDo(i_wnd)
{
	var params = gui_GetParams(i_wnd.elContent, fu_putmulti_params);
	if (RULES.put.ftp)
		gui_GetParams(i_wnd.elContent, fu_putftp_params, params);

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

	var put = c_PathPM_Client2Server(RULES.put.cmd);
	if (RULES.put.ftp)
	{
		put += ' --ftp ' + params.host;
		if (params.user.length)
			put += ' --ftpuser ' + params.user;
		if (params.pass.length)
			put += ' --ftppass ' + params.pass;
	}
	put += ' -d "' + result.dest + '"';

	for (var i = 0; i < result.results.length; i++)
	{
		var res = result.results[i];

		if (res.error)
			continue;

		if (res.exist && params.skipexisting)
			continue;

		let name = res.name;
		if (res.file)
			name = res.file;

		var cmd = put;
		cmd += ' -s "' + res.src + '"';
		cmd += ' -n "' + name + '"';

		var task = {};
		task.name = name;
		task.command = cmd;
		block.tasks.push(task);
	}

	if (block.tasks.length == 0)
		c_Error('No results to put.');
	else
	{
		n_SendJob(job);
		for (var i = 0; i < i_wnd.m_res_btns_show.length; i++)
			i_wnd.m_res_btns_show[i].style.display = 'none';
	}
	// console.log(JSON.stringify(job));
}

/* ---------------- [ Archive structs and functions ] ---------------------------------------------------- */
var fu_arch_params = {
	dest: {'label': 'Destination'},
	split: {'tooltip': 'Split archive size (MB).'},
	af_capacity: {'label': 'Capacity', 'tooltip': 'Afanasy tasks capacity.', 'width': '25%'},
	af_maxtasks:
		{'label': 'Max Run Tasks', 'tooltip': 'Maximum running tasks.', 'width': '25%', 'lwidth': '150px'},
	af_perhost: {
		'label': 'Max Per Host',
		'tooltip': 'Maximum running tasks per host.',
		'default': -1,
		'width': '25%'
	},
	af_hostsmaks: {'label': 'Hosts Mask', 'tooltip': 'Hosts Mask.', 'width': '25%'}
};

function fu_Archive(i_args)
{
	// console.log( JSON.stringify( i_args));
	var title = i_args.archive ? 'Archive Folder(s)' : 'Extract Archive(s)';
	var wnd = new cgru_Window({'name': 'archive', 'title': title});
	wnd.m_args = i_args;

	var params = {};

	gui_Create(wnd.elContent, fu_arch_params, [RULES.archive]);
	if (i_args.archive)
		gui_CreateChoices({
			'wnd': wnd.elContent,
			'name': 'type',
			'value': RULES.archive.default,
			'label': 'Type:',
			'keys': RULES.archive.types
		});

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.style.clear = 'both';
	elBtns.classList.add('buttons');

	var elAfDiv = document.createElement('div');
	elBtns.appendChild(elAfDiv);
	elAfDiv.classList.add('param');

	var elLabel = document.createElement('div');
	elAfDiv.appendChild(elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://' + cgru_Config.af_servername + ':' + cgru_Config.af_serverport +
		'" target="_blank">AFANASY</a>';

	var elSend = document.createElement('div');
	elAfDiv.appendChild(elSend);
	elSend.textContent = 'Send Job';
	elSend.classList.add('button');
	elSend.m_wnd = wnd;
	elSend.onclick = function(e) {
		fu_ArchivateProcessGUI(e.currentTarget.m_wnd);
	};

	var elResults = document.createElement('div');
	wnd.elContent.appendChild(elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');

	for (var i = 0; i < i_args.paths.length; i++)
	{
		i_args.paths[i] = c_PathPM_Rules2Client(i_args.paths[i]);
		var el = document.createElement('div');
		elResults.appendChild(el);
		el.textContent = i_args.paths[i];
	}
}

function fu_ArchivateProcessGUI(i_wnd)
{
	var paths = i_wnd.m_args.paths;
	for (var i = 0; i < paths.length; i++)
		paths[i] = c_PathPM_Client2Server(paths[i]);

	var params = gui_GetParams(i_wnd.elContent, fu_arch_params);
	if (i_wnd.elContent.m_choises)
		for (var key in i_wnd.elContent.m_choises)
			params[key] = i_wnd.elContent.m_choises[key].value;

	var job = {};
	job.folders = {};
	job.folders.input = c_PathDir(paths[0]);

	var arch_cmd = null;

	if (i_wnd.m_args.archive)
	{
		job.name = 'Archive ' + params.type;
		arch_cmd = c_PathPM_Client2Server(RULES.archive.cmd, true);
		arch_cmd += ' -t ' + params.type;
		if (params.split != '')
			arch_cmd += ' -s ' + params.split;
	}
	else
	{
		job.name = 'Extract';
		arch_cmd = c_PathPM_Client2Server(RULES.archive.cmd_x, true);
	}

	// Output path for thumbnails frequency:
	arch_cmd += ' --thumbsec 5';

	job.name += ' ' + c_PathBase(c_PathDir(paths[0])) + ' x' + paths.length;
	job.max_running_tasks = parseInt(params.af_maxtasks);
	job.max_running_tasks_per_host = parseInt(params.af_perhost);
	job.hosts_mask = params.af_hostsmaks;

	var block = {};
	block.name = c_PathDir(paths[0]);
	block.service = RULES.archive.af_service;
	block.parser = RULES.archive.af_parser;
	block.capacity = parseInt(params.af_capacity);
	block.tasks = [];
	block.working_directory = c_PathDir(paths[0]);
	job.blocks = [block];

	for (var i = 0; i < paths.length; i++)
	{
		var cmd = arch_cmd;
		var task = {};

		var input = c_PathBase(paths[i]);
		cmd += ' -i "' + input + '"';

		var output = '';
		if (params.dest.length)
		{
			output = c_PathPM_Client2Server(params.dest) + '/';
			if (i_wnd.m_args.extract)
			{
				cmd += ' -o "' + output + '"';
			}
		}

		if (i_wnd.m_args.archive)
		{
			output += c_PathBase(input);
			cmd += ' -o "' + output + '"';
			task.name = c_PathBase(output);
			if (paths.length == 1)
				job.name = c_PathBase(output);
		}
		else
		{
			task.name = c_PathBase(input);
			if (paths.length == 1)
				job.name = c_PathBase(input);
		}

		task.command = cmd;
		block.tasks.push(task);
	}

	n_SendJob(job);

	i_wnd.destroy();
}

/* ---------------- [ Extract sound structs and functions ]
 * ---------------------------------------------------- */
var fu_extract_sound_params = {};
function fu_ExtractSound(i_args)
{
	let path = c_PathPM_Rules2Server(i_args.paths[0]);
	i_args.probe_path = path;
	let cmd = 'rules/bin/ffprobe';
	cmd += ' -v quiet  -print_format json -show_format -show_streams';
	cmd += ' "' + path + '"'
	n_Request({
		'send': {'cmdexec': {'cmds': [cmd]}},
		'func': fu_ExtractSoundShowGUI,
		'info': 'ffprobe',
		'args': i_args,
		'local': true
	});
}
function fu_ExtractSoundShowGUI(i_data, i_args)
{
	i_args = i_args.args;
	let wnd = new cgru_Window({'name': 'extractsound', 'title': 'Extract Sound'});
	wnd.m_args = i_args;

	gui_Create(wnd.elContent, fu_extract_sound_params, [RULES.dailies]);
	let types = {
		'wav': {'name': 'WAV', 'tooltip': 'Waveform Audio File Format'},
		'flac': {'name': 'FLAC', 'tooltip': 'Free Lossless Audio Codec'}
	};
	gui_CreateChoices(
		{'wnd': wnd.elContent, 'name': 'type', 'value': 'wav', 'label': 'Type:', 'keys': types});

	let elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.style.clear = 'both';
	elBtns.classList.add('buttons');

	let elResults = document.createElement('div');
	wnd.elContent.appendChild(elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output', 'error');


	if ((i_data.cmdexec == null) && (i_data.cmdexec.length == null) && (i_data.cmdexec.length == 0))
	{
		elResults.textContent = 'ERROR:<br>' + JSON.stringify(i_data);
		return;
	}
	let data = i_data.cmdexec[0];
	if (data.streams == null)
	{
		elResults.textContent = 'No streams found in ' + i_args.probe_path;
		return;
	}

	let output = '<p>' + i_args.probe_path + '</p>';
	let audio_found = false;
	let count = 0;
	let fps = null;
	for (let stream of data.streams)
	{
		// console.log(JSON.stringify(stream));
		output += '<p>';

		if (stream.codec_type && (stream.codec_type == 'video'))
		{
			fps = stream.r_frame_rate;
			fps = fps.split('/');
			fps = parseInt(fps[0]) / parseInt(fps[1]);

			output += 'Video #' + count + ':';
			output += ' ' + stream.codec_name;
			output += ' ' + (parseFloat(stream.duration)).toFixed(3) + ' sec';
			output += ' ' + fps + ' FPS';
			output += ' ' + (parseFloat(stream.bit_rate) / 1000).toFixed() + ' kb/s';
		}

		if (stream.codec_type && (stream.codec_type == 'audio'))
		{
			audio_found = true;
			output += 'Audio #' + count + ':';
			output += ' ' + stream.codec_name;
			output += ' ' + (parseFloat(stream.duration)).toFixed(3) + ' sec';
			output += '*' + stream.channels;
			output += ' ' + (parseFloat(stream.sample_rate) / 1000).toFixed(1) + ' kHZ';
			output += ' ' + (parseFloat(stream.bit_rate) / 1000).toFixed() + ' kb/s';
		}

		output += '</p>';
		count += 1;
	}


	if (false == audio_found)
	{
		output += '<p>No audio streams found.</p>';
		elResults.innerHTML = output;
		return;
	}

	if (fps != RULES.fps)
	{
		output += '<p>Project and video FPS mismatch!</p>';
	}
	else
		elResults.classList.remove('error');

	elResults.innerHTML = output;

	let elBntExtractSound = document.createElement('div');
	elBtns.appendChild(elBntExtractSound);
	elBntExtractSound.textContent = 'Extract Sound';
	elBntExtractSound.classList.add('button');
	elBntExtractSound.m_wnd = wnd;
	elBntExtractSound.onclick = function(e) {
		fu_ExtractSoundProcessGUI(e.currentTarget.m_wnd);
	};
}
function fu_ExtractSoundProcessGUI(i_wnd)
{
	let paths = i_wnd.m_args.paths;
	for (let i = 0; i < paths.length; i++)
		paths[i] = c_PathPM_Rules2Server(paths[i]);

	let params = gui_GetParams(i_wnd.elContent, fu_arch_params);
	if (i_wnd.elContent.m_choises)
		for (let key in i_wnd.elContent.m_choises)
			params[key] = i_wnd.elContent.m_choises[key].value;

	let cmds = [];
	for (let path of paths)
	{
		let cmd = 'rules/bin/ffmpeg';
		cmd += ' -i "' + path + '"';
		cmd += ' -vn';
		cmd += ' "' + path + '.' + params.type + '"';
		cmds.push(cmd);
	}

	n_Request({
		'send': {'cmdexec': {'cmds': cmds}},
		'func': fu_ExtractSoundFinished,
		'info': 'ffmpeg',
		'wnd': i_wnd,
		'local': true
	});
}
function fu_ExtractSoundFinished(i_data, i_args)
{
	let wnd = i_args.wnd;
	let elResults = wnd.m_elResults;

	elResults.classList.add('error');

	if ((i_data.cmdexec == null) || (i_data.cmdexec.length == null) || (i_data.cmdexec.length == 0))
	{
		elResults.textContent = JSON.stringify(i_data);
		return;
	}
	for (let cmdexec of i_data.cmdexec)
	{
		if (cmdexec.error)
		{
			elResults.textContent = cmdexec.error;
			return;
		}
	}

	elResults.classList.remove('error');
	wnd.m_args.filesview.refresh();
	wnd.destroy();
}
/* ---------------- [ Walk structs and functions ] ------------------------------------------------------- */
var fu_walk_params = {
	path: {},
	verbose: {'label': 'Verbose Level', 'width': '25%', 'lwidth': '170px', 'default': 2},
	upparents: {
		'label': 'Update Parent Folders',
		'width': '25%',
		'lwidth': '170px',
		'type': 'bool',
		'default': true
	},
	genthumbs:
		{'label': 'Generate Thumbnails', 'width': '25%', 'lwidth': '170px', 'type': 'bool', 'default': false},
	mediainfo:
		{'label': 'Get Media Info', 'width': '25%', 'lwidth': '170px', 'type': 'bool', 'default': false},
	af_hostsmask: {'label': 'Hosts Mask', 'width': '50%', 'lwidth': '160px'},
	af_paused: {'label': 'Paused', 'width': '50%', 'lwidth': '50px', 'type': 'bool'}
};

function fu_Walk(i_args)
{
	var wnd = new cgru_Window({'name': 'walk', 'title': 'Send Walk Job'});
	wnd.m_args = i_args;
	var params = {};
	params.path = c_PathPM_Rules2Client(i_args.path);

	gui_Create(wnd.elContent, fu_walk_params, [RULES.walk, params]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.style.clear = 'both';
	//	elBtns.classList.add('buttons');

	var elAfDiv = document.createElement('div');
	elBtns.appendChild(elAfDiv);
	elAfDiv.classList.add('param');

	var elLabel = document.createElement('div');
	elAfDiv.appendChild(elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://' + cgru_Config.af_servername + ':' + cgru_Config.af_serverport +
		'" target="_blank">AFANASY</a>';

	var elSend = document.createElement('div');
	elAfDiv.appendChild(elSend);
	elSend.textContent = 'Send Job';
	elSend.classList.add('button');
	elSend.m_wnd = wnd;
	elSend.onclick = function(e) {
		fu_WalkProcessGUI(e.currentTarget.m_wnd);
	};

	var elRules = document.createElement('div');
	wnd.elContent.appendChild(elRules);
	elRules.classList.add('rules');
	elRules.textContent = 'RULES.walk=' + JSON.stringify(RULES.walk).replace(/,/g, ', ');
}

function fu_WalkProcessGUI(i_wnd)
{
	var params = gui_GetParams(i_wnd.elContent, fu_walk_params);

	var job = {};
	job.name = 'Walk ' + params.path;
	job.hosts_mask = params.af_hostsmask;
	job.offline = params.af_paused;

	var block = {};
	block.name = 'walk';
	block.service = RULES.walk.af_service;
	block.parser = RULES.walk.af_parser;
	block.capacity = RULES.walk.af_capacity;
	job.blocks = [block];

	var task = {};
	task.name = params.path;
	block.tasks = [task];

	var cmd = c_PathPM_Client2Server(RULES.walk.cmd);
	cmd += ' --progress';
	cmd += ' --thumb 128';
	cmd += ' --report 256';
	if (params.upparents)
		cmd += ' --upparents -1';
	if (params.mediainfo)
		cmd += ' --mediainfo';
	if (params.genthumbs)
		cmd += ' --genthumbs';
	cmd += ' -V ' + params.verbose;
	cmd += ' "' + c_PathPM_Client2Server(params.path) + '"';
	task.command = cmd;
	// console.log( cmd);

	n_SendJob(job);

	i_wnd.destroy();
}


/* ---------------- [ TMPFIO function ] ------------------------------------------------------------------ */
function fu_TmpFio(i_args)
{
	console.log(JSON.stringify(i_args.fview.path));

	var args = {};

	args.template = RULES.tmpfio.template;
	args.destination = i_args.fview.path;
	args.name = 'tmpfio_' + Math.random().toString(36).substring(2);

	a_Copy(args);
}

/* ---------------- [ BUFFER structs and functions ] ----------------------------------------------------- */
var fu_bufferItems = [];

function fu_BufferAdd(i_path)
{
	$('buffer_div').style.display = 'block';

	for (var i = 0; i < fu_bufferItems.length; i++)
		if (fu_bufferItems[i].m_path == i_path)
			return;

	var elItem = document.createElement('div');
	$('buffer').appendChild(elItem);
	fu_bufferItems.push(elItem);
	elItem.classList.add('item');
	elItem.m_path = i_path;

	var el = document.createElement('div');
	elItem.appendChild(el);
	el.classList.add('button');
	el.m_elItem = elItem;
	el.ondblclick = function(e) {
		fu_BufferRemove(e.currentTarget.m_elItem)
	};
	el.title = 'Double click to delete an item.';

	var el = document.createElement('div');
	elItem.appendChild(el);
	el.classList.add('name');
	el.textContent = i_path;
	el.m_elItem = elItem;
	el.onclick = function(e) {
		fu_BufferClick(e.currentTarget.m_elItem)
	};

	fv_BufferAdded();
}

function fu_BufferClick(i_elItem)
{
	fu_BufferSelect(i_elItem, null);
}

function fu_BufferSelect(i_elItem, i_sel)
{
	if (i_sel === null)
	{
		if (i_elItem.selected)
			i_sel = false;
		else
			i_sel = true;
	}

	if (i_sel)
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

function fu_BufferSelectAll(i_sel)
{
	if (i_sel !== false)
		i_sel = true;
	for (var i = 0; i < fu_bufferItems.length; i++)
		fu_BufferSelect(fu_bufferItems[i], i_sel);
}

function fu_BufferSelectNone()
{
	fu_BufferSelectAll(false);
}

function fu_BufferRemove(i_elItem)
{
	for (var i = 0; i < fu_bufferItems.length; i++)
		if (fu_bufferItems[i] == i_elItem)
		{
			fu_bufferItems.splice(i, 1);
			break;
		}

	$('buffer').removeChild(i_elItem);

	if (fu_bufferItems.length == 0)
	{
		$('buffer_div').style.display = 'none';
		fv_BufferEmpty();
	}
}

function fu_BufferClear()
{
	while (fu_bufferItems.length)
		fu_BufferRemove(fu_bufferItems[0]);
}

function fu_BufferExists()
{
	return fu_bufferItems.length != 0;
}

function fu_BufferTakeSelected()
{
	var items = [];
	for (var i = 0; i < fu_bufferItems.length; i++)
		if (fu_bufferItems[i].selected)
			items.push(fu_bufferItems[i]);

	var paths = [];
	for (var i = 0; i < items.length; i++)
	{
		paths.push(items[i].m_path);
		fu_BufferRemove(items[i]);
	}

	return paths;
}
