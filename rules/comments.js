function cm_Process()
{
}

function cm_Finish()
{
}

function cm_OpenOnClick()
{
}

function cm_NewOnClick()
{
	var el = document.createElement('div');
	u_el.comments.appendChild( el);
	el.classList.add('comment');

	var elText = document.createElement('div');
	el.appendChild( elText);
	el.m_elText = elText;
	elText.classList.add('text');
	elText.contentEditable = 'true';
	elText.focus();

	var elCtrl = document.createElement('div');
	el.appendChild( elCtrl);
	elCtrl.classList.add('ctrl');

	var elSave = document.createElement('div');
	elCtrl.appendChild( elSave);
	elSave.classList.add('button');
	elSave.textContent = 'Save';
	elSave.m_el = el;
	elSave.onclick = function(e){cm_Save(e.currentTarget.m_el)};

	var elCancel = document.createElement('div');
	elCtrl.appendChild( elCancel);
	elCancel.classList.add('button');
	elCancel.textContent = 'Cancel';
	elCancel.m_el = el;
}

function cm_Save( i_el)
{
	var date = new Date();

	var cm = {};
	cm.text = i_el.m_elText.textContent;
	cm.user_name = localStorage.user_name;
	cm.user_title = localStorage.user_title;
	cm.ctime = date.getTime();
	cm.mtime = cm.ctime;
	cm.duration = 1.5;

	var comments = {};
	comments[cm.ctime+'_'+cm.user_name] = cm;
	var edit = {};
	edit.object = {"comments":comments};
	edit.add = true;
	edit.file = g_elCurFolder.m_path+'/'+RULES.rufolder+'/comments.json';

window.console.log( JSON.stringify( edit));
//	n_Request({"editobj":obj});
}

