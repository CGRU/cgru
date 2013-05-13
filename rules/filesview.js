fv_elThumbnails = [];
fv_thumbnails_elMakeBtns = [];
fv_thumbnails_tomake = 0;


function FilesView( i_elParent, i_path, i_walk)
{
	this.elParent = i_elParent;
	this.path = i_path;
	this.walk = i_walk;

	this.elRoot = document.createElement('div');
	this.elParent.appendChild( this.elRoot);
	this.elRoot.classList.add('files_view');

	var elPanel = document.createElement('div');
	this.elRoot.appendChild( elPanel);
	elPanel.classList.add('panel');

	var elTitle = document.createElement('div');
	elPanel.appendChild( elTitle);
	elTitle.classList.add('title');
	var title = '';
	if( ASSET && ASSET.name )
		title = ASSET.name;
	elTitle.textContent = title;

	var elPath = document.createElement('a');
	elPanel.appendChild( elPath);
	elPath.href = '#' + i_path;
	var path = i_path;
	if( ASSET && ASSET.path )
	{
		path = path.replace( ASSET.path, '');
		if( path[0] == '/' ) path = path.substr( 1);
	}
	elPath.classList.add('path');
	elPath.textContent = path;

	c_CreateOpenButton( elPanel, i_path);

	var elThumbDiv = document.createElement('div');
	elPanel.appendChild( elThumbDiv);
	elThumbDiv.classList.add('thumbsdiv');
	
	var elThumbBigger = document.createElement('div');
	elThumbDiv.appendChild( elThumbBigger);
	elThumbBigger.classList.add('button');
	elThumbBigger.textContent = '+';
	elThumbBigger.onclick = this.ThumbsBigger;
	elThumbBigger.title = 'Show thumbnails bigger';

	var elThumbSmaller = document.createElement('div');
	elThumbDiv.appendChild( elThumbSmaller);
	elThumbSmaller.classList.add('button');
	elThumbSmaller.textContent = '-';
	elThumbSmaller.onclick = this.ThumbsSmaller;
	elThumbSmaller.title = 'Show thumbnails smaller';

	var elThumbCrop = document.createElement('div');
	elThumbDiv.appendChild( elThumbCrop);
	elThumbCrop.classList.add('button');
	elThumbCrop.textContent = '[c]';
	elThumbCrop.onclick = this.ThumbsCrop;
	elThumbCrop.title = 'Show thumbnails cropped';

	var elMakeThumbnails = document.createElement('div');
	elThumbDiv.appendChild( elMakeThumbnails);
	elMakeThumbnails.classList.add('button');
	elMakeThumbnails.textContent = 'Thumbs:';
	elMakeThumbnails.onclick = this.ThumbsMake;
//	fv_thumbnails_elMakeBtns.push( elMakeThumbnails);
	elMakeThumbnails.title = 'Generate thumbnails';

	this.elView = document.createElement('div');
	this.elRoot.appendChild( this.elView);
	this.elView.classList.add('view');

	this.show();
//	elMakeThumbnails.m_elFiles = this.elFiles;
//	return this.elFiles;
}

FilesView.prototype.destroy = function()
{
	this.elParent.removeChild( this.elRoot);
}

FilesView.prototype.show = function()
{
	this.elView.innerHTML = '';
	this.elFiles = [];
	this.elThumbnails = [];

	if( this.walk == null )
		return;

	if( this.walk.folders)
	{
		this.walk.folders.sort( c_CompareFiles );
		for( var i = 0; i < this.walk.folders.length; i++)
			if( false == fv_SkipFile( this.walk.folders[i].name))
				this.elFiles.push( fv_ShowFolder( this.elView, this.path, this.walk.folders[i], this.walk));
	}

	if( this.walk.files)
	{
		this.walk.files.sort( c_CompareFiles );
		for( var i = 0; i < this.walk.files.length; i++)
			if( false == fv_SkipFile( this.walk.files[i].name))
				this.elFiles.push( fv_ShowFile( this.elView, this.path, this.walk.files[i], this.walk));
	}
}

function fv_showFolder( i_element, i_path, i_folder, i_walk)
{
	var name = i_folder.name;
	i_path = (i_path + '/' + name).replace( /\/\//g, '/');

	var elFolder = document.createElement('div');
	elFolder.classList.add('folder');
	i_element.appendChild( elFolder);

	fv_MakeFileThumbEl( elFolder, i_path, i_walk, 'folder');

	var elOpen = c_CreateOpenButton( elFolder, i_path);
	if( elOpen ) elOpen.style.cssFloat = 'left';

	var elLinkA = document.createElement('a');
	elFolder.appendChild( elLinkA);
	elLinkA.setAttribute('href', '#'+i_path);
	elLinkA.textContent = name;

	var elLinkA = document.createElement('a');
	elFolder.appendChild( elLinkA);
	elLinkA.setAttribute('href', 'player.html#'+i_path);
	elLinkA.setAttribute('target', '_blank');
	elLinkA.textContent = 'play';
	elLinkA.style.cssFloat = 'right';

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
			cmd = cmd.replace('@PATH@', cgrfv_PM('/'+RULES.root + i_path));
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
		elMakeDailies.m_path = i_path;
		elMakeDailies.onclick = function(e){
			d_Make( e.currentTarget.m_path, ASSET.path+'/'+ASSET.dailies.path[0])};
	}

	if( i_folder.mtime != null )
	{
		var elMTime = document.createElement('div');
		elFolder.appendChild( elMTime);
		elMTime.classList.add('mtime');
		elMTime.textContent = c_DT_FormStrFromSec( i_folder.mtime);
	}

	return elFolder;
}

function fv_ShowFile( i_element, i_path, i_file, i_walk)
{
	var path = i_path + '/' + i_file.name;

	var elFile = document.createElement('div');
	i_element.appendChild( elFile);
	elFile.classList.add('folder');
	elFile.classList.add('file');

	fv_MakeFileThumbEl( elFile, path, i_walk, 'file');

	var elLinkA = document.createElement('a');
	elFile.appendChild( elLinkA);
	elLinkA.href = RULES.root + path;
	elLinkA.target = '_blank';
	elLinkA.textContent = i_file.name;

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
			cmd = cmd.replace('@PATH@', cgrfv_PM('/'+RULES.root + path));
			cmd = cmd.replace('@FPS@', RULES.fps);
			elCmd.setAttribute('cmdexec', JSON.stringify([cmd]));
		}

		var elExplode = document.createElement('div');
		elFile.appendChild( elExplode);
		elExplode.classList.add('button');
		elExplode.textContent = 'Explode';
		elExplode.title = 'Explode movie to images sequence';
		elExplode.m_path = path;
		elExplode.onclick = function(e){ d_Explode( e.currentTarget.m_path)};
	}

	if( c_FileIsImage( i_file.name))
	{
		var elCvt = document.createElement('div');
		elFile.appendChild( elCvt);
		elCvt.classList.add('button');
		elCvt.textContent = 'JPG';
		elCvt.m_file = i_file.name;
		elCvt.onclick = function(e){ fv_ImgConvertDialog( e.currentTarget.m_file)};
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

	return elFile;
}

function fv_MakeFileThumbEl( i_el, i_path, i_walk, i_type)
{
	var elThumbnal = document.createElement('span');
	i_el.appendChild( elThumbnal);
	i_el.m_elThumbnail = elThumbnal;
	elThumbnal.classList.add('thumbnail');
	elThumbnal.m_type = i_type;

	elThumbnal.m_path = i_path;
	var thumbFile = RULES.root + c_GetThumbFileName( i_path);
	var thumbName = c_PathBase( thumbFile);
	elThumbnal.m_thumbFile = thumbFile;
	fv_elThumbnails.push( elThumbnal);

	var elImg = document.createElement('img');
	elThumbnal.appendChild( elImg);
	elThumbnal.m_elImg = elImg;
//	if( i_walk && i_walk.rufiles && ( i_walk.rufiles.indexOf( thumbName) != -1))
	if( c_FileCanThumbnail( i_path) && (( i_walk == null ) || ( i_walk.rufiles && ( i_walk.rufiles.indexOf( thumbName) != -1))))
		elImg.src = thumbFile;
	else
		elThumbnal.style.display = 'none';

	fv_FileThumbResize( elImg);
	elImg.onload = fv_FileThumbOnLoad;
}

function fv_FileThumbsBigger( i_smaller)
{
	var s = parseInt( localStorage.thumb_file_size );
	var ns = s;
	if( i_smaller === true ) ns -= 10;
	else ns += 10;

	if( ns < 10 ) return;
	if( ns > 160 ) return;

	localStorage.thumb_file_size = ''+ns;
	fv_FileThumbResizeAll();
}
function fv_FileThumbsSmaller() { fv_FileThumbsBigger( true); }

function fv_FileThumbsCrop()
{
	if( localStorage.thumb_file_crop === 'true' )
		localStorage.thumb_file_crop = 'false';
	else
		localStorage.thumb_file_crop = 'true';
	fv_FileThumbResizeAll();
}

function fv_FileThumbResizeAll() { for( var i = 0; i < fv_elThumbnails.length; i++) fv_FileThumbResize( fv_elThumbnails[i].m_elImg); }

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
//	wnd.elContent.classList.add('imgconvert');

	var elQualityDiv = document.createElement('div');
	wnd.elContent.appendChild( elQualityDiv);
	var elQualityLabel = document.createElement('div');
	elQualityLabel.textContent = 'Quality';
	elQualityDiv.appendChild( elQualityLabel);
	elQualityLabel.classList.add('label');
	var elQualityInfo = document.createElement('div');
	elQualityInfo.textContent = '1 is the best';
	elQualityDiv.appendChild( elQualityInfo);
	elQualityInfo.classList.add('info');
	var elQuality = document.createElement('div');
	elQuality.textContent = '3';
	elQualityDiv.appendChild( elQuality);
	elQuality.classList.add('editing');

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
	var elResolution = document.createElement('div');
	elResolution.textContent = '-1';
	elResolutionDiv.appendChild( elResolution);
	elResolution.classList.add('editing');

	var elButton = document.createElement('div');
	wnd.elContent.appendChild( elButton);
	elButton.classList.add('button');
	elButton.textContent = 'Convert';
}

