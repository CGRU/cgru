c_logCount = 0;

function c_Init()
{
	cgru_ConstructSettingsGUI();
	cgru_InitParameters();

	document.getElementById('platform').textContent = cgru_Platform;
	document.getElementById('browser').textContent = cgru_Browser;
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
	return JSON.parse( JSON.stringify( RULES_TOP));
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
	elLine.innerHTML = '<i><b>#</b>'+c_logCount+' '+time+':</i> '+i_msg;
	var lines = log.getElementsByTagName('div');
	u_el.log.insertBefore( elLine, lines[0]);
	if( lines.length > 100 )
		u_el.log.removeChild( lines[100]);
	c_logCount++;

	if( u_el && u_el.cycle )
		setTimeout('u_el.cycle.classList.add("timeout");u_el.cycle.style.opacity = ".1";',1)
//	u_el.cycle.style.opacity = '.1';
}

function c_DT_StrFromS( i_time) { return c_DT_StrFromMS( i_time*1000);}
function c_DT_StrFromMS( i_time)
{
	var date = new Date(i_time);
	date = date.toString();
	date = date.substr( 0, date.indexOf(' GMT'));
	return date;
}
function c_DT_CurSeconds() { return Math.round((new Date).valueOf()/1000);}

function c_ElDisplayToggle( i_el)
{
	if( i_el.style.display == 'none')
		i_el.style.display = 'block';
	else
		i_el.style.display = 'none';
}

function c_MakeThumbnail( i_sources, i_path)
{
	var input = null;
	for( var i = 0; i < i_sources.length; i++ )
	{
		if( input ) input += ',';
		else input = '';
			input += cgru_PM('/' + RULES.root + i_sources[i], true);
	}
	var output = cgru_PM('/' + RULES.root + i_path + '/'+RULES.rufolder+'/' + RULES.thumbnail.filename, true);
	var cmd = RULES.thumbnail.create_cmd.replace(/@INPUT@/g, input).replace(/@OUTPUT@/g, output);
	n_Request({"cmdexec":{"cmds":[cmd]}}, false);
}

