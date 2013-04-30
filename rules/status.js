st_wnd = null;
st_elRoot = null;
st_elParent = null;
st_elToHide = null
st_path = null;
st_status = null;
st_FuncApply = null;
st_progress = null;

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
function st_SetElColor( i_status, i_elB, i_elC, i_setNone)
{
	if( i_elB == null ) i_elB = st_elParent;
	if( i_elC == null ) i_elC = i_elB;
	if( i_setNone == null ) i_setNone = true;

	if( i_status &&  i_status.color)
	{
		var c = i_status.color;
		i_elB.style.background = 'rgb('+c[0]+','+c[1]+','+c[2]+')';
		if( c[0]+c[1]+.3*c[2] > 300 )
			i_elC.style.color = '#000';
		else
			i_elC.style.color = '#FFF';
//window.console.log(c[0]+c[1]+c[2])
		return true;
	}
	else if( i_setNone )
	{
		i_elB.style.background = '';
		i_elC.style.color = 'inherit';
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

function st_CreateEditUI( i_elParent, i_path, i_status, i_FuncApply, i_elToHide)
{
//console.log(JSON.stringify(i_status));
	if( i_elToHide )
		i_elToHide.style.display = 'none';

	st_DestroyEditUI();

	st_elParent = i_elParent;
	st_elParent.classList.add('status_editing');
	st_elToHide = i_elToHide;
	st_path = i_path;
	st_progress = null;
	st_FuncApply = i_FuncApply;
	st_status = {};

	if( i_status )
	{
		st_status = c_CloneObj( i_status);
		// Store progress to see whether progress update needed
		if( i_status.progress != null )
			st_progress = i_status.progress;
	}
//console.log(JSON.stringify(st_status));
//	st_status.annotation = i_status.annotation;
//	st_status.color = i_status.color;

	st_elRoot = document.createElement('div');
	st_elParent.appendChild( st_elRoot);
	st_elRoot.classList.add('status_edit');

	var elBtns = document.createElement('div');
	st_elRoot.appendChild( elBtns);
	elBtns.style.cssFloat = 'right';

	var elBtnCancel = document.createElement('div');
	elBtns.appendChild( elBtnCancel);
	elBtnCancel.classList.add('button');
	elBtnCancel.textContent = 'Cancel';
	elBtnCancel.onclick = st_DestroyEditUI;

	var elBtnSave = document.createElement('div');
	elBtns.appendChild( elBtnSave);
	elBtnSave.classList.add('button');
	elBtnSave.textContent = 'Save';
	elBtnSave.onclick = st_SaveOnClick;

	var elFinishDiv = document.createElement('div');
	st_elRoot.appendChild( elFinishDiv);
	elFinishDiv.style.cssFloat = 'right';
	elFinishDiv.style.width = '200px';
	elFinishDiv.style.textAlign = 'center';
	elFinishDiv.style.marginLeft = '10px';
	var elFinishLabel = document.createElement('div');
	elFinishDiv.appendChild( elFinishLabel);
	elFinishLabel.style.cssFloat = 'left';
	elFinishLabel.textContent = 'Finish:';
	elFinishLabel.onclick  = function(){ st_elFinish.textContent = c_DT_FormStrNow();};
	elFinishLabel.style.cursor = 'pointer';
	st_elFinish = document.createElement('div');
	elFinishDiv.appendChild( st_elFinish);
	st_elFinish.classList.add('editing');
	st_elFinish.contentEditable = 'true';

	var elAnnDiv = document.createElement('div');
	st_elRoot.appendChild( elAnnDiv);
	var elAnnLabel = document.createElement('div');
	elAnnDiv.appendChild( elAnnLabel);
	elAnnLabel.textContent = 'Annotation:';
	elAnnLabel.style.cssFloat = 'left';
	st_elAnn = document.createElement('div');
	elAnnDiv.appendChild( st_elAnn);
	st_elAnn.classList.add('editing');
	st_elAnn.contentEditable = 'true';

	var elProgressDiv = document.createElement('div');
	st_elRoot.appendChild( elProgressDiv);
//	elProgressDiv.style.cssFloat = 'left';
	elProgressDiv.classList.add('percent');
	var elProgressLabel = document.createElement('div');
	elProgressDiv.appendChild( elProgressLabel);
	elProgressLabel.textContent = 'Progress:';
//	elProgressLabel.style.cssFloat = 'left';
	elProgressLabel.style.fontSize = '12px';
	st_elProgress = document.createElement('div');
	elProgressDiv.appendChild( st_elProgress);
	st_elProgress.style.width = '40px';
	st_elProgress.style.height = '18px';
	st_elProgress.contentEditable = 'true';
	st_elProgress.classList.add('editing');
	st_elProgress.style.textAlign = 'center';

	st_elArtists = document.createElement('div');
	st_elRoot.appendChild( st_elArtists);
	st_elArtists.classList.add('list');
	st_elArtists.classList.add('artists');
	st_elArtists.m_elBtn = document.createElement('div');
	st_elArtists.appendChild( st_elArtists.m_elBtn);
	st_elArtists.m_elBtn.classList.add('button');
	st_elArtists.m_elBtn.style.cssFloat = 'left';;
	st_elArtists.m_elBtn.textContent = 'Artists:';
	st_elArtists.m_elBtn.onclick = st_EditArtistsShow;
	st_elArtists.m_elList = document.createElement('div');
	st_elArtists.appendChild( st_elArtists.m_elList);

	if( st_status.artists )
		for( var i = 0; i < st_status.artists.length; i++)
		{
			var el = document.createElement('div');
			st_elArtists.m_elList.appendChild( el);
			el.textContent = c_GetUserTitle( st_status.artists[i]);
			el.classList.add('tag');
			el.classList.add('selected');
		}

	st_elTags = document.createElement('div');
	st_elRoot.appendChild( st_elTags);
	st_elTags.classList.add('list');
	st_elTags.classList.add('tags');
	st_elTags.m_elBtn = document.createElement('div');
	st_elTags.appendChild( st_elTags.m_elBtn);
	st_elTags.m_elBtn.classList.add('button');
	st_elTags.m_elBtn.style.cssFloat = 'left';
	st_elTags.m_elBtn.textContent = 'Tags:';
	st_elTags.m_elBtn.onclick = st_EditTagsShow;
	st_elTags.m_elList = document.createElement('div');
	st_elTags.appendChild( st_elTags.m_elList);
	if( st_status.tags )
		for( var i = 0; i < st_status.tags.length; i++)
		{
			var el = document.createElement('div');
			st_elTags.m_elList.appendChild( el);
			el.textContent = c_GetTagTitle( i_status.tags[i]);
			el.title = c_GetTagTip( i_status.tags[i]);
			el.classList.add('tag');
			el.classList.add('selected');
		}

	st_elColor = document.createElement('div');
	st_elRoot.appendChild( st_elColor);
	st_elColor.classList.add('color');
	u_DrawColorBars( st_elColor, st_EditColorOnClick);

	st_SetElAnnotation( st_status, st_elAnn);
	st_SetElColor( st_status);
	if( st_status.finish )
		st_elFinish.textContent = c_DT_FormStrFromSec( st_status.finish);
	if( st_status.progress != null ) st_elProgress.textContent = st_status.progress;

	st_elAnn.focus();
}

function st_DestroyEditUI()
{
	if( st_elParent )
	{
		st_elParent.classList.remove('status_editing');
		if( st_elRoot ) st_elParent.removeChild( st_elRoot);
	}

	if( st_elToHide )
		st_elToHide.style.display = 'block';

	st_elRoot = null;
	st_elParent = null;
	st_elToHide = null;
	st_status = null;
}

function st_EditTagsShow( i_evt)
{
	st_EditShowList( st_elTags, 'tags', RULES.tags);
}
function st_EditArtistsShow( i_evt)
{
	st_EditShowList( st_elArtists, 'artists', g_users);
}
function st_EditShowList( i_elParent, i_stParam, i_list)
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
		if( st_status[i_stParam] && ( st_status[i_stParam].indexOf( item) != -1 ))
		{
			el.m_selected = true;
			el.classList.add('selected');
		}
		el.onclick = c_ElToggleSelected;
		i_elParent.m_elListAll.push( el);
	}
}

function st_EditColorOnClick( i_evt)
{
	var el = i_evt.currentTarget;
	st_status.color = el.m_color
//console.log( st_status);
	st_SetElColor( st_status);
}

function st_SaveOnClick()
{
	var finish = st_elFinish.textContent;
	if( finish.length )
	{
		finish = c_DT_SecFromStr( st_elFinish.textContent);
		if( finish == 0 ) return;
		st_status.finish = finish;
	}

	st_status.annotation = c_Strip( st_elAnn.innerHTML);
	st_status.progress = parseInt( c_Strip( st_elProgress.textContent));
	if( st_elArtists.m_elListAll )
	{
		st_status.artists = [];
		for( var i = 0; i < st_elArtists.m_elListAll.length; i++)
			if( st_elArtists.m_elListAll[i].m_selected )
				st_status.artists.push( st_elArtists.m_elListAll[i].m_item);
	}
	if( st_elTags.m_elListAll )
	{
		st_status.tags = [];
		for( var i = 0; i < st_elTags.m_elListAll.length; i++)
			if( st_elTags.m_elListAll[i].m_selected )
				st_status.tags.push( st_elTags.m_elListAll[i].m_item);
	}

	st_status.muser = g_auth_user.id;
	st_status.mtime = c_DT_CurSeconds();

	st_FuncApply( st_status);
	if( g_CurPath() == st_path )
		g_FolderSetStatus( st_status);

	st_Save( st_status, st_path);
	nw_MakeNews('<i>status</i>', st_path);

	if( st_elProgress.textContent.length )
		if( st_progress != st_status.progress )
			st_UpdateProgresses( st_path);

	st_DestroyEditUI();
}

function st_Save( i_status, i_path, i_wait)
{
	if( i_status == null ) i_status = RULES.status;
	if( i_path == null ) i_path = g_CurPath();
	var obj = {};
	obj.object = {"status":i_status};
	obj.add = true;
	obj.file = RULES.root + i_path + '/' + RULES.rufolder + '/status.json';
	return c_Parse( n_Request({"editobj":obj}, i_wait));
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
	walks = n_WalkDir( paths, 0, RULES.rufolder, ['status'], ['status']);
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

