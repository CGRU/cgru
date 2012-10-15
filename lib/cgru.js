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



function cgru_Dialog( i_document, i_elParent, i_reciever, i_handle, i_parameter, i_type, i_value, i_dialogname, i_title, i_info)
{
	this.document = i_document;
	this.elParent = i_elParent;
	this.reciever = i_reciever;
	this.handle = i_handle;
	this.type = i_type;
	this.parameter = i_parameter;
	this.name = i_dialogname;
	if( this.name == null ) this.name = 'set_parameter';

	for( var i = 0; i < cgru_DialogsAll.length; i++)
		if( cgru_DialogsAll[i].name == this.name )
			cgru_DialogsAll[i].destroy();

	this.elRoot = this.document.createElement('div');
	this.elParent.appendChild( this.elRoot);
	this.elRoot.classList.add('cgru_dialog_back');

	this.elContent = this.document.createElement('div');
	this.elRoot.appendChild( this.elContent);
	this.elContent.classList.add('cgru_dialog_content');

	this.elTitle = this.document.createElement('div');
//	this.elRoot.appendChild( this.elTitle);
	this.elContent.appendChild( this.elTitle);
	this.elTitle.classList.add('cgru_dialog_title');
	if( i_title == null ) i_title = 'Set Parameter';
	this.elTitle.textContent = i_title;

	this.elInfo = this.document.createElement('div');
//	this.elRoot.appendChild( this.elInfo);
	this.elContent.appendChild( this.elInfo);
	this.elInfo.classList.add('cgru_dialog_info');
	if( i_info == null )
	{
		i_info = 'Enter "'+this.parameter+'"';
		switch( this.type)
		{
		case 'num': i_info += ' Number'; break;
		case 'str': i_info += ' String'; break;
		case 'reg': i_info += ' RegExp'; break;
		case 'hrs': i_info += ' Hours'; break;
		case 'tim': i_info += ' Time'; break;
		case 'bl0': case 'bl1': i_info += ' Boolean (0|1|false|true)';
		}
		i_info += ':';
	}
	this.elInfo.textContent = i_info;

//	this.elInput = this.document.createElement('input');
	this.elInput = this.document.createElement('div');
	this.elInput.classList.add('cgru_dialog_input');
//	this.elRoot.appendChild( this.elInput);
	this.elContent.appendChild( this.elInput);
	this.elInput.contentEditable = 'true';
	this.elInput.cgru_Dialog = this;
	this.elInput.onkeydown = function(e){ return e.currentTarget.cgru_Dialog.onKeyDown(e);}

	this.elButtons = this.document.createElement('div');
	this.elContent.appendChild( this.elButtons);

	this.elButtonL = this.document.createElement('div');
	this.elButtons.appendChild( this.elButtonL);
	this.elButtonL.classList.add('cgru_dialog_button_left');

	this.elButtonR = this.document.createElement('div');
	this.elButtons.appendChild( this.elButtonR);
	this.elButtonR.classList.add('cgru_dialog_button_right');

	this.elCancel = this.document.createElement('div');
	this.elButtonL.appendChild( this.elCancel);
	this.elCancel.classList.add('cgru_dialog_button_cancel');
	this.elCancel.textContent = 'Cancel';
	this.elCancel.cgru_Dialog = this;
	this.elCancel.onmousedown = function(e){ e.currentTarget.cgru_Dialog.destroy();}

	this.elApply = this.document.createElement('div');
	this.elButtonR.appendChild( this.elApply);
	this.elApply.classList.add('cgru_dialog_button_apply');
	this.elApply.textContent = 'Apply';
	this.elApply.cgru_Dialog = this;
	this.elApply.onmousedown = function(e){ e.currentTarget.cgru_Dialog.apply();}

	this.elStatus = this.document.createElement('div');
	this.elContent.appendChild( this.elStatus);
	this.elStatus.classList.add('cgru_dialog_status');

	cgru_DialogsAll.push( this);

	if( this.type == 'tim')
		this.elInput.textContent = cgru_TimeToStr( i_value);
	else if( i_value )
	{
		if( this.type == 'hrs')
			this.elInput.textContent = i_value * 1.0 / 3600;
		else
			this.elInput.textContent = i_value;

		var range = this.document.createRange();
		range.setStart( this.elInput, 0);
		range.setEnd( this.elInput, 1);
		var sel = window.getSelection();
		sel.removeAllRanges();
		sel.addRange(range);
	}

//	this.elInput.focus();
	cgru_DialogInput = this.elInput;
	window.setTimeout(function(){cgru_DialogInput.focus();}, 100);
//	this.elInput.click();
//	cgru_SimulateClick( this.elInput);
}
cgru_Dialog.prototype.destroy = function()
{
	var index = cgru_DialogsAll.indexOf( this);
	if( index >= 0 ) cgru_DialogsAll.splice( index, 1);
	this.elParent.removeChild( this.elRoot);
}
cgru_Dialog.prototype.onKeyDown = function(e)
{
	if( e.keyCode == 27 ) // Esc
	{
		this.destroy();
		return;
	}
	var delta = 1;
	if( e.shiftKey ) delta = 10;
	if( e.ctrlKey ) delta = 100;
	if( e.altKey ) delta = 1000;
	if( e.keyCode == 13 ) // Enter
		this.apply();
	else if(( e.keyCode == 38 ) && ( this.type == 'num')) // Up
		this.elInput.textContent = (parseInt(this.elInput.textContent))+delta;
	else if(( e.keyCode == 40 ) && ( this.type == 'num')) // Down
		this.elInput.textContent = (parseInt(this.elInput.textContent))-delta;
}
cgru_Dialog.prototype.apply = function()
{
	var value = this.elInput.textContent.replace(/^\s+|\s+$/g,'');
	switch( this.type)
	{
	case 'num':
		if( value.search(/^-?\d+$/) == -1 )
		{
			this.displayError('You should enter a number.');
			return;
		}
		value = parseInt( value);
		break;
	case 'hrs':
		if( value.search(/^-?(\d+(\.\d+)?)|(\.\d+)$/) == -1 )
		{
			this.displayError('You should enter a float number.');
			return;
		}
		value = Math.round( parseFloat(value)*3600);
		break;
	case 'reg':
		try { RegExp( value); }
		catch( err)
		{
			this.displayError( err);
			return;
		}
		break;
	case 'tim':
		try { var data = new Date( value); }
		catch( err)
		{
			this.displayError( err);
			return;
		}
		value = Math.round( data.getTime() / 1000) + (new Date).getTimezoneOffset()*60;
		break;
	case 'bl1':
		value = value == true;
		break;
	case 'bl0':
		value = value != true;
		break;
	}

	this.reciever[this.handle]( this.parameter, value);
	this.destroy();
}
cgru_Dialog.prototype.displayError = function( i_error)
{
	if( i_error == null )
	{
		this.elStatus.textContent = '';
		this.elStatus.classList.remove('cgru_error_back');
		return;
	}
	this.elStatus.textContent = i_error;
	this.elStatus.classList.add('cgru_error_back');
}



function cgru_Menu( i_document, i_elParent, i_evt, i_receiver , i_name, i_onDestroy)
{
	this.document = i_document;
	this.elParent = i_elParent;
	this.name = i_name;
	this.posX = i_evt.pageX;
	this.posY = i_evt.pageY;
	this.onDestroy = i_onDestroy;
	this.receiver = i_receiver;

	for( var i = 0; i < cgru_MenusAll.length; i++)
		if( cgru_MenusAll[i].name == this.name )
			cgru_MenusAll[i].destroy();

	this.elRoot = this.document.createElement('div');
	this.elParent.appendChild( this.elRoot);
	this.elRoot.classList.add('cgru_menu');

	cgru_MenusAll.push( this);
}
cgru_Menu.prototype.destroy = function()
{
	if( this.onDestroy )
		this.receiver[this.onDestroy]();
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
cgru_Menu.prototype.addItem = function( i_name, i_receiver, i_handle, i_title, i_enabled, i_parameter)
{
	if(( i_name == null ) && ( this.elRoot.lastChild != null ))
	{
		this.elRoot.lastChild.style.borderBottom = '3px solid #888';
		return;
	}

	if( i_enabled == null ) i_enabled = true;
	if( i_title == null ) i_title = i_name;

	var el = this.document.createElement('div');
	this.elRoot.appendChild( el);
	el.classList.add('cgru_menu_item');
	el.innerHTML = i_title;
	el.cgru_Menu = this;
	el.name = i_name;
	el.parameter = i_parameter == null ? i_name : i_parameter;
	el.receiver = i_receiver;
	el.handle = i_handle;

	if( i_enabled )
	{
		el.onmousedown = this.onMouseUpDown;
		el.onmouseup   = this.onMouseUpDown;
	}
	else
		el.classList.add('disabled');
}
cgru_Menu.prototype.onMouseUpDown = function( evt)
{
//	i_evt.stopPropagation();
	var el = evt.currentTarget;
	if( el == null ) return;
	if( el.cgru_MenuItemToggled ) return;
	el.cgru_MenuItemToggled = true;
	if( el.cgru_Menu == null ) return;
//g_Info(el.receiver.params.name+'['+el.handle+']');
	el.receiver[el.handle]( el.parameter);
	el.cgru_Menu.destroy();
}


function cgru_TimeToStr( i_value)
{
	if( i_value )
		i_value *= 1000;
	else
		i_value = (new Date()).getTime();
	return (new Date( i_value - (new Date).getTimezoneOffset()*60*1000)).toISOString();
}
function cgru_SimulateClick( i_el)
{
	var evt = document.createEvent("MouseEvents");
	evt.initMouseEvent("click", true, true, window, 0, 0, 0, 0, 0, false, false, false, false, 0, null);
	return i_el.dispatchEvent(evt);
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

