var $ = function( id ) { return document.getElementById( id ); };

function g_Init()
{
	g_Log('init');
	g_Request({"send":{"init":null},"func":g_Start});
}

function g_Start( i_data, i_args)
{
//console.log( JSON.stringify( i_data));
	g_Log('Started');
	g_RequestJobs();
}

function g_RequestJobs()
{
	g_Log('Requesting jobs statistics...');
	g_Request({"send":{"getjobs":null},"func":g_ShowTables});
}

function g_ShowTables( i_data, i_args)
{
//g_Log( JSON.stringify( i_data));
	$('content').textContent = '';
	if( i_data.tables == null )
	{
		g_Error('No tables founded.');
		return;
	}

	for( var table in i_data.tables)
	{
		var elTableDiv = document.createElement('div');
		$('content').appendChild( elTableDiv);
		elTableDiv.classList.add('table_div');

		var elTableName = document.createElement('div');
		elTableDiv.appendChild( elTableName);
		elTableName.classList.add('caption');
		elTableName.textContent = table;

		var elTable = document.createElement('table');
		elTableDiv.appendChild( elTable);
		elTable.classList.add('table');

		var elRow = document.createElement('tr');
		elTable.appendChild( elRow);

		var elCol = document.createElement('td');
		elRow.appendChild( elCol);
		elCol.textContent = '#';

		for( var col in i_data.tables[table][0])
		{
			var elCol = document.createElement('td');
			elRow.appendChild( elCol);
			if( g_parm[col] && g_parm[col].label )
				elCol.textContent = g_parm[col].label;
			else
				elCol.textContent = [col];
		}

		for( var r = 0; r < i_data.tables[table].length; r++)
		{
			var elRow = document.createElement('tr');
			elTable.appendChild( elRow);

			var elNum = document.createElement('td');
			elRow.appendChild( elNum);
			elNum.textContent = r;

			for( var col in i_data.tables[table][r])
			{
				var elCol = document.createElement('td');
				elRow.appendChild( elCol);

				var value = i_data.tables[table][r][col];

				if( g_parm[col])
				{
					if( g_parm[col].round ) value = Math.round( value);
					if( g_parm[col].time ) value = g_SecToHMS( value);
					if( g_parm[col].suffix ) value += g_parm[col].suffix;
				}

				elCol.textContent = value;
			}
		}
	}

	g_Log('Jobs statictics received.');
}

function g_SecToHMS( i_sec)
{
	var seconds = Math.floor( i_sec );
	var minutes = Math.floor( seconds / 60);
	var hours = Math.floor( minutes / 60);
	var days = Math.floor( hours / 24);
	if( days == 1 )
		days = 0;
	seconds = seconds - minutes*60;
	if( seconds < 10 )
		seconds = '0' + seconds;
	minutes = minutes - hours*60;
	if( minutes < 10 )
		minutes = '0' + minutes;
	hours = hours - days*24;
	var hms = hours + ':' + minutes + ':' + seconds;
	if( days > 1 )
		hms = days + 'd ' + hms;
	return hms;
}
function g_Log( i_msg)
{
	$('info').innerHTML = i_msg;
	$('info').classList.remove('error');
}

function g_Error( i_msg)
{
	g_Log( i_msg);
	$('info').classList.add('error');
}

function g_Parse( i_data)
{
	if( i_data == null ) return null;

	var obj = null;
	try { obj = JSON.parse( i_data);}
	catch( err)
	{
		g_Error( err.message+'<br/><br/>'+i_data);
		obj = null;
	}

	return obj;
}

function g_Request( i_args)
{
	if( i_args.send == null )
	{
		g_Error('Network reqest: send object is null.');
		return;
	}

	if( i_args.func )
	{
		i_args.wait = false;
		if( i_args.parse == null )
			i_args.parse = true;
	}
	else if( i_args.wait == null )
		i_args.wait = true;

	var send_str = JSON.stringify( i_args.send);
	if( i_args.encode == true )
		send_str = btoa( send_str);

	var xhr = new XMLHttpRequest;
	xhr.m_args = i_args;

	xhr.overrideMimeType('application/json');
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
	xhr.open('POST', 'server.php', i_args.wait ? false : true); 
	xhr.send( send_str);

	if( i_args.wait )
	{
		if( xhr.getResponseHeader('WWW-Authenticate'))
		{
			g_Error('Authorization Required');
			return null;
		}
		return xhr.responseText;
	}

	xhr.onreadystatechange = n_XHRHandler;
}

function n_XHRHandler()
{
//console.log( this);
//console.log( this.readyState);
	if( this.readyState == 4 )
	{
		if( this.status == 200 )
		{
			if( this.m_args.func )
			{
				var data = this.responseText;
//console.log( data);
				if( this.m_args.parse )
					data = g_Parse( data);

				if( data.error )
				{
					g_Error( data.error);
					return;
				}

				this.m_args.func( data, this.m_args);
			}
		}
	}
}

g_parm = {};
g_parm.username = {'label':"User Name"};
g_parm.numjobs = {'label':"Jobs Quantity"};
g_parm.sumruntime = {'label':"Sum Run Time","time":true};
g_parm.avgruntime = {'label':"Average Run Time","time":true};
g_parm.usertasksnum = {'label':"Tasks Quantity"};
g_parm.usertasksavg = {'label':"Average Quantity","round":true};
g_parm.service_name = {'label':"Favourite Service"};
g_parm.service_percent = {'label':"Service Percent",'suffix':"%","round":true};

g_parm.service = {'label':"Service"};
g_parm.servicequantity = {'label':"Service Quantity"};
g_parm.tasksquantity = {'label':"Tasks Quantity"};
g_parm.tasksquantityavg = {'label':"Average Quantity","round":true};
g_parm.taskssumruntime = {'label':"Sum Run Time","time":true};
g_parm.tasksavgruntime = {'label':"Average Run Time","time":true};
g_parm.tasksdone = {'label':"Done",'suffix':"%","round":true};
g_parm.user_name = {'label':"Favourite User"};
g_parm.user_percent = {'label':"User Percent",'suffix':"%","round":true};

