cgru_Config = {};
cgru_Platform = ['unix'];
cgru_Browser = null;

cgru_params = [];
cgru_params.push(['background','Background', '#A0A0A0', 'Enter Background Style']);
cgru_params.push(['text_color','Text Color', '#000000', 'Enter Text Color']);

cgru_DialogsAll = [];
cgru_MenusAll = [];
cgru_WindowsAll = [];

cgru_PathSeparators = [' ','"',';','=',',','\'',':'];

function cgru_Init()
{
	if( navigator.platform.indexOf('Linux') != -1 )
		cgru_Platform.push('linux');
	else if( navigator.platform.indexOf('Mac') != -1 )
		cgru_Platform.push('macosx');
	else if( navigator.platform.indexOf('Win') != -1 )
		cgru_Platform = ['windows'];

	if( navigator.userAgent.indexOf('Firefox') != -1 )
		cgru_Browser = 'firefox';
	else if( navigator.userAgent.indexOf('Chrome') != -1 )
		cgru_Browser = 'chrome';
	else if( navigator.userAgent.indexOf('Opera') != -1 )
		cgru_Browser = 'opera';
	else if( navigator.userAgent.indexOf('MSIE') != -1 )
		cgru_Browser = 'iexplorer';
}

function cgru_InitParameters()
{
	for( var i = 0; i < cgru_params.length; i++)
		cgru_SetParameter( cgru_params[i][1]);
}
function cgru_SetParameter( i_param, i_value)
{
	var initial = null;
	var title = null;
	var pos;
	for( pos = 0; pos < cgru_params.length; pos++)
		if( i_param == cgru_params[pos][1] )
		{
			title = cgru_params[pos][2];
			initial = cgru_params[pos][3];
			break;
		}

	var onchange = 'cgru_params_OnChange';
	if( i_value == null )
	{
		onchange = null;
		i_value = localStorage[i_param];
	}
	if( i_value == null )
		i_value = initial;
//window.console.log(i_param+'='+i_value);
	if( i_value == null )
	{
		cgru_SetParameterDialog( i_param)
		return;
	}

	localStorage[i_param] = i_value;

	cgru_params[pos][0].innerHTML = i_value;

	if( onchange && window[onchange] ) window[onchange]( i_param, i_value);
}

function cgru_SetParameterDialog( i_param)
{
	var value = '';
	var title = 'Set Parameter';
	var info = null; 
	for( var i = 0; i < cgru_params.length; i++)
		if( i_param == cgru_params[i][1] )
		{
			title = 'Set '+cgru_params[i][2];
			value = cgru_params[i][3];
			info = cgru_params[i][4];
		}
	if( localStorage[i_param])
		value = localStorage[i_param];
	new cgru_Dialog( window, window, 'cgru_SetParameter', i_param, 'str', value, 'settings', title, info);
}
function cgru_ConstructSettingsGUI()
{
	var elParams = document.getElementById('cgru_parameters');
	for( var i = 0; i < cgru_params.length; i++)
	{
		var elParam = document.createElement('div');
		elParam.classList.add('cgru_parameter');
		elParams.appendChild( elParam);

		var elLabel = document.createElement('div');
		elParam.appendChild( elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = cgru_params[i][1];

		var elVariable = document.createElement('div');
		elParam.appendChild( elVariable);
		elVariable.classList.add('variable');

		var elButton = document.createElement('div');
		elParam.appendChild( elButton);
		elButton.classList.add('button');
		elButton.textContent = 'Edit';
		elButton.param = cgru_params[i][0];
		elButton.onclick = function(e){cgru_SetParameterDialog(e.currentTarget.param);}

		cgru_params[i].splice( 0, 0, elVariable);
	}
}
function cgru_LocalStorageClearClicked()
{
	new cgru_Dialog( window, window, 'cgru_LocalStorageClear', 'local_storage_clear', 'str', '', 'settings', 'Clear Local Storage', 'Are You Sure?<br/>Type "yes".');
}
function cgru_LocalStorageClear( i_name, i_value)
{
	if( i_name && i_value )
		if( i_value != 'yes' )
			return;
	localStorage.clear();
	cgru_InitParameters();
}


function cgru_ConfigLoad( i_obj)
{
	if(( i_obj.length == null ) || ( i_obj.length == 0))
		return false;

	for( var i = 0; i < i_obj.length; i++)
		cgru_ConfigJoin( i_obj[i].cgru_config);

	return true;
}

function cgru_ConfigJoin( i_obj)
{
	if( i_obj == null )
		return;

	for( var attr in i_obj)
	{
		if( attr.length < 1 ) continue;
		if( attr.charAt(0) == '-') continue;
		if( attr.charAt(0) == ' ') continue;
		if( attr == 'include') continue;
		if( attr.indexOf('OS_') == 0 )
		{
			for( var i = 0; i < cgru_Platform.length; i++)
				if( attr == ('OS_'+cgru_Platform[i]))
					cgru_ConfigJoin( i_obj[attr]);
			continue;
		}
		cgru_Config[attr] = i_obj[attr];
	}
}

function cgru_ClosePopus()
{
	cgru_DialogsCloseAll();
	cgru_MenusCloseAll();
	cgru_WindowsCloseAll();
}

function cgru_EscapePopus()
{
	cgru_DialogsCloseAll();
	cgru_MenusCloseAll();
	cgru_WindowsEscapeAll();
}

function cgru_DialogsCloseAll()  { while( cgru_DialogsAll.length > 0 ) cgru_DialogsAll[0].destroy(); }
function cgru_MenusCloseAll()    { while( cgru_MenusAll.length   > 0 ) cgru_MenusAll[0].destroy(); }
function cgru_WindowsCloseAll()  { while( cgru_WindowsAll.length > 0 ) cgru_WindowsAll[0].destroy(); }
function cgru_WindowsEscapeAll()
{
	var wnds = [];
	for( var i = 0; i < cgru_WindowsAll.length; i++ )
		if( cgru_WindowsAll[i].closeOnEsc )
			wnds.push( cgru_WindowsAll[i]);
	for( var i = 0; i < wnds.length; i++ )
		cgru_WindowsAll[i].destroy();
}

function cgru_PathIsAbsolute( i_path)
{
	if( i_path.indexOf('\\')==  0 ) return true;
	if( i_path.indexOf('/') ==  0 ) return true;
	if( i_path.indexOf(':') != -1 ) return true;
	return false;
}
function cgru_PathJoin( i_dir, i_path)
{
	if( cgru_PathIsAbsolute( i_path))
		return i_path;
	var path = i_dir;
	if( cgru_Platform[0] == 'windows' ) path += '\\';
	else path += '/';
	path += i_path;
	return path;
}

function cgru_PM_findPathEnd( i_path)
{
	var pos = 0;
	if( i_path.length <= 1 ) return 1;
	while( pos < i_path.length )
	{
		pos++;
		if( pos >= i_path.length ) break;
		if( cgru_PM_isSeparator( i_path.charAt( pos))) break;
	}
	return pos;
}
function cgru_PM_findNextPos( i_pos, i_path)
{
	var pos = i_pos;
	if( pos >= i_path.length) return -1;
	if( i_path.charAt(pos) != ' ' ) pos += cgru_PM_findPathEnd( i_path.substring(pos));
	while( pos < i_path.length )
		if( cgru_PM_isSeparator( i_path.charAt(pos))) pos++;
		else break;
	if( pos >= i_path.length) return -1;
	return pos;
}
function cgru_PM_isSeparator( i_char){for(var i=0;i<cgru_PathSeparators.length;i++)if(i_char==cgru_PathSeparators[i])return true;return false;}
function cgru_PM_findSeparator( i_path)
{
	var sep = ''
	var seppos1 = i_path.indexOf('\\');
	var seppos2 = i_path.indexOf('/');
	if( seppos1 != -1 )
	{
		if( seppos2 != -1 )
		{
			if( seppos1 < seppos2 ) sep = i_path.charAt(seppos1);
			if( seppos2 < seppos1 ) sep = i_path.charAt(seppos2);
		}
		else sep = i_path.charAt(seppos1);
	}
	else if( seppos2 != -1 ) sep = i_path.charAt(seppos2);
	return sep;
}
function cgru_PM_replaceSeparators( i_path, i_path_from, i_path_to)
{
	var newpath = i_path;
	var sep_from = cgru_PM_findSeparator( i_path_from);
	var sep_to = cgru_PM_findSeparator( i_path_to);
	if( sep_from == sep_to) return newpath;
	if(( sep_from == '' ) || ( sep_to == '' )) return newpath;
	var pathend = cgru_PM_findPathEnd( newpath);
	if(( pathend > 1 ) && ( pathend <= newpath.length ))
	{
		var part1 = newpath.substring( 0, pathend);
		var part2 = newpath.substring( pathend);
		part1 = part1.split( sep_from).join( sep_to);
		newpath = part1 + part2;
	}
	return newpath;
}
function cgru_PM( i_path, i_toserver, i_unixSeparators)
{
	var newpath = i_path;
	if( newpath.length < 1 ) return newpath;
	if( cgru_Config.pathsmap == null ) return newpath;

	var position = 0;
	
	while( cgru_PM_isSeparator( newpath.charAt(position)))
	{
		position++;
		if( position >= newpath.length) return newpath;
	}
	var maxcycles = newpath.length;
	var cycle = 0
	while( position != -1 )
	{
		var path_search = newpath.substring(position);
		for( var i = 0; i < cgru_Config.pathsmap.length; i++)
		{
			if( i_toserver)
			{
				var path_from = cgru_Config.pathsmap[i][0];
				var path_to   = cgru_Config.pathsmap[i][1];
			}
			else
			{
				var path_from = cgru_Config.pathsmap[i][1];
				var path_to   = cgru_Config.pathsmap[i][0];
			}

			var pathfounded = false;

			if(( 'windows' == cgru_Platform[0] ) && i_toserver )
			{
				path_search = path_search.toLowerCase();
				path_from = path_from.replace(/\//g,'\\').toLowerCase();
			}

			if( path_search.indexOf( path_from) == 0)
				pathfounded = true;
			else
			{
				path_from = path_from.replace(/\\/g,'/')
				if( path_search.indexOf( path_from) == 0 )
					pathfounded = true;
			}

			if( pathfounded)
			{
				var part1 = newpath.substring( 0, position);
				var part2 = newpath.substring( position+path_from.length);
				if( i_unixSeparators != true )
					if(( 'windows' == cgru_Platform[0] ) && ( i_toserver != true ))
						path_to = path_to.replace(/\//,'\\');
				part2 = cgru_PM_replaceSeparators( part2, path_from, path_to);
				newpath = part1 + path_to + part2;
				position = (part1 + path_to).length;
				newpath = part1 + path_to + part2;
				break;
			}
		}

		var old_position = position;
		position = cgru_PM_findNextPos( position, newpath);
		if(( position != -1 ) && ( position <= old_position ))
			break;

		cycle++;
		if( cycle > maxcycles )
			break;
	}

	return newpath;
}



function cgru_Dialog( i_window, i_reciever, i_handle, i_parameter, i_type, i_value, i_dialogname, i_title, i_info)
{
	this.window = i_window;
	this.document = this.window.document;
	this.elParent = this.document.body;
	this.reciever = i_reciever;
	this.handle = i_handle;
	this.type = i_type;
	if( i_type == 'text' || i_type == 'json' )
		this.textmode = true;
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
	if( this.textmode )
		this.elContent.classList.add('cgru_dialog_textmode');

	this.elTitle = this.document.createElement('div');
	this.elContent.appendChild( this.elTitle);
	this.elTitle.classList.add('cgru_dialog_title');
	if( i_title == null ) i_title = 'Set Parameter';
	this.elTitle.innerHTML = i_title;

	this.elInfo = this.document.createElement('div');
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
		case 'json': i_info += ' JSON'; break;
		case 'text': i_info += ' Text'; break;
		}
		i_info += ':';
	}
	this.elInfo.innerHTML = i_info;

	this.elInput = this.document.createElement('div');
	this.elInput.classList.add('cgru_dialog_input');
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
		else if( this.textmode )
			this.elInput.innerHTML = i_value.replace(/\n/g,'<br>').replace(/ /g,'&nbsp')
		else
			this.elInput.textContent = i_value;
	}

	// Select current value for fast replacement
	if( i_value && ( this.textmode != true ))
	{
		var range = this.document.createRange();
		range.setStart( this.elInput, 0);
		range.setEnd( this.elInput, 1);
		var sel = this.window.getSelection();
		sel.removeAllRanges();
		sel.addRange(range);
	}

	cgru_DialogInput = this.elInput;
	this.window.setTimeout(function(){cgru_DialogInput.focus();}, 100);
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

	if( this.textmode ) return;

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
	var value = this.elInput.textContent;
	if( this.type == 'json') value = this.elInput.innerHTML;
	value = value.replace(/^\s+|\s+$/g,'');
	value = value.replace(/<br>/g,'\n')
	value = value.replace(/&nbsp;/g,' ')

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
	case 'str':
		value = value.replace(/^\s+|\s+$|^\n+|\n+$|^<br>|<br>$/g,'');
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
	case 'json':
		try { obj = JSON.parse( value);}
		catch( err )
		{
			this.displayError( err.message);
			return;
		}
	}

	if( this.parameter == null ) this.parameter = value;
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

	if( i_receiver == 'cmdexec')
	{
		el.classList.add('cmdexec');
		if( i_handle )
			el.setAttribute('cmdexec', JSON.stringify( i_handle));
		return;
	}

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

function cgru_Window( i_name, i_title)
{
	this.name = i_name;
	if( i_title == null ) i_title = i_name;
	this.closeOnEsc = true;
	this.window = window;
	this.document = this.window.document;
	this.elParent = this.document.body;

	this.elRoot = this.document.createElement('div');
	this.elParent.appendChild( this.elRoot);
	this.elRoot.classList.add('cgru_window_back');

	this.elWindow = this.document.createElement('div');
	this.elRoot.appendChild( this.elWindow);
	this.elWindow.classList.add('cgru_window_window');

	this.elContent = this.document.createElement('div');
	this.elWindow.appendChild( this.elContent);
	this.elContent.classList.add('cgru_window_content');

	this.elTitle = this.document.createElement('div');
	this.elWindow.appendChild( this.elTitle);
	this.elTitle.classList.add('cgru_window_title');
	this.elTitle.textContent = i_title;
	this.elTitle.cgru_window = this;
	this.elTitle.ondblclick = function(e){ e.currentTarget.cgru_window.destroy();}

	this.elClose = this.document.createElement('div');
	this.elWindow.appendChild( this.elClose);
	this.elClose.classList.add('cgru_window_close');
	this.elClose.textContent = 'x';
	this.elClose.cgru_window = this;
	this.elClose.onclick = function(e){ e.currentTarget.cgru_window.destroy();}

	cgru_WindowsAll.push( this);
}
cgru_Window.prototype.resize = function( i_width, i_height )
{
	if( i_height == null ) i_height = i_width;
	this.elWindow.style.width  = i_width + '%';
	this.elWindow.style.height = i_height + '%';
	this.elWindow.style.left = (50-(i_width/2)) + '%';
	this.elWindow.style.top = (50-(i_height/2)) + '%';
}
cgru_Window.prototype.destroy = function()
{
	var index = cgru_WindowsAll.indexOf( this);
	if( index >= 0 ) cgru_WindowsAll.splice( index, 1);
	this.elParent.removeChild( this.elRoot);
}

function cgru_ShowObject( i_obj, i_title)
{
	var wnd = new cgru_Window( i_title, i_title);
	wnd.elContent.innerHTML = JSON.stringify( i_obj, null, '&nbsp&nbsp&nbsp&nbsp').replace(/\n/g,'<br/>');
}
function cgru_ConfigShow() { cgru_ShowObject( cgru_Config, 'CGRU Configuration');}
function cgru_LocalStorageShow() { cgru_ShowObject( localStorage, 'Browser Local Storage');}

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

