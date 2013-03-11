c_logCount = 0;
c_elLogs = [];
c_lastLog = null;
c_lastLogCount = 1;

var $ = function( id ) { return document.getElementById( id ); };

function c_Init()
{
	cgru_ConstructSettingsGUI();
	cgru_InitParameters();

	document.getElementById('platform').textContent = cgru_Platform;
	document.getElementById('browser').textContent = cgru_Browser;

	c_ApplyStyles();
}

function c_ApplyStyles()
{
	if( p_PLAYER ) return;
	document.body.style.background = localStorage.background;
	document.body.style.color = localStorage.text_color;
	$('header').style.background = localStorage.background;
	$('footer').style.background = localStorage.background;
	$('navig').style.background = localStorage.background;
	$('sidepanel').style.background = localStorage.background;
}

function c_GetHashPath()
{
	var path = document.location.hash;
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
		obj = null;
	}

	return obj;
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
			c_Error('RULES file "'+attr+'" in "'+i_dir.dir+'" is invalid.');
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
	u_el.cycle.classList.remove('timeout');
	u_el.cycle.style.opacity = '1';

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

	if( u_el && u_el.cycle )
		setTimeout('u_el.cycle.classList.add("timeout");u_el.cycle.style.opacity = ".1";',1)
//	u_el.cycle.style.opacity = '.1';
}

function c_DT_StrFromSec( i_time) { return c_DT_StrFromMSec( i_time*1000);}
function c_DT_StrFromMSec( i_time)
{
	var date = new Date(i_time);
	date = date.toString();
	date = date.substr( 0, date.indexOf(' GMT'));
	return date;
}
function c_DT_CurSeconds() { return Math.round((new Date).valueOf()/1000);}
function c_DT_FormStrNow() { return c_DT_FormStrFromSec( c_DT_CurSeconds());}
function c_DT_SecFromStr( i_str) { return  Math.round( c_DT_DateFromStr( i_str).valueOf()/1000);}
function c_DT_DateFromStr( i_str)
{
	var nums = c_Strip( i_str).split(/\D{1,}/);
	c_Error('Invalid date: "'+i_str+'"');
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
function c_DT_DaysLeft( i_sec ) { return ( i_sec - (new Date()/1000) ) / ( 60 * 60 * 24 ); }


function c_ElDisplayToggle( i_el)
{
	if( i_el.style.display == 'none')
		i_el.style.display = 'block';
	else
		i_el.style.display = 'none';
}

function c_GetUserTitle( i_uid)
{
	if( i_uid == null ) i_uid = localStorage.user_name;
	if( g_users && g_users[i_uid] && g_users[i_uid].title && ( g_users[i_uid].title != 'Coordinator'))
		return g_users[i_uid].title;
	return i_uid;
}

function c_CompareFolders(a,b)
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

function c_Strip( i_str) { return i_str.replace(/^\s+|\s+$|^<br>|<br>$/g,''); }

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

function c_CreateOpenButton( i_el, i_path)
{
	var el = document.createElement('div');
	i_el.appendChild( el);
	el.textContent = 'Open';
	el.classList.add('cmdexec');
	var cmd = RULES.cmdexec.open_folder;
	cmd = cmd.replace('@PATH@', cgru_PM('/'+RULES.root + i_path));
	el.setAttribute('cmdexec', JSON.stringify([cmd]));
	return el;
}

function c_GetRuFilePath( i_file ) { return RULES.root + g_CurPath() + '/' + RULES.rufolder + '/' + i_file; }
function c_RuFileExists( i_file)
{
	if( g_elCurFolder.m_dir == null ) return false;
	if( g_elCurFolder.m_dir.rufiles == null ) return false;
	if( g_elCurFolder.m_dir.rufiles.length == 0 ) return false;
	if( g_elCurFolder.m_dir.rufiles.indexOf( i_file) == -1 ) return false;
	return true;
}

