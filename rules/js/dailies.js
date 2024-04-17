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
	dailies.js
	Make dailies, convert sequences and movies.
*/

"use strict";

var d_moviemaker = '/cgru/utilities/moviemaker';
var d_makemovie = d_moviemaker + '/makemovie.py';

var d_params_types = {
	general /***/: {"label": 'General' /***/, "tooltip": 'General parameters.'},
	settings /**/: {"label": 'Settings' /**/, "tooltip": 'Other parameters.'}
};

var d_params = {"general": {}, "settings": {}};
d_params.general = {
	project  : {"width": '50%'},
	shot     : {"width": '50%', "lwidth": '70px'},
	artist   : {"width": '50%'},
	activity : {"width": '25%', "lwidth": '70px'},
	version  : {"width": '25%', "lwidth": '70px'},
	input    : {},
	output   : {},
	filename : {"width": '75%'},
	fps      : {"label": 'FPS', "width": '25%', "lwidth": '70px'},
	comments : {}
};
d_params.settings = {
	audio_file     : {"label":'Audio', "default":"REF/sound.flac", "tooltip":'Sound file'},
	af_depend_mask : {"label":'Depends', "tooltip":'Afanasy job depend mask'},
	af_hostsmask   : {'label':'Hosts Mask'},
	cacher_aspect  : {'width':'25%', 'lwidth':'160px', 'label':'Cacher Aspect',  'tooltip':'Cacher aspect (float aspect: 2.39)'},
	cacher_opacity : {'width':'25%', 'lwidth':'160px', 'label':'Cacher Opacity', 'tooltip':'Cacher opacity (integer percentage: 100)'},
	draw169        : {'width':'25%', 'lwidth':'160px', 'label':'Cacher 16x9',    'tooltip':'Cacher 16x9 opacity percentage.'},
	draw235        : {'width':'25%', 'lwidth':'160px', 'label':'Cacher 2.35',    'tooltip':'Cacher 2.35 opacity percentage.'},
	fffirst        : {"label":"F.F.First", "tooltip":'First frame is "1"\nNo matter image file name number.'},
	aspect_in      : {"label":'Aspect In'},
	gamma          : {}
};

function d_Make(i_path, i_outfolder)
{
	c_Log('Make Dailies: ' + i_path);

	var out_path = c_PathDir(i_path);
	if (ASSET && (ASSET.dailies))
	{
		out_path = ASSET.path + '/' + ASSET.dailies.path[0];

		if (ASSET.dailies.folders)
		{
			for (let f in ASSET.dailies.folders)
			{
				if (i_path.toLowerCase().indexOf(f.toLowerCase()) !== -1)
				{
					out_path = ASSET.path + '/' + ASSET.dailies.folders[f];
					break;
				}
			}
		}
	}

	var params = {};

	params.fps = RULES.fps;
	if (RULES.dailies.fps)
		params.fps = RULES.dailies.fps;

	params.project = 'project';
	if (ASSETS.project)
		params.project = ASSETS.project.name;
	params.shot = c_PathBase(i_path);
	if (ASSETS.shot)
		params.shot = ASSETS.shot.name;

	params.version = i_path.split('/');
	params.version = params.version[params.version.length - 1];
	var match = params.version.match(/_v\d{3,3}.*/g);
	if (match)
		params.version = match[match.length - 1].substr(1);

	params.input = i_path;
	params.output = c_PathPM_Rules2Client(out_path);
	params.activity = RULES.dailies.activity;
	if (activity_Current)
		params.activity = activity_Current;

	d_params.general.artist = {"width": '50%'};
	params.artist = c_GetUserTitle();
	if (RULES.status && RULES.status.artists && RULES.status.artists.length)
	{
		if (RULES.status.artists.indexOf(g_auth_user.id) == -1)
			params.artist = c_GetUserTitle(RULES.status.artists[0]);

		var artists = [];
		for (var i = 0; i < RULES.status.artists.length; i++)
			artists.push(c_GetUserTitle(RULES.status.artists[i]));
		if (artists.indexOf(c_GetUserTitle()) == -1)
			artists.push(c_GetUserTitle());
		if (artists.length > 1)
			d_params.general.artist.pulldown = artists;
	}

	var dateObj = new Date();
	var date = '' + dateObj.getFullYear();
	date = date.substr(2);
	date += (dateObj.getMonth() + 1) < 10 ? '0' + (dateObj.getMonth() + 1) : dateObj.getMonth() + 1;
	date += dateObj.getDate() < 10 ? '0' + dateObj.getDate() : dateObj.getDate();

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

	var wnd = new cgru_Window({"name": 'dailies', "title": 'Make Dailies'});

	var cmd = 'rules/bin/walk.sh -m "' + RULES.root + i_path + '"';
	n_Request({
		"send": {"cmdexec": {"cmds": [cmd]}},
		"func": d_DailiesWalkReceived,
		"info": 'walk dailies',
		"wpath": i_path,
		"d_params": params,
		"d_wnd": wnd
	});
}

function d_DailiesWalkReceived(i_data, i_args)
{
	//console.log(JSON.stringify(i_data));
	var wnd = i_args.d_wnd;
	var params = i_args.d_params;

	var data = null;
	if (i_data.cmdexec && i_data.cmdexec[0].walk)
		data = i_data.cmdexec[0].walk;

	params.comments = '';
	if (RULES.status && RULES.status.annotation && RULES.status.annotation.length)
		params.comments = RULES.status.annotation.trim();

	//console.log(JSON.stringify(data));
	// Get files sequence pattern and comments:
	if (data && data.walk && data.walk.exif)
	{
		var exif = data.walk.exif;

		// Get files pattern (from one file):
		var file = exif.file;
		var match = file.match(/\d+\./g);
		if (match)
		{
			match = match[match.length - 1];
			var pos = file.lastIndexOf(match);
			var pattern = file.substr(0, pos);
			for (var d = 0; d < match.length - 1; d++)
				pattern += '#';
			pattern += file.substr(pos - 1 + match.length);
			params.input = c_PathPM_Rules2Client(params.input + '/' + pattern);
		}

		// Get comments (came from image EXIF metadata):
		if (exif.comments)
			params.comments += ' ' + exif.comments.trim();
	}

	params.comments = params.comments.trim();

	// Update filesviews
	if (data)
		fv_UpdateFromWalk(data, i_args.wpath);

	wnd.elTabs =
		gui_CreateTabs({"tabs": d_params_types, "elParent": wnd.elContent, "name": 'd_params_types'});

	for (var type in d_params_types)
		gui_Create(wnd.elTabs[type], d_params[type], [RULES.dailies, params]);
		//gui_Create(wnd.elTabs[type], d_params[type], [params, RULES.dailies]);

	gui_CreateChoices({
		"wnd": wnd.elTabs.general,
		"name": 'colorspace',
		"value": RULES.dailies.colorspace,
		"label": 'Colorspace:',
		"keys": RULES.dailies.colorspaces
	});

	RULES.dailies.formats.asis.disabled = true;
	gui_CreateChoices({
		"wnd": wnd.elTabs.general,
		"name": 'format',
		"value": RULES.dailies.format,
		"label": 'Formats:',
		"keys": RULES.dailies.formats
	});
	RULES.dailies.formats.asis.disabled = false;

	RULES.dailies.codecs.copy.disabled = true;
	gui_CreateChoices({
		"wnd": wnd.elTabs.general,
		"name": 'codec',
		"value": RULES.dailies.codec,
		"label": 'Codec:',
		"keys": RULES.dailies.codecs
	});
	RULES.dailies.codecs.copy.disabled = false;

	gui_CreateChoices({
		"wnd": wnd.elTabs.general,
		"name": 'container',
		"value": RULES.dailies.container,
		"label": 'Container:',
		"keys": RULES.dailies.containers
	});

	gui_CreateChoices({
		"wnd": wnd.elTabs.settings,
		"name": 'slate',
		"value": RULES.dailies.slate,
		"label": 'Slate:',
		"keys": RULES.dailies.slates
	});

	gui_CreateChoices({
		"wnd": wnd.elTabs.settings,
		"name": 'template',
		"value": RULES.dailies.template,
		"label": 'Template:',
		"keys": RULES.dailies.templates
	});

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
	elSend.onclick = function(e) { d_ProcessGUI(e.currentTarget.m_wnd); };
	elSend.m_wnd = wnd;

	wnd.elContent.focus();
	wnd.elContent.m_wnd = wnd;
	wnd.elContent.onkeydown = function(e) {
		// console.log( e.keyCode);
		if (e.keyCode == 13)
			d_ProcessGUI(e.currentTarget.m_wnd);
	}
}

function d_ProcessGUI(i_wnd)
{
	var params = {};
	for (var type in d_params_types)
		gui_GetParams(i_wnd.elTabs[type], d_params[type], params);
	// console.log( JSON.stringify( params)); return;

	for (var key in i_wnd.elTabs.general.m_choises)
		params[key] = i_wnd.elTabs.general.m_choises[key].value;
	for (var key in i_wnd.elTabs.settings.m_choises)
		params[key] = i_wnd.elTabs.settings.m_choises[key].value;

	i_wnd.destroy();

	if (g_auth_user == null)
	{
		c_Error("Guests can't generate dailies.");
		return;
	}

	var job = {};
	// job.offline = true;
	job.name = params.filename;
	if (params.af_depend_mask.length)
		job.depend_mask = params.af_depend_mask;
	if (params.af_hostsmask.length)
		job.hosts_mask = params.af_hostsmask;

	job.folders = {};
	job.folders.input = c_PathDir(c_PathPM_Client2Server(params.input));
	job.folders.output = c_PathPM_Client2Server(params.output);

	var block = {};
	block.name = 'Dailies';
	block.service = 'movgen';
	block.parser = 'generic';
	if (RULES.dailies.af_capacity)
		block.capacity = RULES.dailies.af_capacity;
	block.working_directory = c_PathPM_Rules2Server(g_CurPath());
	job.blocks = [block];

	var task = {};
	task.name = params.filename;
	task.command = d_MakeCmd(params);
	block.tasks = [task];

	//console.log(task.command);
	//console.log(JSON.stringify(job));
	//return;
	n_SendJob(job);

	let news_path = g_CurPath();
	if (ASSET && ASSET.dailies && ASSET.dailies.paths)
		news_path = ASSET.path;
	nw_MakeNews({'title':'dailies','path':news_path});
}

function d_MakeCmd(i_params)
{
	var params = c_CloneObj(RULES.dailies);
	for (var parm in i_params)
		params[parm] = i_params[parm];

	var input = c_PathPM_Client2Server(params.input);
	var output = c_PathPM_Client2Server(params.output) + '/' + params.filename;

	var cmd = 'python3';

	cmd += ' "' + c_PathPM_Client2Server(d_makemovie) + '"';

	cmd += ' -a "' + RULES.avcmd + '"';
	cmd += ' -c "' + params.codec + '"';
	cmd += ' -f ' + params.fps;
	cmd += ' -r ' + params.format;

	if (params.slate && params.slate.length && params.slate != "noslate")
		cmd += ' -s ' + params.slate;

	cmd += ' -t ' + params.template;

	params.comments = params.comments.trim();
	if (params.comments != '')
		cmd += ' --comments "' + params.comments + '"';

	if (RULES.dailies.font)
		cmd += ' --font "' + RULES.dailies.font + '"';

	if (params.container != 'DEFAULT')
		cmd += ' -n ' + params.container;

	cmd += ' --colorspace "' + params.colorspace + '"';

	if (params.gamma != '')
		cmd += ' -g ' + params.gamma;
	if (params.fffirst != '')
		cmd += ' --fffirst';

	if (params.audio_file != '')
		cmd += ' --audio "' + params.audio_file + '"';

	cmd += ' --lgspath "' + params.logo_slate_path + '"';
	cmd += ' --lgssize ' + params.logo_slate_size;
	cmd += ' --lgsgrav ' + params.logo_slate_grav;
	cmd += ' --lgfpath "' + params.logo_frame_path + '"';
	cmd += ' --lgfsize ' + params.logo_frame_size;
	cmd += ' --lgfgrav ' + params.logo_frame_grav;

	cmd += ' --project "' + params.project + '"';
	cmd += ' --shot "' + params.shot + '"';

	cmd += ' --ver "' + params.version + '"';
	cmd += ' --artist "' + params.artist + '"';
	cmd += ' --activity "' + params.activity + '"';

	if (RULES.dailies.imgcmd)
		cmd += ' -i ' + RULES.dailies.imgcmd;

	if (RULES.dailies.preview)
	{
		cmd += ' --pcodec "' + RULES.dailies.preview.codec + '"';
		cmd += ' --pargs "' + RULES.dailies.preview.args + '"';
		cmd += ' --pdir "' + RUFOLDER + '"';
	}

	if ((params.aspect_in != null) && (params.aspect_in != ''))
		cmd += ' --aspect_in ' + params.aspect_in;

	if ((params.cacher_aspect != null) && (params.cacher_aspect != ''))
		cmd += ' --cacher_aspect ' + params.cacher_aspect;
	if ((params.cacher_opacity != null) && (params.cacher_opacity != ''))
		cmd += ' --cacher_opacity ' + params.cacher_opacity;
	if ((params.draw169 != null) && (params.draw169 != ''))
		cmd += ' --draw169 ' + params.draw169;
	if ((params.draw235 != null) && (params.draw235 != ''))
		cmd += ' --draw235 ' + params.draw235;

	cmd += ' --createoutdir';

	cmd += ' "' + input + '"';
	cmd += ' "' + output + '"';

	return cmd;
}


/* ###########################################################################################################
################### [ CONVERT ] ##############################################################################
########################################################################################################### */


var d_cvtguiparams = {
	fps /**********/: {"label": 'FPS', "width": '20%'},
	time_start /***/: {"default": '00:00:00', "width": '20%'},
	duration /*****/: {"default": '00:00:00', "width": '20%'},
	quality /******/: {"label": 'JPEG Quality', 'type': 'int', "default": 100, 'width': '20%',"lwidth":'160px'},
	ipar /*********/: {"label": 'Input pixel aspect', "width": '20%',"lwidth":'160px'},
	padding /******/: {"label": 'Padding', 'width': '20%', "default": 4},
	first_frame /**/: {"label": 'First Frame', 'width': '20%'},
	af_capacity /**/: {'label': 'Capacity', 'width': '15%', 'type': 'int'},
	af_maxtasks /***/: {'label': 'Max Tasks', 'width': '15%', 'type': 'int', 'default': -1},
	af_perhost /****/: {'label': 'Per Host', 'width': '15%', 'type': 'int', 'default': 1},
	af_fpt /********/: {
		'label': 'FPT',
		'width': '15%',
		'type': 'int',
		'default': 10,
		'tooltip': 'Frames Per Task'
	},
	audio_file /****/: {"label": 'Audio', "default": "REF/sound.flac", "tooltip": 'Sound file', 'width': '40%'},
	af_hostsmask /**/: {'label': 'Hosts Mask', 'width': '40%'},
	af_paused /*****/: {'label': 'Start Job Paused', 'width': '20%', 'lwidth': '160px', 'type': 'bool'}
};

var d_cvtmulti_params = {
	input /*********/: {"label": 'Result Paths'},
	skipexisting /**/: {"label": 'Skip Existing', 'type': "bool", 'default': true, "width": '33%'},
	skiperrors /****/: {"label": 'Skip Errors', 'type': "bool", 'default': false, "width": '33%'},
	skipcheck /*****/: {"label": 'Skip Check', 'type': "bool", 'default': false, "width": '34%'},
	dest /**********/: {"label": 'Destination'}
};

function d_Convert(i_args)
{
	var params = {};
	params.fps = RULES.fps;
	if (RULES.dailies.fps)
		params.fps = RULES.dailies.fps;

	var title = 'Convert ';
	if (i_args.images)
		title += ' Images';
	else if (i_args.folders)
		title += ' Sequences';
	else if (i_args.movies)
		title += ' Movies';
	else if (i_args.results)
		title += ' Results';
	var wnd = new cgru_Window({"name": 'dailies', "title": title});
	wnd.m_args = i_args;
	wnd.onDestroy = d_CvtOnDestroy;

	var img_types = {
		jpg /****/: {"name": 'JPG'},
		png /****/: {"name": 'PNG'},
		dpx /****/: {"name": 'DPX'},
		tif8 /***/: {"name": 'TIF8', "tooltip": '8  bits TIF'},
		tif16 /**/: {"name": 'TIF16', "tooltip": '16 bits TIF'}
	};
	if (i_args.movies !== true)
		img_types.exr = {"name": 'EXR'};

	gui_CreateChoices({
		"wnd": wnd.elContent,
		"name": 'format',
		"value": '1280x720',
		"label": 'Formats:',
		"keys": RULES.dailies.formats
	});

	gui_Create(wnd.elContent, d_cvtguiparams, [params, RULES, RULES.dailies]);

	gui_CreateChoices(
		{"wnd": wnd.elContent, "name": 'imgtype', "value": 'jpg', "label": 'Image Type:', "keys": img_types});

	if (i_args.movies !== true)
	{
		RULES.dailies.codecs.copy.disabled = true;
		gui_CreateChoices({
			"wnd": wnd.elContent,
			"name": 'colorspace',
			"value": RULES.dailies.colorspace,
			"label": 'Colorspace:',
			"keys": RULES.dailies.colorspaces
		});
	}
	gui_CreateChoices({
		"wnd": wnd.elContent,
		"name": 'codec',
		"value": RULES.dailies.codec,
		"label": 'Codec:',
		"keys": RULES.dailies.codecs
	});
	RULES.dailies.codecs.copy.disabled = false;

	gui_CreateChoices({
		"wnd": wnd.elContent,
		"name": 'container',
		"value": RULES.dailies.container,
		"label": 'Container:',
		"keys": RULES.dailies.containers
	});

	if (i_args.results)
	{
		if (RULES.put.dest.indexOf('/') !== 0)
			if (ASSETS.project)
				params.dest = c_PathPM_Rules2Client(ASSETS.project.path + '/' + RULES.put.dest);
		gui_Create(wnd.elContent, d_cvtmulti_params, [RULES.put, params]);
	}

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.style.clear = 'both';
	elBtns.classList.add('buttons');
	elBtns.classList.add('param');

	var elLabel = document.createElement('div');
	elBtns.appendChild(elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://' + cgru_Config.af_servername + ':' + cgru_Config.af_serverport +
		'" target="_blank">AFANASY</a>';

	if (i_args.results)
	{
		var el = document.createElement('div');
		elBtns.appendChild(el);
		el.classList.add('button');
		el.style.cssFloat = 'right';
		el.textContent = 'Find Results';
		el.m_wnd = wnd;
		el.onclick = function(e) { fu_ResultsFind(e.currentTarget.m_wnd); }
	}

	wnd.m_res_btns_show = [];

	if (!i_args.images)
	{
		var elCvtBtn = document.createElement('div');
		elBtns.appendChild(elCvtBtn);
		elCvtBtn.textContent = title + ' To Movies';
		elCvtBtn.classList.add('button');
		elCvtBtn.onclick = function(e) { d_CvtProcessGUI(e.currentTarget.m_wnd, false); };
		elCvtBtn.m_wnd = wnd;
		wnd.m_res_btns_show.push(elCvtBtn);
		if (i_args.results)
			elCvtBtn.style.display = 'none';
	}

	var elExpBtn = document.createElement('div');
	elBtns.appendChild(elExpBtn);
	if (i_args.images)
		elExpBtn.textContent = title + ' To Images';
	else
		elExpBtn.textContent = title + ' To Sequences';
	elExpBtn.classList.add('button');
	elExpBtn.onclick = function(e) { d_CvtProcessGUI(e.currentTarget.m_wnd, true); };
	elExpBtn.m_wnd = wnd;
	wnd.m_res_btns_show.push(elExpBtn);
	if (i_args.results)
		elExpBtn.style.display = 'none';

	if (!i_args.images)
	{
		var elWmBtn = document.createElement('div');
		elBtns.appendChild(elWmBtn);
		elWmBtn.textContent = 'Add Movie Watermark';
		elWmBtn.classList.add('button');
		elWmBtn.style.cssFloat = 'right';
		elWmBtn.m_wnd = wnd;
		elWmBtn.onclick = function(e) { d_WmDialog(e.currentTarget.m_wnd) };
		wnd.elWmBtn = elWmBtn;
	}

	wnd.m_elResults = document.createElement('div');
	wnd.elContent.appendChild(wnd.m_elResults);
	if (i_args.results)
		wnd.m_elResults.classList.add('output');
	else
		wnd.m_elResults.classList.add('source');

	for (var i = 0; i < i_args.paths.length; i++)
	{
		i_args.paths[i] = c_PathPM_Rules2Client(i_args.paths[i]);
		var el = document.createElement('div');
		wnd.m_elResults.appendChild(el);
		el.textContent = i_args.paths[i];
	}
}

function d_CvtProcessGUI(i_wnd, i_to_sequence)
{
	// Get GUI parameters:
	var params = gui_GetParams(i_wnd.elContent, d_cvtguiparams);

	if (i_wnd.m_args.results)
		gui_GetParams(i_wnd.elContent, d_cvtmulti_params, params);

	for (var key in i_wnd.elContent.m_choises)
		params[key] = i_wnd.elContent.m_choises[key].value;

	// Process paths:
	var paths = i_wnd.m_args.paths;
	if (i_wnd.m_args.results)
	{
		// Paths are founded with find_results.py:
		var results = i_wnd.m_result.results;
		var res_skipped = [];
		paths = [];
		for (var i = 0; i < results.length; i++)
		{
			if (results[i].error)
				continue;

			if (results[i].exist && params.skipexisting)
				continue;

			paths.push(results[i].src);
			res_skipped.push(results[i]);
		}

		i_wnd.m_result.results = res_skipped;
	}
	else
	{
		// Paths are just selected in filesview:
		for (var i = 0; i < paths.length; i++)
			paths[i] = c_PathPM_Client2Server(paths[i]);
	}

	if (paths.length == 0)
	{
		c_Error('Nothing to convert founded.');
		return;
	}

	i_wnd.m_args.cvt_paths = paths;

	// Run specific functions:
	if (i_wnd.m_args.movies || (i_to_sequence == false))
	{
		d_CvtMovies(i_wnd, params, i_to_sequence);
	}
	else
	{
		d_CvtImages(i_wnd, params);
	}
}

function d_CvtImages(i_wnd, i_params)
{
	// console.log( JSON.stringify( i_wnd.m_args));
	// console.log( JSON.stringify( i_params));
	var paths = i_wnd.m_args.cvt_paths;

	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild(elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var cmd = 'rules/bin/convert.sh -J';

	cmd += ' -t ' + i_params.imgtype;
	cmd += ' -c ' + i_params.colorspace;
	cmd += ' -q ' + i_params.quality;
	if (i_params.padding)
		cmd += ' --renumpad ' + i_params.padding;
	if (i_params.first_frame)
		cmd += ' --renumfirst ' + i_params.first_frame;
	if (i_params.format && (i_params.format != 'asis'))
		cmd += ' -r ' + i_params.format;

	var afanasy = false;
	if (i_wnd.m_args.folders || (paths.length > 1))
	{
		afanasy = true;
		cmd += ' -A';
		cmd += ' --afuser "' + g_auth_user.id + '"';
		cmd += ' --afcap ' + i_params.af_capacity;
		cmd += ' --afmax ' + i_params.af_maxtasks;
		cmd += ' --afmph ' + i_params.af_perhost;
		cmd += ' --affpt ' + i_params.af_fpt;
		if (i_params.af_hostsmask.length)
			cmd += ' --afhostsmask "' + i_params.af_hostsmask + '"';
		if (i_params.af_paused)
			cmd += ' --afpaused';

		if (i_wnd.m_args.results)
			cmd += ' -o "' + c_PathPM_Client2Server(i_wnd.m_result.dest) + '"';
	}

	for (var i = 0; i < paths.length; i++)
		cmd += ' "' + paths[i] + '"'

			n_Request({
				   "send": {"cmdexec": {"cmds": [cmd]}},
				   "func": d_CvtImagesFinished,
				   "wnd": i_wnd,
				   "afanasy": afanasy
			   });
}

function d_CvtImagesFinished(i_data, i_args)
{
	// console.log( JSON.stringify( i_data));
	// console.log( JSON.stringify( i_args));
	if (i_args.afanasy == false)
	{
		i_args.wnd.destroy();
		i_args.wnd.m_args.filesview.refresh();
		//		fv_ReloadAll();
		return;
	}

	i_args.wnd.elContent.removeChild(i_args.wnd.m_elWait);

	var elOut = i_args.wnd.m_elResults;
	elOut.textContent = '';
	elOut.classList.remove('source');
	elOut.classList.add('output');

	if ((i_data.cmdexec == null) || (i_data.cmdexec.length == 0) || (i_data.cmdexec[0].convert == null))
	{
		elOut.textContent = JSON.stringify(i_data);
		return;
	}

	if (i_data.cmdexec[0].error)
		c_Error(i_data.cmdexec[0].error);

	var convert = i_data.cmdexec[0].convert;

	if (convert.error)
		c_Error(convert.error);

	for (var i = 0; i < convert.length; i++)
	{
		var mkdir = convert[i].mkdir;
		var seqs = convert[i].sequences;

		var el = document.createElement('div');
		elOut.appendChild(el);
		if (mkdir)
			el.textContent = mkdir;
		else
			el.textContent = convert[i].input;


		for (var j = 0; j < seqs.length; j++)
		{
			var seq = seqs[j];
			var el = document.createElement('div');
			elOut.appendChild(el);
			var text = c_PathBase(seq.inseq) + ' -> ' + c_PathBase(seq.outseq);

			if (seq.warning)
			{
				text += ' ' + seq.warning;
				el.style.color = '#F82';
			}

			el.textContent = text;
		}
	}
}

function d_CvtMovies(i_wnd, i_params, i_to_sequence)
{
	var paths = i_wnd.m_args.cvt_paths;

	var job = {};
	job.name = c_PathBase(paths[0]);
	job.folders = {};
	job.folders.input = c_PathDir(paths[0]);

	if (i_params.format && (i_params.format != 'asis'))
		job.name += '-' + i_params.format;

	if (i_to_sequence)
		job.name = 'Explode ' + job.name;
	else
		job.name = 'Convert ' + job.name;

	if (paths.length > 1)
		job.name = c_PathDir(paths[0]) + ' x' + paths.length;

	job.max_running_tasks = i_params.af_maxtasks;
	job.max_running_tasks_per_host = i_params.af_perhost;
	if (i_params.af_hostsmask.length)
		job.hosts_mask = i_params.af_hostsmask;
	if (i_params.af_paused)
		job.offline = true;

	var block = {};
	block.service = 'movgen';
	block.parser = 'generic';
	block.capacity = i_params.af_capacity;
	block.working_directory = c_PathDir(paths[0]);
	block.working_directory = c_PathPM_Rules2Server(g_CurPath());
	block.tasks = [];
	job.blocks = [block];

	var cmd = 'movconvert';
	cmd += ' -a ' + RULES.avcmd;
	if (i_params.format && (i_params.format != 'asis'))
		cmd += ' -r "' + i_params.format + '"';
	if (i_params.time_start != d_cvtguiparams.time_start.default)
		cmd += ' -s ' + i_params.time_start;
	if (i_params.duration != d_cvtguiparams.duration.default)
		cmd += ' -d ' + i_params.duration;

	if (i_to_sequence)
	{
		block.name = 'Explode to ' + i_params.imgtype.toUpperCase();

		cmd += ' -t ' + i_params.imgtype;

		var q = i_params.quality;
		q = Math.round(10 - (q / 10));
		if (q < 1)
			q = 1;
		cmd += ' -q ' + q;

		if (i_params.padding)
			cmd += ' -p ' + i_params.padding;

		if (i_params.first_frame)
			cmd += ' --first ' + i_params.first_frame;
	}
	else
	{
		job.name += '.' + i_params.codec.toUpperCase();
		block.name = 'Convert to ' + i_params.codec.toUpperCase();
		cmd += ' -c "' + i_params.codec + '"';
		if (i_params.container != 'DEFAULT')
		{
			cmd += ' -n ' + i_params.container;
			job.name += '.' + i_params.container.toUpperCase();
			block.name += '.' + i_params.container.toUpperCase();
		}
		cmd += ' -f ' + i_params.fps;

		if (i_params.ipar)
			cmd += ' --ipar ' + i_params.ipar;

		if (i_params.audio_file != '')
			cmd += ' --audio "' + i_params.audio_file + '"';

		if (i_wnd.wm)
		{
			cmd += ' -w "' + i_wnd.wm.file + '"';
			cmd += ' -u "' + i_wnd.wm.params.text + '"';
		}
	}

	for (var i = 0; i < paths.length; i++)
	{
		var path = paths[i];
		var task = {};
		block.tasks.push(task);
		task.name = c_PathBase(path);
		task.command = cmd + ' "' + path + '"';

		if (i_wnd.m_args.results)
			task.command += ' -o "' + i_wnd.m_result.dest + '/' + i_wnd.m_result.results[i].name + '"';
	}

	n_SendJob(job);

	i_wnd.destroy();
	// console.log(JSON.stringify(job));
}

function d_CvtOnDestroy(i_wnd)
{
	// Destroy watermark window if any:
	if (i_wnd.wm)
		d_WmDiscard(i_wnd.wm)
}

/* ---------------- [ Watermark structs and functions ] -------------------------------------------------- */

var d_wm_params = {
	file /*******/: {},
	text /*******/: {'width': '35%'},
	color /******/: {'width': '30%', 'default': 'rgba(200,200,200,0.20)'},
	pointsize /**/: {'width': '15%', 'default': '300'},
	size /*******/: {'width': '20%'}
};

function d_WmDialog(i_wnd)
{
	var wm = {};
	wm.wnd = i_wnd;
	i_wnd.wm = wm;
	i_wnd.elContent.style.display = 'none';

	var file = 'watermark';
	file += '_' + g_auth_user.id;
	file += '.png';
	file = g_CurPath() + '/.rules/' + file;

	d_wm_params.file.default = file;

	d_wm_params.size.default = RULES.dailies.format;

	wm.elBack = document.createElement('div');
	document.body.appendChild(wm.elBack);
	wm.elBack.classList.add('watermark_back');

	wm.elControls = document.createElement('div');
	wm.elBack.appendChild(wm.elControls);
	wm.elControls.classList.add('watermark_ctrls');

	wm.elParams = document.createElement('div');
	wm.elControls.appendChild(wm.elParams);
	gui_Create(wm.elParams, d_wm_params);

	wm.elBtns = document.createElement('div');
	wm.elBtns.classList.add('watermark_btns');
	wm.elControls.appendChild(wm.elBtns);

	wm.elCreate = document.createElement('div');
	wm.elBtns.appendChild(wm.elCreate);
	wm.elCreate.classList.add('button');
	wm.elCreate.textContent = 'Create & Show';
	wm.elCreate.wm = wm;
	wm.elCreate.onclick = function(e) { d_WmCreate(e.currentTarget.wm) };

	wm.elEnable = document.createElement('div');
	wm.elBtns.appendChild(wm.elEnable);
	wm.elEnable.classList.add('button');
	wm.elEnable.textContent = 'Enable & Exit';
	wm.elEnable.wm = wm;
	wm.elEnable.onclick = function(e) { d_WmEnable(e.currentTarget.wm) };
	wm.elEnable.style.display = 'none';

	wm.elDiscard = document.createElement('div');
	wm.elBtns.appendChild(wm.elDiscard);
	wm.elDiscard.classList.add('button');
	wm.elDiscard.textContent = 'Discard Watermark';
	wm.elDiscard.wm = wm;
	wm.elDiscard.onclick = function(e) { d_WmDiscard(e.currentTarget.wm) };
}

function d_WmCreate(i_wm)
{
	var params = gui_GetParams(i_wm.elParams, d_wm_params);
	i_wm.params = params;

	if (params.text.length == 0)
		return;

	i_wm.file = c_PathPM_Rules2Server(params.file);

	var cmd = 'bin/convert';
	cmd += ' -size "' + params.size + '"';
	cmd += ' xc:none';
	cmd += ' -gravity "center"';
	cmd += ' -fill "' + params.color + '"';
	cmd += ' -pointsize ' + params.pointsize;
	cmd += ' -annotate 0';
	cmd += ' "' + params.text + '"';
	cmd += ' "' + i_wm.file + '"';

	var folder = c_PathDir(i_wm.file);

	n_Request({
		"send": {"makefolder": {"path": folder}, "cmdexec": {"cmds": [cmd]}},
		"func": d_WmCreateFinished,
		"wm": i_wm,
		"info": 'watermark'
	});
	//	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":d_WmCreateFinished,"wm":i_wm,"info":'watermark'});
}

function d_WmCreateFinished(i_data, i_args)
{
	var wm = i_args.wm;

	if (wm.elImg)
		wm.elBack.removeChild(wm.elImg);

	wm.elImg = document.createElement('img');
	wm.elBack.appendChild(wm.elImg);
	wm.elImg.src = RULES.root + wm.params.file + '#' + (new Date().getTime());

	wm.elEnable.style.display = 'block';
}

function d_WmEnable(i_wm)
{
	d_WmDiscard(i_wm);

	i_wm.wnd.wm = i_wm;

	i_wm.wnd.elWmBtn.classList.add('watermark_enabled');
}

function d_WmDiscard(i_wm)
{
	i_wm.wnd.wm = null;
	i_wm.wnd.elContent.style.display = 'block';
	i_wm.wnd.elWmBtn.classList.remove('watermark_enabled');

	document.body.removeChild(i_wm.elBack);
}

/* ###########################################################################################################
################### [ CUT ] ##################################################################################
########################################################################################################### */

var d_cutparams = {
	cut_name      : {},
	input         : {},
	fps           : {"label": 'FPS', 'width': '25%'},
	af_pertask    : {"label": 'Frames Per Task', 'width': '25%', 'lwidth': '140px'},
	af_maxtasks   : {"label": 'Max Run Tasks', 'width': '25%', 'lwidth': '120px'},
	af_perhost    : {"label": 'Per Host', 'width': '25%', 'lwidth': '140px'},
	af_capacity   : {"label": 'Capacity', 'width': '20%'},
	af_maxruntime : {"label": 'Max Run Time', 'width': '20%', 'lwidth': '140px'},
	skipnosrc     : {"label": 'Skip No Src', 'width': '20%', 'lwidth': '120px', 'type':'bool', default: false},
	getcomments   : {"label": 'Get Comments', 'width': '20%', 'lwidth': '140px', 'type':'bool', default: false},
	flipversion   : {"label": 'Flip Version', 'width': '20%', 'lwidth': '140px', 'type':'bool', default: false},
	output        : {}
};

function d_MakeCut(i_args)
{
	// console.log( JSON.stringify( i_args));
	var wnd = new cgru_Window({"name": 'cut', "title": 'Make Cut'});
	wnd.m_args = i_args;

	var params = {};
	params.cut_name = i_args.cut_name;
	params.output = c_PathPM_Rules2Client(i_args.output);
	params.input = RULES.assets.shot.result.path.join(',');
	params.fps = RULES.fps;
	if (RULES.dailies.fps)
		params.fps = RULES.dailies.fps;
	if (RULES.cut.input)
		params.input = RULES.cut.input;

	gui_Create(wnd.elContent, d_cutparams, [RULES.dailies, RULES.cut, params]);
	gui_CreateChoices({
		"wnd": wnd.elContent,
		"name": 'format',
		"value": RULES.dailies.format,
		"label": 'Formats:',
		"keys": RULES.dailies.formats
	});
	gui_CreateChoices({
		"wnd": wnd.elContent,
		"name": 'colorspace',
		"value": RULES.dailies.colorspace,
		"label": 'Colorspace:',
		"keys": RULES.dailies.colorspaces
	});
	gui_CreateChoices({
		"wnd": wnd.elContent,
		"name": 'codec',
		"value": RULES.dailies.codec,
		"label": 'Codecs:',
		"keys": RULES.dailies.codecs
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

	var elTest = document.createElement('div');
	elAfDiv.appendChild(elTest);
	elTest.textContent = 'Find Results';
	elTest.classList.add('button');
	elTest.m_wnd = wnd;
	elTest.onclick = function(e) { d_CutProcessGUI(e.currentTarget.m_wnd, true); };

	var elSend = document.createElement('div');
	elAfDiv.appendChild(elSend);
	elSend.textContent = 'Send Job';
	elSend.classList.add('button');
    elSend.style.display = 'none';
	elSend.m_wnd = wnd;
    wnd.m_elSend = elSend;
	elSend.onclick = function(e) { d_CutProcessGUI(e.currentTarget.m_wnd, false); };

	var elResults = document.createElement('div');
	wnd.elContent.appendChild(elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');

	for (var i = 0; i < i_args.shots.length; i++)
	{
		var el = document.createElement('div');
		elResults.appendChild(el);
		el.textContent = i_args.shots[i];
	}
}

function d_CutProcessGUI(i_wnd, i_test)
{
	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild(elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var shots = i_wnd.m_args.shots;
	var params = gui_GetParams(i_wnd.elContent, d_cutparams);
	for (var key in i_wnd.elContent.m_choises)
		params[key] = i_wnd.elContent.m_choises[key].value;

	var cmd = 'rules/bin/makecut.sh';

	cmd += ' -i "' + params.input + '"';
	cmd += ' -n "' + params.cut_name + '"';
	cmd += ' -u "' + g_auth_user.id + '"';
	cmd += ' -f "' + params.fps + '"';
	cmd += ' -r "' + params.format + '"';
	cmd += ' -c "' + params.codec + '"';
	cmd += ' --colorspace "' + params.colorspace + '"';

	if (params.getcomments)
		cmd += ' --getcomments';

	if (params.skipnosrc)
		cmd += ' --skipnosrc'

	if (params.flipversion)
		cmd += ' --flipversion'

	if (RULES.dailies.font)
		cmd += ' --font "' + RULES.dailies.font + '"';

	cmd += ' --afcapacity ' + parseInt(params.af_capacity);
	cmd += ' --afmaxtasks ' + parseInt(params.af_maxtasks);
	cmd += ' --afperhost ' + parseInt(params.af_perhost);
	cmd += ' --afpertask ' + parseInt(params.af_pertask);
	cmd += ' --afmaxruntime ' + parseInt(params.af_maxruntime);

	cmd += ' -o "' + c_PathPM_Client2Server(params.output) + '"';
	if (i_test)
		cmd += ' -t';

	for (var i = 0; i < shots.length; i++)
		cmd += ' "' + c_PathPM_Rules2Server(shots[i]) + '"'
			n_Request({"send": {"cmdexec": {"cmds": [cmd], 'ignore_errors': true}}, "func": d_CutFinished, "wnd": i_wnd});
}

function d_CutFinished(i_data, i_args)
{
	// console.log( JSON.stringify( i_data));
	// console.log( JSON.stringify( i_args));
	i_args.wnd.elContent.removeChild(i_args.wnd.m_elWait);
	var elResults = i_args.wnd.m_elResults;
	elResults.textContent = '';

	if ((i_data.cmdexec == null) || (!i_data.cmdexec.length) || (i_data.cmdexec[0].cut == null))
	{
		elResults.textContent = (JSON.stringify(i_data));
        i_args.wnd.m_elSend.style.display = 'none';
		return;
	}

    i_args.wnd.m_elSend.style.display = 'block';

	var cut = i_data.cmdexec[0].cut;

	for (var i = cut.length - 1; i >= 0; i--)
	{
		var el = document.createElement('div');
		elResults.appendChild(el);
		var text = '';
		for (var msg in cut[i])
		{
			if (msg == 'sequence')
			{
				text = cut[i][msg] + ': ' + cut[i].first + ' - ' + cut[i].last + ' = ' + cut[i].count;
				break;
			}
			text += ' ' + msg + ': ' + cut[i][msg];
		}

		if (text.indexOf('warning') != -1)
			el.style.color = '#FF2';

		if (text.indexOf('error') != -1)
        {
			el.style.color = '#F42';
            i_args.wnd.m_elSend.style.display = 'none';
        }

		el.textContent = text;
	}

	//	i_wnd.destroy();
}
