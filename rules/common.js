function c_Init()
{
	for( var i = 0; i < g_elements.length; i++)
		g_el[g_elements[i]] = document.getElementById( g_elements[i]);

	c_Info('HTML body load.');

	cgru_ConstructSettingsGUI();
	cgru_InitParameters();

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
