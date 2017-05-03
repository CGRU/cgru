SERVER = null;
RULES = {};
RULES.rufolder = 'rules';
RULES_TOP = {};

c_movieTypes = ['mpg','mpeg','mov','avi','mp4','m4v','webm','ogg','ogv','mxf','flv'];
c_movieTypesHTML = ['mp4','webm','ogg'];
c_imageTypes = ['jpg','jpeg','png','exr','dpx','tga','tif','tiff','psd','xcf'];
c_imageEditableTypes = ['jpg','jpeg','png'];
c_archives = ['zip','rar','7z','001'];

c_logCount = 0;
c_elLogs = [];
c_lastLog = null;
c_lastLogCount = 1;

var $ = function( id ) { return document.getElementById( id ); };

function c_Init()
{
	cgru_ConstructSettingsGUI();
	cgru_InitParameters();
	cgru_Info = c_Info;
	cgru_Error = c_Error;

	document.getElementById('platform').textContent = cgru_Platform;
	document.getElementById('browser').textContent = cgru_Browser;

	u_ApplyStyles();
}

function c_GetHash()
{
	var path = decodeURI( document.location.hash);
//window.console.log( 'hash = ' + path);
	if( path.indexOf('#') == 0 )
		path = path.substr(1);
	else
		path = '/';

	if( path.charAt(0) != '/' )
		path = '/'+path;

	while( path.indexOf('//') != -1 )
		path = path.replace(/\/\//g,'/');

	if(( path.length > 1 ) && ( path.charAt(path.length-1) == '/'))
		path = path.substr( 0, path.length-1);

	return path;
}

function c_Parse( i_data)
{
	if( i_data == null ) return null;

	var obj = null;
	try { obj = JSON.parse( i_data);}
	catch( err)
	{
		c_Error( err.message+'<br/><br/>'+i_data);
		console.log( err.message+'\n'+i_data);
		obj = null;
	}

	if( obj && obj.nonce && SERVER ) SERVER.nonce = obj.nonce;

	return obj;
}

function c_NullOrErrorMsg( i_obj)
{
//console.log(JSON.stringify(i_obj));
	if( i_obj == null )
	{
		c_Error('No responce received.');
		return true;
	}
	if( i_obj.error )
	{
		c_Error( i_obj.error);
		return true;
	}
	return false;
}
function c_NullOrErrorCmd( i_obj, i_name)
{
//console.log(JSON.stringify(i_obj));
	if( c_NullOrErrorMsg( i_obj ))
		return true;

	if( i_obj.cmdexec == null )
	{
		c_Error('Null command execution data.');
		return true;
	}

	if( ! i_obj.cmdexec.length )
	{
		c_Error('Zero command execution data.');
		return true;
	}

	var ret = false;
	for( var i = 0; i < i_obj.cmdexec.length; i++)
	{
		var obj = i_obj.cmdexec[i];
		if( obj[i_name] == null )
		{
			c_Error('Command execution has no "' + i_name + '" data.');
			ret = true;
		}

		if( c_NullOrErrorMsg( obj[i_name]))
			ret = true;
	}

	return ret;
}

function c_CloneObj( i_obj)
{
	return JSON.parse( JSON.stringify( i_obj));
}

function c_RulesMergeDir( o_rules, i_dir)
{
	if( i_dir == null ) return;
	if( i_dir.rules == null ) return;
	for( var attr in i_dir.rules)
	{
		var obj = i_dir.rules[attr];
		if( obj == null )
			c_Error('RULES file "'+attr+'" in "'+g_CurPath()+'/'+RULES.rufolder+'" is invalid.');
		else
			c_RulesMergeObjs( o_rules, obj);
	}
}

function c_RulesMergeObjs( o_rules, i_rules_new)
{
	if(( o_rules == null ) || ( i_rules_new == null )) return;

	for( var attr in i_rules_new)
	{
		if( attr.length < 1 ) continue;
		if( attr.charAt(0) == '-') continue;
		if( attr.charAt(0) == ' ') continue;
		if( attr.indexOf('OS_') == 0 )
		{
			for( var i = 0; i < cgru_Platform.length; i++)
				if( attr == ('OS_'+cgru_Platform[i]))
					c_RulesMergeObjs( o_rules, i_rules_new[attr]);
			continue;
		}
		if(( typeof( i_rules_new[attr]) == 'object') && ( o_rules[attr] != null ))
		{
			c_RulesMergeObjs( o_rules[attr], i_rules_new[attr]);
			continue;
		}
		o_rules[attr] = i_rules_new[attr];
	}
}
function c_PadZero( i_num, i_len)
{
	if( i_len == null ) i_len = 2;
	var str = ''+i_num;
	while( str.length < i_len) str = '0'+str;
	return str;
}

function c_Info( i_msg, i_log)
{
	if( i_log == null ) i_log = true;
	u_el.info.innerHTML = i_msg;
	if( i_log ) c_Log( i_msg);
}
function c_Error( i_err)
{
	c_Info('<b style="font-size:15px;color:#700">Error:</b> ' + i_err);
}
function c_Log( i_msg)
{
	var date = new Date();
	var time = c_PadZero(date.getHours())+':'+c_PadZero(date.getMinutes())+':'+c_PadZero(date.getSeconds())+'.'+c_PadZero(date.getMilliseconds(),3);
	var elLine = document.createElement('div');
	var lastEl = c_elLogs[c_elLogs.length-1];
	var elLine = lastEl;
	var innerHTML = '<i><b>#</b>'+c_logCount+' '+time+':</i> '+i_msg;
	if( c_lastLog == i_msg )
	{
		c_lastLogCount++;
		innerHTML = c_lastLogCount + 'x ' + innerHTML;
	}
	else
	{
		c_lastLog = i_msg;
		c_lastLogCount = 1;
		elLine = document.createElement('div');
		u_el.log.insertBefore( elLine, lastEl);
		c_elLogs.push( elLine);
		if( c_elLogs.length > 100 ) u_el.log.removeChild( c_elLogs.shift());
	}
	elLine.innerHTML = innerHTML;
	c_logCount++;
}

function c_AuxFolder( i_folder)
{
	if( i_folder.status )
	{
		if( i_folder.status.flags && ( i_folder.status.flags.indexOf('aux') != -1 ))
			return true;

		if( i_folder.status.progress && ( i_folder.status.progress < 0 ))
			return true;
	}

	if( i_folder.name )
	{
		var name = c_PathBase( i_folder.name);
		for( var i = 0; i < RULES.aux_folders.length; i++)
			if( name.toLowerCase().indexOf( RULES.aux_folders[i]) === 0 )
				return true;
	}

	return false;
}

function c_DT_StrFromSec( i_time, i_nosec) { if( i_time == null) return ''; return c_DT_StrFromMSec( i_time*1000, i_nosec);}
function c_DT_StrFromMSec( i_time, i_nosec)
{
	if( i_time == null ) return '';
	var date = new Date(i_time);
	date = date.toString();
	date = date.substr( 0, date.indexOf(' GMT'));
	if( i_nosec )
		date = date.substr( 0, date.lastIndexOf(':'));
	return date;
}
function c_DT_CurSeconds() { return Math.round((new Date).valueOf()/1000);}
function c_DT_FormStrNow() { return c_DT_FormStrFromSec( c_DT_CurSeconds());}
function c_DT_SecFromStr( i_str) { return Math.round( c_DT_DateFromStr( i_str).valueOf()/1000);}
function c_DT_DateFromStr( i_str)
{
	var nums = c_Strip( i_str).split(/\D+/);
//	c_Error('Invalid date: "'+i_str+'"');
	var date = new Date(0);
	if( nums.length < 3 ) return date;
	var day = nums[0];
	var mon = nums[1];
	var year = nums[2];
	var hour = 0;
	var mins = 0;
	if( nums[0].length == 4 ) { day = nums[2]; year = nums[0]; }
	if( nums.length > 3 ) hour = nums[3];
	if( nums.length > 4 ) mins = nums[4];
	if( year < 100 ) year += 2000;

	date.setFullYear( year);
	date.setMonth( mon-1);
	date.setDate( day);
	date.setHours( hour);
	date.setMinutes( mins);

	return date;
}
function c_DT_FormStrFromSec( i_sec)
{
    var date = new Date( i_sec * 1000);
    var str = date.getFullYear();
    str += '.'+c_PadZero(date.getMonth()+1,2);
    str += '.'+c_PadZero(date.getDate(),2);
    str += ' '+c_PadZero(date.getHours(),2);
    str += ':'+c_PadZero(date.getMinutes(),2);
	return str;
}
function c_DT_DurFromSec( i_sec)
{
	var hours = Math.floor( i_sec / 3600 );
	var secs  = i_sec - ( hours * 3600 );
	var mins  = Math.floor( secs / 60 );
	secs      = Math.round( secs - ( mins * 60 ));
	return hours + ':' + c_PadZero( mins) + ':' + c_PadZero( secs);
}
function c_DT_DurFromNow( i_sec) { return c_DT_DurFromSec( (new Date()) / 1000 - i_sec); }
function c_DT_DaysLeft( i_sec ) { return ( i_sec - (new Date()/1000) ) / ( 60 * 60 * 24 ); }

function c_TC_FromFrame( i_frame, fps, clamp)
{
    if( fps == null )
        fps = RULES.fps;

	var sec = Math.floor( i_frame / fps );
	var frm = Math.round( i_frame - sec * fps);
	var min = Math.floor( sec / 60);
	sec = sec - min * 60;
	var hrs = Math.floor( min / 60);
	min = min - hrs * 60;

	if( hrs < 10 ) hrs = '0' + hrs;
	if( min < 10 ) min = '0' + min;
	if( sec < 10 ) sec = '0' + sec;
	if( frm < 10 ) frm = '0' + frm;

    var tc = sec + ':' + frm;
    if(( min != '00') || ( clamp !== true)) tc = min + ':' + tc;
    if(( hrs != '00') || ( clamp !== true)) tc = hrs + ':' + tc;

	return tc;
}

function c_TC_FromSting( i_str)
{
//console.log('c_TC_FromSting: ' + i_str);
	var nums = i_str.split(/\D+/);
	if( nums.length == 0 )
	{
		c_Error('Invalid time code: ' + i_str);
		return null;
	}

	var fps = RULES.fps;

	var frms = 0;
//console.log( nums);
	var mult = [ 1, fps, fps*60, fps*3600];
//console.log( mult);
	var j = 0;
	for( var i = nums.length-1; i >=0; i-- )
	{
//console.log(nums[i]);
		if( nums[i].length == 0 ) continue;
		frms += parseInt(nums[i]) * mult[j];
		j++;
	}

	return frms;
}

function c_ElDisplayToggle( i_el)
{
	if( i_el.style.display == 'none')
		i_el.style.display = 'block';
	else
		i_el.style.display = 'none';
}

function c_IsNotAnArtist(    i_user) { return c_IsUserStateSet( i_user,'notart'   ); }
function c_CanEditPlaylist(  i_user) { return c_IsUserStateSet( i_user,'playlist' ); }
function c_CanAssignArtists( i_user) { return c_IsUserStateSet( i_user,'assignart'); }
function c_CanEditTasks(     i_user) { return c_IsUserStateSet( i_user,'edittasks'); }
function c_CanEditBody(      i_user) { return c_IsUserStateSet( i_user,'editbody'); }
function c_IsUserStateSet(   i_user, i_state)
{
	if( i_user == null ) i_user = g_auth_user;
	if( i_user == null ) return false;

	if((['playlist','assignart','edittasks','editbody']).indexOf( i_state ) != -1 )
		if((['admin','coord','user']).indexOf( i_user.role ) != -1 ) return true;

	if( i_user.states == null ) return false;
	if( i_user.states.indexOf( i_state) != -1 ) return true;

	return false;
}
function c_CanCreateShot( i_user)
{
	if( i_user == null ) i_user = g_auth_user;
	if( i_user == null ) return false;

	if((['admin','coord','user']).indexOf( i_user.role ) != -1 )
		return true;
	return false;
}

// Construct from g_users sorted roles with sorted artists:
// Provide i_users to show specified users even if he is disabled or not an artist
function c_GetRolesArtists( i_users)
{
	var roles_obj = {};
	for( var uid in g_users )
	{
//console.log(g_users[uid].states);
		if(( i_users == null ) || ( i_users[uid] == null ))
		{
			if( g_users[uid].disabled )
				continue;
			if( c_IsNotAnArtist( g_users[uid]))
				continue;
		}

		var role = g_users[uid].role;

		if( roles_obj[role] == null )
			roles_obj[role] = [];

		roles_obj[role].push( g_users[uid]);
	}

	var roles = [];
	for( var role in roles_obj )
	{
		roles_obj[role].sort(function(a,b){return a.title > b.title});
		roles.push({"role":role,"artists":roles_obj[role]});
	}
	roles.sort(function(a,b){return a.role < b.role});

	return roles;
}

function c_GetUserTitle( i_uid, i_guest, i_short)
{
	if( i_uid == null )
	{
		if( g_auth_user == null ) return 'Guest';
		i_uid = g_auth_user.id;
	}
	if( i_uid == null ) return null;

	var title = i_uid;

	if( g_users && g_users[i_uid] )
	{
		if( g_users[i_uid].title ) title = g_users[i_uid].title;
	}
	else if( i_guest && i_guest.title )
	{
		title = i_guest.title;
	}
	
	if( i_short && ( title.length > 4 ))
	{
		title = title.split(' ');
		if( title.length > 1 )
			title = title[0].substr(0,1)+title[1].substr(0,3);
		else
			title = title[0].substr(0,4);
	}
	return title;
}

function c_GetFlagTitle( i_flag) { return c_GetTagProp( i_flag,'flag','title'); }
function c_GetTagTitle(  i_tag ) { return c_GetTagProp( i_tag, 'tag', 'title'); }
function c_GetFlagTip (  i_flag) { return c_GetTagProp( i_flag,'flag','tip'); }
function c_GetTagTip(    i_tag ) { return c_GetTagProp( i_tag, 'tag', 'tip'); }
function c_GetFlagShort( i_flag) { return c_GetTagProp( i_flag,'flag','short', 3); }
function c_GetTagShort(  i_tag ) { return c_GetTagProp( i_tag, 'tag', 'short', 3); }
function c_GetTagProp( i_name, i_type, i_key, i_clamp )
{
	var val = i_name;
	var types = i_type + 's';
	if( RULES[types][i_name] && RULES[types][i_name][i_key] )
		val = RULES[types][i_name][i_key];
	if( i_clamp && ( val.length > i_clamp ))
		val = val.substr(0, i_clamp);
	return val;
}

function c_CompareFiles(a,b)
{
	var attr = 'name';
	if( a[attr] < b[attr]) return -1;
	if( a[attr] > b[attr]) return 1;
	return 0;
}

function c_ElToggleSelected( i_e)
{
	var el = i_e;
	if( i_e.currentTarget ) el = i_e.currentTarget;
	c_ElSetSelected( el, el.m_selected != true );
	return el.m_selected;
}

function c_ElSetSelected( i_e, i_selected )
{
	var el = i_e;
	if( i_e.currentTarget ) el = i_e.currentTarget;
	if( i_selected )
	{
		el.classList.add('selected');
		el.m_selected = true;
	}
	else
	{
		el.classList.remove('selected');
		el.m_selected = false;
	}
}

function c_Strip( i_str) { return i_str.replace(/^\s+|\s+$|^\n+|\n+$|^<br>|<br>$/g,''); }

function c_GetElInteger( i_el)
{
	var str = c_Strip( i_el.textContent );
	if( str.length == 0 ) return null;
	var num = parseInt( str);
	if( isNaN( num ))
	{
		c_Error('Invalid number: "'+str+'"');
		return null;
	}
	return num;
}

function c_FileDragStart( i_evt, i_path)
{
	var el = i_evt.currentTarget;
	var path = c_PathPM_Rules2Client( i_path);
	if( cgru_Platform.indexOf('windows') == -1 )
		path = 'file://' + path;
	var dt = i_evt.dataTransfer;
	dt.setData('text/plain', path);
	dt.setData('text/uri-list', path);
//console.log(path);
}

function c_GetRuFilePath( i_file, i_path )
{
	var path = i_path;
	if( path == null )
		path = g_CurPath();

	path += '/' + RULES.rufolder + '/' + i_file;
	path = RULES.root + path;

	return path;
}
function c_RuFileExists( i_file)
{
	if( g_elCurFolder.m_dir == null ) return false;
	if( g_elCurFolder.m_dir.rufiles == null ) return false;
	if( g_elCurFolder.m_dir.rufiles.length == 0 ) return false;
	if( g_elCurFolder.m_dir.rufiles.indexOf( i_file) == -1 ) return false;
	return true;
}
function c_RuFileAdd( i_file)
{
	if( g_elCurFolder.m_dir == null ) g_elCurFolder.m_dir = {};
	if( g_elCurFolder.m_dir.rufiles == null ) g_elCurFolder.m_dir.rufiles = [];
	if( g_elCurFolder.m_dir.rufiles.indexOf( i_file) == -1 )
		g_elCurFolder.m_dir.rufiles.push( i_file);
}

function c_FileIsMovie( i_file)
{
	var type = i_file.substr( i_file.lastIndexOf('.')+1).toLowerCase();
	if( c_movieTypes.indexOf( type) != -1) return true;
	return false;
}
function c_FileIsMovieHTML( i_file)
{
	var type = i_file.substr( i_file.lastIndexOf('.')+1).toLowerCase();
	if( c_movieTypesHTML.indexOf( type) != -1) return true;
	return false;
}
function c_FileIsImage( i_file)
{
	var type = i_file.substr( i_file.lastIndexOf('.')+1).toLowerCase();
	if( c_imageTypes.indexOf( type) != -1) return true;
	return false;
}
function c_FileCanEdit( i_file)
{
	var type = i_file.substr( i_file.lastIndexOf('.')+1).toLowerCase();
	if( c_imageEditableTypes.indexOf( type) != -1) return true;
	return false;
}
function c_FileCanThumbnail( i_file)
{
	if( c_FileIsImage( i_file) ) return true;
	if( c_FileIsMovie( i_file) ) return true;
	return false;
}
function c_FileIsArchive( i_file)
{
	var type = i_file.substr( i_file.lastIndexOf('.')+1).toLowerCase();
	if( c_archives.indexOf( type) != -1) return true;
	return false;
}

function c_Bytes2KMG( i_bytes)
{
	var lables = ['B','KB','MB','GB','TB'];
	var th = 1, log = 0;
	while( th*1024 < i_bytes ) { th *= 1024, log++ };
//console.log( i_bytes + ': ' + th + ', ' + log);
	return (i_bytes/th).toFixed(1) + ' ' + lables[log];
}

function c_NumToStr( i_num, i_prec)
{
	if( i_prec == null )
		i_prec = 2;

	var str = '';
	if( i_num.toFixed )
		str = i_num.toFixed( i_prec);
	else
		str += i_num;

	str = str.replace(/0*$/,'').replace(/\.$/,'');

	return str;
}

function c_GetThumbFileName( i_file)
{
	var name = c_PathBase( i_file);
	var path = c_PathDir( i_file);
	return path + '/' + RULES.rufolder + '/thumbnail.' + name + '.jpg';
}

function c_MakeThumbnail( i_file, i_func)
{
	var cmd = RULES.thumbnail.create_file;
	cmd = cmd.replace(/@INPUT@/g, RULES.root + i_file);
	cmd = cmd.replace(/@OUTPUT@/g, RULES.root + c_GetThumbFileName( i_file));
	cmd += ' -c ' + RULES.thumbnail.colorspace;
	n_Request({"send":{"cmdexec":{"cmds":[cmd]}},"func":i_func,"file":i_file,"info":'thumbnail'});
}

function c_PathBase( i_file) { return i_file.substr( i_file.lastIndexOf('/')+1);}
function c_PathDir( i_file) { return i_file.substr( 0, i_file.lastIndexOf('/'));}
function c_PathPM_Rules2Server(  i_path) { if(RULES.root_link) return (RULES.root_link + i_path); else return ('/' + RULES.root + i_path); }
function c_PathPM_Rules2Client(  i_path) { return cgru_PM( c_PathPM_Rules2Server( i_path)); }
function c_PathPM_Client2Server( i_path) { return cgru_PM( i_path, true); }
function c_PathPM_Server2Client( i_path) { return cgru_PM( i_path); }
function c_CreateOpenButton( i_args)
{
    if( RULES.has_filesystem === false )
        return null;

    i_args.path = c_PathPM_Rules2Client( i_args.path);

    return cgru_CmdExecCreateOpen( i_args);
}

function c_MD5( i_str) { return hex_md5( i_str);}

function c_EmailFromTitle()
{
	return 'RULES: ' + RULES.company;
}
function c_EmailValidate( i_email)
{ 
	var re = /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
	return re.test( i_email);
}
function c_EmailEncode( i_email)
{
	if( i_email.indexOf('@') == -1 ) return i_email;
	return btoa( JSON.stringify( i_email.split('@')));
}
function c_EmailDecode( i_email)
{
	if( i_email.indexOf('@') != -1 ) return i_email;
	var email = null;
	try { email = JSON.parse( atob( i_email)).join('@');}
	catch( err) { email = null; c_Error( err);}
	return email;
}

function c_GetAvatar( i_user_id, i_guest )
{
	var avatar = null;

	var user = null;
	if( i_user_id )
		user = g_users[i_user_id];
	else
		user = g_auth_user; 

	if( user == null )
	{
		if( i_guest )
			user = i_guest;
		else
			return null
	}

	if( user.avatar && user.avatar.length )
	{
		avatar = user.avatar; 
	}
	else if( user.email && user.email.length )
	{
		avatar = user.email;
		if( i_guest )
			avatar = c_EmailDecode( avatar);
		avatar = c_MD5( avatar.toLowerCase());
		avatar = 'https://www.gravatar.com/avatar/' + avatar;
	}

	if( avatar && avatar.length )
		return avatar;

	return null;
}

function c_LinksProcess( i_text)
{
	return c_LinksToRelative( c_HttpToLinks( i_text));
}
function c_HttpToLinks( i_text)
{
//console.log('c_HttpToLinks in:'+i_text);
	var a_re = /(((\b(https?|ftp|file):\/\/)|(#\/))[-A-Z0-9+&@#\/%?=~_|!:,.;"\{\}]*[-A-Z0-9+&@#\/%=~_|"\{\}])/ig;
	var a_parts = i_text.split(/<a /gi);
	var out = null;
	for( var i = 0; i < a_parts.length; i++)
	{
		text = a_parts[i];
		link = '';
		var pos = text.indexOf('</a>');
		if( pos > 0 )
		{
			link = text.substr( 0, pos);
			text = text.substr( pos);
		}
//console.log('l='+link); console.log('t='+text);
		text = text.replace( a_re, '<a target="_blank" class="link_auto" href="$1">$1</a>');
		text = link + text;

		if( out == null) out = text;
		else out += '<a ' + text;
	}
//console.log('c_HttpToLinks out:'+out);
	return out;
}
function c_LinksToRelative( i_text)
{
	var address = document.location.protocol + '//' + document.location.host + document.location.pathname;
	while( i_text.indexOf( address) != -1 )
		i_text = i_text.replace( address, '');
	return i_text;
}

function c_elMarkupRemove( i_el)
{
	var html = i_el.innerHTML;

	html = html.replace(/<br/gi, '@BR@<br');
	html = html.replace(/<div/gi, '@BR@<div');

	html = html.replace(/<p/gi, '@BR@<p');
	html = html.replace(/\/p>/gi, '/p>@BR@');

	html = html.replace(/<blockquote/gi, '@BR@<blockquote');
	html = html.replace(/\/blockquote>/gi, '/blockquote>@BR@');

	while( html.indexOf('@BR@') == 0 )
		html = html.replace(/@BR@/, '');

	i_el.innerHTML = html;
	i_el.innerHTML = i_el.textContent.replace(/@BR@/g, '<br>');
}

function c_LoadingElSet( i_el)
{
	if( i_el.m_elWaiting )
		return;

	var el = document.createElement('div');
	i_el.appendChild( el);
	el.classList.add('loading');
	var elText = document.createElement('div');
	el.appendChild( elText);
	elText.textContent = 'Loading...';
	i_el.m_elWaiting = el;
}
function c_LoadingElReset( i_el)
{
	if( i_el.m_elWaiting )
	{
		i_el.removeChild( i_el.m_elWaiting);
		i_el.m_elWaiting = null;
	}
}

function c_GuestCheck( i_msg)
{
	if( g_auth_user ) return false;
	if( i_msg == null ) i_msg = 'Guests are not allowed here.';
	c_Error( i_msg);
}
