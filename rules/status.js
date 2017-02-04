st_Status = null;
st_MultiValue = '[...]';

function st_InitAuth()
{
	$('status_edit_btn').style.display = 'block';
}

function st_Finish()
{
	if( st_Status )
		st_Status.editCancel();

	st_Status = null;
}

function st_BodyModified( i_st_obj, i_path)
{
	var st_obj = i_st_obj;
	if( st_obj == null ) st_obj = RULES.status;
	if( st_obj == null ) st_obj = {};

	if( st_obj.body == null )
	{
		st_obj.body = {};
		st_obj.body.cuser = g_auth_user.id;
		st_obj.body.ctime = c_DT_CurSeconds();
	}
	else
	{
		st_obj.body.muser = g_auth_user.id;
		st_obj.body.mtime = c_DT_CurSeconds();
	}

	st_Save({'body':st_obj.body}, i_path, /*func*/null, /*args*/null, /*navig folder update params*/{});
}

function st_Show( i_status)
{
	if( ASSET && ASSET.subfolders_status_hide && ( ASSET.path != g_CurPath() ))
	{
		$('status').style.display = 'none';
		return;
	}

	$('status').style.display = 'block';
		
/*	if( i_status != null )
		RULES.status = c_CloneObj( i_status);
	else
		i_status = RULES.status;*/
	if( st_Status )
	{
		st_Status.show( i_status);
	}
	else
	{
		st_Status = new Status( i_status);
	}
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
		this.elAdinfo      = $('status_adinfo');
		this.elProgress    = $('status_progress');
		this.elProgressBar = $('status_progressbar');
		this.elPercentage  = $('status_percentage');
		this.elArtists     = $('status_artists');
		this.elFlags       = $('status_flags');
		this.elTags        = $('status_tags');
		this.elFramesNum   = $('status_framesnum');
		this.elFinish      = $('status_finish');
		this.elModified    = $('status_modified');
		this.elReportsDiv  = $('status_reports_div');
		this.elReports     = $('status_reports');
		this.elTasksDiv    = $('status_tasks_div');
		this.elTasks       = $('status_tasks');
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

	this.editCancel();

	if( this.elShow       ) this.elShow.style.display = 'block';
	if( this.elColor      ) st_SetElColor(      this.obj, this.elColor);
	if( this.elAnnotation ) st_SetElAnnotation( this.obj, this.elAnnotation);
	if( this.elProgress   ) st_SetElProgress(   this.obj, this.elProgressBar, this.elProgress, this.elPercentage);
	if( this.elArtists    ) st_SetElArtists(    this.obj, this.elArtists);
	if( this.elFlags      ) st_SetElFlags(      this.obj, this.elFlags);
	if( this.elTags       ) st_SetElTags(       this.obj, this.elTags);
	if( this.elFramesNum  ) st_SetElFramesNum(  this.obj, this.elFramesNum);
	if( this.elFinish     ) st_SetElFinish(     this.obj, this.elFinish);
	if( this.elTimeCode && this.obj && this.obj.timecode_start && this.obj.timecode_finish )
	{
		this.elTimeCode.textContent = 'TC:' + this.obj.timecode_start + '-' + this.obj.timecode_finish;
	}
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

	if( this.elAdinfo )
	{
		if( g_admin && this.obj && this.obj.adinfo )
		{
			this.elAdinfo.textContent = atob( this.obj.adinfo);
			this.elAdinfo.style.display = 'block';
		}
		else
		{
			this.elAdinfo.textContent = '';
			this.elAdinfo.style.display = 'none';
		}
	}

	var args = {};
	args.statuses = [this.obj];
	args.elReports = this.elReports;
	args.elReportsDiv = this.elReportsDiv;
	args.elTasks = this.elTasks;
	args.elTasksDiv = this.elTasksDiv;

	stcs_Show( args);
}

function st_SetElProgress( i_status, i_elProgressBar, i_elProgressHide, i_elPercentage)
{
	var clr = 'rgba(0,255,0,.4)';

	if( i_elPercentage )
	{
		i_elPercentage.classList.remove('done');
		i_elPercentage.classList.remove('started');
		i_elPercentage.classList.add('notstarted');
	}
	if( i_elProgressBar )
	{
		i_elProgressBar.classList.remove('done');
		i_elProgressBar.classList.remove('started');
		i_elProgressBar.classList.add('notstarted');
	}
	if( i_elProgressHide )
	{
		i_elProgressHide.classList.remove('done');
		i_elProgressHide.classList.remove('startred');
		i_elProgressHide.classList.add('notstarted');
	}

	if( i_status && ( i_status.progress != null ) && ( i_status.progress >= 0 ))
	{
		if( i_status.progress < 100 )
		{
			clr = 255 - Math.round( 2.55 * i_status.progress );
			clr = 'rgba('+clr+',255,0,.8)';
		}

		if( i_elProgressBar)
		{
			i_elProgressBar.style.width = i_status.progress+'%';
			if( i_status.progress > 0 )
			{
				i_elProgressBar.classList.add('startred');
				i_elProgressBar.classList.remove('notstarted');
			}
			if( i_status.progress >= 100 )
				i_elProgressBar.classList.add('done');
		}
		if( i_elPercentage)
		{
			i_elPercentage.style.display = 'block';
			i_elPercentage.textContent = i_status.progress+'%';
			if( i_status.progress > 0 )
			{
				i_elPercentage.classList.add('startred');
				i_elPercentage.classList.remove('notstarted');
			}
			if( i_status.progress >= 100 )
				i_elPercentage.classList.add('done');
		}
		if( i_elProgressHide)
		{
			i_elProgressHide.style.display = 'block';
			i_elProgressHide.title = i_status.progress+'%';
			if( i_status.progress > 0 )
			{
				i_elProgressHide.classList.add('startred');
				i_elProgressHide.classList.remove('notstarted');
			}
			if( i_status.progress >= 100 )
				i_elProgressHide.classList.add('done');
		}
	}
	else
	{
		if( i_elProgressBar) i_elProgressBar.style.width = '0';
		if( i_elPercentage)
		{
			i_elPercentage.textContent = '';
			i_elPercentage.style.display = 'none';
		}
		if( i_elProgressHide)
		{
			i_elProgressHide.style.display = 'none';
			i_elProgressHide.title = null;
		}
	}

	if( i_elProgressBar )
		i_elProgressBar.style.backgroundColor = clr;
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
		if( i_full )
			i_el.parentNode.style.display = 'block';
		else
			num = 'F:' + num;
	}
	else if ( i_full )
		i_el.parentNode.style.display = 'none';

	if( i_full )
	{
		i_el.parentNode.classList.remove('updated');
		i_el.parentNode.title = null;
	}

	i_el.textContent = num;
}
function st_SetElPrice( i_status, i_el)
{
	var price = '';
	if( i_status && i_status.price )
		price = i_status.price;

	i_el.textContent = price;
}
function st_SetElDuration( i_status, i_el)
{
	var duration = '';
	if( i_status && i_status.duration )
		duration = i_status.duration;

	i_el.textContent = duration;
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
function st_SetElArtists( i_status, i_el, i_short)
{
	i_el.textContent = '';

	if(( i_status == null ) || ( i_status.artists == null ))
		return;

	for( var i = 0; i < i_status.artists.length; i++)
	{
		var el = document.createElement('div');
		i_el.appendChild( el);
		el.classList.add('tag');
		el.classList.add('artist');
		el.textContent = c_GetUserTitle( i_status.artists[i], null, i_short);

		if( g_users[i_status.artists[i]] && g_users[i_status.artists[i]].disabled )
			el.classList.add('disabled');

		if( i_short )
		{
			el.classList.add('short');
			if( g_auth_user && ( g_auth_user.id == i_status.artists[i] ))
				el.title = 'It`s me!';
			else
				el.title = c_GetUserTitle( i_status.artists[i]);
		}

		if( g_auth_user && ( g_auth_user.id == i_status.artists[i] ))
		{	
			el.classList.add('me');

			if( i_short !== true )
			{
				el.title = 'It`s me!\n - may be i should do something here?';
			}
		}

		var avatar = c_GetAvatar( i_status.artists[i]);
		if( avatar)
		{
			el.classList.add('with_icon');
			el.style.backgroundImage = 'url(' + avatar + ')';
		}
	}
}
function st_SetElFlags( i_status, i_elFlags, i_short)
{
	if( i_short )
	{
		var flags = '';
		if( i_status && i_status.flags )
			for( var i = 0; i < i_status.flags.length; i++)
			{
				if( i ) flags += ' ';
				flags += c_GetFlagShort( i_status.flags[i]);
			}
		i_elFlags.textContent = flags;
		return;
	}

	if( i_elFlags.m_elFlags )
		for( i = 0; i < i_elFlags.m_elFlags.length; i++ )
			i_elFlags.removeChild( i_elFlags.m_elFlags[i]);
	i_elFlags.m_elFlags = [];

	if( i_status && i_status.flags )
		for( var i = 0; i < i_status.flags.length; i++)
		{
			var el = document.createElement('div');
			i_elFlags.appendChild( el);
			i_elFlags.m_elFlags.push( el);
			el.classList.add('flag');
			el.textContent = c_GetFlagTitle( i_status.flags[i]);
			el.title = c_GetFlagTip( i_status.flags[i]);

			var clr = null;
			if( RULES.flags[i_status.flags[i]] && RULES.flags[i_status.flags[i]].clr )
				clr = RULES.flags[i_status.flags[i]].clr;
			if( clr )
				st_SetElColor({"color":clr}, el);
		}
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

	var c = null;
	if( i_status && i_status.color)
	{
		c = i_status.color;
	}
	else if( i_status && i_status.flags && i_status.flags.length )
	{
		var flag = i_status.flags[i_status.flags.length-1];
		if( RULES.flags[flag] && RULES.flags[flag].clr )
			c = RULES.flags[flag].clr;
	}

	if( c )
	{ 
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
		if( localStorage.background && localStorage.background.length )
			i_elBack.style.background = localStorage.background;
		else
			i_elBack.style.backgroundColor = u_backgroundColor;

		if( localStorage.text_color && localStorage.text_color.length )
			i_elColor.style.color = localStorage.text_color;
		else
			i_elColor.style.color = u_textColor;
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
Status.prototype.edit = function( i_args)
{
//console.log( JSON.stringify( i_args));
//console.log(JSON.stringify(i_status));
	if( g_auth_user == null )
	{
		c_Error('Guests can`t edit status.');
		return;
	}

	if( this.obj == null ) this.obj = {};

	// If editing element is exists, status is already in edit mode:
	if( this.elEdit ) return;

	// Hide status show element, if any:
	if( this.elShow ) this.elShow.style.display = 'none';

	// Create editing GUI:
	this.elParent.classList.add('status_editing');

	this.elEdit = document.createElement('div');
	this.elParent.appendChild( this.elEdit);
	this.elEdit.classList.add('status_edit');
	this.elEdit.onclick = function(e){e.stopPropagation();};
	this.elEdit.m_status = this;
	this.elEdit.onkeydown = function(e){ e.currentTarget.m_status.editOnKeyDown(e, i_args);};

	var elBtns = document.createElement('div');
	this.elEdit.appendChild( elBtns);
	elBtns.classList.add('buttons_div');

	var elBtnCancel = document.createElement('div');
	elBtns.appendChild( elBtnCancel);
	elBtnCancel.classList.add('button');
	elBtnCancel.textContent = 'Cancel';
	elBtnCancel.m_status = this;
	elBtnCancel.onclick = function(e){e.currentTarget.m_status.show();};

	var elBtnSave = document.createElement('div');
	elBtns.appendChild( elBtnSave);
	elBtnSave.classList.add('button');
	elBtnSave.textContent = 'Save';
	elBtnSave.m_status = this;
	elBtnSave.m_args = i_args;
	elBtnSave.onclick = function(e){e.currentTarget.m_status.editSave( i_args);};

	var elDiv = document.createElement('div');
	this.elEdit.appendChild(elDiv);
	elDiv.classList.add('status_edit_div');

	var elFinishDiv = document.createElement('div');
	elDiv.appendChild( elFinishDiv);
	elFinishDiv.classList.add('finish_div');
	var elFinishLabel = document.createElement('div');
	elFinishDiv.appendChild( elFinishLabel);
	elFinishLabel.classList.add('label');
	elFinishLabel.textContent = 'Fin:';
	elFinishLabel.onclick  = function(e){ e.currentTarget.m_elEdit_finish.textContent = c_DT_FormStrNow();};
	elFinishLabel.title = 'Click to set current date.';
	this.elEdit_finish = document.createElement('div');
	elFinishDiv.appendChild( this.elEdit_finish);
	this.elEdit_finish.classList.add('editing');
	this.elEdit_finish.contentEditable = 'true';
	elFinishLabel.m_elEdit_finish = this.elEdit_finish;

	var elProgressDiv = document.createElement('div');
	elDiv.appendChild( elProgressDiv);
	elProgressDiv.classList.add('progress_div');
	var elProgressLabel = document.createElement('div');
	elProgressDiv.appendChild( elProgressLabel);
	elProgressLabel.classList.add('label');
	elProgressLabel.textContent = '%';
	this.elEdit_progress = document.createElement('div');
	elProgressDiv.appendChild( this.elEdit_progress);
	this.elEdit_progress.contentEditable = 'true';
	this.elEdit_progress.classList.add('editing');
	this.elEdit_progress.onkeydown = function(e){ if( e.keyCode == 13 ) return false; };

	var elAnnDiv = document.createElement('div');
	elDiv.appendChild( elAnnDiv);
	elAnnDiv.classList.add('ann_div');
	var elAnnLabel = document.createElement('div');
	elAnnDiv.appendChild( elAnnLabel);
	elAnnLabel.classList.add('label');
	elAnnLabel.textContent = 'Annotation:';
	this.elEdit_annotation = document.createElement('div');
	elAnnDiv.appendChild( this.elEdit_annotation);
	this.elEdit_annotation.classList.add('editing');
	this.elEdit_annotation.contentEditable = 'true';

	var artists = {};
	var flags = {};
	var tags = {};

	if( this.obj.artists )
		for( var a = 0; a < this.obj.artists.length; a++)
			artists[this.obj.artists[a]] = {"title":c_GetUserTitle( this.obj.artists[a])};

	if( this.obj.flags )
		for( var a = 0; a < this.obj.flags.length; a++)
			flags[this.obj.flags[a]] = {"title":c_GetFlagTitle( this.obj.flags[a]),"tooltip":c_GetFlagTip( this.obj.flags[a])};

	if( this.obj.tags )
		for( var a = 0; a < this.obj.tags.length; a++)
			tags[this.obj.tags[a]] = {"title":c_GetTagTitle( this.obj.tags[a]),"tooltip":c_GetTagTip( this.obj.tags[a])};


	if( i_args && i_args.statuses )
		for( var s = 0; s < i_args.statuses.length; s++)
		{
			if( i_args.statuses[s].obj && i_args.statuses[s].obj.artists )
			for( var a = 0; a < i_args.statuses[s].obj.artists.length; a++)
			{
				for( var id in artists )
					if( i_args.statuses[s].obj.artists.indexOf( id) == -1 ) 
						artists[id].half = true;

				var id = i_args.statuses[s].obj.artists[a];
				if( artists[id] == null )
					artists[id] = {"title":c_GetUserTitle(id),"half":true};
			}
			else
				for( var id in artists ) artists[id].half = true;

			if( i_args.statuses[s].obj && i_args.statuses[s].obj.flags )
			for( var a = 0; a < i_args.statuses[s].obj.flags.length; a++)
			{
				for( var id in flags )
					if( i_args.statuses[s].obj.flags.indexOf( id) == -1 ) 
						flags[id].half = true;

				var id = i_args.statuses[s].obj.flags[a];
				if( flags[id] == null )
					flags[id] = {"title":c_GetFlagTitle(id),"half":true,"tooltip":c_GetFlagTip(id)};
			}
			else
				for( var id in flags ) flags[id].half = true;

			if( i_args.statuses[s].obj && i_args.statuses[s].obj.tags )
			for( var a = 0; a < i_args.statuses[s].obj.tags.length; a++)
			{
				for( var id in tags )
					if( i_args.statuses[s].obj.tags.indexOf( id) == -1 ) 
						tags[id].half = true;

				var id = i_args.statuses[s].obj.tags[a];
				if( tags[id] == null )
					tags[id] = {"title":c_GetTagTitle(id),"half":true,"tooltip":c_GetTagTip(id)};
			}
			else
				for( var id in tags ) tags[id].half = true;
		}

	if( c_CanAssignArtists())
		this.editListShow({"name":'artists',"label":'Artists:',"list":artists,"list_all":g_users,"elEdit":this.elEdit});

	this.editListShow({"name":'flags',"label":'Flags:',"list":flags,"list_all":RULES.flags,"elEdit":this.elEdit});
	this.editListShow({"name":'tags', "label":'Tags:', "list":tags, "list_all":RULES.tags, "elEdit":this.elEdit});

	this.elEdit_Color = document.createElement('div');
	this.elEdit.appendChild( this.elEdit_Color);
	this.elEdit_Color.classList.add('color');
	u_DrawColorBars({"el":this.elEdit_Color,"onclick":st_EditColorOnClick,"data":this});


	if( g_admin )
	{
		this.elEdit_adinfo = document.createElement('div');
		this.elEdit.appendChild( this.elEdit_adinfo);
		this.elEdit_adinfo.classList.add('adinfo');
		this.elEdit_adinfo.classList.add('editing');
		this.elEdit_adinfo.contentEditable = 'true';
	}


	this.elEdit_tasks = document.createElement('div');
	this.elEdit.appendChild( this.elEdit_tasks);
	this.elEdit_tasks.classList.add('edit_tasks');

	if( c_CanEditTasks())
	{
		this.elTasksPanel = document.createElement('div');
		this.elEdit_tasks.appendChild( this.elTasksPanel);

		var el = document.createElement('div');
		this.elTasksPanel.appendChild( el);
		el.classList.add('button');
		el.textContent = 'Tasks';
		el.m_args = i_args;
		el.m_status = this;
		el.onclick = function(e){ e.currentTarget.m_status.editTasksShow( e, e.currentTarget.m_args); }
	}


	// Get values:
	var annotation = this.obj.annotation;
	var adinfo = this.obj.adinfo;
	var progress = this.obj.progress;
	var finish = this.obj.finish;

	if( i_args && i_args.statuses && i_args.statuses.length )
	{
		// Several statuses (shots) selected:
		annotation = this.getMultiVale('annotation', i_args.statuses);
		progress = this.getMultiVale('progress', i_args.statuses);
		finish = this.getMultiVale('finish', i_args.statuses);
	}

	// Set values:
	if( annotation != null )
	{
		this.elEdit_annotation.textContent = annotation;
	}
	if( g_admin && ( adinfo != null ))
	{
		this.elEdit_adinfo.textContent = atob( adinfo);
	}
	if( finish != null )
	{
		if( finish != st_MultiValue ) finish = c_DT_FormStrFromSec( finish);
		this.elEdit_finish.textContent = finish;
	}
	if( progress != null )
	{
		this.elEdit_progress.textContent = progress;
	}

	this.elEdit_annotation.focus();
}
Status.prototype.editTasksShow = function( i_evt, i_args)
{
	// Show tasks only once:
	if( this.elEdit_tasks.elTasks ) return;

	// And remember it creating an array for task elements:
	this.elEdit_tasks.elTasks = [];

	var elTasksBtn = i_evt.currentTarget;
	elTasksBtn.classList.remove('button');

	var el = document.createElement('div');
	this.elTasksPanel.insertBefore( el, elTasksBtn);
	el.textContent = 'Add Task';
	el.classList.add('button');
	el.classList.add('task_add');
	el.m_status = this;
	el.onclick = function(e){ e.currentTarget.m_status.addTaskOnClick()};

	// If this is scene shots multiselection:
	if( i_args && i_args.statuses && i_args.statuses.length )
	{
		var el = document.createElement('div');
		this.elTasksPanel.appendChild( el);
		el.textContent = 'WARNING! This is scene shots multiselection, all previous tasks information will be dropped!';
		el.classList.add('tasks_multiedit_message');
		return;
	}

	this.editTasksShowTasks();
}
Status.prototype.editOnKeyDown = function(e, i_args)
{
	if( e.keyCode == 27 ) this.show();                 // ESC
	if( e.keyCode == 13 ) this.editSave( i_args);      // ENTER
}
function st_EditColorOnClick( i_clr, i_data)
{
	i_data.elEdit_Color.m_color = i_clr;
	i_data.elEdit_Color.m_color_changed = true
	st_SetElColor({"color": i_clr}, i_data.elColor);
}
Status.prototype.getMultiVale = function( i_key, i_statuses)
{
	if( i_statuses.indexOf( this) == -1 ) i_statuses.push( this);

	var value = this.obj[i_key];

	if(( i_statuses.length == 1 ) && ( i_statuses[0] == this ))
		return value;

	for( var i = 0; i < i_statuses.length; i++)
	{
		var other = null;
		if(( i_statuses[i].obj ) && ( i_statuses[i].obj[i_key] ))
			other = i_statuses[i].obj[i_key];

		if( value != other )
		{
			value = st_MultiValue;
			return value;
		}
	}
	return value;
}
Status.prototype.editListShow = function( i_args)
{
	var elRoot = document.createElement('div');
	i_args.elEdit.appendChild( elRoot);
	i_args.elRoot = elRoot;
	elRoot.classList.add('list');
	elRoot.classList.add( i_args.name);
	if( localStorage.background && localStorage.background.length )
		elRoot.style.background = localStorage.background;
	else
		elRoot.style.backgroundColor = u_backgroundColor;
	if( localStorage.text_color && localStorage.text_color.length )
		elRoot.style.color = localStorage.text_color;
	else
		elRoot.style.color = u_textColor;

	elRoot.m_elBtn = document.createElement('div');
	elRoot.appendChild( elRoot.m_elBtn);
	elRoot.m_elBtn.classList.add('button');
	elRoot.m_elBtn.textContent = i_args.label;
	elRoot.m_elBtn.m_status = this;
	elRoot.m_elBtn.onclick = function(e){ e.currentTarget.m_status.editListEdit( i_args);};
	elRoot.m_elList = document.createElement('div');
	elRoot.appendChild( elRoot.m_elList);
	for( var id in i_args.list )
	{
		var el = document.createElement('div');
		elRoot.m_elList.appendChild( el);
		el.textContent = i_args.list[id].title;
		el.classList.add('tag');
		if( i_args.name == 'flags' )
		{
			el.classList.add('flag');
			if( RULES.flags[id] )
				st_SetElColor({"color":RULES.flags[id].clr}, el);
		}
		if( i_args.name == 'artists' )
		{
			el.classList.add('artist');
			if( id == g_auth_user.id )
				el.classList.add('me');
		}

		var icon = null;
		if( i_args.name == 'artists')
			icon = c_GetAvatar( id);
		if( icon)
		{
			el.classList.add('with_icon');
			el.style.backgroundImage = 'url(' + icon + ')';
		}

		if( i_args.list_all[id] && i_args.list_all[id].disabled )
			el.classList.add('disabled');

		if( i_args.list[id].tooltip ) el.title = i_args.list[id].tooltip;

		if( i_args.list[id].half )
			el.classList.add('half_selected');
		else
			el.classList.add('selected');
	}
}
Status.prototype.editListEdit = function( i_args)
{
	if( i_args.elRoot.m_edit ) return;
	i_args.elRoot.m_edit = true;
	i_args.elRoot.m_elBtn.classList.remove('button');
	i_args.elRoot.m_elList.style.display = 'none';
	i_args.elRoot.classList.add('edit');

	i_args.elEdit[i_args.name] = [];

	if( i_args.name == 'artists')
	{
		this.editArtistsEdit( i_args);
		return;
	}

	for( var item in i_args.list_all)
	{
		var el = document.createElement('div');
		i_args.elRoot.appendChild( el);
		el.classList.add('tag');
		if( i_args.name == 'flags' )
		{
			el.classList.add('flag');
			if( RULES.flags[item] && RULES.flags[item].clr )
			{
				var c = RULES.flags[item].clr;
				el.style.borderColor = 'rgb('+c[0]+','+c[1]+','+c[2]+')';
			}
		}
		el.m_item = item;

		if( i_args.list_all[item].title )
			el.textContent = i_args.list_all[item].title;
		else
			el.textContent = item;

		if( i_args.list_all[item].tip )
			el.title = i_args.list_all[item].tip;

		if( i_args.list[item] )
		{
			if( i_args.list[item].half )
			{
				el.m_half_selected = true;
				el.classList.add('half_selected');
			}
			else
			{
				el.m_selected = true;
				el.classList.add('selected');
			}
		}

		el.onclick = status_elToggleSelection;
		i_args.elEdit[i_args.name].push( el);
	}
}
Status.prototype.editArtistsEdit = function( i_args)
{
	var roles = c_GetRolesArtists( i_args.list);

	for( var r = 0; r < roles.length; r++)
	{
		var elRole = document.createElement('div');
		i_args.elRoot.appendChild( elRole);
		elRole.classList.add('role');

		var elLabel = document.createElement('div');
		elRole.appendChild( elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = roles[r].role + ':';

		for( var a = 0; a < roles[r].artists.length; a++)
		{
			var artist = roles[r].artists[a];

			var el = document.createElement('div');
			elRole.appendChild( el);
			el.classList.add('tag');
			el.classList.add('artist');
			el.m_item = artist.id;
			if( artist.id == g_auth_user.id )
				el.classList.add('me');

			if( g_users[artist.id] && g_users[artist.id].disabled )
				el.classList.add('disabled');

			if( artist.title )
				el.textContent = artist.title;
			else
				el.textContent = artist.id;

			if( artist.tip )
				el.title = artist.tip;

			var avatar = c_GetAvatar( artist.id);
			if( avatar)
			{
				el.classList.add('with_icon');
				el.style.backgroundImage = 'url(' + avatar + ')';
			}

			if( i_args.list[artist.id] )
			{
				if( i_args.list[artist.id].half )
				{
					el.m_half_selected = true;
					el.classList.add('half_selected');
				}
				else
				{
					el.m_selected = true;
					el.classList.add('selected');
				}
			}

			el.onclick = status_elToggleSelection;
			i_args.elEdit[i_args.name].push( el);
		}
	}
}
function status_elToggleSelection( e)
{
	var el = e.currentTarget;
	if( el.m_selected )
	{
		el.m_selected = false;
		el.classList.remove('selected');
	}
	else if( el.classList.contains('half_selected'))
	{
		el.m_selected = true;
		el.classList.add('selected');
		el.classList.remove('half_selected');
	}
	else if( el.m_half_selected )
	{
		el.classList.add('half_selected');
	}
	else
	{
		el.m_selected = true;
		el.classList.add('selected');
	}
}

Status.prototype.addTaskOnClick = function()
{
	if( this.obj.tasks == null )
		this.obj.tasks = [];

	var task = {};
	task.duration = 1;
	task.price = 0;
	task.tags = [];
	task.artists = [];

	this.editTasksShowTasks({"new":task});
}

Status.prototype.editTasksShowTasks = function( i_args)
{
	var tasks = this.obj.tasks;
	if( i_args && i_args.new )
		tasks = [i_args.new]

	if( tasks == null ) return;

	for( var t = 0; t < tasks.length; t++)
	{
		var el = document.createElement('div');
		this.elEdit_tasks.appendChild( el);
		el.classList.add('task');

		var elDel = document.createElement('div');
		el.appendChild( elDel);
		elDel.classList.add('button');
		elDel.classList.add('delete');
		//elDel.textContent = 'Delete';
		elDel.title = 'Delete Task\n(by double click)';
		elDel.m_status = this;
		elDel.m_elTask = el;
		elDel.ondblclick = function(e){
			var st = e.currentTarget.m_status;
			var el = e.currentTarget.m_elTask;
			st.elEdit_tasks.elTasks.splice( st.elEdit_tasks.elTasks.indexOf(el),1);
			st.elEdit_tasks.removeChild( el);
		}

		var elDurDiv = document.createElement('div');
		el.appendChild( elDurDiv);
		elDurDiv.classList.add('dur_div');

		var elDurLabel = document.createElement('div');
		elDurDiv.appendChild( elDurLabel);
		elDurLabel.textContent = 'Duration: ';
		elDurLabel.classList.add('dur_label');

		var elDur = document.createElement('div');
		elDurDiv.appendChild( elDur);
		elDur.textContent = tasks[t].duration;
		elDur.contentEditable = true;
		elDur.classList.add('editing');
		elDur.classList.add('duration');

		var elPrcDiv = document.createElement('div');
		el.appendChild( elPrcDiv);
		elPrcDiv.classList.add('prc_div');
		elPrcDiv.classList.add('dur_div');

		var elPrcLabel = document.createElement('div');
		elPrcDiv.appendChild( elPrcLabel);
		elPrcLabel.textContent = 'Price: ';
		elPrcLabel.classList.add('prc_label');
		elPrcLabel.classList.add('dur_label');

		var elPrice = document.createElement('div');
		elPrcDiv.appendChild( elPrice);
		elPrice.textContent = tasks[t].price;
		elPrice.contentEditable = true;
		elPrice.classList.add('editing');
		elPrice.classList.add('price');
		elPrice.classList.add('duration');

		var tags = {};
		if( tasks[t].tags )
		for( var g = 0; g < tasks[t].tags.length; g++)
		{
			var id = tasks[t].tags[g];
			tags[id] = {"title":c_GetTagTitle(id),"tooltip":c_GetTagTip(id)};
		}
		var elTags = document.createElement('div');
		elTags.classList.add('tags');
		el.appendChild( elTags);
		this.editListShow({"name":'tags',"label":'Tags:',"list":tags,"list_all":RULES.tags,"elEdit":elTags});

		if( c_CanAssignArtists())
		{
			var artists = {};
			if( tasks[t].artists )
			{
				for( var g = 0; g < tasks[t].artists.length; g++)
				{
					var id = tasks[t].artists[g];
					artists[id] = {"title":c_GetUserTitle(id)};
				}
			}
			var elArtists = document.createElement('div');
			el.appendChild( elArtists);
			this.editListShow({"name":'artists',"label":'Artists:',"list":artists,"list_all":g_users,"elEdit":elArtists});
		}

		el.m_task = tasks[t];
		el.m_elDur = elDur;
		el.m_elPrice = elPrice;
		el.m_elTags = elTags;
		el.m_elArtists = elArtists;
		this.elEdit_tasks.elTasks.push( el);
	}
}

Status.prototype.editCancel = function()
{
	if( this.elEdit == null ) return;

	if( this.elParent )
	{
		this.elParent.removeChild( this.elEdit);
		this.elParent.classList.remove('status_editing');
	}
	this.elEdit = null;
}

Status.prototype.editSave = function( i_args)
{
	if( this.obj == null ) this.obj = {};

	var finish = null;
	var annotation = null;
	var adinfo = null;
	var progress = null;
	var artists = null;
	var flags = null;
	var tags = null;
	var tasks = null;

	// Get values from GUI:

	var finish_edit = this.elEdit_finish.textContent;
	if( finish_edit.length && ( finish_edit != st_MultiValue ))
	{
		finish_edit = c_DT_SecFromStr( finish_edit);
		if( finish_edit != 0 )
			finish = finish_edit;
		else
			c_Error('Invalid date format: ' + this.elEdit_finish.textContent);
	}

	var progress_edit = this.elEdit_progress.textContent;
	if( progress_edit.length && ( progress_edit != st_MultiValue ))
	{
		progress_edit = c_Strip( progress_edit);
		progress = parseInt( progress_edit);
		if( isNaN( progress) )
		{
			progress = null;
			c_Error('Invalid progress: ' + c_Strip( progress_edit));
		}
		if( progress <  -1 ) progress = -1;
		if( progress > 100 ) progress = 100;
	}

	if( this.elEdit_annotation.textContent != st_MultiValue )
	{
		this.elEdit_annotation.innerHTML = this.elEdit_annotation.textContent;
		annotation = c_Strip( this.elEdit_annotation.textContent);
	}

	if( g_admin )
	{
		this.elEdit_adinfo.innerHTML = this.elEdit_adinfo.textContent;
		adinfo = c_Strip( this.elEdit_adinfo.textContent);
		adinfo = btoa( adinfo);
	}

	if( this.elEdit.artists )
	{
		artists = {};
		var elList = this.elEdit.artists;
		for( var i = 0; i < elList.length; i++)
		{
			if( elList[i].m_selected )
				artists[elList[i].m_item] = 'selected';
			else if( elList[i].m_half_selected )
				artists[elList[i].m_item] = 'half_selected';
		}
	}

	if( this.elEdit.flags )
	{
		flags = {};
		var elList = this.elEdit.flags;
		for( var i = 0; i < elList.length; i++)
		{
			if( elList[i].m_selected )
				flags[elList[i].m_item] = 'selected';
			else if( elList[i].m_half_selected )
				flags[elList[i].m_item] = 'half_selected';
		}
	}

	if( this.elEdit.tags )
	{
		tags = {};
		var elList = this.elEdit.tags;
		for( var i = 0; i < elList.length; i++)
		{
			if( elList[i].m_selected )
				tags[elList[i].m_item] = 'selected';
			else if( elList[i].m_half_selected )
				tags[elList[i].m_item] = 'half_selected';
		}
	}

	if( this.elEdit_tasks.elTasks )
	{
		tasks = [];

		for( var t = 0; t < this.elEdit_tasks.elTasks.length; t++)
		{
			var elTask = this.elEdit_tasks.elTasks[t];
			var task = {};

			var duration = parseFloat( c_Strip( elTask.m_elDur.textContent ));
			if( ! isNaN( duration ))
				task.duration = duration;

			var price = parseFloat( c_Strip( elTask.m_elPrice.textContent ));
			if( ! isNaN( price ))
				task.price = price;

			if( elTask.m_elTags.tags)
			{
				task.tags = [];
				elList = elTask.m_elTags.tags;
				for( var i = 0; i < elList.length; i++)
					if( elList[i].m_selected )
					{
						var tag = elList[i].m_item;
						task.tags.push( tag);
					}
			}
			else if( elTask.m_task.tags )
				task.tags = elTask.m_task.tags;

			if( elTask.m_elArtists && elTask.m_elArtists.artists)
			{
				task.artists = [];
				elList = elTask.m_elArtists.artists;
				for( var i = 0; i < elList.length; i++)
					if( elList[i].m_selected )
					{
						var artist = elList[i].m_item;
						task.artists.push( artist);
					}
			}
			else if( elTask.m_task.artists )
				task.artists = elTask.m_task.artists;

			tasks.push( task);
		}
	}

	// Collect statuses to change
	// ( this and may be others selected )
	var statuses = [this];
	if( i_args && i_args.statuses && i_args.statuses.length )
	{
		statuses = i_args.statuses;
		if( statuses.indexOf( this) == -1 )
			statuses.push( this);
	}

	// Set values to statuses
	var some_progress_changed = false;
	var progresses = {};

	for( var i = 0; i < statuses.length; i++)
	{
		if( statuses[i].obj == null ) statuses[i].obj = {};

		if( annotation !== null ) statuses[i].obj.annotation = annotation;
		if( adinfo     !== null ) statuses[i].obj.adinfo     = adinfo;
		if( finish     !== null ) statuses[i].obj.finish     = finish;
		if( progress   !== null )
		{
			progresses[statuses[i].path] = progress;
			if( progress != statuses[i].obj.progress )
				some_progress_changed = true;
			statuses[i].obj.progress = progress;
		}

		if( artists )
		{
			if( statuses[i].obj.artists == null )
				statuses[i].obj.artists = [];

			for( var a = 0; a < statuses[i].obj.artists.length; )
				if( artists[statuses[i].obj.artists[a]] == null )
					statuses[i].obj.artists.splice(a,1);
				else a++;

			for( var id in artists )
				if(( artists[id] == 'selected' ) && ( statuses[i].obj.artists.indexOf(id) == -1 ))
					statuses[i].obj.artists.push( id);
		}

		if( flags )
		{
			if( statuses[i].obj.flags == null )
				statuses[i].obj.flags = [];

			for( var a = 0; a < statuses[i].obj.flags.length; )
				if( flags[statuses[i].obj.flags[a]] == null )
					statuses[i].obj.flags.splice(a,1);
				else a++;

			// Store existing flags to check was it ON before:
			var _flags = [];
			for( var a = 0; a < statuses[i].obj.flags.length; a++)
				_flags.push(statuses[i].obj.flags[a]);

			for( var id in flags )
				if(( flags[id] == 'selected' ) && ( _flags.indexOf(id) == -1 ))
				{
					if( RULES.flags[id])
					{
						// Flag can limit minium and maximum progress percentage:
						var p_min = RULES.flags[id].p_min;
						var p_max = RULES.flags[id].p_max;
						var progress = statuses[i].obj.progress;

						if( p_min && (( progress == null ) || ( progress < p_min )))
							progress = p_min;

						if( p_max && (( p_max < 0 ) || ( progress > p_max )))
							progress = p_max;

						if( progress != null )
						{
							statuses[i].obj.progress = progress;

							// This needed to update upper pogresses:
							progresses[statuses[i].path] = progress;
							some_progress_changed = true;
						}

						// Flag can be exclusive, so we should delete other flags:
						if( RULES.flags[id].excl )
							statuses[i].obj.flags = [];
					}

					statuses[i].obj.flags.push( id);
				}
		}

		if( tags )
		{
			if( statuses[i].obj.tags == null )
				statuses[i].obj.tags = [];

			for( var a = 0; a < statuses[i].obj.tags.length; )
				if( tags[statuses[i].obj.tags[a]] == null )
					statuses[i].obj.tags.splice(a,1);
				else a++;

			for( var id in tags )
				if(( tags[id] == 'selected' ) && ( statuses[i].obj.tags.indexOf(id) == -1 ))
					statuses[i].obj.tags.push( id);
		}

		if( tasks )
		{
			var duration = 0;
			var price = 0;
			for( var t = 0; t < tasks.length; t++)
			{
				if( tasks[t].duration )
					duration += tasks[t].duration;
				if( tasks[t].price )
					price += tasks[t].price;

				// Add tag to status:
				if( tasks[t].tags && tasks[t].tags.length )
				{
					if( statuses[i].obj.tags == null ) 
						statuses[i].obj.tags = [];

					for( var j = 0; j < tasks[t].tags.length; j++)
					{
						var tag = tasks[t].tags[j];

						if( statuses[i].obj.tags.indexOf( tag) == -1 )
							statuses[i].obj.tags.push( tag);
					}
				}

				// Add artist to status:
				if( tasks[t].artists && tasks[t].artists.length )
				{
					if( statuses[i].obj.artists == null )
						statuses[i].obj.artists = [];

					for( var j = 0; j < tasks[t].artists.length; j++)
					{
						var artist = tasks[t].artists[j];

						if( statuses[i].obj.artists.indexOf( artist) == -1 )
							statuses[i].obj.artists.push( artist);
					}
				}
			}
			
			statuses[i].obj.tasks = tasks;
			statuses[i].obj.duration = duration;
			statuses[i].obj.price = price;

		}

		if( this.elEdit_Color.m_color_changed )
			statuses[i].obj.color = this.elEdit_Color.m_color;

		statuses[i].save();
		statuses[i].show();
		//^ Status showing causes values redraw,
		// and destoys edit GUI if any.
	}

	// News & Bookmarks:
	nw_StatusesChanged( statuses);

	if( some_progress_changed )
		st_UpdateProgresses( this.path, progresses);

	c_Info('Status(es) saved.');
}

Status.prototype.save = function()
{
	this.obj.muser = g_auth_user.id;
	this.obj.mtime = c_DT_CurSeconds();

	if( this.path == g_CurPath())
		RULES.status = this.obj;

	st_Save( this.obj, this.path);
}

function st_Save( i_status, i_path, i_func, i_args, i_navig_params_update)
{
	if( i_status == null ) i_status = RULES.status;
	if( i_path == null ) i_path = g_CurPath();

	g_FolderSetStatusPath( i_status, i_path, i_navig_params_update);
	n_walks[i_path] = null;

	var obj = {};
	obj.object = {"status":i_status};
	obj.add = true;
	obj.file = c_GetRuFilePath('status.json', i_path);

	n_Request({"send":{"editobj":obj},"func":i_func,"args":i_args,"wait":false,'info':'status save'});
}

function st_SetFramesNumber( i_num)
{
	if( RULES.status == null ) RULES.status = {};

	if( RULES.status.frames_num == i_num )
		return;

	RULES.status.frames_num = i_num;
	st_Save();
	st_Show( RULES.status);

	$('status_framesnum_div').classList.add('updated');
}

function st_SetTimeCode( i_tc)
{
	if(( i_tc == null ) || ( i_tc.length < 1 ))
	{
		return;
	}

	var tc = i_tc;
	tc = tc.split('-');
	if( tc.length != 2 )
	{
		c_Error('Invalid time code: ' + i_tc );
		return;
	}

	var frame_start = c_TC_FromSting(tc[0].replace());
	if( frame_start === null )
	{
		c_Error('Invalid start time code: ' + tc[0]);
		return;
	}

	var frame_finish = c_TC_FromSting(tc[1]);
	if( frame_finish === null )
	{
		c_Error('Invalid finish time code: ' + tc[1]);
		return;
	}
	var timecode_start = c_TC_FromFrame( frame_start);

	var frames_num = frame_finish - frame_start + 1;
	if( frames_num <= 0 )
	{
		c_Error('Start time code is grater than finish: ' + tc[1]);
		return;
	}
	var timecode_finish = c_TC_FromFrame( frame_finish);

	if( RULES.status == null ) RULES.status = {};

	if(( RULES.status.timecode_start == timecode_start ) && ( RULES.status.timecode_finish == timecode_finish ))
		return;

	RULES.status.timecode_start = timecode_start;
	RULES.status.timecode_finish = timecode_finish;

	var save_fields = {};
	save_fields.timecode_start = timecode_start;
	save_fields.timecode_finish = timecode_finish;

	var navig_params_update = {};
		
	if(( RULES.status.frames_num == null ) || ( RULES.status.frames_num <= 0 ))
	{
		RULES.status.frames_num = frames_num;
		save_fields.frames_num = frames_num;
		navig_params_update.frames_num = true;

		st_SetElFramesNum( RULES.status, $('status_framesnum'));
	}

	st_Save( save_fields, null, null, null, navig_params_update);
}

function st_UpdateProgresses( i_path, i_progresses)
{
	var paths = [];
	var progresses = {};
	if( i_progresses ) progresses = i_progresses;

	var paths_skip_save = [];
	// Skip saving of provided progresses:
	// ( as they provided after edit and already saved )
	for( var path in progresses ) paths_skip_save.push(path);

	var folders = i_path.split('/');
	var path = '';
	for( var i = 1; i < folders.length-1; i++)
	{
		path += '/'+folders[i];
		paths.push( path);
	}
//console.log( paths);
//console.log(JSON.stringify(i_progresses));
	n_WalkDir({"paths":paths,"wfunc":st_UpdateProgressesWalkReceived,
		"progresses":progresses,"paths_skip_save":paths_skip_save,
		"info":'walk upstatuses',"rufiles":['status'],"lookahead":['status']});
}
function st_UpdateProgressesWalkReceived( i_walks, i_args)
{
	if( i_walks == null ) return;

//console.log(JSON.stringify(i_args));
	var paths = i_args.paths;
	var progresses = i_args.progresses;

	// Update only progess in navig:
	var navig_params_update = {};
	navig_params_update.progress = true;

	for( var w = i_walks.length-1; w >= 0; w--)
	{
//console.log(JSON.stringify(i_walks[w]));
		if( i_walks[w].error )
		{
			c_Error( i_walks[w].error);
			return;
		}
		if(( i_walks[w].folders == null ) || ( i_walks[w].folders.length == 0 ))
		{
			c_Error('Can`t find folders in ' + paths[w]);
			return;
		}

		var progress = 0;
		var progress_count = 0;
		for( var f = 0; f < i_walks[w].folders.length; f++ )
		{
			var folder = i_walks[w].folders[f];

			if( folder.name == RULES.rufolder ) continue;
			if( c_AuxFolder( folder)) continue;

			var path = paths[w] + '/' + folder.name;
			if(( progresses[path] != null ) && ( progresses[path] != -1 ))
			{
				progress += progresses[path];
			}
			else
			{
				if(( folder.status == null ) || ( folder.status.progress == null ))
				{
				// Here we set and save 0% progress on a neighbour folders,
				// if status or progress is not set at all:

					// Siblings are only at last walk ( earlier are parents )
					if( w != (i_walks.length-1)) continue;

					// Save only progress:
					st_Save({"progress":0}, path,/*func=*/null,/*args=*/null, navig_params_update);
				}
				else if( folder.status.progress < 0 )
					continue;
				else
					progress += folder.status.progress;
			}
			progress_count++;
		}

		if( progress_count == 0 )
		{
			return;
		}

		progress = Math.floor( progress / progress_count);
		progresses[paths[w]] = progress;

//console.log(paths[w]+': '+progress_count+': '+progress);
	}

	for( var path in progresses)
	{
		if( i_args.paths_skip_save.indexOf(path) != -1 )
			continue;

		st_Save({"progress":progresses[path]}, path,/*func=*/null,/*args=*/null, navig_params_update);
//console.log(path);
	}
}

