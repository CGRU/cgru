cm_file = 'comments.json';
cm_opened = false;
cm_durations = ['.1','.2','.3','.5','1','2','3','4','5','6','7','8','9','10','11','12','18','24','36','48','60'];

function cm_CurFileName()
{
	return RULES.root+g_elCurFolder.m_path+'/'+RULES.rufolder+'/'+cm_file;
}

function cm_Process()
{
//window.console.log('cm_Process():');
//cm_OpenOnClick();
}

function cm_Finish()
{
	u_el.comments.textContent = '';
	cm_opened = false;
	document.getElementById('comments_open_btn').classList.add('button');
}

function cm_OpenOnClick()
{
	if( cm_opened ) return;

	document.getElementById('comments_open_btn').classList.remove('button');

	var obj = c_Parse( n_Request({"readobj":cm_CurFileName()}));
//window.console.log( JSON.stringify( obj));
	if( obj == null ) return;
	if( obj.comments == null ) return;

	cm_opened = true;
	for( key in obj.comments )
		cm_Add( obj.comments[key]);
}

function cm_Add( i_obj)
{
//window.console.log( JSON.stringify( i_obj));
	var el = document.createElement('div');
//	u_el.comments.appendChild( el);
	u_el.comments.insertBefore( el, u_el.comments.firstChild);
	el.classList.add('comment');

	var elPanel = document.createElement('div');
	el.appendChild( elPanel);
	elPanel.classList.add('panel');

	var elEdit = document.createElement('div');
	elPanel.appendChild( elEdit);
	elEdit.classList.add('button');
	elEdit.textContent = 'Edit';
	elEdit.onclick = function(e){cm_Edit(e.currentTarget.m_el);}
	elEdit.m_el = el;
	el.m_elEdit = elEdit;

	var elSave = document.createElement('div');
	elPanel.appendChild( elSave);
	elSave.classList.add('button');
	elSave.textContent = 'Save';
	elSave.style.display = 'none';
	elSave.onclick = function(e){cm_Save(e.currentTarget.m_el)};
	elSave.m_el = el;
	el.m_elSave = elSave;

	var elCancel = document.createElement('div');
	elPanel.appendChild( elCancel);
	elCancel.classList.add('button');
	elCancel.textContent = 'Cancel';
	elCancel.onclick = function(e){cm_Cancel(e.currentTarget.m_el)};
	elCancel.m_el = el;
	el.m_elCancel = elCancel;

	var elDel = document.createElement('div');
	elPanel.appendChild( elDel);
	elDel.classList.add('button');
	elDel.textContent = 'Delete';
	elDel.onclick = function(e){cm_Delete(e.currentTarget.m_el)};
	elDel.m_el = el;
	el.m_elDel = elDel;

	var elInfo = document.createElement('div');
	elInfo.classList.add('info');
	elPanel.appendChild( elInfo);
	el.m_elInfo = elInfo;

	var elText = document.createElement('div');
	el.appendChild( elText);
	el.m_elText = elText;
	elText.classList.add('text');

	el.m_obj = i_obj;
	cm_Init( el);
	return el;
}

function cm_Init( i_el)
{
	if( i_el.m_elDCtrl ) i_el.removeChild( i_el.m_elDCtrl );
	if( i_el.m_elColor ) i_el.removeChild( i_el.m_elColor );

	i_el.m_elDel.style.display = 'none';
	i_el.m_elCancel.style.display = 'none';
	i_el.m_elSave.style.display = 'none';
	i_el.m_elEdit.style.display = 'block';

	i_el.m_elText.contentEditable = 'false';
	i_el.m_elText.classList.remove('editing');

	if( i_el.m_obj == null )
	{
		var cm = {};
		cm.user_name = localStorage.user_name;
		cm.user_title = localStorage.user_title;
		cm.ctime = (new Date()).getTime();
		cm.duration = 1.5;
		i_el.m_obj = cm;
		i_el.m_new = true;
	}

	var date = c_DT_StrFromMS( i_el.m_obj.ctime);

	var info = i_el.m_obj.user_title+' '+date;
	if( i_el.m_obj.duration )
		info = i_el.m_obj.duration+' '+info;
	if( i_el.m_obj.mtime )
	{
		var date = c_DT_StrFromMS( i_el.m_obj.mtime);
		info += '<br>Modified: ';
		info += i_el.m_obj.muser_title+' '+date;
	}

	i_el.m_elInfo.innerHTML = info;

	if( i_el.m_obj.text )
		i_el.m_elText.innerHTML = i_el.m_obj.text;

	i_el.color = i_el.m_obj.color;
	st_SetElColor( i_el, i_el);

	if( i_el.m_obj.deleted )
		i_el.style.display = 'none';
}

function cm_Edit( i_el)
{
	if( i_el.m_new != true )
		i_el.m_elDel.style.display = 'block';
	i_el.m_elCancel.style.display = 'block';
	i_el.m_elSave.style.display = 'block';
	i_el.m_elEdit.style.display = 'none';

	var elDCtrl = document.createElement('div');
	i_el.appendChild( elDCtrl);
	elDCtrl.classList.add('duration');
	i_el.m_elDCtrl = elDCtrl;

	var elDLabel = document.createElement('div');
	elDCtrl.appendChild( elDLabel);
	elDLabel.textContent = 'Duration:';

	var elDrtn = document.createElement('div');
	elDCtrl.appendChild( elDrtn);
	elDrtn.classList.add('editing');
	elDrtn.contentEditable = 'true';
	i_el.m_elDrtn = elDrtn;
	i_el.m_elDrtn.textContent = i_el.m_obj.duration;

	for( var i = 0; i < cm_durations.length; i++)
	{
		var elDr = document.createElement('div');
		elDCtrl.appendChild( elDr);
		elDr.classList.add('sample')
		elDr.textContent = cm_durations[i];
		elDr.m_elDrtn = elDrtn;
		elDr.onclick = function(e){e.currentTarget.m_elDrtn.textContent = e.currentTarget.textContent;}
	}

	var elColor = document.createElement('div');
	i_el.appendChild( elColor);
	i_el.m_elColor = elColor;
	u_DrawColorBars( elColor, cm_ColorOnclick);

	i_el.m_elText.classList.add('editing');
	i_el.m_elText.contentEditable = 'true';
	i_el.m_elText.focus();
}

function cm_ColorOnclick( e)
{
	var el = e.currentTarget;
	var color = el.m_color;
	el = el.parentNode.parentNode.parentNode;
	el.color = color;
	st_SetElColor( el, el);
}

function cm_Cancel( i_el)
{
	if( i_el.m_new )
		u_el.comments.removeChild( i_el);
	else
		cm_Init( i_el);
}

function cm_NewOnClick()
{
	if( false == cm_opened )
		cm_OpenOnClick();
	var el = cm_Add();
	cm_Edit( el);
}

function cm_Delete( i_el)
{
	var cm = i_el.m_obj;
	cm.deleted = true;
	cm_Save( i_el);
}

function cm_Save( i_el)
{
	i_el.m_obj.text = i_el.m_elText.innerHTML;
	i_el.m_obj.duration = i_el.m_elDrtn.textContent;
	i_el.m_obj.color = i_el.color;

	if( i_el.m_new != true )
	{
		i_el.m_obj.mtime = (new Date()).getTime();
		i_el.m_obj.muser_name = localStorage.user_name;
		i_el.m_obj.muser_title = localStorage.user_title;
	}

	cm_Init( i_el);

	var cm = i_el.m_obj;
	var key = cm.ctime+'_'+cm.user_name;

	var comments = {};
	comments[key] = cm;
	var edit = {};
	edit.object = {"comments":comments};
	edit.add = true;
	edit.file = cm_CurFileName();

//window.console.log( JSON.stringify( edit));
	n_Request({"editobj":edit});

	nw_MakeNews('<i>comments</i>');
}

