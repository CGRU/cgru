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

function FilesView( i_elParent, i_path, i_walk, i_show_limits, i_show_thumbs)
{
	this.elParent = i_elParent;
	this.path = i_path;
	this.walk = i_walk;

	this.has_limits = true;
	if( i_show_limits === false ) this.has_limits = false;
	this.has_thumbs = true;
	if( i_show_thumbs === false ) this.has_thumbs = false;

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

	c_CreateOpenButton( this.elPanel, i_path);

	var elPath = document.createElement('a');
	this.elPanel.appendChild( elPath);
	elPath.href = '#' + i_path;
	var path = i_path;
	if( ASSET && ASSET.path )
	{
		path = path.replace( ASSET.path, '');
		if( path[0] == '/' ) path = path.substr( 1);
	}
	elPath.classList.add('path');
	elPath.textContent = path;

	if( this.has_thumbs )
	{
		var elThumbDiv = document.createElement('div');
		this.elPanel.appendChild( elThumbDiv);
		elThumbDiv.classList.add('thumbsdiv');

		var elThumbBigger = document.createElement('div');
		elThumbDiv.appendChild( elThumbBigger);
		elThumbBigger.classList.add('button');
		elThumbBigger.textContent = '+';
		elThumbBigger.m_view = this;
		elThumbBigger.onclick = function(e){ e.currentTarget.m_view.thumbsBigger()};
		elThumbBigger.title = 'Show thumbnails bigger';

		var elThumbSmaller = document.createElement('div');
		elThumbDiv.appendChild( elThumbSmaller);
		elThumbSmaller.classList.add('button');
		elThumbSmaller.textContent = '-';
		elThumbSmaller.m_view = this;
		elThumbSmaller.onclick = function(e){ e.currentTarget.m_view.thumbsSmaller()};
		elThumbSmaller.title = 'Show thumbnails smaller';

		var elThumbCrop = document.createElement('div');
		elThumbDiv.appendChild( elThumbCrop);
		elThumbCrop.classList.add('button');
		elThumbCrop.textContent = '[c]';
		elThumbCrop.m_view = this;
		elThumbCrop.onclick = function(e){ e.currentTarget.m_view.thumbsCrop()};
		elThumbCrop.title = 'Show thumbnails cropped';

		this.elThumbsBtn = document.createElement('div');
		elThumbDiv.appendChild( this.elThumbsBtn);
		this.elThumbsBtn.classList.add('button');
		this.elThumbsBtn.textContent = 'Thumbs:';
		this.elThumbsBtn.m_view = this;
		this.elThumbsBtn.onclick = function(e){ e.currentTarget.m_view.thumbsMake()};
		this.elThumbsBtn.title = 'Generate thumbnails';
	}

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
	this.walk = n_WalkDir( [this.path], 0, RULES.rufolder)[0];
	this.show();
}

FilesView.prototype.show = function()
{
	this.elView.innerHTML = '';
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
}

FilesView.prototype.createItem = function( i_path)
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

FilesView.prototype.showFolder = function( i_folder)
{
	var name = i_folder.name;
	var path = ( this.path + '/' + name).replace( /\/\//g, '/');

	var elFolder = this.createItem( path);
	elFolder.classList.add('folder');

	if( this.has_thumbs )
		this.makeThumbEl( elFolder, path, 'folder');

	var elOpen = c_CreateOpenButton( elFolder, path);
	if( elOpen ) elOpen.style.cssFloat = 'left';

	var elLinkA = document.createElement('a');
	elFolder.appendChild( elLinkA);
	elLinkA.setAttribute('href', '#'+path);
	elLinkA.textContent = name;

	if( ASSET && (( ASSET.path != g_CurPath()) || ( ASSET.play_folders !== false )))
	{
		var elLinkA = document.createElement('a');
		elFolder.appendChild( elLinkA);
		elLinkA.setAttribute('href', 'player.html#'+path);
		elLinkA.setAttribute('target', '_blank');
		elLinkA.textContent = 'play';
		elLinkA.style.cssFloat = 'right';
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
		}
	}

	if( ASSET && ( ASSET.dailies ) && ( RULES.afanasy_enabled !== false ))
	{
		var elMakeDailies = document.createElement('div');
		elFolder.appendChild( elMakeDailies);
		elMakeDailies.classList.add('button');
		elMakeDailies.textContent = 'Dailies';
		elMakeDailies.title = 'Make dailies';
		elMakeDailies.m_path = path;
		elMakeDailies.onclick = function(e){
			e.stopPropagation();
			d_Make( e.currentTarget.m_path, ASSET.path+'/'+ASSET.dailies.path[0])};
	}

	if( i_folder.mtime != null )
	{
		var elMTime = document.createElement('div');
		elFolder.appendChild( elMTime);
		elMTime.classList.add('mtime');
		elMTime.textContent = c_DT_FormStrFromSec( i_folder.mtime);
	}
}

FilesView.prototype.showFile = function( i_file)
{
	var path = this.path + '/' + i_file.name;

	var elFile = this.createItem( path);

	if( this.has_thumbs )
		this.makeThumbEl( elFile, path, 'file');

	var elLinkA = document.createElement('a');
	elFile.appendChild( elLinkA);
	elLinkA.href = RULES.root + path;
	elLinkA.target = '_blank';
	elLinkA.textContent = i_file.name;

	if( c_FileCanEdit( i_file.name))
	{
		var elLinkA = document.createElement('a');
		elFile.appendChild( elLinkA);
		elLinkA.setAttribute('href', 'player.html#'+path);
		elLinkA.setAttribute('target', '_blank');
		elLinkA.textContent = 'edit';
		elLinkA.style.cssFloat = 'right';
	}

	if( c_FileIsMovie( i_file.name))
	{
		var cmds = RULES.cmdexec.play_movie;
		if( cmds ) for( var c = 0; c < cmds.length; c++)
		{
			var elCmd = document.createElement('div');
			elFile.appendChild( elCmd);
			elCmd.classList.add('cmdexec');
			elCmd.textContent = cmds[c].name;
			var cmd = cmds[c].cmd;
			cmd = cmd.replace('@PATH@', cgru_PM('/'+RULES.root + path));
			cmd = cmd.replace('@FPS@', RULES.fps);
			elCmd.setAttribute('cmdexec', JSON.stringify([cmd]));
		}

		var elExplode = document.createElement('div');
		elFile.appendChild( elExplode);
		elExplode.classList.add('button');
		elExplode.textContent = 'Explode';
		elExplode.title = 'Explode movie to images sequence';
		elExplode.m_path = path;
		elExplode.onclick = function(e){ e.stopPropagation(); d_Explode( e.currentTarget.m_path)};
	}

	if( c_FileIsImage( i_file.name))
	{
		var elCvt = document.createElement('div');
		elFile.appendChild( elCvt);
		elCvt.classList.add('button');
		elCvt.textContent = 'JPG';
		elCvt.m_file = this.path + '/' + i_file.name;
		elCvt.onclick = function(e){ e.stopPropagation(); fv_ImgConvertDialog( e.currentTarget.m_file)};
	}

	if( i_file.size != null )
	{
		var elSize = document.createElement('div');
		elFile.appendChild( elSize);
		elSize.classList.add('size');
		elSize.textContent = c_Bytes2KMG( i_file.size);
	}

	if( i_file.mtime != null )
	{
		var elMTime = document.createElement('div');
		elFile.appendChild( elMTime);
		elMTime.classList.add('mtime');
		elMTime.textContent = c_DT_FormStrFromSec( i_file.mtime);
	}
}

FilesView.prototype.onClick = function( i_evt)
{
	i_evt.stopPropagation();
	this.selectItemToggle( i_evt.currentTarget);
}

FilesView.prototype.selectItem = function( i_el)
{
	this.selectNone();
	i_el.classList.add('selected');
	fv_cur_item = i_el;
}

FilesView.prototype.selectItemToggle = function( i_el)
{
	if( i_el.classList.contains('selected'))
		this.deselectItem( i_el);
	else
		this.selectItem( i_el);
}

FilesView.prototype.deselectItem = function( i_el)
{
	i_el.classList.remove('selected');
	if( fv_cur_item == i_el )
		fv_cur_item = null;
}

FilesView.prototype.selectNone = function()
{
	for( var i = 0; i < this.elItems.length; i++)
		this.elItems[i].classList.remove('selected');
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
//if( c_FileCanThumbnail( i_path) && (( i_walk == null ) || ( i_walk.rufiles && ( i_walk.rufiles.indexOf( thumbName) != -1))))
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

function fv_UpdateThumbnail( i_msg)
{
	for( var v = 0; v < fv_views.length; v++)
	for( var i = 0; i < fv_views[v].elThumbnails.length; i++)
	{
		if( fv_views[v].elThumbnails[i].m_thumbFile == i_msg.thumbnail )
		{
			fv_views[v].elThumbnails[i].m_elImg.src = i_msg.thumbnail;
			fv_views[v].elThumbnails[i].style.display = 'block';
			break;
		}
	}
}

function fv_MakeThumbnail()
{
	if( fv_thumbnails_tomake == 0 )
	{
		fv_MakeThumbnailsFinish();
		return;
	}
	fv_thumbnails_tomake--;
	c_MakeThumbnail( RULES.root + fv_thumbnails_tomake_files.shift());
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

function fv_ImgConvertDialog( i_file)
{
	var wnd = new cgru_Window('imgconvert','Image Convert');
	wnd.resize( 40, 50);
	wnd.elContent.classList.add('imgconvert');
	wnd.m_file = RULES.root + i_file;

	var elQualityDiv = document.createElement('div');
	wnd.elContent.appendChild( elQualityDiv);
	var elQualityLabel = document.createElement('div');
	elQualityLabel.textContent = 'Quality';
	elQualityDiv.appendChild( elQualityLabel);
	elQualityLabel.classList.add('label');
	var elQualityInfo = document.createElement('div');
	elQualityInfo.textContent = '100 is the best';
	elQualityDiv.appendChild( elQualityInfo);
	elQualityInfo.classList.add('info');

	wnd.elQuality = document.createElement('div');
	wnd.elQuality.textContent = '75';
	elQualityDiv.appendChild( wnd.elQuality);
	wnd.elQuality.classList.add('editing');
	wnd.elQuality.contentEditable = true;

	var elResolutionDiv = document.createElement('div');
	wnd.elContent.appendChild( elResolutionDiv);
	var elResolutionLabel = document.createElement('div');
	elResolutionLabel.textContent = 'Resolution';
	elResolutionDiv.appendChild( elResolutionLabel);
	elResolutionLabel.classList.add('label');
	var elResolutionInfo = document.createElement('div');
	elResolutionInfo.textContent = '-1 means original';
	elResolutionDiv.appendChild( elResolutionInfo);
	elResolutionInfo.classList.add('info');

	wnd.elResolution = document.createElement('div');
	wnd.elResolution.textContent = '1280';
	elResolutionDiv.appendChild( wnd.elResolution);
	wnd.elResolution.classList.add('editing');
	wnd.elResolution.contentEditable = true;

	var elButton = document.createElement('div');
	wnd.elContent.appendChild( elButton);
	elButton.classList.add('button');
	elButton.textContent = 'Convert';
	elButton.m_wnd = wnd;
	elButton.onclick = function(e){ fv_ImgConvert( e.currentTarget.m_wnd )};
}
function fv_ImgConvert( i_wnd)
{
	var quality = i_wnd.elQuality.textContent;
	var resolution = i_wnd.elResolution.textContent;

	var out = i_wnd.m_file;
	out += '.q' + quality;
	if( resolution != '-1' )
		out += '.' + resolution;
	out += '.jpg';

	var cmd = 'rules/bin/convert.py';
	cmd += ' -i "' + i_wnd.m_file + '"';
	cmd += ' -q ' + quality;
	if( resolution != '-1' ) cmd += ' -r ' + resolution;
	cmd += ' -o "' + out + '"';
	i_wnd.destroy();

	n_Request({"cmdexec":{"cmds":[cmd]}}, false);
}

function fv_Converted( i_msg)
{
	var file = i_msg.convert;
	if( i_msg.error )
	{
		c_Error( i_msg.error);
		return;
	}
	if( file == null )
	{
		c_Error('Converted null file.');
		return;
	}

	c_Info('Converted ' + c_PathBase( file));

	for( var i = 0; i < fv_views.length; i++)
		if( file.indexOf( fv_views[i].path ) != -1 )
			fv_views[i].refresh();
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
			return;
		}
	}
	c_Error('Item not founded: ' + i_path);
}

