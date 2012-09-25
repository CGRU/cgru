cgru_DialogsAll = [];
cgru_MenusAll = [];

function cgru_ClosePopus()
{
	cgru_DialogsCloseAll();
	cgru_MenusCloseAll();
}

function cgru_DialogsCloseAll()
{
	while( cgru_DialogsAll.length > 0 )
		cgru_DialogsAll[0].destroy();
}

function cgru_MenusCloseAll()
{
	while( cgru_MenusAll.length > 0 )
		cgru_MenusAll[0].destroy();
}



function cgru_Dialog( i_document, i_elParent, i_reciever, i_name, i_title, i_info)
{
	this.document = i_document;
	this.elParent = i_elParent;
	this.reciever = i_reciever;
	this.name = i_name;

	for( var i = 0; i < cgru_DialogsAll.length; i++)
		if( cgru_DialogsAll[i].name == this.name )
			cgru_DialogsAll[i].destroy();

	this.elRoot = this.document.createElement('div');
	this.elParent.appendChild( this.elRoot);
	this.elRoot.classList.add('cgrudialogback');

	this.elContent = this.document.createElement('div');
	this.elRoot.appendChild( this.elContent);
	this.elContent.classList.add('cgrudialog');
//	this.elTitle.innerHTML = i_title;

	this.elTitle = this.document.createElement('div');
//	this.elRoot.appendChild( this.elTitle);
	this.elContent.appendChild( this.elTitle);
	this.elTitle.classList.add('cgrudialogtitle');
	this.elTitle.innerHTML = i_title;

	this.elInfo = this.document.createElement('div');
//	this.elRoot.appendChild( this.elInfo);
	this.elContent.appendChild( this.elInfo);
	this.elInfo.classList.add('cgrudialoginfo');
	this.elInfo.innerHTML = i_info;

	this.elInput = this.document.createElement('input');
//	this.elInput = this.document.createElement('div');
	this.elInput.classList.add('cgrudialoginput');
//	this.elRoot.appendChild( this.elInput);
	this.elContent.appendChild( this.elInput);
//	this.elInput.contentEditable = 'true';
	this.elInput.innerHTML = 'This is some default text';
	this.elInput.cgru_Dialog = this;
	this.elInput.onkeydown = this.onKeyDown;
//	this.elRoot.onkeydown = this.onKeyDown;
//	this.elRoot.onkeyup = this.onKeyDown;

	cgru_DialogsAll.push( this);
}
cgru_Dialog.prototype.destroy = function()
{
	var index = cgru_DialogsAll.indexOf( this);
	if( index >= 0 ) cgru_DialogsAll.splice( index, 1);
	this.elParent.removeChild( this.elRoot);
}
cgru_Dialog.onKeyDown = function(e)
{ 
	if( !e ) return;
//	if( e.keyCode == 27 ) // ESC
//	this.elInput.innerHTML = e.keyCode;
	g_Info('e.keyCode');	
}

function cgru_Menu( i_elParent, i_evt, i_reciever, i_name)
{
	this.elParent = i_elParent;
	this.reciever = i_reciever;
	this.EType = 'cgru_Menu';
	this.name = i_name;
	this.posX = i_evt.pageX;
	this.posY = i_evt.pageY;

	for( var i = 0; i < cgru_MenusAll.length; i++)
		if( cgru_MenusAll[i].name == this.name )
			cgru_MenusAll[i].destroy();

	this.elRoot = document.createElement('div');
	this.elParent.appendChild( this.elRoot);
	this.elRoot.classList.add('cgrumenu');

	cgru_MenusAll.push( this);
}
cgru_Menu.prototype.destroy = function()
{
	var index = cgru_MenusAll.indexOf( this);
	if( index >= 0 ) cgru_MenusAll.splice( index, 1);
	this.elParent.removeChild( this.elRoot);
}
cgru_Menu.prototype.show = function()
{
	this.elRoot.style.display = 'block';

	var w = this.elRoot.offsetWidth;
	var h = this.elRoot.offsetHeight;

	if( this.posX + w > window.innerWidth  + window.pageXOffset) this.posX -= w;
	if( this.posY + h > window.innerHeight + window.pageYOffset) this.posY -= h;

	this.elRoot.style.left = this.posX + 'px';
	this.elRoot.style.top  = this.posY + 'px';
}

cgru_Menu.prototype.addItem = function( i_name, i_actionName, i_enabled)
{
	if( i_enabled == null )	i_enabled = true;

	var el = document.createElement('div');
	this.elRoot.appendChild( el);
	el.classList.add('cgrumenuitem');
	el.innerHTML = i_name;
	el.cgrumenu = this;
	el.actionName = i_actionName;

	if( i_enabled )
	{
		el.onmousedown = this.onMouseUpDown;
		el.onmouseup   = this.onMouseUpDown;
	}
	else
		el.classList.add('disabled');
}

cgru_Menu.prototype.addSeparator = function( evt)
{
	this.elRoot.lastChild.style.borderBottom = '3px solid #888';
}

cgru_Menu.prototype.onMouseUpDown = function( evt)
{
	var el = evt.currentTarget;
	if( el == null ) return;
	if( el.cgrumenu == null ) return;

	el.cgrumenu.reciever.action( el.actionName);
	el.cgrumenu.destroy();
}



function cgru_LoadCSS( i_filename, i_document)
{
	if( i_document == null )
		i_document = document;
	var fileref = i_document.createElement('link');
	fileref.setAttribute('rel','stylesheet');
	fileref.setAttribute('type','text/css');
	fileref.setAttribute('href', i_filename);
	i_document.getElementsByTagName('head')[0].appendChild( fileref);
}

