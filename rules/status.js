st_Status = null;
st_MultiValue = '[...]';

function st_InitAuth()
{
	$('status_edit_btn').style.display = 'block';
}

function st_Finish()
{
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
		this.elProgress    = $('status_progress');
		this.elProgressBar = $('status_progressbar');
		this.elPercentage  = $('status_percentage');
		this.elArtists     = $('status_artists');
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

//	if( this.elTasks )
//		this.showTasks();

	var args = {};
	args.statuses = [this.obj];
	args.elReports = this.elReports;
	args.elReportsDiv = this.elReportsDiv;
	args.elTasks = this.elTasks;
	args.elTasksDiv = this.elTasksDiv;

	stcs_Show( args);
}
Status.prototype.showTasks = function()
{
	this.elTasks.textContent = '';
	if(( this.obj == null )
	|| ( this.obj.tasks == null )
	|| ( this.obj.tasks.length == 0 ))
	{
		this.elTasksDiv.style.display = 'none';
		return;
	}
	else
		this.elTasksDiv.style.display = 'block';

	var total_duration = 0;
	for( var t = 0; t < this.obj.tasks.length; t++)
	{
		var task = this.obj.tasks[t];

		var el = document.createElement('div');
		this.elTasks.appendChild( el);
		el.classList.add('task');

		var elDur = document.createElement('div');
		el.appendChild( elDur);
		elDur.classList.add('duration');
		elDur.textContent = task.duration;

		if( task.tags && task.tags.length )
		{
			var elTags = document.createElement('div');
			el.appendChild( elTags);
			elTags.classList.add('tags');

			for( var g = 0; g < task.tags.length; g++)
			{
				var elTag = document.createElement('div');
				elTags.appendChild( elTag);
				elTag.classList.add('tag');
				elTag.textContent = c_GetTagTitle( task.tags[g]);
			}
		}

		if( task.artists && task.artists.length )
		{
			var elTags = document.createElement('div');
			el.appendChild( elTags);
			elTags.classList.add('artists');

			for( var g = 0; g < task.artists.length; g++)
			{
				var elTag = document.createElement('div');
				elTags.appendChild( elTag);
				elTag.classList.add('tag');
				elTag.textContent = c_GetUserTitle( task.artists[g]);
			}
		}

		total_duration += task.duration;
	}

	var el = document.createElement('div');
	this.elTasks.appendChild( el);
	el.textContent = 'Total Duration: ' + total_duration;
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
		el.textContent = c_GetUserTitle( i_status.artists[i], null, i_short);

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
Status.prototype.edit = function( i_args)
{
//console.log( JSON.stringify( i_args));
//console.log(JSON.stringify(i_status));
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
	elBtns.style.cssFloat = 'right';

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
	this.elEdit_progress.onkeydown = function(e){ if( e.keyCode == 13 ) return false; };

	var artists = {};
	var tags = {};

	if( this.obj.artists )
		for( var a = 0; a < this.obj.artists.length; a++)
			artists[this.obj.artists[a]] = {"title":c_GetUserTitle( this.obj.artists[a])};

	if( this.obj.tags )
		for( var a = 0; a < this.obj.tags.length; a++)
			tags[this.obj.tags[a]] = {"title":c_GetTagTitle( this.obj.tags[a]),"tooltip":c_GetTagTip( this.obj.tags[a])};


	if( i_args && i_args.statuses )
		for( var s = 0; s < i_args.statuses.length; s++)
		{
			for( var id in artists ) artists[id].half = true;
			for( var id in tags    ) tags[id].half    = true;

			if( i_args.statuses[s].obj && i_args.statuses[s].obj.artists )
			for( var a = 0; a < i_args.statuses[s].obj.artists.length; a++)
			{
				var id = i_args.statuses[s].obj.artists[a];
				if( artists[id] )
					artists[id].half = false;
				else
					artists[id] = {"title":c_GetUserTitle(id),"half":true};
			}

			if( i_args.statuses[s].obj && i_args.statuses[s].obj.tags )
			for( var a = 0; a < i_args.statuses[s].obj.tags.length; a++)
			{
				var id = i_args.statuses[s].obj.tags[a];
				if( tags[id] )
					tags[id].half = false;
				else
					tags[id] = {"title":c_GetTagTitle(id),"half":true,"tooltip":c_GetTagTip(id)};
			}
		}

	this.editListShow({"name":'artists',"label":'Artists:',"list":artists,"list_all":g_users,   "elEdit":this.elEdit});
	this.editListShow({"name":'tags',   "label":'Tags:',   "list":tags,   "list_all":RULES.tags,"elEdit":this.elEdit});

	this.elEdit_Color = document.createElement('div');
	this.elEdit.appendChild( this.elEdit_Color);
	this.elEdit_Color.classList.add('color');
	u_DrawColorBars({"el":this.elEdit_Color,"onclick":st_EditColorOnClick,"data":this});


	this.elEdit_tasks = document.createElement('div');
	this.elEdit.appendChild( this.elEdit_tasks);
	this.elEdit_tasks.classList.add('edit_tasks');

	var elTasksPanel = document.createElement('div');
	this.elEdit_tasks.appendChild( elTasksPanel);

	var el = document.createElement('div');
	elTasksPanel.appendChild( el);
	el.textContent = 'Add';
	el.classList.add('button');
	el.style.cssFloat = 'right';
	el.m_status = this;
	el.onclick = function(e){ e.currentTarget.m_status.addTaskOnClick()};

	var el = document.createElement('div');
	elTasksPanel.appendChild( el);
	el.textContent = 'Tasks:';

	this.editTasksShow();


	// Get values:
	var annotation = this.obj.annotation;
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
		var other = i_statuses[i].obj[i_key];
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
	elRoot.m_elBtn = document.createElement('div');
	elRoot.appendChild( elRoot.m_elBtn);
	elRoot.m_elBtn.classList.add('button');
	elRoot.m_elBtn.style.cssFloat = 'left';;
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
	i_args.elRoot.m_elBtn.style.display = 'none';

	var roles_obj = {};
	for( var item in i_args.list_all)
	{
		var role = i_args.list_all[item].role;

		if( roles_obj[role] == null )
			roles_obj[role] = [];

		roles_obj[role].push( i_args.list_all[item]);
	}

	var roles = [];
	for( var role in roles_obj )
	{
		roles_obj[role].sort(function(a,b){return a.title > b.title});
		roles.push({"role":role,"artists":roles_obj[role]});
	}
	roles.sort(function(a,b){return a.role < b.role});

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
			el.m_item = artist.id;
			if( artist.title )
				el.textContent = artist.title;
			else
				el.textContent = artist.id;

			if( artist.tip )
				el.title = artist.tip;

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
	task.tags = [];
	task.artists = [];

	this.editTasksShow({"new":task});
}

Status.prototype.editTasksShow = function( i_args)
{
	var tasks = this.obj.tasks;
	if( i_args && i_args.new )
		tasks = [i_args.new]

	if( tasks == null ) return;

	if( this.elEdit_tasks.elTasks == null )
		this.elEdit_tasks.elTasks = [];

	for( var t = 0; t < tasks.length; t++)
	{
		var el = document.createElement('div');
		this.elEdit_tasks.appendChild( el);
		el.classList.add('task');

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

		var elDel = document.createElement('div');
		el.appendChild( elDel);
		elDel.classList.add('button');
		elDel.textContent = '-';
		elDel.title = 'Delete Task\n(by double click)';
		elDel.m_status = this;
		elDel.m_elTask = el;
		elDel.ondblclick = function(e){
			var st = e.currentTarget.m_status;
			var el = e.currentTarget.m_elTask;
			st.elEdit_tasks.elTasks.splice( st.elEdit_tasks.elTasks.indexOf(el),1);
			st.elEdit_tasks.removeChild( el);
		}

		var tags = {};
		if( tasks[t].tags )
		for( var g = 0; g < tasks[t].tags.length; g++)
		{
			var id = tasks[t].tags[g];
			tags[id] = {"title":c_GetTagTitle(id),"tooltip":c_GetTagTip(id)};
		}
		var elTags = document.createElement('div');
		el.appendChild( elTags);
		this.editListShow({"name":'tags',"label":'Tags:',"list":tags,"list_all":RULES.tags,"elEdit":elTags});

		var artists = {};
		if( tasks[t].artists )
		for( var g = 0; g < tasks[t].artists.length; g++)
		{
			var id = tasks[t].artists[g];
			artists[id] = {"title":c_GetUserTitle(id)};
		}
		var elArtists = document.createElement('div');
		el.appendChild( elArtists);
		this.editListShow({"name":'artists',"label":'Artists:',"list":artists,"list_all":g_users,"elEdit":elArtists});

		el.m_task = tasks[t];
		el.m_elDur = elDur;
		el.m_elTags = elTags;
		el.m_elArtists = elArtists;
		this.elEdit_tasks.elTasks.push( el);
	}
}

Status.prototype.editSave = function( i_args)
{
	if( this.obj == null ) this.obj = {};

	var finish = null;
	var annotation = null;
	var progress = null;
	var artists = null;
	var tags = null;

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
		annotation = c_Strip( this.elEdit_annotation.innerHTML);
	}

	if( this.elEdit.artists )
	{
		artists = {};
		var elList = this.elEdit.artists;
		for( var i = 0; i < elList.length; i++)
		{
			if( elList[i].m_selected )
				artists[elList[i].m_item] = 'selected';
			else if( elList[i].classList.contains('half_selected'))
				artists[elList[i].m_item] = 'half';
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
			else if( elList[i].classList.contains('half_selected'))
				tags[elList[i].m_item] = 'half';
		}
	}

	if( this.elEdit_tasks.elTasks )
	{
		// Task are set to the current status only.
		// Mutli selection edit is not supported.

		this.obj.tasks = [];

		for( var t = 0; t < this.elEdit_tasks.elTasks.length; t++)
		{
			var elTask = this.elEdit_tasks.elTasks[t];
			var task = {};

			var duration = parseInt( c_Strip( elTask.m_elDur.textContent ));
			if( ! isNaN( duration ))
				task.duration = duration;

			if( elTask.m_elTags.tags)
			{
				task.tags = [];
				elList = elTask.m_elTags.tags;
				for( var i = 0; i < elList.length; i++)
					if( elList[i].m_selected )
					{
						var tag = elList[i].m_item;
						task.tags.push( tag);

						// Add tag to status:
						if( this.obj.tags == null ) this.obj.tags = [];
						if( this.obj.tags.indexOf( tag) == -1 )
							this.obj.tags.push( tag);
					}
			}
			else if( elTask.m_task.tags )
				task.tags = elTask.m_task.tags;

			if( elTask.m_elArtists.artists)
			{
				task.artists = [];
				elList = elTask.m_elArtists.artists;
				for( var i = 0; i < elList.length; i++)
					if( elList[i].m_selected )
					{
						var artist = elList[i].m_item;
						task.artists.push( artist);

						// Add artist to status:
						if( this.obj.artists == null ) this.obj.artists = [];
						if( this.obj.artists.indexOf( artist) == -1 )
							this.obj.artists.push( artist);
					}
			}
			else if( elTask.m_task.artists )
				task.artists = elTask.m_task.artists;

			this.obj.tasks.push( task);
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
	var load_news = false;
	for( var i = 0; i < statuses.length; i++)
	{
		if( statuses[i].obj == null ) statuses[i].obj = {};

		if( annotation !== null ) statuses[i].obj.annotation = annotation;
		if( finish     !== null ) statuses[i].obj.finish     = finish;
		if( progress   !== null )
		{
			progresses[statuses[i].path] = progress;
			if( progress != statuses[i].obj.progress ) some_progress_changed = true;
			statuses[i].obj.progress = progress;
		}

		if( artists )
		{
			if( statuses[i].obj.artists == null )
				statuses[i].obj.artists = [];

			for( var a = 0; a < statuses[i].obj.artists.length; a++ )
				if( artists[statuses[i].obj.artists[a]] == null )
					statuses[i].obj.artists.splice(a,1);

			for( var id in artists )
				if(( artists[id] == 'selected' ) && ( statuses[i].obj.artists.indexOf(id) == -1 ))
					statuses[i].obj.artists.push( id);
		}

		if( tags )
		{
			if( statuses[i].obj.tags == null )
				statuses[i].obj.tags = [];

			for( var a = 0; a < statuses[i].obj.tags.length; a++ )
				if( tags[statuses[i].obj.tags[a]] == null )
					statuses[i].obj.tags.splice(a,1);

			for( var id in tags )
				if(( tags[id] == 'selected' ) && ( statuses[i].obj.tags.indexOf(id) == -1 ))
					statuses[i].obj.tags.push( id);
		}

		if( this.elEdit_Color.m_color_changed )
			statuses[i].obj.color = this.elEdit_Color.m_color;

		// Status saving produce news.
		// Making news produce loading them by default.
		// We should reload news only at last:
		if( i == (statuses.length - 1)) load_news = true;
		statuses[i].save({"load_news":load_news});

		// Status showing causes values redraw,
		// and destoys edit GUI if any.
		statuses[i].show();
	}

//	this.save();
//	this.show();

	if( some_progress_changed )
		st_UpdateProgresses( this.path, progresses);

	c_Info('Status(es) saved.');
}

Status.prototype.save = function( i_args)
{
	this.obj.muser = g_auth_user.id;
	this.obj.mtime = c_DT_CurSeconds();

	st_Save( this.obj, this.path);
	nw_MakeNews({"title":'status',"path":this.path,"artists":this.obj.artists},{"load":i_args.load_news});
}

function st_Save( i_status, i_path, i_func, i_args, i_navig_up)
{
	if( i_status == null ) i_status = RULES.status;
	if( i_path == null ) i_path = g_CurPath();

	g_FolderSetStatusPath( i_status, i_path, i_navig_up);
	n_walks[i_path] = null;

	var obj = {};
	obj.object = {"status":i_status};
	obj.add = true;
	obj.file = RULES.root + i_path + '/' + RULES.rufolder + '/status.json';

	n_Request({"send":{"editobj":obj},"func":i_func,"args":i_args,"wait":false});
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
	$('status_framesnum_div').title = 'Frames number updated\nPrevous value: ' + RULES.status.frames_num;
}

function st_UpdateProgresses( i_path, i_progresses)
{
	paths = [];
	progresses = {};
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
	n_WalkDir({"paths":paths,"wfunc":st_UpdateProgressesWalkReceived,"paths_skip_save":paths_skip_save,
		"info":'walk upstatuses',"rufiles":['status'],"lookahead":['status']});
}
function st_UpdateProgressesWalkReceived( i_walks, i_args)
{
	if( i_walks == null ) return;

	for( var w = i_walks.length-1; w >= 0; w--)
	{
//window.console.log( i_walks[w]);
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
					if( w != (i_walks.length-1)) continue;
					st_Save({"progress":0}, path);
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

		progress = Math.floor( progress / progress_count);
		progresses[paths[w]] = progress;

//console.log(paths[w]+': '+progress_count+': '+progress);
	}

	// Update only progess in navig:
	var navig_up = {};
	navig_up.progress = true;

	for( var path in progresses)
	{
		if( i_args.paths_skip_save.indexOf(path) != -1 )
			continue;

		st_Save({"progress":progresses[path]}, path,/*func=*/null,/*args=*/null, navig_up);
//console.log(path);
	}
}

