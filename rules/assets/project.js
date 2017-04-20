if( ASSETS.project && ( ASSETS.project.path == g_CurPath()))
{
	prj_Init();
}

function prj_Init()
{
	a_SetLabel('Project');
	n_GetFile({"path":'rules/assets/project.html',"func":prj_InitHTML,"info":'project.html',"parse":false});
}
function prj_InitHTML( i_data)
{
	$('asset').innerHTML = i_data;
}

prj_deploy_shots_params = {};
prj_deploy_shots_params.sources = {};
prj_deploy_shots_params.references = {};
prj_deploy_shots_params.template = {};
prj_deploy_shots_params.destination = {};
prj_deploy_shots_params.prefix    = {"width":'33%',"tooltip":'Add a prefix to each shot name.'};
prj_deploy_shots_params.regexp    = {"width":'33%',"tooltip":'Perform a regular expression replace.'};
prj_deploy_shots_params.substr    = {"width":'33%',"tooltip":'Perform a regular expression replace.'};
prj_deploy_shots_params.sameshot  = {"width":'25%','type':"bool",'default':false,"tooltip":'Example: "NAME" and "NAME-1" will be one shot.'};
prj_deploy_shots_params.extract   = {"width":'25%','type':"bool",'default':false,"tooltip":'Extract sources folder.'};
prj_deploy_shots_params.uppercase = {"width":'25%','type':"bool",'default':false,"tooltip":'Convert shot names to upper case.'};
prj_deploy_shots_params.padding   = {"width":'25%',"tooltip":'Example: "432" - first number will have 4 padding, next 3 and so on.'};

function prj_ShotsDeploy()
{
	var wnd = new cgru_Window({"name":'deploy_shots',"title":'Deploy Shots'});
	wnd.elContent.classList.add('deploy_shots');

	var params = {};
	params.sources = c_PathPM_Rules2Client( g_CurPath() + '/IN');
	params.template = c_PathPM_Server2Client( RULES.assets.shot.template);

//console.log( JSON.stringify( g_elCurFolder.m_dir));
	params.destination = RULES.assets.scenes.seek[0];
	for( var s = 0; s < RULES.assets.scenes.seek.length; s++)
		for( var f = 0; f < g_elCurFolder.m_dir.folders.length; f++)
			if( RULES.assets.scenes.seek[s].indexOf( g_elCurFolder.m_dir.folders[f]) != -1 )
				params.destination = RULES.assets.scenes.seek[s];
	params.destination = params.destination.replace('[project]', ASSETS.project.path) + '/deploy';
	params.destination = c_PathPM_Rules2Client( params.destination);

	gui_Create( wnd.elContent, prj_deploy_shots_params, [params]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild( elBtns);
	elBtns.classList.add('param');
	elBtns.style.clear = 'both';

	var elLabel = document.createElement('div');
	elBtns.appendChild( elLabel);
	elLabel.classList.add('label');
	elLabel.innerHTML = '<a href="http://'+cgru_Config.af_servername+':'+cgru_Config.af_serverport+'" target="_blank">AFANASY</a>';

	var elCopy = document.createElement('div');
	elBtns.appendChild( elCopy);
	wnd.m_elCopy = elCopy;
	elCopy.style.display = 'none';
	elCopy.textContent = 'Send Copy Job';
	elCopy.classList.add('button');
	elCopy.m_wnd = wnd;
	elCopy.onclick = function(e){ prj_ShotsDeployDo( e.currentTarget.m_wnd,{"test":false,"move":false});}

	var elMove = document.createElement('div');
	elBtns.appendChild( elMove);
	wnd.m_elMove = elMove;
	elMove.style.display = 'none';
	elMove.textContent = 'Move Sources';
	elMove.classList.add('button');
	elMove.m_wnd = wnd;
	elMove.onclick = function(e){ prj_ShotsDeployDo( e.currentTarget.m_wnd, {"test":false,"move":true});}

	var elTest = document.createElement('div');
	elBtns.appendChild( elTest);
	elTest.textContent = 'Test Sources';
	elTest.classList.add('button');
	elTest.m_wnd = wnd;
	elTest.style.cssFloat = 'right';
	elTest.onclick = function(e){ prj_ShotsDeployDo( e.currentTarget.m_wnd, {"test":true});}

	var elResults = document.createElement('div');
	wnd.elContent.appendChild( elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');

	elResults.textContent = 'Test sources at first.';
}

function prj_ShotsDeployDo( i_wnd, i_args)
{
	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild( elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var cmd = 'rules/bin/deploy_shots.sh';
	var params = gui_GetParams( i_wnd.elContent, prj_deploy_shots_params);

	cmd += ' -s "' + c_PathPM_Client2Server( params.sources) + '"';
	if( params.references.length )
		cmd += ' -r "' + c_PathPM_Client2Server( params.references) + '"';
	cmd += ' -t "' + c_PathPM_Client2Server( params.template) + '"';
	cmd += ' -d "' + c_PathPM_Client2Server( params.destination) + '"';
	cmd += ' --shot_src "' + RULES.assets.shot.source.path[0] + '"'
	cmd += ' --shot_ref "' + RULES.assets.shot.references.path[0] + '"'
	if( params.sameshot ) cmd += ' --sameshot';
	if( params.extract ) cmd += ' --extract';
	if( params.uppercase ) cmd += ' -u';
	if( params.padding.length ) cmd += ' -p ' + params.padding;
	if( params.prefix.length ) cmd += ' --prefix "' + params.prefix + '"';
	if( params.regexp.length ) cmd += ' --regexp "' + params.regexp + '"';
	if( params.substr.length ) cmd += ' --substr "' + params.substr + '"';

	if( i_args.move )
		cmd += ' -m';
	else
	{
		cmd += ' -A';
		cmd += ' --afuser "' + g_auth_user.id + '"';
		cmd += ' --afcap ' + RULES.put.af_capacity;
		cmd += ' --afmax ' + RULES.put.af_maxtasks;
	}

	if( i_args.test ) cmd += ' --test';

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":prj_ShotsDeployFinished,"wnd":i_wnd});
}

function prj_ShotsDeployFinished( i_data, i_args)
{
	i_args.wnd.m_elCopy.style.display = 'block';
	i_args.wnd.m_elMove.style.display = 'block';
	i_args.wnd.elContent.removeChild( i_args.wnd.m_elWait);

	var elResults = i_args.wnd.m_elResults;
	elResults.textContent = '';


	if(( i_data.cmdexec == null ) || ( ! i_data.cmdexec.length ) || ( i_data.cmdexec[0].deploy == null ))
	{
		elResults.textContent = ( JSON.stringify( i_data));
		return;
	}

	var deploy = i_data.cmdexec[0].deploy;
//console.log(JSON.stringify(deploy));

	var elStat = document.createElement('div');
	elResults.appendChild( elStat);

	var elTable = document.createElement('table');
	elResults.appendChild( elTable);

	var elTr = document.createElement('tr');
	elTable.appendChild( elTr);
	var el = document.createElement('th'); elTr.appendChild(el); el.textContent = 'Name';
	var el = document.createElement('th'); elTr.appendChild(el); el.textContent = 'Folder';
	var el = document.createElement('th'); elTr.appendChild(el); el.textContent = 'Sequences';
	var el = document.createElement('th'); elTr.appendChild(el); el.textContent = 'Files';
	var el = document.createElement('th'); elTr.appendChild(el); el.textContent = 'Refs';
	var el = document.createElement('th'); elTr.appendChild(el); el.textContent = 'Comments';

	var shots_count = 0;
	var paths = {};

	for( var d = deploy.length - 1; d >= 0; d--)
	{
//console.log(JSON.stringify(deploy[d]));
		var elTr = document.createElement('tr');
		elTable.appendChild( elTr);

		for( var key in deploy[d])
		{
			elTr.classList.add( key);

			if( key == 'shot' )
			{
				shots_count++;
				var shot = deploy[d][key];
//console.log(JSON.stringify(shot));

				// Shot name:
				var el = document.createElement('td');
				elTr.appendChild( el);
				el.textContent = shot.name;
				el.classList.add('deploy_name');

				// Main source:
				var el = document.createElement('td');
				elTr.appendChild( el);
				el.textContent = shot.FOLDER;
				el.classList.add('deploy_folder');

				// Same shot folders:
				var el = document.createElement('td');
				elTr.appendChild( el);
				el.classList.add('deploy_folders');
				var src = '';
				for( var i = 0; i < shot.FOLDERS.length; i++)
					src += ' ' + shot.FOLDERS[i];
				el.textContent = src;

				// Same shot files:
				var el = document.createElement('td');
				elTr.appendChild( el);
				el.classList.add('deploy_files');
				var src = '';
				for( var i = 0; i < shot.FILES.length; i++)
					src += ' ' + shot.FILES[i];
				el.textContent = src;
				
				// References:
				var el = document.createElement('td');
				elTr.appendChild( el);
				el.classList.add('deploy_ref');
				var ref = '';
				if( shot.REF && shot.REF.length )
				{
					for( var i = 0; i < shot.REF.length; i++)
						ref += ' ' + shot.REF[i];
				}
				el.textContent = ref;

				// Comments:
				var el = document.createElement('td');
				elTr.appendChild( el);
				el.classList.add('deploy_info');
				var comm = '';
				if( shot.exists )
				{
					comm = 'EXISTS';
					elTr.classList.add('deploy_exist');
				}
				el.textContent = comm;

//console.log( JSON.stringify( shot));
				break;
			}
			else if( key == 'sources' || key == 'template' || key == 'dest')
			{
				paths[key] = c_PathPM_Server2Client( deploy[d][key]);
				break;
			}
			else
			{
				var el = document.createElement('td');
				elTr.appendChild( el);
				el.textContent = key + ': ' + deploy[d][key];
			}
		}
	}

	elStat.textContent = shots_count + ' shots founded in "' + paths.sources + '":';

	if( paths.dest )
	{
		var el = document.createElement('div');
		elResults.appendChild( el);
		el.textContent = 'Destination: ' + paths.dest;
	}

	if( paths.template )
	{
		var el = document.createElement('div');
		elResults.appendChild( el);
		el.textContent = 'Template: ' + paths.template;
	}

}

