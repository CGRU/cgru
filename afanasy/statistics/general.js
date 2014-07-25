var $ = function( id ) { return document.getElementById( id ); };

g_actions = {};
g_actions.jobs_table = {"label":'Jobs Table'};
g_actions.tasks_table = {"label":'Tasks Table'};
g_actions.tasks_graph = {"label":'Tasks Graph'};

NS = 'http://www.w3.org/2000/svg';

g_time_ids = ['time_min','time_max'];
g_graph_intervals = {"names":['minute','hour','day','week']};
g_graph_intervals.minute = {"seconds":60,"intervals":[ 1, 5, 10, 15, 20, 30, 60, 60*3, 60*6, 60*12, 60*24 ],"offsets":[[0]]};
g_graph_intervals.hour   = {"seconds":60 * 60,"intervals":[ 1, 3, 6, 12, 24 ],"offsets":[[0]]};
g_graph_intervals.day    = {"seconds":60 * 60 * 24,"intervals":[ 1, 7, 7, 7 ],"offsets":[[0],[1,3,5],[1,5],[1]]};
g_graph_intervals.week   = {"seconds":60 * 60 * 24 * 7,"intervals":[ 1 ],"offsets":[[0]]};

function g_Init()
{
	g_Info('init');
	g_Request({"send":{"init":null},"func":g_Start});
}

function g_Start( i_data, i_args)
{
//console.log( JSON.stringify( i_data));
	g_Info('Started');

	for( var action in g_actions )
	{
		var elAct = document.createElement('span');
		$('actions').appendChild( elAct);
		elAct.classList.add('action');
		elAct.textContent = g_actions[action].label;
		elAct.m_action = action;
		elAct.onclick = g_ActionClicked;
		g_actions[action].element = elAct;
	}

	for( var i = 0; i < g_time_ids.length; i++)
		$(g_time_ids[i]).onkeydown = g_TimeKeyDown;

	window.onhashchange = g_HashChanged;
	g_HashChanged();
}

function g_ActionClicked( i_evt)
{
	var action = i_evt.currentTarget.m_action;
	var hash = document.location.hash.split(',');
	if( hash.length > 2 )
		action += ',' + hash[1] + ',' + hash[2];
	document.location.hash = action;
}

function g_HashChanged()
{
	var hash = document.location.hash;
	if( hash.indexOf('#') == 0 ) hash = hash.substr(1);

	hash = hash.split(',');
	var action = hash[0];
	var args = {};
	if( hash.length > 2 )
	{
		args.time_min = hash[1];
		args.time_max = hash[2];
	}

	for( var a in g_actions )
		g_actions[a].element.classList.remove('enabled');

	if( action == '' )
	{
		document.location.hash = 'jobs_table';
		return;
	}

	$('content').textContent = '';

	if( g_actions[action] )
	{
		if( g_actions[action].element )
			g_actions[action].element.classList.add('enabled');
	}
	else
	{
		g_Error('Unknown action: "' + action + '"');
		return;
	}

	g_Info( action);

	if( action == 'jobs_table')
	{
		g_Info('Requesting jobs tasks statistics table...');
		g_GetTimeInterval( args);
		args.select = 'service';
		args.favorite = 'username';
		g_Request({"send":{"get_jobs_table":args},"func":g_ShowTable,"args":args,"service":'get_jobs_table'});
		return;
	}
	if( action == 'tasks_table')
	{
		g_Info('Requesting tasks services statistics table...');
		g_GetTimeInterval( args);
		args.select = 'service';
		args.favorite = 'username';
		g_Request({"send":{"get_tasks_table":args},"func":g_ShowTable,"args":args,"service":'get_tasks_table'});
		return;
	}
	if( action == 'tasks_graph')
	{
		g_Info('Requesting tasks services statistics graph...');
		args.notime = {};
		args.notime.time_max = Math.round((new Date).valueOf()/1000);
		args.notime.time_min = args.notime.time_max - 1000000;
		g_GetTimeInterval( args);
		args.select = 'service';
		g_Request({"send":{"get_tasks_graph":args},"func":g_ShowGraph,"args":args});
		return;
	}
}

function g_ShowTable( i_data, i_args)
{
//console.log( JSON.stringify( i_args));
//console.log( JSON.stringify( i_data));
//console.log('O:'+i_data.time_min+'-'+i_data.time_max);
	if( i_data.table == null )
	{
		g_Error('No table found.');
		return;
	}

	g_ShowTimeInterval( i_data);

	var select   = i_data.select;
	var vaforite = i_data.favorite;

	var elTableDiv = document.createElement('div');
	$('content').appendChild( elTableDiv);
	elTableDiv.classList.add('table_div');

	var elTableName = document.createElement('div');
	elTableDiv.appendChild( elTableName);
	elTableName.classList.add('title');
	elTableName.textContent = select;

	var elTable = document.createElement('table');
	elTableDiv.appendChild( elTable);
	elTable.classList.add('table');

	var elRow = document.createElement('tr');
	elTable.appendChild( elRow);

	var elCol = document.createElement('th');
	elRow.appendChild( elCol);
	elCol.textContent = '#';

	for( var col in i_data.table[0])
	{
		var elCol = document.createElement('th');
		elRow.appendChild( elCol);
		if( g_parm[col] && g_parm[col].label )
			elCol.textContent = g_parm[col].label;
		else
			elCol.textContent = [col];
	}

	for( var r = 0; r < i_data.table.length; r++)
	{
		var elRow = document.createElement('tr');
		elTable.appendChild( elRow);

		var elNum = document.createElement('td');
		elRow.appendChild( elNum);
		elNum.textContent = r;

		for( var col in i_data.table[r])
		{
			var elCol = document.createElement('td');
			elRow.appendChild( elCol);

			var value = i_data.table[r][col];

			if( g_parm[col])
			{
				if( g_parm[col].percent ) value = Math.round( 100 * value) + '%';
				else if( g_parm[col].round ) value = Math.round( value);
				else if( g_parm[col].time ) value = g_SecToHMS( value);

				if( g_parm[col].suffix ) value += g_parm[col].suffix;
			}

			elCol.textContent = value;
		}
	}

	if( select == 'service' )
	{
		g_Info('Requesting tasks users statistics table...');
		var args = i_args.args;
		args.select = 'username';
		args.favorite = 'service';
		var send = {};
		send[i_args.service] = args;
		g_Request({"send":send,"func":g_ShowTable,"args":args});
	}
	else
		g_Info('Statistics received.');
}

function g_ShowGraph( i_data, i_args)
{
//$('content').textContent = JSON.stringify( i_data).replace(/,/g,', ');
//console.log( JSON.stringify( i_args));
//console.log( JSON.stringify( i_data));

	var width = 1200;
	var height = 300;
	var border = {'left':150,'top':20,'right':90,'bottom':40};

	var interval = i_data.interval;
	var time_min = i_data.time_min;
	var time_max = i_data.time_max;
	var select   = i_data.select;

	g_ShowTimeInterval( i_data);

	var elDiv = document.createElement('div');
	$('content').appendChild( elDiv);

	var elTitle = document.createElement('div');
	elDiv.appendChild( elTitle);
	elTitle.classList.add('title');
	elTitle.textContent = select;

	// Create SVG element:
	var svg = document.createElementNS( NS,'svg');
	elDiv.appendChild( svg);
//	$('content').appendChild( svg);
	svg.style.width = width + 'px';
	svg.style.height = height + 'px';
	svg.setAttribute('version','1.1');
	svg.setAttribute('width', width + 'px');
	svg.setAttribute('height', height + 'px');

	// Draw graph border:	
	var rect = document.createElementNS( NS,'rect');
	svg.appendChild( rect);
	rect.setAttribute('x', border.left);
	rect.setAttribute('y', border.top);
	rect.setAttribute('width', width - border.left - border.right);
	rect.setAttribute('height', height - border.top - border.bottom);
	rect.setAttribute('stroke','rgb(0,0,0)');
	rect.setAttribute('stroke-width','1');
	rect.setAttribute('fill','none');

	// Calculate graph scale:
	var quantity_max = 0;
	for( var s = 0; s < i_data.table.length; s++)
	{
		var name = i_data.table[s][select];
		for( var time in i_data.graph )
		{
			var quantity = 0;
			var record = i_data.graph[time][name];
			if( record ) quantity = parseInt( record.quantity);
			if( quantity_max < quantity ) quantity_max = quantity;
		}
	}
	var scale_y = .95 * ( height - border.top - border.bottom ) / quantity_max;
	var scale_x = ( width - border.left - border.right ) / ( time_max - time_min )

	// Draw X:
	var text_intervals = [1,2,5,10,50,100];
	var int_name = null;
	for( var name in g_graph_intervals )
	{
		if( g_graph_intervals[name].seconds == interval )
		{
			int_name = name;
			text_intervals = g_graph_intervals[name].intervals;
		}
	}

	var text_interval = 1;
	var text_interval_offsets = [0];
	var i = 0;
	var count = ( time_max - time_min ) / interval + 1;
	while( count / text_interval * text_interval_offsets.length > 20 )
	{
		if( i < text_intervals.length )
			text_interval = text_intervals[i];
		else
			text_interval *= 2;

		if( i < g_graph_intervals[int_name].offsets.length )
			text_interval_offsets = g_graph_intervals[int_name].offsets[i];

		i++;
	}

	var text_interval_offset = 0;
	if( int_name == 'hour' )
		text_interval_offset = - Math.round((new Date()).getTimezoneOffset()/60);
	if( int_name == 'day' )
		text_interval_offset = ( new Date( time_min * 1000 ).getDay());

//console.log("Count: " + count + ", interval: " + interval + ', type: "' + int_name + '", text interval: ' + text_interval + ', offset: ' + text_interval_offset + ', offsets: ' + text_interval_offsets.join(','));

	var date_prev = new Date( 0);
	i = text_interval_offset - 1;
	for( var time in i_data.graph )
	{
		i++;
		var draw_text = false;
		if( text_interval_offsets.indexOf( i % text_interval ) != -1 )
			draw_text = true;

		var color = 'rgb(0,0,0)';
		var date = new Date( 1000 * parseInt( time));
		if(( date.getDay() == 0 ) || ( date.getDay() == 6 ))
			color = 'rgb(60,20,0)';

		var path = document.createElementNS( NS,'path');
		svg.appendChild( path);
		path.setAttribute('stroke', color);
		path.setAttribute('stroke-width','1');
		path.setAttribute('fill','none');
		var x = border.left + scale_x * ( time - time_min );
		var y = height - border.bottom;
		var line = 'M ' + x + ' ' + y;
		line += ' L ' + x + ' ' + ( y + ( draw_text ? 10 : 5 ) );
		path.setAttribute('d', line);
		path.setAttribute('title', date.toLocaleString());
//console.log( line)

		if( false == draw_text )
			continue;

		var draw_week_day = true;
		var date_txt = '';
		if( date_prev.getMonth() == date.getMonth())
		{
			date_txt = '' + date.getDate();
			if( date_prev.getDate() == date.getDate())
			{
				date_txt = date.getHours() + ':00';
				draw_week_day = false;
			}
		}
		else
		{
			date_txt = (date.getMonth()+1) + '/' + date.getDate();
		}

		var text = document.createElementNS( NS, 'text');
		svg.appendChild( text);
		text.setAttribute('x', x - 4 * ( date_txt.length ));
		text.setAttribute('y', y + 20);
		text.setAttribute('font-size', 12);
		text.setAttribute('fill', color);
		text.textContent = date_txt;
		text.setAttribute('title', date.toLocaleString());

		date_prev = date;

		if( false == draw_week_day )
			continue;

		var week_days = ['SUN','MON','TUE','WED','THU','FRI','SAT'];
		
		date_txt = week_days[ date.getDay()];
		var text = document.createElementNS( NS, 'text');
		svg.appendChild( text);
		text.setAttribute('x', x - 4 * ( date_txt.length ));
		text.setAttribute('y', y + 34);
		text.setAttribute('font-size', 12);
		text.setAttribute('fill', color);
		text.textContent = date_txt;
		text.setAttribute('title', date.toLocaleString());
	}

	// Draw Y:
	var quantity = 0;
	var quantity_pow = 1;
	while( quantity_pow * 10 < quantity_max ) quantity_pow *= 10;
	if( quantity_max / quantity_pow < 4 ) quantity_pow /= 2;
	while( quantity < quantity_max )
	{
		var path = document.createElementNS( NS,'path');
		svg.appendChild( path);
		path.setAttribute('stroke','rgba(0,0,0,.5)');
		path.setAttribute('stroke-width','1');
		path.setAttribute('fill','none');
		var y = height - border.bottom - scale_y * quantity;
		var line = 'M ' + (border.left-5) + ' ' + y;
		line += ' L ' + (width-border.right+5) + ' ' + y;
		path.setAttribute('d', line);

		var text = document.createElementNS( NS, 'text');
		svg.appendChild( text);
		text.setAttribute('x', 6 + width - border.right);
		text.setAttribute('y', y+5);
		text.setAttribute('font-size', 16);
//		text.setAttribute('stroke', color);
//		text.setAttribute('stroke-width','.5');
		text.setAttribute('fill','rgb(0,0,0)');
		text.textContent = quantity;

		quantity += quantity_pow;
//console.log( line)
	}

	// Select names loop:
	var select_num = i_data.table.length;
	if( select_num > 10 ) select_num = 10;
	for( var s = 0; s < select_num; s++)
	{
		var select_name = i_data.table[s][select];

		// Choose color:
		var c = Math.round((s*3) % select_num);
		var g = 1; var b = 0;
		var r = 1 - c * s / select_num;
		if( r < 1 ) { b = 3 * (c-select_num/3) / select_num; }
		if( b > 1 ) { g = 1 - 3 * (c-2*select_num/3) / select_num; }
		if(r<0)r=0;if(g<0)g=0;if(b<0)b=0;
		if(r>1)r=1;if(g>1)g=1;if(b>1)b=1;
		r=Math.round(255*r);g=Math.round(255*g);b=Math.round(255*b);
		var color = 'rgb('+r+','+g+','+b+')';

		// Service name:
		var text = document.createElementNS( NS, 'text');
		svg.appendChild( text);
		text.setAttribute('x', 10);
		text.setAttribute('y', 20+20*s);
		text.setAttribute('font-size', 16);
		text.setAttribute('fill', color);
		text.textContent = select_name;

		// Start curve path:
		var path = document.createElementNS( NS,'path');
		svg.appendChild( path);
//		path.id = 'path0';
		path.setAttribute('stroke', color);
		path.setAttribute('stroke-width','1.5');
//		path.setAttribute('stroke-line-join','miter');
		path.setAttribute('fill','none');
		path.setAttribute('title', select_name);

		// Time interval loop to store values:
		var px = [];
		var py = [];
		for( var time in i_data.graph )
		{
			var value = 0;
			var record = i_data.graph[time][select_name];
			if( record ) value = record.quantity;

			px.push( scale_x * ( time - time_min ));
			py.push( scale_y * value);
		}
		// Calculate tangents:
		var tx = [];
		var ty = [];
		for( var i = 0; i < px.length; i++ )
		{
			var tl = .2;
			if( i == 0 )
			{
				tx.push( px[i] + 2*tl * ( px[i+1] - px[i] ));
				ty.push( py[i] + 2*tl * ( py[i+1] - py[i] ));
			}
			else if( i == ( px.length - 1 ))
			{
				tx.push( px[i] - 2*tl * ( px[i] - px[i-1] ));
				ty.push( py[i] - 2*tl * ( py[i] - py[i-1] ));
			}
			else
			{
				tx.push( px[i] - tl * ( px[i+1] - px[i-1] ));
				if(( py[i] >= py[i-1] ) && ( py[i] >= py[i+1] ))
					ty.push( py[i]);
				else if(( py[i] <= py[i-1] ) && ( py[i] <= py[i+1] ))
					ty.push( py[i]);
				else
				{
//					var _ty = py[i] - tl * ( py[i+1] - py[i-1] );
					var _ty = tl * ( py[i+1] - py[i-1] );
//					if(( py[i] + _ty < 0 ) || ( py[i] - _ty < 0 ))
					if( py[i] + _ty < 0 )
						ty.push( py[i] * 2 );
					else if( py[i] - _ty < 0 )
						ty.push( 0 );
					else
						ty.push( py[i] - _ty );
//						ty.push( py[i] - tl * ( py[i+1] - py[i-1] ));
				}
			}
		}

		if( 0 ) { /* Try without smooth*/ if( 0 ) /* Use tangents */
		{	var _px = []; var _py = [];
			for( var i = 0; i < px.length; i++ ) { _px.push( px[i]); _px.push( tx[i]); _py.push( py[i]); _py.push( ty[i]); }
		px = _px; py = _py; }
		var line = ''; for( var i = 0; i < px.length; i++ )
		{	if( i == 0 ) line += 'M'; else line += ' L ';
			line += (px[i] + border.left) + ' ' + (height - border.bottom - py[i]); }
		path.setAttribute('d', line); continue; }

		// Draw curve:
//d="M10 80 C 40 10, 65 10, 95 80 S 150 150, 180 90, 250 60, 350 100" // <- example
//   p1     t1right  t2left p2      t3left   p3      t4left  p4
		var line = '';
		for( var i = 0; i < px.length; i++ )
		{
			if( i == 0 )
			{
				line += 'M';
				var x = px[i] + border.left;
				var y = height - border.bottom - py[i];
				line += x + ' ' + y;
				line += ' C ';
				var x = tx[i] + border.left;
				var y = height - border.bottom - ty[i];
				line += x + ' ' + y;
			}
			else
			{
				if( i == 2) line += ' S ';
				else line += ', ';
				var x = tx[i] + border.left;
				var y = height - border.bottom - ty[i];
				line += x + ' ' + y;
				line += ', ';
				var x = px[i] + border.left;
				var y = height - border.bottom - py[i];
				line += x + ' ' + y;
			}
		}
		path.setAttribute('d', line);
//console.log( line)
	}

	if( select == 'service' )
	{
		g_Info('Requesting tasks users statistics graph...');
		var args = i_args.args;
		args.select = 'username';
		g_Request({"send":{"get_tasks_graph":args},"func":g_ShowGraph,"args":args});
	}
	else
		g_Info('Statistics received.');
}

function g_TimeKeyDown( i_evt)
{
	if( i_evt.keyCode != 13 ) // Enter
		return;

	i_evt.stopPropagation();

	var args = {};
	g_GetTimeInterval( args);

	var hash = document.location.hash.split(',')[0];
	hash += ',' + args.time_min + ',' + args.time_max;

	document.location.hash = hash;
}

function g_ShowTimeInterval( i_args)
{
	for( var i = 0; i < g_time_ids.length; i++ )
	{
		var date = new Date( 1000 * i_args[g_time_ids[i]]);
		date = date.toISOString();
		date = date.substr( 0, date.indexOf('T'));
		$(g_time_ids[i]).textContent = date;
	}	
}

function g_GetTimeInterval( io_args)
{
	if( io_args == null ) io_args = {};

//console.log('g_GetTimeInterval:' + io_args.time_min + '-' + io_args.time_max);
	for( var i = 0; i < g_time_ids.length; i++)
	{
		var time = io_args[g_time_ids[i]];
		if(( time == null ) && ( $(g_time_ids[i]).textContent != '' ))
		{
			var date = new Date( Date.parse( $(g_time_ids[i]).textContent));
			time = Math.round( date.valueOf() / 1000 );
		}

		if(( time == null ) && ( io_args.notime ) && ( io_args.notime[g_time_ids[i]] ))
			time = io_args.notime[g_time_ids[i]];

		if( time )
		{
			var date = new Date( time * 1000 );
			date = date.toISOString();
			date = date.substr( 0, date.indexOf('T'));
			$(g_time_ids[i]).textContent = date;

			io_args[g_time_ids[i]] = time;
		}
	}

	if( io_args.time_min && io_args.time_max )
	{
		io_args.interval = 1;
		var i = 0;
		while(( io_args.time_max - io_args.time_min ) / io_args.interval > 100 )
		{
			if( i < g_graph_intervals.names.length )
				io_args.interval = g_graph_intervals[g_graph_intervals.names[i]].seconds;
			else
				io_args.interval *= 10;
			i++;
		}
		io_args.time_min = Math.floor( io_args.time_min / io_args.interval ) * io_args.interval;
		io_args.time_max = Math.ceil(  io_args.time_max / io_args.interval ) * io_args.interval;
	}
//console.log('g_GetTimeInterval:' + io_args.time_min + '-' + io_args.time_max);
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
function g_Info( i_msg)
{
	$('info').innerHTML = i_msg;
	$('info').classList.remove('error');
}
function g_Error( i_msg)
{
	g_Info('ERROR: ' + i_msg);
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

g_parm.capacity_avg       = {"label":'Average Capacity', "round":true};
g_parm.error_avg          = {"label":'Average Error',    "percent":true};
g_parm.fav_name           = {"label":'Favourite Name'};
g_parm.fav_percent        = {"label":'Favourive Percent', "percent":true};
g_parm.jobs_quantity      = {"label":'Jobs Quantity'};
g_parm.run_time_avg       = {"label":'Average Run Time', "time":true};
g_parm.run_time_sum       = {"label":'Sum Run Time',     "time":true};
g_parm.service            = {"label":'Service'};
g_parm.tasks_done_percent = {"label":'Done',             "percent":true};
g_parm.tasks_quantity     = {"label":'Tasks Quantity'};
g_parm.tasks_quantity_avg = {"label":'Average Quantity', "round":true};
g_parm.username           = {"label":'User'};

