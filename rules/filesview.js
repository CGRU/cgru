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
	elThumbBigger.onclick = u_FileThumbsBigger;
	elThumbBigger.title = 'Show thumbnails bigger';

	var elThumbSmaller = document.createElement('div');
	elThumbDiv.appendChild( elThumbSmaller);
	elThumbSmaller.classList.add('button');
	elThumbSmaller.textContent = '-';
	elThumbSmaller.onclick = u_FileThumbsSmaller;
	elThumbSmaller.title = 'Show thumbnails smaller';

	var elThumbCrop = document.createElement('div');
	elThumbDiv.appendChild( elThumbCrop);
	elThumbCrop.classList.add('button');
	elThumbCrop.textContent = '[c]';
	elThumbCrop.onclick = u_FileThumbsCrop;
	elThumbCrop.title = 'Show thumbnails cropped';

	var elMakeThumbnails = document.createElement('div');
	elThumbDiv.appendChild( elMakeThumbnails);
	elMakeThumbnails.classList.add('button');
	elMakeThumbnails.textContent = 'Thumbs:';
	elMakeThumbnails.onclick = u_MakeThumbnails;
	u_thumbnails_elMakeBtns.push( elMakeThumbnails);
	elMakeThumbnails.title = 'Generate thumbnails';

	this.elFiles = [];
	if( this.walk.folders)
	{
		this.walk.folders.sort( c_CompareFiles );
		for( var i = 0; i < this.walk.folders.length; i++)
			if( false == u_SkipFile( this.walk.folders[i].name))
				this.elFiles.push( u_ShowFolder( i_element, i_path, this.walk.folders[i], this.walk));
	}

	if( this.walk.files)
	{
		this.walk.files.sort( c_CompareFiles );
		for( var i = 0; i < this.walk.files.length; i++)
			if( false == u_SkipFile( this.walk.files[i].name))
				this.elFiles.push( u_ShowFile( i_element, i_path, this.walk.files[i], this.walk));
	}

//	elMakeThumbnails.m_elFiles = this.elFiles;
//	return this.elFiles;
}

FilesView.prototype.destroy = function()
{
	this.elParent.removeChild( this.elRoot);
}

