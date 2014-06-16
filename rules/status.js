st_Status = null;
/*
st_wnd = null;
this.elEdit = null;
st_elParent = null;
st_elToHide = null
st_path = null;
st_status = null;
st_FuncApply = null;
st_progress = null;
*/
function st_InitAuth()
{
	$('status_edit_btn').style.display = 'block';
}

function st_Finish()
{
//	st_DestroyEditUI();
//	st_Show( null);
	st_Status = null;
}

function st_BodyModified()
{
	if( RULES.status == null ) RULES.status = {};
	if( RULES.status.body == null )
	{
		RULES.status.body = {};
		RULES.status.body.cuser = g_auth_user.id;
		RULES.status.body.ctime = c_DT_CurSeconds();
	}
	else
	{
		RULES.status.body.muser = g_auth_user.id;
		RULES.status.body.mtime = c_DT_CurSeconds();
	}

	st_Save();
}

function st_Show( i_status)
{
/*	if( i_status != null )
		RULES.status = c_CloneObj( i_status);
	else
		i_status = RULES.status;*/
	if( st_Status )
		st_Status.show();
	else
		new Status( i_status);
}

function Status( i_obj, i_args)
{
	if( i_args == null ) i_args = {};

	if( i_args.createGUI )
		i_args.createGUI( this);
	else
	{
		this.elParent      = $('status');
		this.elShow        = $('status_show');
		this.elEditBtn     = $('status_edit_btn');
		this.elColor       = $('status');
		this.elAnnotation  = $('status_annotation');
		this.elProgress    = $('status_progress');
		this.elProgressBar = $('status_progressbar');
		this.elPercentage  = $('status_percentage');
		this.elArtists     = $('status_artists');
		this.elTags        = $('status_tags');
		this.elFramesNum   = $('status_framesnum');
		this.elFinish      = $('status_finish');
		this.elModified    = $('status_modified');
		this.elReports     = $('status_reports');
	}

	if( this.elEditBtn )
	{
		this.elEditBtn.m_status = this;
		this.elEditBtn.onclick = function(e){ e.stopPropagation(); e.currentTarget.m_status.edit();};
	}

	this.path = i_args.path;
	if( this.path == null ) this.path = g_CurPath();

	this.obj = i_obj;
	this.args = i_args;

	this.show();
}

Status.prototype.show = function( i_status)
{
	if( i_status ) this.obj = i_status;

	if( this.elShow       ) this.elShow.style.display = 'block';
	if( this.elColor      ) st_SetElColor(      this.obj, this.elColor);
	if( this.elAnnotation ) st_SetElAnnotation( this.obj, this.elAnnotation);
	if( this.elProgress   ) st_SetElProgress(   this.obj, this.elProgressBar, this.elProgress, this.elPercentage);
	if( this.elArtists    ) st_SetElArtists(    this.obj, this.elArtists);
	if( this.elTags       ) st_SetElTags(       this.obj, this.elTags);
	if( this.elFramesNum  ) st_SetElFramesNum(  this.obj, this.elFramesNum);
	if( this.elFinish     ) st_SetElFinish(     this.obj, this.elFinish);
	if( this.elModified   )
	{
		var modified = '';
		if( this.obj )
		{
			if( this.obj.muser ) modified += ' by ' + c_GetUserTitle(  this.obj.muser);
			if( this.obj.mtime ) modified += ' at ' + c_DT_StrFromSec( this.obj.mtime);
			if( modified != '' ) modified = 'Modified' + modified;
		}
		this.elModified.textContent = modified;
	}
	if( this.elEdit )
	{
		if( this.elParent )
		{
			this.elParent.removeChild( this.elEdit);
			this.elParent.classList.remove('status_editing');
		}
		this.elEdit = null;
	}

	// Reports:
	if( this.elReports == null ) return;

	this.elReports.textContent = '';
	if( RULES.status == null ) return;
	if( RULES.status.reports == null ) return;
	if( RULES.status.reports.length == 0 ) return;

	var reps_types = {};
	var reps_duration = 0;

	for( var i = 0; i < RULES.status.reports.length; i++)
	{
		var report = RULES.status.reports[i];

		if(( report.tags == null ) || ( report.tags.length == 0 ))
			report.tags = ['other'];

		for( var t = 0; t < report.tags.length; t++)
		{
			var rtype;
			if( reps_types[report.tags[t]])
			{
				rtype = reps_types[report.tags[t]];
			}
			else
			{
				rtype = {};
				rtype.duration = 0;
				rtype.artists = [];
				reps_types[report.tags[t]] = rtype;
			}

			rtype.duration += report.duration;

			if( rtype.artists.indexOf( report.artist) == -1 )
				rtype.artists.push( report.artist);
		}

		reps_duration += report.duration;
//console.log( JSON.stringify( report));
	}

	reps_types.total = {"duration":reps_duration,"artists":[]};

	for( var rtype in reps_types )
	{
		var el = document.createElement('div');
		this.elReports.appendChild( el);
		var info = c_GetTagTitle( rtype) + ': ' + reps_types[rtype].duration;
		for( var a = 0; a < reps_types[rtype].artists.length; a++)
		{
			if( a ) info += ',';
			info += ' ' + c_GetUserTitle( reps_types[rtype].artists[a]);
		}
		el.textContent = info;
	}
//console.log( JSON.stringify( RULES.status.reports));
}

function st_SetElProgress( i_status, i_elProgressBar, i_elProgressHide, i_elPercentage)
{
	if( i_status && ( i_status.progress != null ) && ( i_status.progress >= 0 ))
	{
		if( i_elProgressBar) i_elProgressBar.style.width = i_status.progress+'%';
		if( i_elPercentage) i_elPercentage.textContent = i_status.progress+'%';
		if( i_elProgressHide)
		{
			i_elProgressHide.style.display = 'block';
			i_elProgressHide.title = i_status.progress+'%';
		}
	}
	else
	{
		if( i_elProgressBar) i_elProgressBar.style.width = '0';
		if( i_elPercentage) i_elPercentage.textContent = '';
		if( i_elProgressHide)
		{
			i_elProgressHide.style.display = 'none';
			i_elProgressHide.title = null;
		}
	}
}

function st_SetElLabel( i_status, i_el, i_full)
{
	if( i_full == null ) i_full = false;
	st_SetElAnnotation( i_status, i_el, i_full);
}
function st_SetElFramesNum( i_status, i_el, i_full)
{
	if( i_full == null ) i_full = true;
	var num = '';
	if( i_status && i_status.frames_num )
	{
		num = i_status.frames_num;
		i_el.title = ' ~ ' + Math.round(num/RULES.fps) + 'sec';
		if( i_full ) num = 'Frames number = ' + num;
		else num = 'F:' + num;
	}
	i_el.textContent = num;
}
function st_SetElAnnotation( i_status, i_el, i_full) { st_SetElText( i_status, i_el,'annotation', i_full);}
function st_SetElText( i_status, i_el, i_field, i_full)
{
	if( i_full == null ) i_full = true;
	if( i_status && i_status[i_field])
	{
		if( i_full )
			i_el.innerHTML = i_status[i_field];
		else
			i_el.innerHTML = i_status[i_field].replace(/^\s+/,'').split(' ')[0];
	}
	else
		i_el.innerHTML = '';
}
function st_SetElArtists( i_status, i_elArtists, i_short)
{
	var text = '';
	if( i_status && i_status.artists )
		for( var i = 0; i < i_status.artists.length; i++)
		{
			if( i )
			{
				if( i_short ) text += ',';
				else text += ', ';
			}
			text += c_GetUserTitle( i_status.artists[i], null, i_short);
		}
	i_elArtists.textContent = text;
}
function st_SetElTags( i_status, i_elTags, i_short)
{
	if( i_short )
	{
		var tags = '';
		if( i_status && i_status.tags )
			for( var i = 0; i < i_status.tags.length; i++)
			{
				if( i ) tags += ' ';
				tags += c_GetTagShort( i_status.tags[i]);
			}
		i_elTags.textContent = tags;
		return;
	}

	if( i_elTags.m_elTags )
		for( i = 0; i < i_elTags.m_elTags.length; i++ )
			i_elTags.removeChild( i_elTags.m_elTags[i]);
	i_elTags.m_elTags = [];

	if( i_status && i_status.tags )
		for( var i = 0; i < i_status.tags.length; i++)
		{
			var el = document.createElement('div');
			i_elTags.appendChild( el);
			i_elTags.m_elTags.push( el);
			el.classList.add('tag');
			el.textContent = c_GetTagTitle( i_status.tags[i]);
			el.title = c_GetTagTip( i_status.tags[i]);
		}
}
function st_SetElColor( i_status, i_elBack, i_elColor, i_setNone)
{
	if( i_elColor == null ) i_elColor = i_elBack;
	if( i_setNone == null ) i_setNone = true;

	if( i_status && i_status.color)
	{
		var c = i_status.color;
		i_elBack.style.background = 'rgb('+c[0]+','+c[1]+','+c[2]+')';
		if( c[0]+c[1]+.3*c[2] > 300 )
			i_elColor.style.color = '#000';
		else
			i_elColor.style.color = '#FFF';
//window.console.log(c[0]+c[1]+c[2])
		return true;
	}
	else if( i_setNone )
	{
//		i_elBack.style.background = '';
//		i_elColor.style.color = 'inherit';
		i_elBack.style.background = localStorage.background;
		i_elColor.style.color = localStorage.text_color;
	}
	return false;
}
function st_SetElFinish( i_status, i_elFinish, i_full)
{
	if( i_full == null ) i_full = true;
	var text = '';
	var shadow = '';
	var alpha = 0;
	if( i_status && i_status.finish )
	{
		var days = c_DT_DaysLeft ( i_status.finish );
		var alpha = 1 /( 1 + days );
		days = Math.round( days*10) / 10;
		if( alpha < 0 ) alpha = 0;
		if( alpha > 1 ) alpha = 1;
		if( days <= 1 ) alpha = 1;
		if( i_full )
		{
			text += 'Finish at: ';
			text += c_DT_StrFromSec( i_status.finish).substr(0,15);
			text += ' ('+days+' days left)';
		}
		else
			text += days+'days';
	}
	shadow = '0 0 2px rgba(255,0,0,'+alpha+'),';
	shadow+= '0 0 4px rgba(255,0,0,'+alpha+')';
	i_elFinish.style.textShadow = shadow;
	i_elFinish.style.color = 'rgb('+Math.round(150*alpha)+',0,0)';
	i_elFinish.textContent = text;
}
/*
function st_StatusEditOnClick()
{
	st_CreateEditUI( $('status'), g_CurPath(), RULES.status, st_Show, $('status_show'));
}
*/
//function st_CreateEditUI( i_elParent, i_path, i_status, i_FuncApply, i_elToHide)
Status.prototype.edit = function( i_args)
{
//console.log( JSON.stringify( i_args));
//console.log(JSON.stringify(i_status));
	if( this.elEdit ) return;

	if( this.elShow ) this.elShow.style.display = 'none';

	this.elParent.classList.add('status_editing');

//	st_DestroyEditUI();
	this.elEdit = document.createElement('div');
	this.elParent.appendChild( this.elEdit);
	this.elEdit.classList.add('status_edit');

//	st_elParent = i_elParent;
//	st_elToHide = i_elToHide;
//	st_path = i_path;
//	st_progress = null;
//	st_FuncApply = i_FuncApply;
//	st_status = {};
/*
	if( i_status )
	{
		st_status = c_CloneObj( i_status);
		// Store progress to see whether progress update needed
		if( i_status.progress != null )
			st_progress = i_status.progress;
	}
*/

//	this.elEdit = document.createElement('div');
//	st_elParent.appendChild( this.elEdit);
//	this.elEdit.classList.add('status_edit');
	this.elEdit.onclick = function(e){e.stopPropagation();};

	var elBtns = document.createElement('div');
	this.elEdit.appendChild( elBtns);
	elBtns.style.cssFloat = 'right';

	var elBtnCancel = document.createElement('div');
	elBtns.appendChild( elBtnCancel);
	elBtnCancel.classList.add('button');
	elBtnCancel.textContent = 'Cancel';
	elBtnCancel.m_status = this;
//	elBtnCancel.onclick = function(e){e.currentTarget.m_status.editCancel();};
	elBtnCancel.onclick = function(e){e.currentTarget.m_status.show();};

	var elBtnSave = document.createElement('div');
	elBtns.appendChild( elBtnSave);
	elBtnSave.classList.add('button');
	elBtnSave.textContent = 'Save';
	elBtnSave.m_status = this;
	elBtnSave.onclick = function(e){e.currentTarget.m_status.editSave();};

	var elFinishDiv = document.createElement('div');
	this.elEdit.appendChild( elFinishDiv);
	elFinishDiv.style.cssFloat = 'right';
	elFinishDiv.style.width = '200px';
	elFinishDiv.style.textAlign = 'center';
	elFinishDiv.style.marginLeft = '10px';
	var elFinishLabel = document.createElement('div');
	elFinishDiv.appendChild( elFinishLabel);
	elFinishLabel.style.cssFloat = 'left';
	elFinishLabel.textContent = 'Finish:';
	elFinishLabel.onclick  = function(e){ e.currentTarget.m_elEdit_finish.textContent = c_DT_FormStrNow();};
	elFinishLabel.title = 'Double click to set current date.';
	elFinishLabel.style.cursor = 'pointer';
	this.elEdit_finish = document.createElement('div');
	elFinishDiv.appendChild( this.elEdit_finish);
	this.elEdit_finish.classList.add('editing');
	this.elEdit_finish.contentEditable = 'true';
	elFinishLabel.m_elEdit_finish = this.elEdit_finish;

	var elAnnDiv = document.createElement('div');
	this.elEdit.appendChild( elAnnDiv);
	var elAnnLabel = document.createElement('div');
	elAnnDiv.appendChild( elAnnLabel);
	elAnnLabel.textContent = 'Annotation:';
	elAnnLabel.style.cssFloat = 'left';
	this.elEdit_annotation = document.createElement('div');
	elAnnDiv.appendChild( this.elEdit_annotation);
	this.elEdit_annotation.classList.add('editing');
	this.elEdit_annotation.contentEditable = 'true';

	var elProgressDiv = document.createElement('div');
	this.elEdit.appendChild( elProgressDiv);
//	elProgressDiv.style.cssFloat = 'left';
	elProgressDiv.classList.add('percent');
	var elProgressLabel = document.createElement('div');
	elProgressDiv.appendChild( elProgressLabel);
	elProgressLabel.textContent = 'Progress:';
//	elProgressLabel.style.cssFloat = 'left';
	elProgressLabel.style.fontSize = '12px';
	this.elEdit_progress = document.createElement('div');
	elProgressDiv.appendChild( this.elEdit_progress);
	this.elEdit_progress.style.width = '40px';
	this.elEdit_progress.style.height = '18px';
	this.elEdit_progress.contentEditable = 'true';
	this.elEdit_progress.classList.add('editing');
	this.elEdit_progress.style.textAlign = 'center';

	this.elEdit_artists = document.createElement('div');
	this.elEdit.appendChild( this.elEdit_artists);
	this.elEdit_artists.classList.add('list');
	this.elEdit_artists.classList.add('artists');
	this.elEdit_artists.m_elBtn = document.createElement('div');
	this.elEdit_artists.appendChild( this.elEdit_artists.m_elBtn);
	this.elEdit_artists.m_elBtn.classList.add('button');
	this.elEdit_artists.m_elBtn.style.cssFloat = 'left';;
	this.elEdit_artists.m_elBtn.textContent = 'Artists:';
	this.elEdit_artists.m_elBtn.m_status = this;
	this.elEdit_artists.m_elBtn.onclick = function(e){ e.currentTarget.m_status.editArtistsShow();};
	this.elEdit_artists.m_elList = document.createElement('div');
	this.elEdit_artists.appendChild( this.elEdit_artists.m_elList);

	if( this.obj.artists )
		for( var i = 0; i < this.obj.artists.length; i++)
		{
			var el = document.createElement('div');
			this.elEdit_artists.m_elList.appendChild( el);
			el.textContent = c_GetUserTitle( this.obj.artists[i]);
			el.classList.add('tag');
			el.classList.add('selected');
		}

	this.elEdit_tags = document.createElement('div');
	this.elEdit.appendChild( this.elEdit_tags);
	this.elEdit_tags.classList.add('list');
	this.elEdit_tags.classList.add('tags');
	this.elEdit_tags.m_elBtn = document.createElement('div');
	this.elEdit_tags.appendChild( this.elEdit_tags.m_elBtn);
	this.elEdit_tags.m_elBtn.classList.add('button');
	this.elEdit_tags.m_elBtn.style.cssFloat = 'left';
	this.elEdit_tags.m_elBtn.textContent = 'Tags:';
	this.elEdit_tags.m_elBtn.m_status = this;
	this.elEdit_tags.m_elBtn.onclick = function(e){ e.currentTarget.m_status.editTagsShow();};
	this.elEdit_tags.m_elList = document.createElement('div');
	this.elEdit_tags.appendChild( this.elEdit_tags.m_elList);
	if( this.obj.tags )
		for( var i = 0; i < this.obj.tags.length; i++)
		{
			var el = document.createElement('div');
			this.elEdit_tags.m_elList.appendChild( el);
			el.textContent = c_GetTagTitle( this.obj.tags[i]);
			el.title = c_GetTagTip( this.obj.tags[i]);
			el.classList.add('tag');
			el.classList.add('selected');
		}

	var elColor = document.createElement('div');
	this.elEdit.appendChild( elColor);
	elColor.classList.add('color');
	u_DrawColorBars({"el":elColor,"onclick":st_EditColorOnClick,"data":this});

	st_SetElAnnotation( this.obj, this.elEdit_annotation);
//	st_SetElColor( this.obj);
	if( this.obj.finish )
		this.elEdit_finish.textContent = c_DT_FormStrFromSec( this.obj.finish);
	if( this.obj.progress != null ) this.elEdit_progress.textContent = this.obj.progress;

	this.elEdit_annotation.focus();
}
function st_EditColorOnClick( i_clr, i_data)
{
	i_data.edit_color = i_clr;
	st_SetElColor({"color": i_clr}, i_data.elColor);
}

/*
function st_DestroyEditUI()
{
	if( st_elParent )
	{
		st_elParent.classList.remove('status_editing');
		if( this.elEdit ) st_elParent.removeChild( this.elEdit);
	}

	if( st_elToHide )
		st_elToHide.style.display = 'block';

	this.elEdit = null;
	st_elParent = null;
	st_elToHide = null;
	st_status = null;
}
*/
Status.prototype.editTagsShow = function()
{
	this.editShowList( this.elEdit_tags, 'tags', RULES.tags);
}
Status.prototype.editArtistsShow = function()
{
	this.editShowList( this.elEdit_artists, 'artists', g_users);
}
Status.prototype.editShowList = function( i_elParent, i_stParam, i_list)
{
	if( i_elParent.m_edit ) return;
	i_elParent.m_edit = true;
	i_elParent.m_elBtn.classList.remove('button');
	i_elParent.m_elList.style.display = 'none';
	i_elParent.m_elListAll = [];
	for( var item in i_list)
	{
		var el = document.createElement('div');
		i_elParent.appendChild( el);
		el.classList.add('tag');
		if( i_list[item].title )
			el.textContent = i_list[item].title;
		else
			el.textContent = item;
		if( i_list[item].tip ) el.title = i_list[item].tip;
		el.m_item = item;
		if( this.obj[i_stParam] && ( this.obj[i_stParam].indexOf( item) != -1 ))
		{
			el.m_selected = true;
			el.classList.add('selected');
		}
		el.onclick = c_ElToggleSelected;
		i_elParent.m_elListAll.push( el);
	}
}

Status.prototype.editSave = function()
{
	if( this.obj == null ) this.obj = {};
	var old_progress = this.obj.progress;

	this.obj.color = this.edit_color;

	var finish = this.elEdit_finish.textContent;
	if( finish.length )
	{
		finish = c_DT_SecFromStr( this.elEdit_finish.textContent);
		if( finish == 0 ) return;
		this.obj.finish = finish;
	}

	this.obj.annotation = c_Strip( this.elEdit_annotation.innerHTML);
	this.obj.progress = parseInt( c_Strip( this.elEdit_progress.textContent));
	if( this.elEdit_artists.m_elListAll )
	{
		this.obj.artists = [];
		for( var i = 0; i < this.elEdit_artists.m_elListAll.length; i++)
			if( this.elEdit_artists.m_elListAll[i].m_selected )
				this.obj.artists.push( this.elEdit_artists.m_elListAll[i].m_item);
	}
	if( this.elEdit_tags.m_elListAll )
	{
		this.obj.tags = [];
		for( var i = 0; i < this.elEdit_tags.m_elListAll.length; i++)
			if( this.elEdit_tags.m_elListAll[i].m_selected )
				this.obj.tags.push( this.elEdit_tags.m_elListAll[i].m_item);
	}

//	st_FuncApply( st_status);
//	st_DestroyEditUI();
	this.save();
	this.show();

	if( this.elEdit_progress.textContent.length )
		if( old_progress != this.obj.progress )
			st_UpdateProgresses( this.path);
}

Status.prototype.save = function()
{
	if( g_CurPath() == this.path )
		g_FolderSetStatus( this.obj);

	this.obj.muser = g_auth_user.id;
	this.obj.mtime = c_DT_CurSeconds();

	st_Save( this.obj, this.path);
	nw_MakeNews({"title":'status',"path":this.path,"artists":this.obj.artists});
}

function st_Save( i_status, i_path, i_wait)
{
	if( i_status == null ) i_status = RULES.status;
	if( i_path == null ) i_path = g_CurPath();
	var obj = {};
	obj.object = {"status":i_status};
	obj.add = true;
	obj.file = RULES.root + i_path + '/' + RULES.rufolder + '/status.json';
	return c_Parse( n_Request_old({"editobj":obj}, i_wait));
}

function st_SetFramesNumber( i_num)
{
	if( RULES.status == null ) RULES.status = {};
	RULES.status.frames_num = i_num;
	st_Save();
	st_Show( RULES.status);
}

function st_UpdateProgresses( i_path)
{
	var folders = i_path.split('/');
	var path = '';

	paths = [];
	progresses = {};

	for( var i = 1; i < folders.length-1; i++)
	{
		path += '/'+folders[i];
		paths.push( path);
	}
//window.console.log( paths);
	walks = n_WalkDir({"paths":paths,"rufiles":['status'],"lookahead":['status']});
	if( walks == null ) return;

	for( var w = walks.length-1; w >= 0; w--)
	{
//window.console.log( walks[w]);
		if( walks[w].error )
		{
			c_Error( walks[w].error);
			return;
		}
		if(( walks[w].folders == null ) || ( walks[w].folders.length == 0 ))
		{
			c_Error('Can`t find folders in ' + paths[w]);
			return;
		}

		var progress = 0;
		var progress_count = 0;
		for( var f = 0; f < walks[w].folders.length; f++ )
		{
			var folder = walks[w].folders[f];
			if( folder.name == RULES.rufolder ) continue;
			var path = paths[w] + '/' + folder.name;
			if( progresses[path] != null )
			{
				progress += progresses[path];
			}
			else
			{
//if( folder.status ) console.log( folder.name+': '+folder.status.progress);
				if(( folder.status == null ) || ( folder.status.progress == null ))
				{
//console.log(folder.name+': null');
					if( w != (walks.length-1)) continue;
					st_Save({"progress":0}, path, false);		
				}
				else if( folder.status.progress < 0 ) continue;
				else progress += folder.status.progress;
			}
			progress_count++;
		}

		if( progress_count == 0 )
		{
			return;
		}

		progress = Math.round( progress / progress_count);
		progresses[paths[w]] = progress;

//console.log(paths[w]+': '+progress_count+': '+progress);
	}

	for( var path in progresses)
		st_Save({"progress":progresses[path]}, path, false);
}

