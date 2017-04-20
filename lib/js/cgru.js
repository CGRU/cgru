cgru_Config = {};
cgru_Environment = {};
cgru_Platform = ['unix'];
cgru_Browser = null;

cgru_params = [];
cgru_params.push(['background','Background', '', 'Enter background style']);
cgru_params.push(['text_color','Text Color', '', 'Enter text color']);

cgru_DialogsAll = [];
cgru_MenusAll = [];
cgru_WindowsAll = [];

cgru_PathSeparators = [' ','"',';','=',',','\'',':'];

function cgru_Info( i_str) { console.log( i_str); }
function cgru_Error( i_str) { console.log('Error: ' + i_str); }

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
		cgru_SetParameter( null, cgru_params[i][1]);
}
function cgru_SetParameter( i_value, i_param)
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
	new cgru_Dialog({"handle":'cgru_SetParameter',"param":i_param,"type":'str',"value":value,"name":'settings',"title":title,"info":info});
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
	new cgru_Dialog({"handle":'cgru_LocalStorageClear',"type":'str',"name":'settings',"title":'Clear Local Storage',"info":'Are You Sure?<br/>Type "yes".'});
}
function cgru_LocalStorageClear( i_value)
{
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
		if( i_obj[i].cgru_config )
			cgru_ConfigJoin( i_obj[i].cgru_config);
		else if( i_obj[i].cgru_environment )
			cgru_Environment = i_obj[i].cgru_environment;

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
	var closed = false;
	if( cgru_DialogsCloseAll()) closed = true;
	if( cgru_MenusCloseAll())   closed = true;
	if( cgru_WindowsCloseAll()) closed = true;
	return closed;
}

function cgru_EscapePopus()
{
	var closed = false;
	if( cgru_DialogsCloseAll())  closed = true;
	if( cgru_MenusCloseAll())    closed = true;
	if( cgru_WindowsEscapeAll()) closed = true;
	return closed;
}

function cgru_DialogsCloseAll()
{
	if( cgru_DialogsAll.length == 0 ) return false;
	while( cgru_DialogsAll.length > 0 )
		cgru_DialogsAll[0].destroy();
	return true;
}
function cgru_MenusCloseAll()
{
	if( cgru_MenusAll.length == 0 ) return false;
	while( cgru_MenusAll.length > 0 )
		cgru_MenusAll[0].destroy();
	return true;
}
function cgru_WindowsCloseAll()
{
	if( cgru_WindowsAll.length == 0 ) return false;
	while( cgru_WindowsAll.length > 0 )
		cgru_WindowsAll[0].destroy();
	return true;
}
function cgru_WindowsEscapeAll()
{
	var toClose = [];
	for( var i = 0; i < cgru_WindowsAll.length; i++ )
		if( cgru_WindowsAll[i].closeOnEsc )
			toClose.push( cgru_WindowsAll[i]);
	if( toClose.length == 0 ) return false;
	for( var i = 0; i < toClose.length; i++ )
		toClose[i].destroy();
	return true;
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

function cgru_RulesLink( i_path)
{
	if( cgru_Config.rules_url == null )
		return null;

	var link = i_path;
	for( var i = 0; i < cgru_Config.projects_root.length; i++)
		if( link.indexOf(cgru_Config.projects_root[i]) == 0 )
			link = link.replace( cgru_Config.projects_root[i], '')

	link = link.replace(/^\/+|\/+$/g,'');
	link = link.replace(/\/\//g,'/');

	var folders = link.split('/');
	var link = '';
	for( var i = 0; i < folders.length; i++)
	{
		if( folders[i].length == 0 ) continue;
		link += '/' + folders[i];
		if( i == 3 ) break;
	}

	link = link.replace(/\/\//g,'/');

	link = cgru_Config.rules_url.replace(/^\/+|\/+$/g,'') + link;

	return link;
}

// ############################## PATHS MAP ########################################


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
	if( i_path == null )
	{
		console.log('ERROR: cgru_PM: i_path is null.');
		return;
	}

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

			var pathfound = false;

			if(( 'windows' == cgru_Platform[0] ) && i_toserver )
			{
				path_search = path_search.toLowerCase();
				path_from = path_from.replace(/\//g,'\\').toLowerCase();
			}

			if( path_search.indexOf( path_from) == 0)
				pathfound = true;
			else
			{
				path_from = path_from.replace(/\\/g,'/')
				if( path_search.indexOf( path_from) == 0 )
					pathfound = true;
			}

			if( pathfound)
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


// ############################## CMDEXEC #######################################

function cgru_CmdExecCreate( i_args)
{
	var i_elParent = i_args.parent;
	var i_elType   = i_args.type;
	var i_label    = i_args.label;
	var i_tooltip  = i_args.tooltip;
	var i_open     = i_args.open;
	var i_cmd      = i_args.cmd;
	var i_cmds     = i_args.cmds;

	if( i_elType == null ) i_elType = 'div';
	var el = document.createElement( i_elType);

	if( i_elParent )
		i_elParent.appendChild( el);

	if( i_label )
		el.textContent = i_label;

	if( i_tooltip )
		el.title = i_tooltip;

	cgru_CmdExecProcess({"element":el,"open":i_open,"cmd":i_cmd,"cmds":i_cmds});

	return el;
}
function cgru_CmdExecProcess( i_args)
{
	var i_el   = i_args.element;
	var i_open = i_args.open;
	var i_cmd  = i_args.cmd;
	var i_cmds = i_args.cmds;

	i_el.classList.add('cgru_cmdexec');
	i_el.m_open = i_open;
	i_el.m_cmd  = i_cmd;
	i_el.m_cmds = i_cmds;
	i_el.onclick = function(e){e.stopPropagation();return false;}
	i_el.ondblclick = cgru_CmdExecClicked;
}
function cgru_CmdExecClicked( i_evt)
{
	i_evt.stopPropagation();

	var el = i_evt.currentTarget;
	el.classList.remove('timeout');
	el.classList.add('clicked');

    var cmdexec = {};

	var cmds = [];
	if( el.m_cmd )
		cmds.push( el.m_cmd);
	if( el.m_cmds )
		for( var i = 0; i < el.m_cmds.length; i++)
			cmds.push( el.m_cmds[i]);
	if( cmds.length )
	{
        cmdexec.cmds = cmds;
        for( var i = 0; i < cmds.length; i++)
            cgru_Info('Executing:\n' + cmds[i]);
	}

    if( el.m_open )
    {
        cmdexec.open = el.m_open;
        cgru_Info('Opening:\n' + cmdexec.open);
    }

    var headers = {};
    headers.username = 'browser';
    headers.hostname = 'localhost';

    var xhr = new XMLHttpRequest;
    xhr.open('POST', 'https://localhost:' + cgru_Config.keeper_port_https + '/', true);
    xhr.send( JSON.stringify({"headers":headers,"cmdexec":cmdexec}));
    xhr.onreadystatechange = cgru_CmdExecHandler;
    xhr.m_el = el;
}
function cgru_CmdExecHandler()
{
	//console.log(this);
	var el = this.m_el;

	if( this.status == 0 )
	{
		el.classList.add('error');
		cgru_Error('Can`t execute commands. Ensure that Keeper is running and authorized.');
		return;
	}

	el.classList.remove('error');

	if( this.readyState == 4 )
	{
		// Everythin is OK.
		setTimeout( cgru_CmdExecFinishOK, 500, el);
	}
}
function cgru_CmdExecFinishOK( i_el)
{
	i_el.classList.add('timeout');
	i_el.classList.remove('clicked');
}
function cgru_CmdExecCreateOpen( i_args)
{
	var i_parent = i_args.parent;
	var i_path   = i_args.path;
	var i_type   = i_args.type;

	var tooltip = 'Open location in a file browser.\nDouble click to run.'

	var el = cgru_CmdExecCreate({"parent":i_parent,"type":i_type,"open":i_path,"tooltip":tooltip});

	el.classList.add('open');

	return el;
}



// ############################## DIALOG ########################################

function cgru_Dialog( i_args)
{
	this.window    = i_args.wnd;
	this.receiver  = i_args.receiver;
	this.handle    = i_args.handle;
	this.type      = i_args.type;
	this.param     = i_args.param;
	this.value     = i_args.value;
	this.name      = i_args.name;
	this.title     = i_args.title;
	this.info      = i_args.info;

	if( this.window == null ) this.window = window;
	this.document = this.window.document;
	this.elParent = this.document.body;

	if( this.receiver == null ) this.receiver = this.window;

	if( this.type == null )
		this.type = 'str';
	else if( this.type == 'text' || this.type == 'json' )
		this.textmode = true;

	if( this.name == null ) this.name = 'set_parameter';

	if( this.title == null ) this.title = 'Set Parameter';

	if( this.info == null )
	{
		this.info = 'Enter "'+this.param+'"';
		switch( this.type)
		{
		case 'num': this.info += ' Number'; break;
		case 'str': this.info += ' String'; break;
		case 'reg': this.info += ' RegExp'; break;
		case 'hrs': this.info += ' Hours'; break;
		case 'tim': this.info += ' Time'; break;
		case 'bl0': case 'bl1': this.info += ' Boolean (0|1|false|true)';
		case 'json': this.info += ' JSON'; break;
		case 'text': this.info += ' Text'; break;
		}
		this.info += ':';
	}

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
	this.elTitle.innerHTML = this.title;

	this.elInfo = this.document.createElement('div');
	this.elContent.appendChild( this.elInfo);
	this.elInfo.classList.add('cgru_dialog_info');
	this.elInfo.innerHTML = this.info;

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
		this.elInput.textContent = cgru_TimeToStr( this.value);
	else if( this.value )
	{
		if( this.type == 'hrs')
			this.elInput.textContent = this.value * 1.0 / 3600;
		else if( this.textmode )
			this.elInput.innerHTML = this.value.replace(/\n/g,'<br>').replace(/ /g,'&nbsp')
		else
			this.elInput.textContent = this.value;
	}

	// Select current value for fast replacement
	if( this.value && ( this.textmode != true ))
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
		e.stopPropagation();
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
	if( this.textmode ) value = this.elInput.innerHTML;
	value = value.replace(/^\s+|\s+$/g,'');
	value = value.replace(/<br>/g,'\n');
	value = value.replace(/<\/?pre>/g,'');
	value = value.replace(/&nbsp;/g,' ');

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

	this.receiver[this.handle]( value, this.param);
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


//######################## MENU ##################################################3

function cgru_Menu( i_args)
{
	this.document  = i_args.doc;
	this.elParent  = i_args.parent;
	this.receiver  = i_args.receiver;
	this.name      = i_args.name;
	this.onDestroy = i_args.destroy;
	if( i_args.evt )
	{
		this.posX = i_args.evt.pageX;
		this.posY = i_args.evt.pageY;
		i_args.evt.stopPropagation();
	}

	if( this.document == null ) this.document = document;

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
	if( this.onDestroy && this.receiver && this.receiver[this.onDestroy] )
		this.receiver[this.onDestroy]();
	var index = cgru_MenusAll.indexOf( this);
	if( index >= 0 ) cgru_MenusAll.splice( index, 1);
	this.elParent.removeChild( this.elRoot);
}
cgru_Menu.prototype.show = function( i_evt)
{
	if( i_evt )
	{
		this.posX = i_evt.pageX;
		this.posY = i_evt.pageY;
	}

	this.elRoot.style.display = 'block';

	var w = this.elRoot.offsetWidth;
	var h = this.elRoot.offsetHeight;

	if( this.posX + w > window.innerWidth  + window.pageXOffset) this.posX -= w;
	if( this.posY + h > window.innerHeight + window.pageYOffset) this.posY -= h;

	this.elRoot.style.left = this.posX + 'px';
	this.elRoot.style.top  = this.posY + 'px';
}
cgru_Menu.prototype.addItem = function( i_args)
{
	if( i_args == null ) i_args = {};
	if(( i_args.name == null ) && ( this.elRoot.lastChild != null ))
	{
		this.elRoot.lastChild.style.borderBottom = '3px solid #888';
		return;
	}

	var el = this.document.createElement('div');
	this.elRoot.appendChild( el);
	el.classList.add('cgru_menu_item');
	el.cgru_Menu = this;

	el.innerHTML = i_args.label == null ? i_args.name : i_args.label;
	el.name      = i_args.name;
	el.param     = i_args.param == null ? i_args.name : i_args.param;
	el.receiver  = i_args.receiver;
	el.handle    = i_args.handle;

	if( i_args.receiver == 'cgru_cmdexec')
	{
		cgru_CmdExecProcess({"element":el,"cmds":i_args.handle});
		return;
	}

	if( i_args.enabled !== false )
	{
		el.onmousedown = this.onMouseUpDown;
		el.onmouseup   = this.onMouseUpDown;
	}
	else
		el.classList.add('disabled');
}
cgru_Menu.prototype.onMouseUpDown = function( i_evt)
{
	var el = i_evt.currentTarget;
	if( el == null ) return;
	if( el.cgru_MenuItemToggled ) return;
	el.cgru_MenuItemToggled = true;
	if( el.cgru_Menu == null ) return;
	el.receiver[el.handle]( el.param, i_evt);
	el.cgru_Menu.destroy();
}



//######################## WINDOW ##################################################3

function cgru_Window( i_args)
{
	this.name = i_args.name;
	if( i_args.title == null ) i_args.title = i_args.name;
	this.closeOnEsc = true;
	if( i_args.closeOnEsc === false ) this.closeOnEsc = false;
	this.window = window;
	if( i_args.wnd ) this.window = i_args.wnd;
	this.document = this.window.document;
	this.elParent = this.document.body;

	this.elRoot = this.document.createElement('div');
	this.elParent.appendChild( this.elRoot);
	this.elRoot.classList.add('cgru_window_back');
	if( i_args.padding )
		this.elRoot.style.padding = i_args.padding;

	this.elWindow = this.document.createElement('div');
	this.elRoot.appendChild( this.elWindow);
	this.elWindow.classList.add('cgru_window_window');

	this.elClose = this.document.createElement('div');
	this.elWindow.appendChild( this.elClose);
	this.elClose.classList.add('cgru_window_close');
	this.elClose.textContent = 'x';
	this.elClose.cgru_window = this;
	this.elClose.onclick = function(e){ e.currentTarget.cgru_window.destroy();}

	this.elTitle = this.document.createElement('div');
	this.elWindow.appendChild( this.elTitle);
	this.elTitle.classList.add('cgru_window_title');
	this.elTitle.textContent = i_args.title;
	this.elTitle.cgru_window = this;
	this.elTitle.ondblclick = function(e){ e.currentTarget.cgru_window.destroy();}

	this.elContent = this.document.createElement('div');
	this.elWindow.appendChild( this.elContent);
	this.elContent.classList.add('cgru_window_content');

	if( i_args.addClasses )
		for( var i = 0; i < i_args.addClasses.length; i++)
		{
			this.elRoot.classList.add( i_args.addClasses[i]);
			this.elWindow.classList.add( i_args.addClasses[i]);
			this.elContent.classList.add( i_args.addClasses[i]);
		}

	cgru_WindowsAll.push( this);
}
cgru_Window.prototype.setTitle = function( i_title )
{
	this.elTitle.textContent = i_title;
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
	if( this.onDestroy ) this.onDestroy( this)
}

function cgru_ShowObject( i_obj, i_title, i_window)
{
	var wnd = new cgru_Window({"name":i_title,"wnd":i_window});
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

