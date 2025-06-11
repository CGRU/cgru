shot_thumb_paths = [];
shot_results_masks = [];

function shot_Init()
{
	shot_thumb_paths = [];
	shot_results_masks = [];

	a_SetLabel('Shot');

	a_SetTooltip('Info:\n\
This is Shot asset.\n\
It is designed to view shot sources and results on one page.\n');

	// Results naming masks:

	// Any at first, if other will not match, name is invalid:
	shot_results_masks.push({
		're': new RegExp('.*'),
		'bg': 'rgba( 255, 0, 0, .1)',
		'tip': 'Wrong result name.\n\
It must be:\n\
shot name + "_v" + version number with padding 3.\n\
 ( "SHOTNAME_v###" ) \n\
Example: "UNI5_v001"'
	});
	// Incorrect:
	shot_results_masks.push({
		're': new RegExp('^v\\d{3,3}'),
		'bg': 'rgba( 255, 150, 0, .2)',
		'tip': 'Folder does not contain shot name.'
	});
	shot_results_masks.push({
		're': new RegExp('^v\\d{3,3}$'),
		'bg': 'rgba( 255, 255, 0, .2)',
		'tip': 'Folder does not contain shot name.'
	});
	shot_results_masks.push({
		're': new RegExp('^' + ASSET.name + '_v\\d{3,3}', 'i'),
		'bg': 'rgba( 255, 0, 255, .2)',
		'tip': 'Folder name letters case mismatch.'
	});
	shot_results_masks.push({
		're': new RegExp('^' + ASSET.name + '_v\\d{3,3}'),
		'bg': 'rgba( 150, 255, 0, .2)',
		'tip': 'Folder name has characters after version 3 digits.'
	});
	// Correct:
	shot_results_masks.push({
		're': new RegExp('^' + ASSET.name + '_v\\d{3,3}$'),
		'bg': 'rgba(   0, 255, 0, .2)',
		'tip': 'Correct results folder name.'
	});
	// Movies:
	shot_results_masks.push({
		're': new RegExp('^' + ASSET.name + '_v\\d{3,3}\\.(mp4|mov)$'),
		'bg': 'rgba(   0, 255, 0, .2)',
		'tip': 'Correct results movie name.'
	});
	shot_results_masks.push({
		're': new RegExp('^' + ASSET.name + '_v\\d{3,3}_.*\\.(mp4|mov)$'),
		'bg': 'rgba(   0, 255, 0, .2)',
		'tip': 'Correct results movie name.'
	});
	shot_results_masks.push({
		're': new RegExp('^' + ASSET.name + '_(\\w+)_v\\d{3,3}_.*\\.(mp4|mov)$'),
		'bg': 'rgba(   0, 255, 0, .2)',
		'tip': 'Correct results movie name with activity.'
	});


	// Get page:
	n_GetFile({'path': 'rules/assets/shot.html', 'func': shot_InitHTML, 'info': 'shot.html', 'parse': false});
}
function shot_InitHTML(i_data)
{
	$('asset').innerHTML = i_data;

	// Show top buttons:
	if (c_CanCreateShot())
	{
		var el = document.createElement('div');
		$('asset_top_left').appendChild(el);
		el.classList.add('button');
		el.textContent = 'NEW SHOT';
		el.title = 'Create new shot.';
		el.onclick = shot_Create;
	}
	if (g_admin)
	{
		var el = document.createElement('div');
		$('asset_top_left').appendChild(el);
		el.classList.add('button');
		el.textContent = 'RENAME SHOT';
		el.title = 'Rename new shot.';
		el.onclick = shot_Rename;
	}

	// Create shot process buttons:
	if (c_CanExecuteSoft())
		u_CreateActions(ASSET.shot_process, $('shot_process_div'));

	shot_ResultsRead(true);
}

function shot_ResultsRead(i_first_time)
{
	// Collect walks to show result folders:
	var walk = {};
	walk.paths = [];

	if (ASSET.references)
		$('shot_refs_div').style.display = 'block';

	if (ASSET.source)
		$('shot_src_div').style.display = 'block';

	if (ASSET.result)
		for (var r = 0; r < ASSET.result.path.length; r++)
			walk.paths.push(ASSET.path + '/' + ASSET.result.path[r]);

	if (ASSET.dailies)
		for (var r = 0; r < ASSET.dailies.path.length; r++)
			walk.paths.push(ASSET.path + '/' + ASSET.dailies.path[r]);

	if (walk.paths.length == 0)
	{
		$('shot_results').textContent('No results paths defined in the shot asset.');
		return;
	}


	if (i_first_time)
		walk.cache_time = RULES.cache_time;

	walk.wfunc = shot_ResultsReceived;
	walk.info = 'walk results';
	walk.first_time = i_first_time;

	n_WalkDir(walk);
}

function shot_ResultsReceived(i_data, i_args)
{
	res_filesviews = [];

	var el = $('shot_results');
	el.textContent = '';
	var found = false;
	for (let i = 0; i < i_data.length; i++)
	{
		let folders = i_data[i].folders;
		let files = i_data[i].files;
		let path = i_args.paths[i];

		if (((folders == null) || (folders.length == 0)) && ((files == null) || (files.length == 0)))
			continue;

		// Last path is DAILIES,
		// We skip it for thumbnails, if some other path already added.
		// As better to make thumbnail from sequences, not movies (that just converted from that sequences)
		if ((i < (i_data.length - 1)) || (shot_thumb_paths.length == 0))
			if (false == path.includes('PRECOMP'))
				shot_thumb_paths.push(path);
		//^ Skip precomps (masks, cleanup, key can spoil animatic thumbnail)

		let fva = {};
		fva.el = el;
		fva.path = path;
		fva.name = c_PathBase(path);
		fva.walk = i_data[i];
		fva.show_walk = false;
		fva.can_count = true;
		fva.masks = shot_results_masks;
		fva.count_images = true;

		res_filesviews.push(new FilesView(fva));

		found = true;
	}

	if (false == found)
	{
		var e = document.createElement('div');
		el.appendChild(e);
		e.textContent = 'No results found in: ';
		e.classList.add('shot_empty_result');

		for (var i = 0; i < i_args.paths.length; i++)
		{
			var path = i_args.paths[i];
			path = path.replace(ASSET.path, '');
			if (path.indexOf('/') == 0)
				path = path.substring(1);

			var e = document.createElement('div');
			el.appendChild(e);
			e.textContent = path;
			e.classList.add('shot_empty_result');
		}
	}

	shot_MakeThumbnail();

	// Count frames numbers:
	for (var r = 0; r < res_filesviews.length; r++)
	{
		var fv = res_filesviews[r];

		if (fv.walk.folders == null)
			continue;
		if (fv.walk.folders.length == 0)
			continue;

		// Count frames number only in the last folder:
		// Find the last folder, but not '.commented':
		var folder = null;
		for (var f = fv.walk.folders.length - 1; f >= 0; f--)
		{
			if (fv.walk.folders[f].name.indexOf('.commented') == -1)
			{
				folder = fv.walk.folders[f];
				break;
			}
		}
		if (folder == null)
			continue;

		if (folder.num_images != null)
		{
			// if status frames numbers is undefined, but no update needed, we set status frames number:
			if (r == (res_filesviews.length - 1))
			{
				if ((RULES.status == null) || (RULES.status.frames_num == null))
				{
					st_SetFramesNumber(folder.num_images);
					c_Log(
						'Shot length updated from "' + (fv.path + '/' + folder.name) +
						'": ' + folder.num_images);
				}
			}

			continue;
		}

		var path = fv.path + '/' + folder.name;

		// Update status frames number if it is still not defined:
		var args = null;
		if ((RULES.status == null) || (RULES.status.frames_num == null))
		{
			args = {};
			args.func = shot_FilesCounted;
			args.path = path;
		}

		fv.countFiles(path, args);
	}

	if (i_args.first_time)
		shot_Post();
}

function shot_Post()
{
	g_PostLaunchFunc('shot');
}

function shot_FilesCounted(i_args, i_walk)
{
	if (i_walk.num_images)
	{
		st_SetFramesNumber(i_walk.num_images);
		c_Log('Shot length updated from "' + i_args.path + '": ' + i_walk.num_images);
		return;
	}
}

function shot_MakeThumbnail(i_args)
{
	if (shot_thumb_paths.length == 0)
		return;

	if (i_args == null)
		i_args = {};
	i_args.info = 'shot';
	i_args.paths = [shot_thumb_paths];

	u_ThumbnailMake(i_args);
}

function shot_RefsShow()
{
	$('shot_refs_btn').style.display = 'none';
	$('shot_refs_div').style.clear = 'both';
	$('shot_refs_div').style.cssFloat = 'none';
	$('shot_refs').textContent = '';
	$('shot_refs').style.display = 'block';
	$('shot_refs').classList.add('waiting');

	let walk = {};
	walk.paths = [];
	for (let i = 0; i < ASSET.references.path.length; i++)
		walk.paths.push(ASSET.path + '/' + ASSET.references.path[i]);

	walk.mediainfo = true;
	walk.wfunc = shot_RefsReceived;
	walk.info = 'walk refs';

	n_WalkDir(walk);
}
function shot_RefsClose()
{
	$('shot_refs_div').style.clear = 'none';
	$('shot_refs_div').style.cssFloat = 'left';
	$('shot_refs').style.display = 'none';
	$('shot_refs_btn').style.display = 'block';
	$('shot_refs_close_btn').style.display = 'none';
}
function shot_RefsReceived(i_data, i_args)
{
	$('shot_refs_close_btn').style.display = 'block';

	var walk = i_args;
	walk.walks = i_data;

	var el = $('shot_refs');
	el.textContent = '';
	el.classList.remove('waiting');
	// console.log( i_data);
	var found = false;
	var not_empty_paths = [];
	for (var i = 0; i < walk.paths.length; i++)
	{
		var folders = walk.walks[i].folders;
		var files = walk.walks[i].files;

		if (((folders == null) || (folders.length == 0)) && ((files == null) || (files.length == 0)))
			continue;
		not_empty_paths.push(walk.paths[i]);
		new FilesView(
			{'el': el, 'path': walk.paths[i], 'walk': walk.walks[i], 'limits': false, 'can_count': true})
		found = true;
	}
	if (false == found)
		el.textContent = JSON.stringify(walk.paths);

	if (shot_thumb_paths.length == 0)
	{
		shot_thumb_paths = not_empty_paths;
		shot_MakeThumbnail();
	}
}

function shot_SourcesScan()
{
	$('shot_src_div').style.clear = 'both';
	$('shot_src_div').style.cssFloat = 'none';
	$('shot_src').style.display = 'block';
	$('shot_src').classList.add('waiting');

	var paths = [];
	for (var i = 0; i < ASSET.source.path.length; i++)
		paths.push(ASSET.path + '/' + ASSET.source.path[i]);
	n_WalkDir({'paths': paths, 'depth': 5, 'wfunc': shot_SourcesReceived, 'info': 'walk src', 'local': true});
}
function shot_SourcesClose()
{
	$('shot_src_div').style.clear = 'none';
	$('shot_src_div').style.cssFloat = 'left';
	$('shot_src').style.display = 'none';
	$('shot_src_close_btn').style.display = 'none';
}
function shot_SourcesReceived(i_data, i_args)
{
	$('shot_src_btn').textContent = 'Rescan Sources';
	$('shot_src_close_btn').style.display = 'block';

	var el = $('shot_src');
	el.textContent = '';
	el.classList.remove('waiting');
	var found = false;
	var not_empty_paths = [];
	var thumb_skip_movies = false;
	for (var i = 0; i < i_data.length; i++)
	{
		var walk = {} walk.folders = [];
		walk.files = [];
		walk.rufiles = [];

		shot_SourceWalkFind(i_data[i], walk);

		// Update shot frames number if not set:
		if ((RULES.status == null) || (RULES.status.frames_num == null))
		{
			var frames_num = null;
			// Update only if all source sequences have the same length:
			for (var f = 0; f < walk.folders.length; f++)
			{
				if ((frames_num == null) && walk.folders[f].files && walk.folders[f].files.length)
				{
					frames_num = walk.folders[f].num_images;
				}
				else if (walk.folders[f].num_images && (frames_num != walk.folders[f].num_images))
				{
					frames_num = null;
					break;
				}
			}

			if (frames_num != null)
				st_SetFramesNumber(frames_num);
		}

		if (walk.folders.length || walk.files.length)
		{
			new FilesView({
				'el': el,
				'path': i_args.paths[i],
				'walk': walk,
				'limits': false,
				'count_images': true,
				'can_refresh': false
			});
			not_empty_paths.push(i_args.paths[i]);
			found = true;
		}

		if (walk.folders.length)
			thumb_skip_movies = true;
	}

	if (false == found)
	{
		var e = document.createElement('div');
		el.appendChild(e);
		e.textContent = 'No sources found in: ';
		e.classList.add('shot_empty_result');

		for (var i = 0; i < ASSET.source.path.length; i++)
		{
			var path = ASSET.source.path[i];
			var e = document.createElement('div');
			el.appendChild(e);
			e.textContent = path;
			e.classList.add('shot_empty_result');
		}
	}


	if (shot_thumb_paths.length == 0)
	{
		shot_thumb_paths = not_empty_paths;
		shot_MakeThumbnail({'skip_movies': thumb_skip_movies});
	}
}

function shot_SourceWalkFind(i_walk, o_walk, i_path, i_parent_walk)
{
	if (i_walk.files && i_walk.files.length)
	{
		var img_num = 0;
		for (f = 0; f < i_walk.files.length; f++)
		{
			var name = i_walk.files[f].name;

			if (name.indexOf('.') == 0)
				continue;

			if (c_FileIsMovie(name) || c_FileIsArchive(name))
			{
				if (i_path)
					i_walk.files[f].name = i_path + '/' + i_walk.files[f].name;
				o_walk.files.push(i_walk.files[f]);
			}

			if (false == c_FileIsImage(name))
				continue;

			img_num++;
		}
		//			if( img_num < 2 ) continue;

		if (img_num)
		{
			i_walk.num_images = img_num;

			if (i_path)
				i_walk.name = i_path;
			else if (i_walk.name == null)
				i_walk.name = '/';

			o_walk.folders.push(i_walk);

			// Add parent folder rufiles, needed to show thumbnails:
			if (i_parent_walk && i_parent_walk.rufiles)
				for (var r = 0; r < i_parent_walk.rufiles.length; r++)
					if (o_walk.rufiles.indexOf(i_parent_walk.rufiles[r]) == -1)
						o_walk.rufiles.push(i_parent_walk.rufiles[r]);

			//			break;
		}
	}

	if ((i_walk.folders == null) || (i_walk.folders.length == 0))
		return;

	i_walk.folders.sort(c_CompareFiles);
	for (var f = 0; f < i_walk.folders.length; f++)
	{
		var fobj = i_walk.folders[f];
		var path = i_path;
		if (path)
			path += '/' + fobj.name;
		else
			path = fobj.name;

		shot_SourceWalkFind(fobj, o_walk, path, i_walk);
	}
}

function shot_Create()
{
	let args = {};
	args.title = 'Create New Shot';
	args.template = RULES.assets.shot.template;
	args.destination = c_PathDir(g_CurPath());

	// Try to increment latest number in name by 10:
	// Find all numbers
	let numbers = ASSET.name.match(/\d+/g);
	if ((numbers != null) && numbers.length)
	{
		let number = numbers[numbers.length - 1];
		let numplus = '' + (parseInt(number) + 10);
		// Apply padding
		while (numplus.length < number.length)
			numplus = '0' + numplus;
		// Lenght may be bigger on 'SHOT_990'
		if (numplus.length == number.length)
			args.name = ASSET.name.replace(number, numplus);
	}

	a_Copy(args);
}

shot_rename_params = {};
shot_rename_params.new_name = {};
function shot_Rename()
{
	//	var args = {};
	var wnd = new cgru_Window({'name': 'rename', 'title': 'Rename Shot'});
	//	wnd.m_args = args;

	var params = {};
	params.new_name = c_PathBase(g_CurPath()) + '-01';

	gui_Create(wnd.elContent, shot_rename_params, [params]);

	var elBtns = document.createElement('div');
	wnd.elContent.appendChild(elBtns);
	elBtns.style.clear = 'both';
	elBtns.classList.add('buttons');

	var elCreate = document.createElement('div');
	elBtns.appendChild(elCreate);
	elCreate.textContent = 'Rename';
	elCreate.classList.add('button');
	elCreate.m_wnd = wnd;
	elCreate.onclick =
		function(e) {
		shot_RenameSend(e.currentTarget.m_wnd);
	}

	var elResults = document.createElement('div');
	wnd.elContent.appendChild(elResults);
	wnd.m_elResults = elResults;
	elResults.classList.add('output');
}
function shot_RenameSend(i_wnd)
{
	var params = gui_GetParams(i_wnd.elContent, shot_rename_params);
	// console.log(JSON.stringify(params));

	var elWait = document.createElement('div');
	i_wnd.elContent.appendChild(elWait);
	i_wnd.m_elWait = elWait;
	elWait.classList.add('wait');

	var old_name = g_CurPath();
	var new_name = c_PathDir(old_name) + '/' + params.new_name;

	var cmd = 'rules/bin/move.py';
	cmd += ' "' + c_PathPM_Rules2Server(old_name) + '"';
	cmd += ' "' + c_PathPM_Rules2Server(new_name) + '"';

	i_wnd.m_go_path = new_name;

	n_Request({'send': {'cmdexec': {'cmds': [cmd]}}, 'func': shot_RenameReceived, 'wnd': i_wnd});

	// Clear walk cache, as we need to navigate there later:
	n_walks[old_name] = null;
	n_walks[new_name] = null;
}
function shot_RenameReceived(i_data, i_args)
{
	// console.log(JSON.stringify(i_data));
	i_args.wnd.elContent.removeChild(i_args.wnd.m_elWait);
	var elResults = i_args.wnd.m_elResults;
	elResults.textContent = '';

	if ((i_data.cmdexec == null) || (!i_data.cmdexec.length))
	{
		elResults.textContent = (JSON.stringify(i_data));
		return;
	}

	i_args.wnd.destroy();

	g_RemoveFolder(g_elCurFolder);

	g_GO(i_args.wnd.m_go_path);
}

if (ASSETS.shot && (ASSETS.shot.path == g_CurPath()))
{
	shot_Init();
}
