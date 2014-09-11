fv_views = [];

fv_thumbnails_tomake = 0;
fv_thumbnails_tomake_files = [];

fv_cur_item = null;

if( localStorage.filesview == null ) localStorage.filesview = '0';

function fv_Finish()
{
	fv_views = [];
	fv_cur_item = null;
}

function FilesView( i_args)
{
	this.elParent = i_args.el;
	this.path = i_args.path;
	this.walk = i_args.walk;

	this.can_refresh = ! ( i_args.can_refresh === false );
	this.can_count   =     i_args.can_count;
	this.show_walk   = ! ( i_args.show_walk === false );

	this.has_limits = true;
	if( i_args.limits === false ) this.has_limits = false;

	this.has_thumbs = true;
	if( i_args.thumbs === false ) this.has_thumbs = false;

	this.elRoot = document.createElement('div');
	this.elParent.appendChild( this.elRoot);
	this.elRoot.classList.add('files_view');

	this.elPanel = document.createElement('div');
	this.elRoot.appendChild( this.elPanel);
	this.elPanel.classList.add('panel');

	if( this.has_limits )
		this.limitsAdd();

	var elTitle = document.createElement('div');
	this.elPanel.appendChild( elTitle);
	elTitle.classList.add('title');
	var title = '';
	if( ASSET && ASSET.name )
		title = ASSET.name;
	elTitle.textContent = title;

	el = document.createElement('div');
	this.elPanel.appendChild( el);
	el.classList.add('infotooltip');
	el.style.cssFloat = 'right';
//	el.style.cssFloat = 'left';
	el.title = "This is files view.\n\
You can create folder,\n\
arhive and unpack,\n\
convert images/movies,\n\
put in other location (may be FTP),\n\
genetate thumbnails.";

	c_CreateOpenButton( this.elPanel, this.path);

	var elPath = document.createElement('a');
	this.elPanel.appendChild( elPath);
	elPath.href = '#' + this.path;
	var path = this.path;
	if( ASSET && ASSET.path )
	{
		path = path.replace( ASSET.path, '');
		if( path[0] == '/' ) path = path.substr( 1);
	}
	elPath.classList.add('path');
	elPath.textContent = path;

	if( this.can_refresh )
	{
		var el = document.createElement('div');
		this.elPanel.appendChild( el);
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/refresh.png)';
		el.title = 'Refresh this files view';
		el.m_view = this;
		el.onclick = function(e){ e.currentTarget.m_view.refresh()};

		var el = document.createElement('div');
		this.elPanel.appendChild( el);
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/folder_new.png)';
		el.title = 'Add a new folder';
		el.m_view = this;
		el.onclick = function(e){ e.currentTarget.m_view.makeFolder()};
	}

	var el = document.createElement('div');
	this.elPanel.appendChild( el);
	el.classList.add('button');
	el.style.backgroundImage = 'url(rules/icons/select_all.png)';
	el.title = 'Select all';
	el.m_view = this;
	el.onclick = function(e){ e.currentTarget.m_view.selectAll()};

	var el = document.createElement('div');
	this.elPanel.appendChild( el);
	el.classList.add('button');
	el.style.backgroundImage = 'url(rules/icons/convert.png)';
	el.m_view = this;
	el.onclick = function(e){ e.currentTarget.m_view.convert();}
	el.title = 'Convert selected sequences (folders) or movies (files)';

	var el = document.createElement('div');
	this.elPanel.appendChild( el);
	el.classList.add('button');
	el.style.backgroundImage = 'url(rules/icons/archive.png)';
	el.m_view = this;
	el.onclick = function(e){ e.currentTarget.m_view.archivate();}
	el.title = 'Archivate foles and folders';

	var el = document.createElement('div');
	this.elPanel.appendChild( el);
	el.classList.add('button');
	el.style.backgroundImage = 'url(rules/icons/put.png)';
	el.m_view = this;
	el.onclick = function(e){ e.currentTarget.m_view.put();}
	el.title = 'Put selected folders';

	if( this.show_walk )
	{
		var el = document.createElement('div');
		this.elPanel.appendChild( el);
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/walk.png)';
		el.m_path = this.path;
		el.onclick = function(e){ fu_Walk({"path":e.currentTarget.m_path});}
		el.title = 'Top secret feature.';
	}

	if( this.has_thumbs )
	{
		var elThumbDiv = document.createElement('div');
		this.elPanel.appendChild( elThumbDiv);
		elThumbDiv.classList.add('thumbsdiv');

		var el = document.createElement('div');
		elThumbDiv.appendChild( el);
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/increase.png)';
		el.m_view = this;
		el.onclick = function(e){ e.currentTarget.m_view.thumbsBigger()};
		el.title = 'Show thumbnails bigger';

		var el = document.createElement('div');
		elThumbDiv.appendChild( el);
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/decrease.png)';
		el.m_view = this;
		el.onclick = function(e){ e.currentTarget.m_view.thumbsSmaller()};
		el.title = 'Show thumbnails smaller';

		var el = document.createElement('div');
		elThumbDiv.appendChild( el);
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/crop.png)';
		el.m_view = this;
		el.onclick = function(e){ e.currentTarget.m_view.thumbsCrop()};
		el.title = 'Show thumbnails cropped';

		this.elThumbsBtn = document.createElement('div');
		elThumbDiv.appendChild( this.elThumbsBtn);
		this.elThumbsBtn.classList.add('button');
		this.elThumbsBtn.style.backgroundImage = 'url(rules/icons/thumbnails.png)';
		this.elThumbsBtn.m_view = this;
		this.elThumbsBtn.onclick = function(e){ e.currentTarget.m_view.thumbsMake()};
		this.elThumbsBtn.title = 'Generate thumbnails';
	}

	if( RULES.checksum )
	for( var sum in RULES.checksum )
	{
		if( RULES.checksum[sum].enabled != true ) continue;

		this.elGenBtn = document.createElement('div');
		this.elPanel.appendChild( this.elGenBtn);
		this.elGenBtn.classList.add('button');
		this.elGenBtn.textContent = RULES.checksum[sum].name;
		this.elGenBtn.m_view = this;
		this.elGenBtn.onclick = function(e){
			fu_Checksum({"path":e.currentTarget.m_view.path,"walk":e.currentTarget.m_view.walk,"type":sum})
		};
		this.elGenBtn.title = 'Generate ' + RULES.checksum[sum].name;
	}

	this.elCounts = document.createElement('div');
	this.elPanel.appendChild( this.elCounts);
	this.elCounts.classList.add('counts');

	this.elView = document.createElement('div');
	this.elRoot.appendChild( this.elView);
	this.elView.classList.add('view');

	fv_views.push( this);

	this.show();
}

FilesView.prototype.destroy = function()
{
	this.elParent.removeChild( this.elRoot);
}

FilesView.prototype.limitsAdd = function()
{
	var limits = [3,10,30,0];

	this.elLimits = [];

	for( var i = 0; i < limits.length; i++)
	{
		var elLimit = document.createElement('div');
		this.elPanel.appendChild( elLimit);
		this.elLimits.push( elLimit);
		elLimit.classList.add('limit');

		var text = limits[i];
		if( text == 0 )
		{
			text = 'all';
			elLimit.title = 'Show all items';
		}
		else
			elLimit.title = 'Show last '+limits[i]+' items';
		elLimit.textContent = text;

		elLimit.m_limit = limits[i];
		elLimit.m_view = this;
		elLimit.onclick = function(e){
			localStorage.filesview = ''+e.currentTarget.m_limit;
			e.currentTarget.m_view.limitApply();
		}
	}
}

FilesView.prototype.limitApply = function()
{
	if( false == this.has_limits )
		return;

	var limit = 0;
	for( var j = 0; j < this.elLimits.length; j++)
	{
		var el = this.elLimits[j];
		if( parseInt( localStorage.filesview ) == el.m_limit )
		{
			limit = el.m_limit;
			el.classList.remove('button');
			if( limit ) el.classList.add('selected');
		}
		else
		{
			el.classList.add('button');
			el.classList.remove('selected');
		}
	}

	for( var f = 0; f < this.elItems.length; f++)
		if( limit && ( f < ( this.elItems.length-limit )))
			this.elItems[f].style.display = 'none';
		else
			this.elItems[f].style.display = 'block';
}

FilesView.prototype.refresh = function()
{
	n_WalkDir({"paths":[this.path],"wfunc":this.walkReceived,"this":this});
	c_LoadingElSet( this.elRoot);
}
FilesView.prototype.walkReceived = function( i_data, i_args)
{
	fv_cur_item = null;
	// Store selected items paths:
	var sel_paths = [];
	for( var i = 0; i < i_args.this.elItems.length; i++)
		if( i_args.this.elItems[i].classList.contains('selected'))
			sel_paths.push( i_args.this.elItems[i].m_path);

	i_args.this.walk = i_data[0];
	i_args.this.show();

	// Select items back:
	for( var i = 0; i < i_args.this.elItems.length; i++)
		if( sel_paths.indexOf( i_args.this.elItems[i].m_path) != -1 )
			i_args.this.selectItem( i_args.this.elItems[i]);
}

FilesView.prototype.show = function()
{
	c_LoadingElReset( this.elRoot);
	this.elView.textContent = '';
	this.elCounts.textContent = '';
	this.elItems = [];
	this.elThumbnails = [];

	if( this.walk == null )
		return;

	if( this.walk.folders)
	{
		this.walk.folders.sort( c_CompareFiles );
		for( var i = 0; i < this.walk.folders.length; i++)
			if( false == fv_SkipFile( this.walk.folders[i].name))
				this.showFolder( this.walk.folders[i]);
	}

	if( this.walk.files)
	{
		this.walk.files.sort( c_CompareFiles );
		for( var i = 0; i < this.walk.files.length; i++)
			if( false == fv_SkipFile( this.walk.files[i].name))
				this.showFile( this.walk.files[i]);
	}

	this.limitApply();
	this.showCounts();
}

FilesView.prototype.showCounts = function()
{
	var folders_count = 0;
	var files_count = 0;
	var frames_count = 0;
	var size_count = 0;

	if( this.walk.folders)
		for( var i = 0; i < this.walk.folders.length; i++)
			if( false == fv_SkipFile( this.walk.folders[i].name))
			{
				folders_count++;

				if( this.walk.folders[i].size_total )
					size_count += this.walk.folders[i].size_total;

				if( this.walk.folders[i].num_files )
					frames_count += this.walk.folders[i].num_files;
			}

	if( this.walk.files)
		for( var i = 0; i < this.walk.files.length; i++)
			if( false == fv_SkipFile( this.walk.files[i].name))
			{
				if( this.walk.files[i].size )
					size_count += this.walk.files[i].size;

				files_count++;
			}

	var counts = '';
	if( folders_count ) counts += ' Dirs:' + folders_count;
	if( files_count ) counts += ' Files:' + files_count;
	if( size_count ) counts += ' Size:' + c_Bytes2KMG( size_count);

	this.elCounts.textContent = counts;

	if( frames_count )
	{
		var el = document.createElement('div');
		this.elCounts.appendChild( el);
		el.classList.add('frames_count');
		el.textContent = 'F:' + frames_count;
		el.title = 'All folders files sum.\nDouble click to update status frames number.';
		el.m_frames_count = frames_count;
		el.onclick = function(e){e.stopPropagation();};
		el.ondblclick = function(e){e.stopPropagation();st_SetFramesNumber( e.currentTarget.m_frames_count);};
	}
}
FilesView.prototype.createItem = function( i_path, i_obj)
{
	var el = document.createElement('div');
	el.classList.add('item');
	this.elView.appendChild( el);
	this.elItems.push( el);
	el.m_path = i_path;
	el.id = i_path;
	el.m_view = this;
	el.onclick = function(e) { e.currentTarget.m_view.onClick( e);};

	var elAnchor = document.createElement('a');
	el.appendChild( elAnchor);
	elAnchor.classList.add('anchor');
	elAnchor.textContent = '@';
	elAnchor.href = g_GetLocationArgs({"fv_Goto":i_path});

	return el;
}

FilesView.prototype.showGenericButtons = function( i_el, i_obj)
{
	if( this.can_refresh )
	{
		var el = document.createElement('div');
		i_el.appendChild( el);
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/rename.png)';
		el.m_view = this;
		el.m_path = i_el.m_path;
		el.onclick = function(e){ e.stopPropagation(); e.currentTarget.m_view.rename( i_el.m_path)};
	}

	if( RULES.files_detele )
	{
		var el = document.createElement('div');
		i_el.appendChild( el);
		el.classList.add('button');
		el.textContent = 'DEL';
		el.m_view = this;
		el.ondblclick = function(e){ e.stopPropagation(); e.currentTarget.m_view.deleteFilesDialog( i_el.m_path)};
	}
}
FilesView.prototype.showAttrs = function( i_el, i_obj)
{
	if( i_obj.mtime != null )
	{
		if( i_el.m_el_mtime == null )
		{
			i_el.m_el_mtime = document.createElement('div');
			i_el.appendChild( i_el.m_el_mtime);
			i_el.m_el_mtime.classList.add('mtime');
		}

		i_el.m_el_mtime.textContent = c_DT_FormStrFromSec( i_obj.mtime);
	}

	var size = i_obj.size_total;
	if( size == null ) size = i_obj.size;
	if( size != null )
	{
		if( i_el.m_el_size == null )
		{
			i_el.m_el_size = document.createElement('div');
			i_el.appendChild( i_el.m_el_size);
			i_el.m_el_size.classList.add('size');
		}

		i_el.m_el_size.textContent = c_Bytes2KMG( size);
		if(( i_obj.size_total != null ) && ( i_obj.size != null ))
			i_el.m_el_size.title = 'Files size without subfolders: ' + c_Bytes2KMG( i_obj.size);
	}

	var num_files = null;
	if( i_obj.num_files != null )
		num_files = i_obj.num_files;
	if( i_obj.files && i_obj.files.length )
		num_files = i_obj.files.length;
	if( num_files != null )
	{
		if( i_el.m_el_num_files == null )
		{
			i_el.m_el_num_files = document.createElement('div');
			i_el.appendChild( i_el.m_el_num_files);
			i_el.m_el_num_files.classList.add('filesnum');
		}

		i_el.m_el_num_files.textContent = 'F:' + num_files;

		var title = 'Files quantity: ' + num_files + ' (without subfolders)';
		title += '\nDouble click to update status frames number.';
		if(( i_obj.num_folders_total != null ) && ( i_obj.num_files_total != null ))
		{
			title += '\nTotal count with subfolders:';
			title += '\nFolders: ' + i_obj.num_folders_total;
			title += '\nFiles: ' + i_obj.num_files_total;
		}
		i_el.m_el_num_files.title = title;

		i_el.m_el_num_files.m_num_files = num_files;
		i_el.m_el_num_files.onclick = function(e){e.stopPropagation();};
		i_el.m_el_num_files.ondblclick = function(e){e.stopPropagation();st_SetFramesNumber( e.currentTarget.m_num_files);};
	}
/*
	if( i_obj.checksum )
	{
		var elSums = [];
		var time = null;
		for( var sum in i_obj.checksum )
		{
			if( sum == 'time' )
			{
				time = i_obj.checksum[sum];
				continue;
			}

			var elSum = document.createElement('div');
			i_el.appendChild( elSum);
			elSum.classList.add('checksum');
			elSum.textContent = sum+':'+i_obj.checksum[sum];
			elSums.push( elSum);
		}

		if( time && elSums.length)
		for( var i = 0; i < elSums.length; i++)
			elSums[i].title = c_DT_FormStrFromSec( time);
	}
*/
}

FilesView.prototype.showFolder = function( i_folder)
{
	var name = i_folder.name;
	var path = ( this.path + '/' + name).replace( /\/\//g, '/');

	var elFolder = this.createItem( path, i_folder);
	elFolder.classList.add('folder');

	if( this.has_thumbs )
		this.makeThumbEl( elFolder, path, 'folder');

	var elOpen = c_CreateOpenButton( elFolder, path);
	if( elOpen ) elOpen.style.cssFloat = 'left';

	var el = document.createElement('a');
	elFolder.appendChild( el);
	el.setAttribute('href', '#'+path);
	el.textContent = name;

	if( this.can_count )
	{
		var el = document.createElement('a');
		elFolder.appendChild( el);
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/count.png)';
		el.title = "Count files.";
		el.style.cssFloat = 'right';
		el.m_view = this;
		el.m_path = path;
		el.onclick = function(e){
			e.stopPropagation();
			e.currentTarget.m_view.countFiles( e.currentTarget.m_path);
		};
	}

	if( ASSET && (( ASSET.path != g_CurPath()) || ( ASSET.play_folders !== false )))
	{
		var play_path = path;
		if( ASSET.path ) play_path = play_path.replace(ASSET.path, ASSET.path + '/');
		var el = document.createElement('a');
		elFolder.appendChild( el);
		el.classList.add('button');
		el.setAttribute('href', 'player.html#'+play_path);
		el.setAttribute('target', '_blank');
		el.style.backgroundImage = 'url(rules/icons/player.png)';
		el.title = "Open RULES player in a new window.";
		el.style.cssFloat = 'right';
	}

	if( RULES.has_filesystem !== false )
	{
		var cmds = RULES.cmdexec.play_sequence;
		if( cmds ) for( var c = 0; c < cmds.length; c++)
		{
			var elCmd = document.createElement('div');
			elFolder.appendChild( elCmd);
			elCmd.classList.add('cmdexec');
			elCmd.textContent = cmds[c].name;
			var cmd = cmds[c].cmd;
			cmd = cmd.replace('@PATH@', cgru_PM('/'+RULES.root + path));
			cmd = cmd.replace('@FPS@', RULES.fps);
			elCmd.setAttribute('cmdexec', JSON.stringify([cmd]));
			if( cmds[c].tooltip ) elCmd.title = cmds[c].tooltip;
		}
	}

	if( RULES.afanasy_enabled !== false )
	if( ASSET && ASSET.subfolders_dailies_hide && ( ASSET.path == g_CurPath() ))
	{
		var out_path = c_PathDir( path);
		if( ASSET && ( ASSET.dailies ))
			out_path = ASSET.path+'/'+ASSET.dailies.path[0];

		var el = document.createElement('div');
		elFolder.appendChild( el);
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/dailies.png)';
		el.title = 'Make dailies';
		el.m_path = path;
		el.onclick = function(e){
			e.stopPropagation();
			d_Make( e.currentTarget.m_path, out_path)};
	}

	this.showGenericButtons( elFolder, i_folder);
	this.showAttrs( elFolder, i_folder);
}

FilesView.prototype.showFile = function( i_file)
{
	var path = this.path + '/' + i_file.name;

	var elFile = this.createItem( path, i_file);
	elFile.classList.add('file');

	if( this.has_thumbs )
		this.makeThumbEl( elFile, path, 'file');

	var el = document.createElement('a');
	elFile.appendChild( el);
	el.href = RULES.root + path;
	el.target = '_blank';
	el.textContent = i_file.name;

	if( c_FileCanEdit( i_file.name))
	{
		var el = document.createElement('a');
		elFile.appendChild( el);
		el.setAttribute('href', 'player.html#'+path);
		el.setAttribute('target', '_blank');
		el.classList.add('button');
		el.classList.add('edit');
		el.style.cssFloat = 'right';
	}

	if( c_FileIsMovie( i_file.name))
	{
		var cmds = RULES.cmdexec.play_movie;
		if( cmds && ( RULES.has_filesystem !== false )) for( var c = 0; c < cmds.length; c++)
		{
			var elCmd = document.createElement('div');
			elFile.appendChild( elCmd);
			elCmd.classList.add('cmdexec');
			elCmd.textContent = cmds[c].name;
			var cmd = cmds[c].cmd;
			cmd = cmd.replace('@PATH@', cgru_PM('/'+RULES.root + path));
			cmd = cmd.replace('@FPS@', RULES.fps);
			elCmd.setAttribute('cmdexec', JSON.stringify([cmd]));
			if( cmds[c].tooltip ) elCmd.title = cmds[c].tooltip;
		}
	}

	this.showGenericButtons( elFile, i_file);
	this.showAttrs( elFile, i_file);

	if( c_FileIsMovieHTML( i_file.name))
		elFile.m_preview_file = elFile.m_path;

	if( this.walk.rules && ( this.walk.rufiles.indexOf( i_file.name + '.ogg') != -1 ))
		elFile.m_preview_file = c_PathDir( elFile.m_path) + '/' + RULES.rufolder + '/' + i_file.name + '.ogg';

	if( elFile.m_preview_file )
	{
		var el = document.createElement('div');
		elFile.m_elPreviewBtn = el;
		elFile.appendChild( el);
		el.classList.add('preview_btn');
		el.classList.add('button');
		el.style.backgroundImage = 'url(rules/icons/play.png)';
		el.title = "Preview";
		el.m_el_file = elFile;
		el.onclick = function(e){ e.stopPropagation(); fv_PreviewOpen(e.currentTarget.m_el_file);};
	}
}

FilesView.prototype.onClick = function( i_evt)
{
	i_evt.stopPropagation();
	var el = i_evt.currentTarget;
	this.selectItem( el, el.m_selected !== true );
	if( i_evt.shiftKey && fv_cur_item )
	{
		var i_s = this.elItems.indexOf( el);
		var i_c = this.elItems.indexOf( fv_cur_item );
		if( i_s != i_c )
		{
			var select = false;
			if( el.classList.contains('selected')) select = true;
			var step = 1;
			if( i_s < i_c ) step = -1;
			while( i_c != i_s )
			{
				this.selectItem( this.elItems[i_c], select);
				i_c += step;
			}
		}
	}

	fv_cur_item = el;

	var sel_count = 0;
	for( var i = 0; i < this.elItems.length; i++)
		if( this.elItems[i].m_selected )
			sel_count++;
	c_Info(sel_count + ' items selected.');
}
FilesView.prototype.selectItem = function( i_el, i_select)
{
	if( i_select === false )
	{
		i_el.m_selected = false;
		i_el.classList.remove('selected');
	}
	else
	{
		i_el.m_selected = true;
		i_el.classList.add('selected');
	}
}
FilesView.prototype.selectAll = function( i_select)
{
	if( i_select == null )
	{
		if( this.getSelected().length == this.elItems.length )
			i_select = false;
		else
			i_select = true;
	}

	for( var i = 0; i < this.elItems.length; i++)
		this.selectItem( this.elItems[i], i_select);

	if( i_select == false )
	{
		fv_cur_item = null;
		c_Info('All items deselected.');
	}
	else
		c_Info('All items selected.');
}
FilesView.prototype.selectNone = function() { this.selectAll( false); }

FilesView.prototype.getSelected = function()
{
	var o_items = []
	for( var i = 0; i < this.elItems.length; i++)
		if( this.elItems[i].m_selected )
			o_items.push( this.elItems[i]);
	return o_items;
}

FilesView.prototype.countFiles = function( i_path, i_args)
{
	c_LoadingElSet( this.elRoot);
	var cmd = 'rules/bin/walk.py "' + RULES.root + i_path + '"';
	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":this.countFilesFinished,"this":this,"wpath":i_path,"post_args":i_args});
}
FilesView.prototype.countFilesFinished = function( i_data, i_args) { i_args.this.countFilesUpdate(i_data, i_args);}
FilesView.prototype.countFilesUpdate = function( i_data, i_args)
{
	if( i_data.error)
		c_Error( i_data.error);

	if(( i_data.cmdexec == null ) || ( i_data.cmdexec[0].walk == null ))
	{
		c_Error('Invalid walk output received.');
		return;
	}

	var data = i_data.cmdexec[0].walk;

	for( key in data )
		if( key != 'walk')
			if( key.indexOf('error') != -1 )
				c_Error('Walk[' + key + ']: ' + data[key])

	if( data.error )
	{
		c_Error( error);
		return;
	}

	if( data.walk == null )
	{
		c_Error('Walk result does not contain walk object.');
		return;
	}

	// Update folder item attrs:
	for( var i = 0; i < this.elItems.length; i++)
	{
		if( this.elItems[i].m_path != i_args.wpath )
			continue;

		this.showAttrs( this.elItems[i], data.walk);

		break;
	}

	// Update this class instance walk object:
	var name = c_PathBase( i_args.wpath);
	for( var i = 0; i < this.walk.folders.length; i++)
	{
		if( this.walk.folders[i].name != name )
			continue

		for( var key in data.walk )
			this.walk.folders[i][key] = data.walk[key]

		break;
	}

	this.showCounts();

	c_LoadingElReset( this.elRoot);

	if( i_args.post_args && i_args.post_args.func )
		i_args.post_args.func( i_args.post_args, data.walk);	
}
FilesView.prototype.put = function()
{
	var args = {};
	args.paths = [];
	for( var i = 0; i < this.elItems.length; i++)
		if( this.elItems[i].m_selected )
			args.paths.push( this.elItems[i].m_path);
	if( args.paths.length < 1 )
		c_Error('No items selected.');
	else
		fu_Put( args);
}
FilesView.prototype.convert = function()
{
	var args = {};
	args.paths = [];
	args.filesview = this;

	args.folders = true;
	args.images = true;
	args.movies = true;

	for( var i = 0; i < this.elItems.length; i++)
	{
		if( this.elItems[i].m_selected != true ) continue;

		if( this.elItems[i].classList.contains('folder') && args.folders )
		{
			args.images = false;
			args.movies = false;
		}
		else if( c_FileIsImage( this.elItems[i].m_path) && args.images )
		{
			args.folders = false;
			args.movies = false;
		}
		else if( c_FileIsMovie( this.elItems[i].m_path) && args.movies )
		{
			args.folders = false;
			args.images = false;
		}
		else
			continue;

		args.paths.push( this.elItems[i].m_path);
	}
	if( args.paths.length < 1 )
		c_Error('No items selected.');
	else
		d_Convert( args);
}

FilesView.prototype.archivate = function()
{
	var args = {};
	args.paths = [];
	args.archive = true;

	var items = this.getSelected();
	for( var i = 0; i < items.length; i++)
	{
		if( args.paths.length == 0 )
		{
			if( items[i].classList.contains('file') && c_FileIsArchive( items[i].m_path))
			{
				args.archive = false;
				args.extract = true;
			}
			else
				args.extract = false;
		}
		else
		{
			if( items[i].classList.contains('file') && c_FileIsArchive( items[i].m_path))
			{
				if( args.archive )
					continue;
			}
			else
			{
				if( args.extract )
					continue;
			}
		}

		args.paths.push( items[i].m_path);
	}

	if( args.paths.length < 1 )
		c_Error('No items selected.');
	else
		fu_Archivate( args);
}

FilesView.prototype.getItemPath = function( i_path)
{
	for( var i = 0; i < this.elItems.length; i++)
		if( this.elItems[i].m_path == i_path )
			return this.elItems[i];
	return null;
}

FilesView.prototype.makeThumbEl = function( i_el, i_path, i_type)
{
	var elThumbnal = document.createElement('span');
	i_el.appendChild( elThumbnal);
	this.elThumbnails.push( elThumbnal);
	elThumbnal.classList.add('thumbnail');
	elThumbnal.m_type = i_type;

	elThumbnal.m_path = i_path;
	var thumbFile = RULES.root + c_GetThumbFileName( i_path);
	var thumbName = c_PathBase( thumbFile);
	elThumbnal.m_thumbFile = thumbFile;

	var elImg = document.createElement('img');
	elThumbnal.appendChild( elImg);
	elThumbnal.m_elImg = elImg;
	if( this.walk.rufiles && ( this.walk.rufiles.indexOf( thumbName) != -1))
		elImg.src = thumbFile;
	else
		elThumbnal.style.display = 'none';

	fv_FileThumbResize( elImg);
	elImg.onload = fv_FileThumbOnLoad;
}

FilesView.prototype.thumbsBigger = function( i_bigger)
{
	var s = parseInt( localStorage.thumb_file_size );
	var ns = s;
	if( i_bigger === false ) ns -= 10;
	else ns += 10;

	if( ns < 10 ) return;
	if( ns > 160 ) return;

	localStorage.thumb_file_size = ''+ns;
	this.thumbsResize();
}
FilesView.prototype.thumbsSmaller = function() { this.thumbsBigger( false); }
FilesView.prototype.thumbsCrop = function()
{
	if( localStorage.thumb_file_crop === 'true' )
		localStorage.thumb_file_crop = 'false';
	else
		localStorage.thumb_file_crop = 'true';
	this.thumbsResize();
}
FilesView.prototype.thumbsResize = function()
{
	for( var i = 0; i < this.elThumbnails.length; i++)
		fv_FileThumbResize( this.elThumbnails[i].m_elImg);
}

FilesView.prototype.thumbsMake = function()
{
	if( fv_thumbnails_tomake > 0 ) return;

	fv_thumbnails_tomake = this.elThumbnails.length;
	if( fv_thumbnails_tomake == 0 ) return;

	for( var i = 0; i < fv_views.length; i++)
		fv_views[i].elThumbsBtn.classList.remove('button');

	fv_thumbnails_tomake_files = [];
	for( var i = 0; i < this.elThumbnails.length; i++)
		fv_thumbnails_tomake_files.push( this.elThumbnails[i].m_path);

	fv_MakeThumbnail();
}
FilesView.prototype.makeFolder = function()
{
	new cgru_Dialog({"receiver":this,"handle":'makeFolderDo',
		"name":'make_folder',"title":'Make Folder',"info":'Ender new folder name:'});
}
FilesView.prototype.makeFolderDo = function( i_name)
{
	var path = cgru_PM( RULES.root + this.path + '/' + i_name, true);
	n_Request({"send":{"makefolder":{"path":path}},"func":fv_makeFolderFinished,"fview":this});
}
function fv_makeFolderFinished( i_data, i_args)
{
	if(( i_data == null ) || ( i_data.error))
	{
		c_Error( i_data.error);
		return;
	}
	i_args.fview.refresh();
	c_Info('Folder created: ' + i_args.fview.path + '/' + i_data.makefolder);
}

FilesView.prototype.rename = function( i_path)
{
	new cgru_Dialog({"receiver":this,"handle":'renameDo',"param":i_path,"value":c_PathBase(i_path),
		"name":'rename',"title":'Rename',"info":'Ender a new name.'});
}
FilesView.prototype.renameDo = function( i_value, i_path)
{
	new_path = RULES.root + c_PathDir( i_path) + '/' + i_value;
	cmd = 'rules/bin/rename.py "' + RULES.root + i_path + '" "' + new_path + '"';

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":this.renameFinished,"this":this,"old_path":i_path,"new_path":new_path,"info":'rename'});
}
FilesView.prototype.renameFinished = function( i_data, i_args)
{
	if( c_NullOrErrorMsg( i_data)) return;

	c_Info('Renamed: ' + c_PathBase(i_args.old_path) + ' -> ' + c_PathBase(i_args.new_path));

	i_args.this.refresh();
}

FilesView.prototype.deleteFilesDialog = function( i_path)
{
	new cgru_Dialog({"receiver":this,"handle":'deleteFiles',"param":i_path,
		"name":'delete',"title":'Delete',"info":'<span style="font-size:20px;font-weight:bold;">'+i_path+'</span><br>Are You Sure?<br>Type "yes".'});
}
FilesView.prototype.deleteFiles = function( i_value, i_path)
{
	if( i_value != 'yes' ) return;

	cmd = 'rules/bin/rm -rf "' + RULES.root + i_path + '"';
//console.log(cmd);

	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":this.filesDeleted,"this":this,"delpath":i_path,"info":'delete',"wait":false,"parse":true});
}
FilesView.prototype.filesDeleted = function( i_data, i_args)
{
//console.log( JSON.stringify( i_args.delpath));
	if( i_data.error )
	{
		c_Error( i_data.error);
		return;
	}

	c_Info('Deleted ' + i_args.delpath);

	i_args.this.refresh();
}

function fv_PreviewOpen( i_el)
{
	if( i_el.m_preview ) return;
	i_el.m_preview = true;

	i_el.m_elPreviewBtn.style.display = 'none';
	
	var elPreview = document.createElement('div');
	i_el.m_elPreview = elPreview;
	i_el.appendChild( elPreview);
	elPreview.classList.add('preview');
	elPreview.onclick = function(e){e.stopPropagation();};

	var el = document.createElement('div')
	elPreview.appendChild( el);
	el.classList.add('close');
	el.classList.add('button');
//	el.textContent = 'X';
	el.style.backgroundImage = 'url(rules/icons/delete.png)';
//	el.style.width = '16px';
//	el.style.height = '16px';
	el.m_el_file = i_el;
	el.onclick = function(e){e.stopPropagation(); fv_PreviewClose( e.currentTarget.m_el_file);};

	var elVideo = document.createElement('video');
	elPreview.appendChild( elVideo);
	elVideo.controls = true;
	elVideo.classList.add('video');

	var elSource = document.createElement('source');
	elVideo.appendChild( elSource);
	elSource.src = RULES.root + i_el.m_preview_file;
	elSource.type = 'video/ogg';
}
function fv_PreviewClose( i_el)
{
	i_el.m_preview = false;
	i_el.removeChild( i_el.m_elPreview);
	i_el.m_elPreviewBtn.style.display = 'block';
}

function fv_FileThumbOnLoad() { fv_FileThumbResize( this);}
function fv_FileThumbResize( i_img)
{
	var iw = i_img.naturalWidth;
	var ih = i_img.naturalHeight;

	var loaded = (( ih > 0 ) && ( iw > 0 ));
	var crop = ( localStorage.thumb_file_crop === 'true' );
	var w = parseInt( localStorage.thumb_file_size);
	var h = Math.round( w * 9 / 16);

	if( c_FileIsMovie( i_img.parentNode.m_path ) || ( i_img.parentNode.m_type == 'folder')) w *= 3;

	if( false == crop )
	{
		i_img.height = h;
		i_img.width = iw * h / ih;
		i_img.style.marginTop = '0';
		i_img.style.marginLeft = '0';
		i_img.parentNode.style.width = 'auto';
		i_img.parentNode.style.height = 'auto';
		return;
	}

	if( false == loaded ) return;

	if(( iw / ih) < (w / h))
	{
		ih = ih*w/iw;
		iw = w;
	}
	else
	{
		iw = iw*h/ih;
		ih = h;
	}

	i_img.width = Math.round(iw);
	i_img.height = Math.round(ih);

	var mw = Math.round((w-iw)/2);
	var mh = Math.round((h-ih)/2);

	i_img.style.marginTop = mh+'px';
	i_img.style.marginLeft = mw+'px';

	i_img.parentNode.style.width = w+'px';
	i_img.parentNode.style.height = h+'px';
}

function fv_UpdateThumbnail( i_data, i_args)
{
	for( var v = 0; v < fv_views.length; v++)
	for( var i = 0; i < fv_views[v].elThumbnails.length; i++)
	{
		if( fv_views[v].elThumbnails[i].m_path == i_args.file )
		{
			fv_views[v].elThumbnails[i].m_elImg.src = fv_views[v].elThumbnails[i].m_thumbFile;
			fv_views[v].elThumbnails[i].style.display = 'block';
			break;
		}
	}
	fv_MakeThumbnail()
}

function fv_MakeThumbnail()
{
	if( fv_thumbnails_tomake == 0 )
	{
		fv_MakeThumbnailsFinish();
		return;
	}
	fv_thumbnails_tomake--;
	c_MakeThumbnail( fv_thumbnails_tomake_files.shift(), fv_UpdateThumbnail);
}

function fv_MakeThumbnailsFinish()
{
	for( var i = 0; i < fv_views.length; i++)
		if( fv_views[i].has_thumbs )
			fv_views[i].elThumbsBtn.classList.add('button');
	fv_thumbnails_tomake = 0;
}

function fv_SkipFile( i_filename)
{
	if( i_filename.indexOf('/') != -1 )
		i_filename = i_filename.substr( i_filename.lastIndexOf('/')+1);
	for( var i = 0; i < RULES.skipfiles.length; i++ )
		if( i_filename.indexOf( RULES.skipfiles[i]) == 0 )
			return true;
	return false;
}

function fv_ReloadAll()
{
	for( var i = 0; i < fv_views.length; i++)
		fv_views[i].refresh();
}

function fv_SelectNone()
{
	for( var v = 0; v < fv_views.length; v++)
		fv_views[v].selectNone();
}

function fv_Goto( i_path )
{
	fv_SelectNone();
	for( var v = 0; v < fv_views.length; v++)
	{
		var el = fv_views[v].getItemPath( i_path )
		if( el )
		{
			fv_views[v].selectItem( el);
			el.scrollIntoView();
			c_Info(c_PathBase(i_path) + ' selected.');
			return;
		}
	}
	c_Error('Item not found: ' + i_path);
}

