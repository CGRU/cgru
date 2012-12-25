function c_Init()
{
	for( var i = 0; i < g_elements.length; i++)
		g_el[g_elements[i]] = document.getElementById( g_elements[i]);

	c_Info('HTML body load.');

	c_ConstructSettingsGUI();
	c_InitSettings();

	document.getElementById('platform').textContent = cgru_Platform;
	document.getElementById('browser').textContent = cgru_Browser;
}

function c_Parse( i_data)
{
	var obj = null;
	try { obj = JSON.parse( i_data);}
	catch( err)
	{
		c_Log(err.message+'\n\n'+i_data);
		obj = null;
	}

	return obj;
}

function c_RulesMerge( i_rules_new)
{
	for( var attr in i_rules_new)
		c_RulesMergeObjs( RULES, i_rules_new[attr])
//	c_Info('RULES='+JSON.stringify( RULES));
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
					g_RulesMerge( o_rules, i_rules_new[attr]);
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

function c_HeaderOpenButtonClicked( i_elBtn, i_id, i_pos)
{
	if( i_elBtn.classList.contains('opened'))
	{
		i_elBtn.classList.remove('opened');
		if( i_id == 'header')
		{
			i_elBtn.innerHTML = '&darr;';
			document.getElementById( i_id).style.top = i_pos;
		}
		else
		{
			i_elBtn.innerHTML = '&uarr;';
			document.getElementById( i_id).style.bottom = i_pos;
		}
	}
	else
	{
		i_elBtn.classList.add('opened');
		if( i_id == 'header')
		{
			i_elBtn.innerHTML = '&uarr;';
			document.getElementById( i_id).style.top = '0px';
		}
		else
		{
			i_elBtn.innerHTML = '&darr;';
			document.getElementById( i_id).style.bottom = '0px';
		}
	}
}

function c_ConstructSettingsGUI()
{
	c_Info('Constructing settings GUI.');
	var elParams = document.getElementById('parameters');
	for( var i = 0; i < g_params.length; i++)
	{
		var elParam = document.createElement('div');
		elParam.classList.add('parameter');
//		elParam.style.top = (i*30)+'px';
		elParams.appendChild( elParam);

		var elLabel = document.createElement('div');
		elParam.appendChild( elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = g_params[i][1];

		var elVariable = document.createElement('div');
		elParam.appendChild( elVariable);
		elVariable.classList.add('variable');

		var elButton = document.createElement('div');
		elParam.appendChild( elButton);
		elButton.classList.add('button');
		elButton.textContent = 'Edit';
		elButton.param = g_params[i][0];
		elButton.onclick = function(e){g_SetParameterDialog(e.currentTarget.param);}

		g_params[i].splice( 0, 0, elVariable);
	}
}

function c_InitSettings()
{
	c_Info('Initializing settings.');

	for( var i = 0; i < g_params.length; i++)
		c_SetParameter( g_params[i][1]);
}
function c_SetParameter( i_param, i_value)
{
	var initial = null;
	var title = null;
	var pos;
	for( pos = 0; pos < g_params.length; pos++)
		if( i_param == g_params[pos][1] )
		{
			title = g_params[pos][2];
			initial = g_params[pos][3];
			break;
		}

	if( i_value == null )
		i_value = localStorage[i_param];
	if( i_value == null )
		i_value = initial;
//window.console.log(i_param+'='+i_value);
	if( i_value == null )
	{
		g_SetParameterDialog( i_param)
		return;
	}

	localStorage[i_param] = i_value;

	g_params[pos][0].innerHTML = i_value;
}
function c_SetParameterDialog( i_param)
{
	var value = '';
	var title = 'Set Parameter';
	var info = null; 
	for( var i = 0; i < g_params.length; i++)
		if( i_param == g_params[i][1] )
		{
			title = 'Set '+g_params[i][2];
			value = g_params[i][3];
			info = g_params[i][4];
		}
	if( localStorage[i_param])
		value = localStorage[i_param];
	new cgru_Dialog( window, window, 'c_SetParameter', i_param, 'str', value, 'settings', title, info);
}

function c_LocalStorageClearClicked()
{
	new cgru_Dialog( window, window, 'g_LocalStorageClear', 'local_storage_clear', 'str', '', 'settings', 'Clear Local Storage', 'Are You Sure?<br/>Type "yes".');
}
function c_LocalStorageClear( i_name, i_value)
{
	if( i_name && i_value )
		if( i_value != 'yes' )
			return;
	localStorage.clear();
	c_Info('Local Storage Cleared.');
	c_InitSettings();
}

function c_Info( i_msg)
{
	g_el.info.textContent=i_msg;
	c_Log( i_msg);
}
function c_Error( i_err)
{
	c_Info('Error: ' + i_err);
}
function c_Log( i_msg)
{
	var elLine = document.createElement('div');
	elLine.innerHTML = '<i>'+g_cycle+':</i> '+i_msg;
	var lines = log.getElementsByTagName('div');
	g_el.log.insertBefore( elLine, lines[0]);
	if( lines.length > 100 )
		g_el.log.removeChild( lines[100]);
}
