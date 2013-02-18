st_wnd = null;
st_elRoot = null;
st_elParent = null;
st_elToHide = null
st_path = null;
st_status = null;
st_FuncApply = null;

function st_SetElProgress( i_status, i_elProgressBar, i_elProgressHide, i_elPercentage)
{
	if( i_status && i_status.progress )
	{
		if( i_elProgressBar) i_elProgressBar.style.width = i_status.progress+'%';
		if( i_elPercentage) i_elPercentage.textContent = i_status.progress+'%';
		if( i_elProgressHide) i_elProgressHide.style.display = 'block';
	}
	else
	{
		if( i_elProgressBar) i_elProgressBar.style.width = '0';
		if( i_elPercentage) i_elPercentage.textContent = '';
		if( i_elProgressHide) i_elProgressHide.style.display = 'none';
	}
}

function st_SetElLabel( i_el, i_status, i_full)
{
	if( i_status && i_status.annotation)
	{
		if( i_full )
			i_el.innerHTML = i_status.annotation;
		else
			i_el.innerHTML = i_status.annotation.split(' ')[0];
	}
	else
		i_el.innerHTML = '';
}
function st_setElArtists( i_status, i_elArtists)
{
	var text = '';
	if( i_status && i_status.artists )
		for( var i = 0; i < i_status.artists.length; i++)
		{
			if( i ) text += ', ';
			text += c_GetUserTitle( i_status.artists[i]);
		}
	i_elArtists.textContent = text;
}
function st_setElTags( i_status, i_elTags)
{
	var text = '';
	if( i_status && i_status.tags )
		for( var i = 0; i < i_status.tags.length; i++)
			text += ' '+i_status.tags[i];
	i_elTags.textContent = text;
}
function st_SetElColor( i_status, i_elB, i_elC)
{
	if( i_elB == null ) i_elB = st_elRoot;
	if( i_elC == null ) i_elC = i_elB;

	if( i_status &&  i_status.color)
	{
		var c = i_status.color;
		i_elB.style.background = 'rgb('+c[0]+','+c[1]+','+c[2]+')';
		if( c[0]+c[1]+c[2] > 200 )
			i_elC.style.color = '#000';
		else
			i_elC.style.color = '#FFF';
//window.console.log(c[0]+c[1]+c[2])
	}
	else
	{
		i_elB.style.background = '';
		i_elC.style.color = '#000';
	}
}

function st_CreateEditUI( i_elParent, i_path, i_status, i_FuncApply, i_elToHide)
{
//console.log(JSON.stringify(i_status));
	if( i_elToHide )
		i_elToHide.style.display = 'none';

	st_DestroyEditUI();

	st_elParent = i_elParent;
	st_elToHide = i_elToHide;
	st_path = i_path;
	st_FuncApply = i_FuncApply;
	st_status = {};
	if( i_status )
		st_status = c_CloneObj( i_status);
//console.log(JSON.stringify(st_status));
//	st_status.annotation = i_status.annotation;
//	st_status.color = i_status.color;

	st_elRoot = document.createElement('div');
	st_elParent.appendChild( st_elRoot);
	st_elRoot.classList.add('status_edit');

	var elBtns = document.createElement('div');
	st_elRoot.appendChild( elBtns);
	elBtns.style.cssFloat = 'right';

	var elBtnSave = document.createElement('div');
	elBtns.appendChild( elBtnSave);
	elBtnSave.classList.add('button');
	elBtnSave.textContent = 'Save';
	elBtnSave.onclick = st_SaveOnClick;

	var elBtnCancel = document.createElement('div');
	elBtns.appendChild( elBtnCancel);
	elBtnCancel.classList.add('button');
	elBtnCancel.textContent = 'Cancel';
	elBtnCancel.onclick = st_DestroyEditUI;

	var elAnnDiv = document.createElement('div');
	st_elRoot.appendChild( elAnnDiv);
	var elAnnLabel = document.createElement('div');
	elAnnDiv.appendChild( elAnnLabel);
	elAnnLabel.textContent = 'Status:';
	elAnnLabel.style.cssFloat = 'left';
	st_elAnn = document.createElement('div');
	elAnnDiv.appendChild( st_elAnn);
	st_elAnn.classList.add('editing');
	st_elAnn.contentEditable = 'true';

	var elProgressDiv = document.createElement('div');
	st_elRoot.appendChild( elProgressDiv);
//	elProgressDiv.style.cssFloat = 'left';
	elProgressDiv.classList.add('progress');
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
			el.classList.add('item');
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
			el.textContent = st_status.tags[i];
			el.classList.add('item');
			el.classList.add('selected');
		}


	st_elColor = document.createElement('div');
	st_elRoot.appendChild( st_elColor);
	st_elColor.classList.add('color');
	u_DrawColorBars( st_elColor, st_EditColorOnClick);

//	st_FillEditUI( i_status);
//}
//function st_FillEditUI( i_status )
//{
//	st_status = i_status;
//	if( st_status == null )
//		st_status = {};

	st_SetElLabel( st_elAnn, st_status, true);
	st_SetElColor( st_status);
	if( st_status.progress != null ) st_elProgress.textContent = st_status.progress;

	st_elAnn.focus();
}

function st_DestroyEditUI()
{
	if( st_elRoot && st_elParent )
	{
//console.log( st_elParent);
//console.log( st_elRoot);
		st_elParent.removeChild( st_elRoot);
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
	var tags = {};
	for( var i = 0; i < RULES.tags.length; i++)
		tags[RULES.tags[i]] = RULES.tags[i];
	st_EditShowList( st_elTags, 'tags', tags);
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
		el.classList.add('item');
		if( i_list[item].title )
//			el.textContent = i_list[item].title;
			el.textContent = c_GetUserTitle( item);
		else
			el.textContent = item;
		el.m_item = i_list[item];
		if( st_status[i_stParam] && ( st_status[i_stParam].indexOf( item) != -1 ))
		{
			el.m_selected = true;
			el.classList.add('selected');
		}
		el.onclick = function(e){
			el = e.currentTarget;
			if( el.m_selected )
			{
				el.m_selected = false;
				el.classList.remove('selected');
			}
			else
			{
				el.m_selected = true;
				el.classList.add('selected');
			}
		};
		i_elParent.m_elListAll.push( el);
	}
}

function st_EditColorOnClick( i_evt)
{
	var el = i_evt.currentTarget;
	st_status.color = el.m_color
	st_SetElColor( st_status);
}

function st_SaveOnClick()
{
	st_status.annotation = st_elAnn.innerHTML;
	st_status.progress = parseInt( st_elProgress.textContent);
	if( st_elArtists.m_elListAll )
	{
		st_status.artists = [];
		for( var i = 0; i < st_elArtists.m_elListAll.length; i++)
			if( st_elArtists.m_elListAll[i].m_selected )
				st_status.artists.push( st_elArtists.m_elListAll[i].m_item.id);
	}
	if( st_elTags.m_elListAll )
	{
		st_status.tags = [];
		for( var i = 0; i < st_elTags.m_elListAll.length; i++)
			if( st_elTags.m_elListAll[i].m_selected )
				st_status.tags.push( st_elTags.m_elListAll[i].m_item);
	}

	st_FuncApply( st_status);
	if( g_CurPath() == st_path )
		g_FolderSetStatus( st_status);

	var obj = {};
	obj.object = {"status":st_status};
	obj.add = true;
	obj.file = RULES.root + st_path + '/' + RULES.rufolder + '/status.json';

	n_Request({"editobj":obj});

	nw_MakeNews('<i>status</i>');

	st_DestroyEditUI();
}
