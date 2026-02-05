var $ = function(id) { return document.getElementById(id); };

var SERVER = '/afstatsrv'
//var SERVER = 'server.php'

g_actions = {};
g_actions.logs_table =  {"label":'Logs Table',  'func':'g_Action_LogsTable'};
g_actions.jobs_table =  {"label":'Jobs Tables' ,'func':'g_Action_JobsTable'};
g_actions.tasks_table = {"label":'Tasks Tables','func':'g_Action_TasksTable'};
g_actions.tasks_graph = {"label":'Tasks Graphs','func':'g_Action_TasksGraph'};

NS = 'http://www.w3.org/2000/svg';

g_time_ids = ['time_min','time_max'];
g_graph_intervals = {"names":['minute','hour','day','week']};
g_graph_intervals.minute = {"seconds":60,"intervals":[ 1, 5, 10, 15, 20, 30, 60, 60*3, 60*6, 60*12, 60*24 ],"offsets":[[0]]};
g_graph_intervals.hour   = {"seconds":60 * 60,"intervals":[ 1, 3, 6, 12, 24 ],"offsets":[[0]]};
g_graph_intervals.day    = {"seconds":60 * 60 * 24,"intervals":[ 1, 7, 7, 7 ],"offsets":[[0],[1,3,5],[1,5],[1]]};
g_graph_intervals.week   = {"seconds":60 * 60 * 24 * 7,"intervals":[ 1 ],"offsets":[[0]]};


g_args = {};
g_args.action = 'logs_table';
//g_args.time_max = Math.round((new Date()).valueOf() / 1000);
//g_args.time_min = Math.round((new Date()).valueOf() / 1000) - (60*60 * 24 * 48);
g_args.folder = '/';

g_folders = [];

g_waiting = false;

function g_Init()
{
	window.onhashchange = g_HashChanged;
	g_Info('Initializing...');
	g_Request({"send":{"init":null},"func":g_Start});
}

function g_Start(i_data, i_args)
{
	g_Log('Server: ' + JSON.stringify(i_data.server, null,' '));

	// Get time min and max:
	/*g_args.time_min = i_data.time.time_min;
	g_args.time_max = i_data.time.time_max;*/


	// Initialize folders:
	g_FoldersInit(i_data);


	// Action buttons on click:
	for (let action in g_actions)
	{
		let elAct = document.createElement('span');
		$('actions').appendChild(elAct);
		elAct.classList.add('action');
		elAct.textContent = g_actions[action].label;
		elAct.m_action = action;
		elAct.onclick = g_ActionClicked;
		g_actions[action].element = elAct;
	}


	// Time fields on key down:
	for (let i = 0; i < g_time_ids.length; i++)
		$(g_time_ids[i]).onkeydown = g_TimeKeyDown;


	let hash = document.location.hash;
	// Override arguments from hash (if  any):
	g_HashGet();
	// And set merged hash back:
	g_HashSet();


	// Start to "listen" hash:
	g_Info('Starting...');
	if (hash == document.location.hash)
		g_HashChanged();
}

function g_Reset()
{
	window.onhashchange = null;
	document.location.hash = '';
	document.location.reload();
}

function g_WaintingSet(i_on)
{
	g_waiting = true;
	let display = 'block';
	if (i_on === false)
	{
		display = 'none';
		g_waiting = false;
	}
	$('waiting').style.display = display;
}
function g_WaintingReset() { g_WaintingSet(false); }

function g_FoldersInit(i_data)
{
	g_folders = i_data.folders;
	if ((g_folders == null) || (g_folders.length == 0))
	{
		$('folders_div').style.display = 'none';
		return;
	}

//$('g_folders').textContent = g_folders;
	// Prepare folders:
	for (let f = 0; f < g_folders.length; f++)
		if ((g_folders[f] == null) || (g_folders[f].length == 0))
			g_folders[f] = '/';
		else if (g_folders[f] != '/')
			g_folders[f] = g_folders[f].replace(/\/+$/g , '');

	// Find root folder (common beginning for  every folder):
	let names = [];
	for (let f = 0; f < g_folders.length; f++)
	{
		if (g_folders[f].length == 0) continue;
		names = g_folders[f].split('/');
		if (names.length > 2) break;
	}

//console.log(names);
	let folder = '';
	for (let n = 0; n < names.length; n++)
	{
		if (names[n].length == 0) continue;

		folder += '/' + names[n];

		let allEqual = true;
		for (let f = 0; f < g_folders.length; f++)
		{
			if (g_folders[f] == '/') continue;

			if (g_folders[f].indexOf(folder) != 0)
			{
				allEqual = false;
				break;
			}
		}
		if (allEqual)
			g_args.folder = folder;
		else break;
	}
//console.log(g_args.folder);
}

function g_FoldersProcess(i_folder)
{
	if ((g_folders == null) || (g_folders.length == 0))
		return;

	if (i_folder) g_args.folder = i_folder;

	$('folders_root').textContent = '';
	let names = g_args.folder.split('/');
//console.log(names);
	let folder = '';
	if (names.length) names[0] = '/';

	for (let n = 0; n < names.length; n++)
	{
		let name = names[n];
		if (name.length == 0) continue;

		if (n > 1) folder += '/';
		folder += name;
//console.log(folder);

		let el = document.createElement('div');
		$('folders_root').appendChild(el);
		el.classList.add('folder');
		el.textContent = name;
		el.title = folder;
		el.m_folder = folder;
		el.onclick = g_FolderClicked;
	}

	$('folders').textContent = '';
	let folders = [];
	for (let f = 0; f < g_folders.length; f++)
	{
		let folder = g_folders[f];
		if (folder.indexOf(g_args.folder) != 0) continue;
		folder = folder.replace(g_args.folder, '');
		folder = folder.replace(/^\/+/g , '');
		folder = folder.substr(0, folder.indexOf('/'));
		if (folder.length == 0) continue;

		if (folders.indexOf(folder) != -1)
			continue;
		folders.push(folder);

		let el = document.createElement('div');
		$('folders').appendChild(el);
		el.classList.add('folder');
		el.textContent = folder;

//		folder = g_args.folder + '/' + folder;
		folder = '/' + folder;
		if (g_args.folder != '/')
			folder = g_args.folder + folder;

		el.title = folder;
		el.m_folder = folder;
		el.onclick = g_FolderClicked;
	}
}
function g_FolderClicked(i_evt)
{
	g_args.folder = i_evt.currentTarget.m_folder;
	g_HashSet();
}

function g_ActionClicked(i_evt)
{
	g_args.action = i_evt.currentTarget.m_action;
	g_HashSet();
}

function g_HashSet()
{
	document.location.hash = JSON.stringify(g_args);
}

function g_HashGet()
{
	let hash = decodeURI(document.location.hash);
	if (hash.indexOf('#') == 0) hash = hash.substr(1);

	let args = null;
	try { args = JSON.parse(hash);}
	catch(err) { args = null; }

	if (args)
		for (let a in args)
			g_args[a] = args[a];
}

function g_HashChanged()
{
	if (g_waiting) return;

	g_WaintingSet();
	g_HashGet();

	g_TimeShow();
	g_FoldersProcess();

	for (let a in g_actions)
		g_actions[a].element.classList.remove('enabled');

	$('content').textContent = '';

	if (g_actions[g_args.action])
	{
		if (g_actions[g_args.action].element)
			g_actions[g_args.action].element.classList.add('enabled');
		g_Info('Action: "' + g_actions[g_args.action].label + '".');
	}
	else
	{
		g_Error('Unknown action: "' + g_args.action + '"');
		return;
	}

	let args = {};
	for (let a in g_args)
		args[a] = g_args[a];

	window[g_actions[g_args.action].func](args);
}

function g_Action_LogsTable(i_args)
{
	g_Info('Requesting logs table...');
	$('folders_div').style.display = 'none';
	i_args.table = 'logs';
	i_args.order = 'time';
	g_Request({"send":{"get_logs_table":i_args},"func":g_ShowTable,"args":i_args});
}

function g_Action_JobsTable(i_args)
{
	g_Info('Requesting jobs folders statistics table...');
	$('folders_div').style.display = 'block';
	i_args.table = 'jobs';
	i_args.select = 'folder'
	g_Request({"send":{"get_jobs_folders":i_args},"func":g_ShowTable,"args":i_args,"service":'get_jobs_table'});
}

function g_Action_TasksTable(i_args)
{
	g_Info('Requesting tasks folders statistics table...');
	$('folders_div').style.display = 'block';
	i_args.table = 'tasks';
	i_args.select = 'folder';
	g_Request({"send":{"get_tasks_folders":i_args},"func":g_ShowTable,"args":i_args,"service":'get_tasks_table'});
}

function g_Action_TasksGraph(i_args)
{
	g_Info('Requesting tasks folders statistics graph...');
	$('folders_div').style.display = 'block';
	i_args.select = 'folder';
	i_args.interval = g_TimeIntervalGet();
	g_Request({"send":{"get_tasks_folders_graph":i_args},"func":g_ShowGraph,"args":i_args});
}

function g_ShowTable(i_data, i_args)
{
//console.log(JSON.stringify(i_args));
//console.log(JSON.stringify(i_data));
//console.log('O:'+i_data.time_min+'-'+i_data.time_max);
	if (i_data.table == null)
	{
		g_Error('No table found.');
		return;
	}

	g_Info('Statistics received.');

	let table    = i_args.args.table;
	let select   = i_data.select;
	let vaforite = i_data.favorite;

	let elTableDiv = document.createElement('div');
	$('content').appendChild(elTableDiv);
	elTableDiv.classList.add('table_div');

	if (select)
	{
		let elTableName = document.createElement('div');
		elTableDiv.appendChild(elTableName);
		elTableName.classList.add('title');
		elTableName.textContent = select;
	}

	let elTable = document.createElement('table');
	elTableDiv.appendChild(elTable);
	elTable.classList.add('table');
	elTable.classList.add(table);

	let elRow = document.createElement('tr');
	elTable.appendChild(elRow);

	let elCol = document.createElement('th');
	elRow.appendChild(elCol);
	elCol.textContent = '#';

	for (let col in i_data.table[0])
	{
		let elCol = document.createElement('th');
		elCol.classList.add(col);
		elRow.appendChild(elCol);
		if (g_parm[col] && g_parm[col].label)
			elCol.textContent = g_parm[col].label;
		else
			elCol.textContent = [col];

		if (table == 'logs')
		{
			elCol.style.backgroundColor = '#FF0';
			elCol.contentEditable = true;
		}
	}

	for (let r = 0; r < i_data.table.length; r++)
	{
		let elRow = document.createElement('tr');
		elTable.appendChild(elRow);

		let elNum = document.createElement('td');
		elRow.appendChild(elNum);
		elNum.classList.add('number');
		if (table == 'logs')
			elNum.textContent = i_data.table.length - r - 1;
		else
			elNum.textContent = r;

		for (let col in i_data.table[r])
		{
			let elCol = document.createElement('td');
			elCol.classList.add(col);
			elRow.appendChild(elCol);

			let value = i_data.table[r][col];

			if (g_parm[col])
			{
				if (g_parm[col].percent) value = Math.round(100 * value) + '%';
				else if (g_parm[col].round) value = Math.round(value);
				else if (g_parm[col].time) value = g_SecToHMS(value);
				else if (g_parm[col].datetime) value = g_SecToStr(value);
			}

			if (col == 'folder') value = g_FolderStrip(value);

			elCol.textContent = value;
		}
	}

	if (select == 'folder')
	{
		let args = i_args.args;
		g_Info('Requesting ' + args.table + ' services statistics table...');
		args.select = 'service';
		args.favorite = 'username';
		let send = {};
		send[i_args.service] = args;
		g_Request({"send":send,"func":g_ShowTable,"args":args,'service':i_args.service});
	}
	else if (select == 'service')
	{
		let args = i_args.args;
		g_Info('Requesting ' + args.table + ' users statistics table...');
		args.select = 'username';
		args.favorite = 'service';
		let send = {};
		send[i_args.service] = args;
		g_Request({"send":send,"func":g_ShowTable,"args":args,'service':i_args.service});
	}
	else
	{
		g_WaintingReset();
	}
}

function g_FolderStrip(i_folder)
{
	let folder = i_folder.replace(g_args.folder, '');
	folder = folder.replace(/^\/+|\/+$/g , '');
	folder = folder.split('/')[0];
	if (folder.length == 0) folder = '/';
	return folder;
}

function g_ShowGraph(i_data, i_args)
{
//$('content').textContent = JSON.stringify(i_data).replace(/,/g,', ');
//console.log(JSON.stringify(i_args));
//console.log(JSON.stringify(i_data));
	g_Info('Statistics received.');

	let width = 1200;
	let height = 300;
	let border = {'left':150,'top':20,'right':90,'bottom':40};

	let interval = i_data.interval;
	let time_min = i_data.time_min;
	let time_max = i_data.time_max;
	let select   = i_data.select;

	let elDiv = document.createElement('div');
	$('content').appendChild(elDiv);

	let elTitle = document.createElement('div');
	elDiv.appendChild(elTitle);
	elTitle.classList.add('title');
	elTitle.textContent = select;

	// Create SVG element:
	let svg = document.createElementNS(NS,'svg');
	elDiv.appendChild(svg);
//	$('content').appendChild(svg);
	svg.style.width = width + 'px';
	svg.style.height = height + 'px';
	svg.setAttribute('version','1.1');
	svg.setAttribute('width', width + 'px');
	svg.setAttribute('height', height + 'px');

	// Draw graph border:	
	let rect = document.createElementNS(NS,'rect');
	svg.appendChild(rect);
	rect.setAttribute('x', border.left);
	rect.setAttribute('y', border.top);
	rect.setAttribute('width', width - border.left - border.right);
	rect.setAttribute('height', height - border.top - border.bottom);
	rect.setAttribute('stroke','rgb(0,0,0)');
	rect.setAttribute('stroke-width','1');
	rect.setAttribute('fill','none');

	// Calculate graph scale:
	let quantity_max = 0;
	for (let s = 0; s < i_data.table.length; s++)
	{
		let name = i_data.table[s][select];
		if (select == 'folder')
			name = g_FolderStrip(name);
		for (let time in i_data.graph)
		{
			let quantity = 0;
			let record = i_data.graph[time][name];
			if (record) quantity = parseInt(record.quantity);
			if (quantity_max < quantity) quantity_max = quantity;
		}
	}
	let scale_y = .95 * (height - border.top - border.bottom) / quantity_max;
	let scale_x = (width - border.left - border.right) / (time_max - time_min)

	// Draw X:
	let text_intervals = [1,2,5,10,50,100];
	let int_name = null;
	for (let name in g_graph_intervals)
	{
		if (g_graph_intervals[name].seconds == interval)
		{
			int_name = name;
			text_intervals = g_graph_intervals[name].intervals;
		}
	}

	let text_interval = 1;
	let text_interval_offsets = [0];
	let i = 0;
	let count = (time_max - time_min) / interval + 1;
	while(count / text_interval * text_interval_offsets.length > 20)
	{
		if (i < text_intervals.length)
			text_interval = text_intervals[i];
		else
			text_interval *= 2;

		if (i < g_graph_intervals[int_name].offsets.length)
			text_interval_offsets = g_graph_intervals[int_name].offsets[i];

		i++;
	}

	let text_interval_offset = 0;
	if (int_name == 'hour')
		text_interval_offset = - Math.round((new Date()).getTimezoneOffset()/60);
	if (int_name == 'day')
		text_interval_offset = (new Date(time_min * 1000).getDay());

//console.log("Count: " + count + ", interval: " + interval + ', type: "' + int_name + '", text interval: ' + text_interval + ', offset: ' + text_interval_offset + ', offsets: ' + text_interval_offsets.join(','));

	let date_prev = new Date(0);
	i = text_interval_offset - 1;
	for (let time in i_data.graph)
	{
		i++;
		let draw_text = false;
		if (text_interval_offsets.indexOf(i % text_interval) != -1)
			draw_text = true;

		let color = 'rgb(0,0,0)';
		let date = new Date(1000 * parseInt(time));
		if ((date.getDay() == 0) || (date.getDay() == 6))
			color = 'rgb(60,20,0)';

		let path = document.createElementNS(NS,'path');
		svg.appendChild(path);
		path.setAttribute('stroke', color);
		path.setAttribute('stroke-width','1');
		path.setAttribute('fill','none');
		let x = border.left + scale_x * (time - time_min);
		let y = height - border.bottom;
		let line = 'M ' + x + ' ' + y;
		line += ' L ' + x + ' ' + (y + (draw_text ? 10 : 5));
		path.setAttribute('d', line);
		path.setAttribute('title', date.toLocaleString());
//console.log(line)

		if (false == draw_text)
			continue;

		let draw_week_day = true;
		let date_txt = '';
		if (date_prev.getMonth() == date.getMonth())
		{
			date_txt = '' + date.getDate();
			if (date_prev.getDate() == date.getDate())
			{
				date_txt = date.getHours() + ':00';
				draw_week_day = false;
			}
		}
		else
		{
			date_txt = (date.getMonth()+1) + '/' + date.getDate();
		}

		let text = document.createElementNS(NS, 'text');
		svg.appendChild(text);
		text.setAttribute('x', x - 4 * (date_txt.length));
		text.setAttribute('y', y + 20);
		text.setAttribute('font-size', 12);
		text.setAttribute('fill', color);
		text.textContent = date_txt;
		text.setAttribute('title', date.toLocaleString());

		date_prev = date;

		if (false == draw_week_day)
			continue;

		let week_days = ['SUN','MON','TUE','WED','THU','FRI','SAT'];
		
		date_txt = week_days[ date.getDay()];
		text = document.createElementNS(NS, 'text');
		svg.appendChild(text);
		text.setAttribute('x', x - 4 * (date_txt.length));
		text.setAttribute('y', y + 34);
		text.setAttribute('font-size', 12);
		text.setAttribute('fill', color);
		text.textContent = date_txt;
		text.setAttribute('title', date.toLocaleString());
	}

	// Draw Y:
	let quantity = 0;
	let quantity_pow = 1;
	while(quantity_pow * 10 < quantity_max) quantity_pow *= 10;
	if (quantity_max / quantity_pow < 4) quantity_pow /= 2;
	while(quantity < quantity_max)
	{
		let path = document.createElementNS(NS,'path');
		svg.appendChild(path);
		path.setAttribute('stroke','rgba(0,0,0,.5)');
		path.setAttribute('stroke-width','1');
		path.setAttribute('fill','none');
		let y = height - border.bottom - scale_y * quantity;
		let line = 'M ' + (border.left-5) + ' ' + y;
		line += ' L ' + (width-border.right+5) + ' ' + y;
		path.setAttribute('d', line);

		let text = document.createElementNS(NS, 'text');
		svg.appendChild(text);
		text.setAttribute('x', 6 + width - border.right);
		text.setAttribute('y', y+5);
		text.setAttribute('font-size', 16);
//		text.setAttribute('stroke', color);
//		text.setAttribute('stroke-width','.5');
		text.setAttribute('fill','rgb(0,0,0)');
		text.textContent = quantity;

		quantity += quantity_pow;
//console.log(line)
	}

	// Select names loop:
	let select_num = i_data.table.length;
	if (select_num > 10) select_num = 10;
	for (let s = 0; s < select_num; s++)
	{
		let select_name = i_data.table[s][select];
		if (select == 'folder') select_name = g_FolderStrip(select_name);

		// Choose color:   //  Red
		                   // 1|___       ___
		let hue_min = 0.8; //  |   \     /   \
		let hue_max = 3.5; //  |____\___/     \___hue
		                   //  0  1  2  3  4  5
		let hue = 0;
		if (select_num > 1) hue = s / (select_num - 1);
		hue = hue * (hue_max - hue_min);
		hue += hue_min;

		let r = hue < 2 ? (2-hue) : (hue-4);
		let g = hue < 3 ? (hue) : (4-hue);
		let b = hue < 4 ? (hue-2) : (6-hue);

		if (r<0)r=0;if (g<0)g=0;if (b<0)b=0;
		if (r>1)r=1;if (g>1)g=1;if (b>1)b=1;

		//console.log('Count='+select_num+'; num='+s+'; hue='+hue+'; rgb= ' + r + ',' + g + ',' + b);
		r=Math.round(255*r);g=Math.round(255*g);b=Math.round(255*b);
		let color = 'rgb('+r+','+g+','+b+')';

		// Select name:
		let text = document.createElementNS(NS, 'text');
		svg.appendChild(text);
		text.setAttribute('x', 10);
		text.setAttribute('y', 20+20*s);
		text.setAttribute('font-size', 16);
		text.setAttribute('fill', color);
		let label = select_name;
		text.textContent = label;

		// Start curve path:
		let path = document.createElementNS(NS,'path');
		svg.appendChild(path);
//		path.id = 'path0';
		path.setAttribute('stroke', color);
		path.setAttribute('stroke-width','1.5');
//		path.setAttribute('stroke-line-join','miter');
		path.setAttribute('fill','none');
		path.setAttribute('title', select_name);

		// Time interval loop to store values:
		let px = [];
		let py = [];
		for (let time in i_data.graph)
		{
			let value = 0;
			let record = i_data.graph[time][select_name];
			if (record) value = record.quantity;

			px.push(scale_x * (time - time_min));
			py.push(scale_y * value);
		}
		// Calculate tangents:
		let tx = [];
		let ty = [];
		for (let i = 0; i < px.length; i++)
		{
			let tl = .2;
			if (i == 0)
			{
				tx.push(px[i] + 2*tl * (px[i+1] - px[i]));
				ty.push(py[i] + 2*tl * (py[i+1] - py[i]));
			}
			else if (i == (px.length - 1))
			{
				tx.push(px[i] - 2*tl * (px[i] - px[i-1]));
				ty.push(py[i] - 2*tl * (py[i] - py[i-1]));
			}
			else
			{
				tx.push(px[i] - tl * (px[i+1] - px[i-1]));
				if ((py[i] >= py[i-1]) && (py[i] >= py[i+1]))
					ty.push(py[i]);
				else if ((py[i] <= py[i-1]) && (py[i] <= py[i+1]))
					ty.push(py[i]);
				else
				{
//					let _ty = py[i] - tl * (py[i+1] - py[i-1]);
					let _ty = tl * (py[i+1] - py[i-1]);
//					if ((py[i] + _ty < 0) || (py[i] - _ty < 0))
					if (py[i] + _ty < 0)
						ty.push(py[i] * 2);
					else if (py[i] - _ty < 0)
						ty.push(0);
					else
						ty.push(py[i] - _ty);
//						ty.push(py[i] - tl * (py[i+1] - py[i-1]));
				}
			}
		}

		if (0) { /* Try without smooth*/ if (0) /* Use tangents */
		{	let _px = []; let _py = [];
			for (let i = 0; i < px.length; i++) { _px.push(px[i]); _px.push(tx[i]); _py.push(py[i]); _py.push(ty[i]); }
		px = _px; py = _py; }
		let line = ''; for (let i = 0; i < px.length; i++)
		{	if (i == 0) line += 'M'; else line += ' L ';
			line += (px[i] + border.left) + ' ' + (height - border.bottom - py[i]); }
		path.setAttribute('d', line); continue; }

		// Draw curve:
//d="M10 80 C 40 10, 65 10, 95 80 S 150 150, 180 90, 250 60, 350 100" // <- example
//   p1     t1right  t2left p2      t3left   p3      t4left  p4
		let line = '';
		for (let i = 0; i < px.length; i++)
		{
			if (i == 0)
			{
				line += 'M';
				let x = px[i] + border.left;
				let y = height - border.bottom - py[i];
				line += x + ' ' + y;
				line += ' C ';
				x = tx[i] + border.left;
				y = height - border.bottom - ty[i];
				line += x + ' ' + y;
			}
			else
			{
				if (i == 2) line += ' S ';
				else line += ', ';
				let x = tx[i] + border.left;
				let y = height - border.bottom - ty[i];
				line += x + ' ' + y;
				line += ', ';
				x = px[i] + border.left;
				y = height - border.bottom - py[i];
				line += x + ' ' + y;
			}
		}
		path.setAttribute('d', line);
//console.log(line)
	}

	if (select == 'folder')
	{
		g_Info('Requesting tasks services statistics graph...');
		let args = i_args.args;
		args.select = 'service';
		g_Request({"send":{"get_tasks_graph":args},"func":g_ShowGraph,"args":args});
	}
	else if (select == 'service')
	{
		g_Info('Requesting tasks users statistics graph...');
		let args = i_args.args;
		args.select = 'username';
		g_Request({"send":{"get_tasks_graph":args},"func":g_ShowGraph,"args":args});
	}
	else
	{
		g_WaintingReset();
	}
}

function g_FolderDelete(i_evt)
{
	let folder = $('folder_delete').textContent;
	g_Log('Deleting folder: ' + folder);
	g_Request({"send":{"folder_delete":{"folder":folder}},"func":g_FoldersDeleted});
}
function g_FoldersDeleted(i_data, i_args)
{
	//console.log(JSON.stringify(i_data));
	let info = '';
	let deleted = 0;
	if (i_data.deleted_jobs)
	{
		deleted += i_data.deleted_jobs;
		info += 'Jobs deleted: ' + i_data.deleted_jobs;
	}
	if (i_data.deleted_tasks)
	{
		deleted += i_data.deleted_tasks;
		info += '; Tasks deleted: ' + i_data.deleted_tasks;
	}
	if (deleted)
	{
		g_Info(info);
		g_HashChanged();
	}
}

function g_TimeKeyDown(i_evt)
{
	if (i_evt.keyCode != 13) // Enter
		return;

	i_evt.stopPropagation();
	i_evt.currentTarget.blur();

	for (let i = 0; i < g_time_ids.length; i++)
	{
		let str = $(g_time_ids[i]).textContent;
		if ((str == null) || (str.length == 0))
			continue;
		let date = new Date(Date.parse(str));
		if (date == null)
			continue;
		time = Math.round(date.valueOf() / 1000);
		g_args[g_time_ids[i]] = time;
	}

	g_HashSet();
}

function g_TimeShow()
{
	for (let i = 0; i < g_time_ids.length; i++)
	{
		let secs = g_args[g_time_ids[i]];
		if (secs == null)
			continue;
		let date = new Date(1000 * g_args[g_time_ids[i]]);
		date = date.toISOString();
		date = date.substr(0, date.indexOf('T'));
		$(g_time_ids[i]).textContent = date;
	}	
}

function g_TimeIntervalGet()
{
	let interval = 1;
	let i = 0;
	time_max = g_args.time_max;
	if ((time_max == null) || (time_max == 0))
		time_max = Math.round((new Date()).valueOf() / 1000);
	while((time_max - g_args.time_min) / interval > 100)
	{
		if (i < g_graph_intervals.names.length)
			interval = g_graph_intervals[g_graph_intervals.names[i]].seconds;
		else
			interval *= 10;
		i++;
	}
	g_args.time_min = Math.floor(g_args.time_min / interval) * interval;
	g_args.time_max = Math.ceil( g_args.time_max / interval) * interval;

	return interval;
}

function g_SecToHMS(i_sec)
{
	let seconds = Math.floor(i_sec);
	let minutes = Math.floor(seconds / 60);
	let hours = Math.floor(minutes / 60);
	let days = Math.floor(hours / 24);
	if (days == 1)
		days = 0;
	seconds = seconds - minutes*60;
	if (seconds < 10)
		seconds = '0' + seconds;
	minutes = minutes - hours*60;
	if (minutes < 10)
		minutes = '0' + minutes;
	hours = hours - days*24;
	let hms = hours + ':' + minutes + ':' + seconds;
	if (days > 1)
		hms = days + 'd ' + hms;
	return hms;
}
function g_SecToStr(i_time)
{
	if (i_time == null)
		return '';
	let date = new Date(i_time*1000);
	str = '';
	str += (date.getMonth() + 1).toString().padStart(2,'0');
	str += '.' + date.getDate().toString().padStart(2,'0');
	str += ' ' + date.getHours().toString().padStart(2,'0');
	str += ':' + date.getMinutes().toString().padStart(2,'0');
	str += ':' + date.getSeconds().toString().padStart(2,'0');
	return str;
}
function g_Info(i_msg)
{
	$('info').innerHTML = i_msg;
	$('info').classList.remove('error');
	g_Log(i_msg);
}
function g_Error(i_msg)
{
	g_Info('ERROR: ' + i_msg);
	$('info').classList.add('error');
	g_Log(i_msg);
}
function g_Log(i_msg)
{
	let el = document.createElement('div');
	el.classList.add('log_line');
	$('log').insertBefore(el, $('log').firstChild);

	let date = new Date();
	let time = g_PadZero(date.getHours())+':'+g_PadZero(date.getMinutes())+':'+g_PadZero(date.getSeconds())+'.'+g_PadZero(date.getMilliseconds(),3);
	let elTime = document.createElement('span');
	elTime.classList.add('log_time');
	elTime.textContent = time;
	el.appendChild(elTime);

	let elMsg = document.createElement('span');
	elMsg.classList.add('log_msg');
	elMsg.textContent = i_msg;
	el.appendChild(elMsg);
}
function g_ShowLog(i_show)
{
	$('log').style.display = 'block';
}
function g_PadZero(i_num, i_len)
{
	if (i_len == null) i_len = 2;
	let str = ''+i_num;
	while(str.length < i_len) str = '0'+str;
	return str;
}
function g_Parse(i_data)
{
	if (i_data == null) return null;

	let obj = null;
	try { obj = JSON.parse(i_data);}
	catch(err)
	{
		g_Error(err.message+'<br/><br/>'+i_data);
		obj = null;
	}

	return obj;
}

function g_Request(i_args)
{
	if (i_args.send == null)
	{
		g_Error('Network reqest: send object is null.');
		return;
	}

	if (i_args.func)
	{
		i_args.wait = false;
		if (i_args.parse == null)
			i_args.parse = true;
	}
	else if (i_args.wait == null)
		i_args.wait = true;

	let send_str = JSON.stringify(i_args.send);
	if (i_args.encode == true)
		send_str = btoa(send_str);

	let xhr = new XMLHttpRequest;
	xhr.m_args = i_args;

	xhr.overrideMimeType('application/json');
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
	xhr.open('POST', SERVER);
	xhr.send(send_str);

	if (i_args.wait)
	{
		if (xhr.getResponseHeader('WWW-Authenticate'))
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
//console.log(this);
//console.log(this.readyState);
	if (this.readyState == 4)
	{
		if (this.status == 200)
		{
			if (this.m_args.func)
			{
				let data = this.responseText;
//console.log(data);
				if (this.m_args.parse)
					data = g_Parse(data);

				if (data.error)
				{
					g_Error(data.error);
					return;
				}

				this.m_args.func(data, this.m_args);
			}
		}
	}
}

g_parm = {};

g_parm.capacity_avg        = {"label":'Average Capacity', "round":true};
g_parm.error_avg           = {"label":'Average Error',    "percent":true};
g_parm.fav_name            = {"label":'Favourite'};
g_parm.fav_percent         = {"label":'Percent',          "percent":true};
g_parm.fav_service         = {"label":'Fav. Service'};
g_parm.fav_service_percent = {"label":'Percent',          "percent":true};
g_parm.fav_user            = {"label":'Fav. User'};
g_parm.fav_user_percent    = {"label":'Percent',          "percent":true};
g_parm.folder              = {"label":'Folder'};
g_parm.jobs_quantity       = {"label":'Jobs Quantity'};
g_parm.run_time_avg        = {"label":'Average Run Time', "time":true};
g_parm.run_time_sum        = {"label":'Sum Run Time',     "time":true};
g_parm.service             = {"label":'Service'};
g_parm.tasks_done_percent  = {"label":'Done',             "percent":true};
g_parm.tasks_quantity      = {"label":'Tasks Quantity'};
g_parm.tasks_quantity_avg  = {"label":'Average Quantity', "round":true};
g_parm.time                = {"label":'Date Time',        "datetime":true};
g_parm.username            = {"label":'User'};

