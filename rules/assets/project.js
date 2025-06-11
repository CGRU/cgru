if (ASSETS.project && (ASSETS.project.path == g_CurPath()))
{
	prj_Init();
}

var prj_tags = {};
var prj_tags_file = 'rules.project.tags.json'

function prj_Init()
{
	a_SetLabel('Project');
	n_GetFile(
		{'path': 'rules/assets/project.html', 'func': prj_InitHTML, 'info': 'project.html', 'parse': false});
}
function prj_InitHTML(i_data)
{
	$('asset').innerHTML = i_data;

	// Show top buttons:
	if (c_CanCreateProject())
	{
		let el = document.createElement('div');
		$('asset_top_left').appendChild(el);
		el.classList.add('button');
		el.textContent = 'CREATE NEW PROJECT';
		el.title = 'Create new project.';
		el.onclick = prj_Create_Project;
	}

	if (c_CanEditProjectTags())
		$('project_tags_edit_btn').style.display = 'block';

	// Load project specific tags:
	prj_TagsLoad();
}

function prj_Create_Project()
{
	let args = {};
	args.title = 'Create New Project';
	args.template = RULES.assets.project.template;
	args.destination = c_PathDir(g_CurPath());
	args.name = 'PROJECT_NAME';
	a_Copy(args);
}


function prj_TagsLoad(i_edit = false)
{
	let args = {};
	args.path = c_GetRuFilePath(prj_tags_file);
	args.info = 'tags';
	args.func = prj_TagsReceived;
	args.edit = i_edit;
	args.cache_time = -1;
	n_GetFile(args);
}

function prj_TagsReceived(i_data, i_args)
{
	prj_tags = {};

	if (i_data == null)
	{
		return;
	}

	if (i_data.error)
	{
		if (i_data.error.indexOf('No such file') != -1)
		{
			if (i_args.edit)
				prj_TagsEdit();
			return;
		}
	}

	if (i_data.tags == null)
	{
		c_Error('Invalid tags data received.');
		c_Log(i_data);
		return;
	}

	let tags = [];
	for (let tag in i_data.tags)
		if (tag.length)
		{
			tags.push(tag);
			prj_tags[tag] = i_data.tags[tag];
		}

	st_SetElTags({'tags': tags}, $('project_tags'));

	let info = '';
	if (i_data.tags_last_edit_by)
		info += ' by ' + c_GetUserTitle(i_data.tags_last_edit_by);
	if (i_data.tags_last_edit_time)
		info += ' at ' + c_DT_StrFromSec(i_data.tags_last_edit_time);
	if (info.length)
		$('project_tags_info').textContent = 'Last modified' + info;
	else
		$('project_tags_info').textContent = '';

	if (i_args.edit)
		prj_TagsEdit();
}


function prj_TagsEdit()
{
	$('project_tags').style.display = 'none';
	$('project_tags_edit').style.display = 'block';
	$('project_tags_edit_btn').style.display = 'none';

	let elTable = document.createElement('table');
	$('project_tags_edit').appendChild(elTable);

	let elTr = document.createElement('tr');
	elTable.appendChild(elTr);

	let elTh = document.createElement('th');
	elTr.appendChild(elTh);
	elTh.textContent = 'name';
	elTh.classList.add('name');
	elTh.style.width = '10%';
	elTh = document.createElement('th');
	elTr.appendChild(elTh);
	elTh.textContent = 'Title';
	elTh = document.createElement('th');
	elTr.appendChild(elTh);
	elTh.textContent = 'Short';
	elTh.style.width = '10%';
	elTh = document.createElement('th');
	elTr.appendChild(elTh);
	elTh.textContent = 'Tip';
	elTh.style.width = '50%';

	let table_tags = {};
	for (tag in prj_tags)
		table_tags[tag] = prj_tags[tag];
	table_tags[''] = {};
	for (let tag in table_tags)
	{
		let elTag = document.createElement('tr');
		elTable.appendChild(elTag);

		let elName = document.createElement('td');
		elTag.appendChild(elName);
		elName.classList.add('name');
		elName.textContent = tag;

		let elTitle = document.createElement('td');
		elTag.appendChild(elTitle);
		elTitle.classList.add('title');
		elTitle.textContent = table_tags[tag].title;

		let elShort = document.createElement('td');
		elTag.appendChild(elShort);
		elShort.classList.add('short');
		elShort.textContent = table_tags[tag].short;

		let elTip = document.createElement('td');
		elTag.appendChild(elTip);
		elTip.classList.add('tip');
		elTip.textContent = table_tags[tag].tip;

		let elTd = document.createElement('td');
		elTag.appendChild(elTd);
		elTd.classList.add('td_btn');
		elTd.style.width = '8%';
		let elBtn = document.createElement('div');
		elTd.appendChild(elBtn);
		elBtn.classList.add('button');

		if (tag.length)
		{
			// Existing tag:
			elBtn.textContent = 'DEL';
			elBtn.title = 'Double click to delete tag.'
			elBtn.m_name = tag;
			elBtn.ondblclick = prj_TagRemove;
			continue;
		}

		// New tag:
		elTag.classList.add('add_new');
		elName.contentEditable = true;
		elName.classList.add('editing');
		elTitle.contentEditable = true;
		elTitle.classList.add('editing');
		elShort.contentEditable = true;
		elShort.classList.add('editing');
		elTip.contentEditable = true;
		elTip.classList.add('editing');

		elBtn.textContent = 'ADD';
		elBtn.title = 'Click to add tag.'
		elBtn.m_elTag = elTag;
		elBtn.onclick = function() {
			let name = elName.textContent;
			let title = elTitle.textContent;
			let short = elShort.textContent;
			let tip = elTip.textContent;
			prj_TagAdd(name, {'title': title, 'short': short, 'tip': tip});
		}
	}
}

function prj_TagRemove(i_evt)
{
	i_evt.stopPropagation();
	let tag = i_evt.currentTarget.m_name;
	delete prj_tags[tag];
	delete RULES.tags[tag];
	prj_TagsSave();
}

function prj_TagAdd(i_name, i_data)
{
	// Check tag props
	if (i_name.length == 0)
	{
		c_Error('Tag name is not specified.');
		return;
	}
	if (i_name.length < 3)
	{
		c_Error('Tag name should be at least 3 characters length.');
		return;
	}
	if (/^[a-z_0-9]+$/.test(i_name) == false)
	{
		c_Error('Tag name should contain only a-z and 0-9.');
		return;
	}
	if (/[0-9]/.test(i_name[0]))
	{
		c_Error('Tag name should not start with a number.');
		return;
	}
	if (i_data.title.length == 0)
	{
		c_Error('Tag title is not specified.');
		return;
	}
	if (i_data.title.length < 3)
	{
		c_Error('Tag title should be at least 3 characters length.');
		return;
	}
	if (i_data.short.length == 0)
	{
		c_Error('Tag short is not specified.');
		return;
	}
	if (i_data.short.length > 4)
	{
		c_Error('Tag short should be maximum 4 characters length.');
		return;
	}
	if (i_data.tip.length == 0)
	{
		c_Error('Tag tip is not specified.');
		return;
	}

	prj_tags[i_name] = i_data;
	RULES.tags[i_name] = i_data;
	let keys = Object.keys(prj_tags).sort();
	let new_tags = {};
	for (let k of keys)
		new_tags[k] = prj_tags[k];
	prj_tags = new_tags;
	prj_TagsSave();
}

function prj_TagsSave()
{
	// console.log(JSON.stringify(prj_tags));
	let obj = {};
	obj.tags = prj_tags;
	obj.tags_last_edit_by = g_auth_user.id;
	obj.tags_last_edit_time = c_DT_CurSeconds();
	n_Request({
		'send': {'save': {'file': c_GetRuFilePath(prj_tags_file), 'data': JSON.stringify(obj, null, 4)}},
		'func': prj_TagsSaveFinished,
		'info': 'tags save'
	});
}

function prj_TagsSaveFinished()
{
	$('project_tags').style.display = 'block';
	$('project_tags_edit').style.display = 'none';
	$('project_tags_edit').textContent = '';
	$('project_tags_edit_btn').style.display = 'block';

	prj_TagsLoad();
}

// ######################################################################
// ##################   Deploy shots   ##################################
// ######################################################################

prj_deploy_shots_params = {};
prj_deploy_shots_params.sources = {};
prj_deploy_shots_params.references = {};
prj_deploy_shots_params.destination = {};
prj_deploy_shots_params.template = {};
prj_deploy_shots_params.prefix = {
	'width': '33%',
	'tooltip': 'Add a prefix to each shot name.'
};
prj_deploy_shots_params.regexp = {
	'width': '33%',
	'tooltip': 'Perform a regular expression replace.'
};
prj_deploy_shots_params.substr = {
	'width': '33%',
	'tooltip': 'Perform a regular expression replace.'
};
prj_deploy_shots_params.sameshot = {
	'width': '25%',
	'type': 'bool',
	'default': true,
	'tooltip': 'Example: "NAME" and "NAME-1" will be one shot.'
};
prj_deploy_shots_params.extract = {
	'width': '25%',
	'type': 'bool',
	'default': false,
	'tooltip': 'Extract sources folder.'
};
prj_deploy_shots_params.uppercase = {
	'width': '25%',
	'type': 'bool',
	'default': false,
	'tooltip': 'Convert shot names to upper case.'
};
prj_deploy_shots_params.padding = {
	'width': '25%',
	'tooltip': 'Example: "432" - first number will have 4 padding, next 3 and so on.'
};

function prj_ShotsDeploy()
{
	var wnd = new cgru_Window({'name': 'deploy_shots', 'title': 'Deploy Shots'});
	wnd.elContent.classList.add('deploy_shots');

	var params = {};
	params.sources = c_PathPM_Rules2Client(g_CurPath() + '/IN');
	params.template = c_PathPM_Server2Client(RULES.assets.shot.template);

	// console.log( JSON.stringify( g_elCurFolder.m_dir));
	params.destination = RULES.assets.area.seek[0];
	for (var s = 0; s < RULES.assets.area.seek.length; s++)
		for (var f = 0; f < g_elCurFolder.m_dir.folders.length; f++)
			if (RULES.assets.area.seek[s].indexOf(g_elCurFolder.m_dir.folders[f]) != -1)
				params.destination = RULES.assets.area.seek[s];
	params.destination = params.destination.replace('[project]', ASSETS.project.path) + '/deploy';
	params.destination = c_PathPM_Rules2Client(params.destination);

	gui_Create(wnd.elContent, prj_deploy_shots_params, [params]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.classList.add('param');
	elBtns.style.clear = 'both';

	var elLabel = document.createElement('div');
	elBtns.appendChild(elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://' + cgru_Config.af_servername + ':' + cgru_Config.af_serverport +
		'" target="_blank">AFANASY</a>';

	var elCopy = document.createElement('div');
	elBtns.appendChild(elCopy);
	wnd.m_elCopy = elCopy;
	elCopy.style.display = 'none';
	elCopy.textContent = 'Send Copy Job';
	elCopy.classList.add('button');
	elCopy.m_wnd = wnd;
	elCopy.onclick =
		function(e) {
		prj_ShotsDeployDo(e.currentTarget.m_wnd, {'test': false, 'move': false});
	}

	var elMove = document.createElement('div');
	elBtns.appendChild(elMove);
	wnd.m_elMove = elMove;
	elMove.style.display = 'none';
	elMove.textContent = 'Move Sources';
	elMove.classList.add('button');
	elMove.m_wnd = wnd;
	elMove.onclick =
		function(e) {
		prj_ShotsDeployDo(e.currentTarget.m_wnd, {'test': false, 'move': true});
	}

	var elTest = document.createElement('div');
	elBtns.appendChild(elTest);
	elTest.textContent = 'Test Sources';
	elTest.classList.add('button');
	elTest.m_wnd = wnd;
	elTest.style.cssFloat = 'right';
	elTest.onclick =
		function(e) {
		prj_ShotsDeployDo(e.currentTarget.m_wnd, {'test': true});
	}

	var elResults = document.createElement('div');
	wnd.elContent.appendChild(elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');

	elResults.textContent = 'Test sources at first.';
}

function prj_ShotsDeployDo(i_wnd, i_args)
{
	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild(elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var cmd = 'rules/bin/deploy_shots.sh';
	var params = gui_GetParams(i_wnd.elContent, prj_deploy_shots_params);

	cmd += ' -s "' + c_PathPM_Client2Server(params.sources) + '"';
	if (params.references.length)
		cmd += ' -r "' + c_PathPM_Client2Server(params.references) + '"';
	cmd += ' -t "' + c_PathPM_Client2Server(params.template) + '"';
	cmd += ' -d "' + c_PathPM_Client2Server(params.destination) + '"';
	cmd += ' --shot_src "' + RULES.assets.shot.source.path[0] + '"'
	cmd += ' --shot_ref "' + RULES.assets.shot.references.path[0] + '"'
	if (params.sameshot) cmd += ' --sameshot';
	if (params.extract)
		cmd += ' --extract';
	if (params.uppercase)
		cmd += ' -u';
	if (params.padding.length)
		cmd += ' -p ' + params.padding;
	if (params.prefix.length)
		cmd += ' --prefix "' + params.prefix + '"';
	if (params.regexp.length)
		cmd += ' --regexp "' + params.regexp + '"';
	if (params.substr.length)
		cmd += ' --substr "' + params.substr + '"';

	if (i_args.move)
		cmd += ' -m';
	else
	{
		cmd += ' -A';
		cmd += ' --afuser "' + g_auth_user.id + '"';
		cmd += ' --afcap ' + RULES.put.af_capacity;
		cmd += ' --afmax ' + RULES.put.af_maxtasks;
	}

	if (i_args.test)
		cmd += ' --test';

	n_Request({'send': {'cmdexec': {'cmds': [cmd]}}, 'func': prj_ShotsDeployFinished, 'wnd': i_wnd});
}

function prj_ShotsDeployFinished(i_data, i_args)
{
	i_args.wnd.m_elCopy.style.display = 'block';
	i_args.wnd.m_elMove.style.display = 'block';
	i_args.wnd.elContent.removeChild(i_args.wnd.m_elWait);

	var elResults = i_args.wnd.m_elResults;
	elResults.textContent = '';


	if ((i_data.cmdexec == null) || (!i_data.cmdexec.length) || (i_data.cmdexec[0].deploy == null))
	{
		elResults.textContent = (JSON.stringify(i_data));
		return;
	}

	var deploy = i_data.cmdexec[0].deploy;
	// console.log(JSON.stringify(deploy));

	var elStat = document.createElement('div');
	elResults.appendChild(elStat);

	var elTable = document.createElement('table');
	elResults.appendChild(elTable);

	var elTr = document.createElement('tr');
	elTable.appendChild(elTr);
	var el = document.createElement('th');
	elTr.appendChild(el);
	el.textContent = 'Name';
	var el = document.createElement('th');
	elTr.appendChild(el);
	el.textContent = 'Folder';
	var el = document.createElement('th');
	elTr.appendChild(el);
	el.textContent = 'Sequences';
	var el = document.createElement('th');
	elTr.appendChild(el);
	el.textContent = 'Files';
	var el = document.createElement('th');
	elTr.appendChild(el);
	el.textContent = 'Refs';
	var el = document.createElement('th');
	elTr.appendChild(el);
	el.textContent = 'Comments';

	var shots_count = 0;
	var paths = {};

	for (var d = deploy.length - 1; d >= 0; d--)
	{
		// console.log(JSON.stringify(deploy[d]));
		var elTr = document.createElement('tr');
		elTable.appendChild(elTr);

		for (var key in deploy[d])
		{
			elTr.classList.add(key);

			if (key == 'shot')
			{
				shots_count++;
				var shot = deploy[d][key];
				// console.log(JSON.stringify(shot));

				// Shot name:
				var el = document.createElement('td');
				elTr.appendChild(el);
				el.textContent = shot.name;
				el.classList.add('deploy_name');

				// Main source:
				var el = document.createElement('td');
				elTr.appendChild(el);
				el.textContent = shot.FOLDER;
				el.classList.add('deploy_folder');

				// Same shot folders:
				var el = document.createElement('td');
				elTr.appendChild(el);
				el.classList.add('deploy_folders');
				var src = '';
				for (var i = 0; i < shot.FOLDERS.length; i++)
					src += ' ' + shot.FOLDERS[i];
				el.textContent = src;

				// Same shot files:
				var el = document.createElement('td');
				elTr.appendChild(el);
				el.classList.add('deploy_files');
				var src = '';
				for (var i = 0; i < shot.FILES.length; i++)
					src += ' ' + shot.FILES[i];
				el.textContent = src;

				// References:
				var el = document.createElement('td');
				elTr.appendChild(el);
				el.classList.add('deploy_ref');
				var ref = '';
				if (shot.REF && shot.REF.length)
				{
					for (var i = 0; i < shot.REF.length; i++)
						ref += ' ' + shot.REF[i];
				}
				el.textContent = ref;

				// Comments:
				var el = document.createElement('td');
				elTr.appendChild(el);
				el.classList.add('deploy_info');
				var comm = '';
				if (shot.exists)
				{
					comm = 'EXISTS';
					elTr.classList.add('deploy_exist');
				}
				el.textContent = comm;

				// console.log( JSON.stringify( shot));
				break;
			}
			else if (key == 'sources' || key == 'template' || key == 'dest')
			{
				paths[key] = c_PathPM_Server2Client(deploy[d][key]);
				break;
			}
			else
			{
				var el = document.createElement('td');
				elTr.appendChild(el);
				el.textContent = key + ': ' + deploy[d][key];
			}
		}
	}

	elStat.textContent = shots_count + ' shots founded in "' + paths.sources + '":';

	if (paths.dest)
	{
		var el = document.createElement('div');
		elResults.appendChild(el);
		el.textContent = 'Destination: ' + paths.dest;
	}

	if (paths.template)
	{
		var el = document.createElement('div');
		elResults.appendChild(el);
		el.textContent = 'Template: ' + paths.template;
	}
}
