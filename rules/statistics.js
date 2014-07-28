function stcs_Show( i_args)
{
//console.log( JSON.stringify( i_args));
	var i_statuses = i_args.statuses;
	var i_elReports = i_args.elReports;
	var i_elReportsDiv = i_args.elReportsDiv;
	var i_elTasks = i_args.elTasks;
	var i_elTasksDiv = i_args.elTasksDiv;
	var i_elDiffer = i_args.elDiffer;
	var i_elDifferDiv = i_args.elDifferDiv;
	var i_draw_bars = i_args.draw_bars;

	if( i_elTasks == null ) return;
	if( i_elReports == null ) return;

	i_elTasks.textContent = '';
	var tasks_types = {};
	var tasks_duration = 0;

	i_elReports.textContent = '';
	var reps_types = {};
	var reps_duration = 0;

	for( var s = 0; s < i_statuses.length; s++)
	{
		if( i_statuses[s] == null ) continue;

		// Tasks collection:
		if( i_statuses[s].tasks )
		for( var t = 0; t < i_statuses[s].tasks.length; t++)
		{
			var task = i_statuses[s].tasks[t];

			tasks_duration += task.duration;

			if( i_statuses.length == 1 )
			{
				tasks_types[t] = task;
				continue;
			}

			if(( task.tags == null ) || ( task.tags.length == 0 ))
				task.tags = ['other'];

			for( var g = 0; g < task.tags.length; g++)
			{
				var tag = task.tags[g];

				if( tasks_types[tag] == null )
					tasks_types[tag] = {"tags":[tag],"duration":0,"artists":[]};

				if( task.artists && task.artists.length )
				for( var a = 0; a < task.artists.length; a++)
					if( tasks_types[tag].artists.indexOf( task.artists[a]) == -1 )
						tasks_types[tag].artists.push( task.artists[a]);

				tasks_types[tag].duration += task.duration;
			}
		}

		// Reports collection:
		if( i_statuses[s].reports )
		for( var r = 0; r < i_statuses[s].reports.length; r++)
		{
			var report = i_statuses[s].reports[r];

			if(( report.tags == null ) || ( report.tags.length == 0 ))
				report.tags = ['other'];

			for( var t = 0; t < report.tags.length; t++)
			{
				var rtype;
				if( reps_types[report.tags[t]])
				{
					rtype = reps_types[report.tags[t]];
				}
				else
				{
					rtype = {};
					rtype.duration = 0;
					rtype.artists = [];
					rtype.tags = [report.tags[t]];
					reps_types[report.tags[t]] = rtype;
				}

				rtype.duration += report.duration;

				if( rtype.artists.indexOf( report.artist) == -1 )
					rtype.artists.push( report.artist);

			}

			reps_duration += report.duration;
		}
//console.log( JSON.stringify( report));
	}


	// Tasks show:
	var tasks = [];
	for( var ttype in tasks_types ) tasks.push( tasks_types[ttype]);
	if( tasks.length )
	{
		i_elTasksDiv.style.display = 'block';
		stcs_ShowTable({"el":i_elTasks,"data":tasks,"total_duration":tasks_duration,"draw_bars":i_draw_bars});
	}
	else
		i_elTasksDiv.style.display = 'none';


	// Reports show:
	var reports = [];
	for( var rtype in reps_types ) reports.push( reps_types[rtype]);
	if( reports.length )
	{
		i_elReportsDiv.style.display = 'block';
		stcs_ShowTable({"el":i_elReports,"data":reports,"total_duration":reps_duration,"draw_bars":i_draw_bars});
	}
	else
		i_elReportsDiv.style.display = 'none';


	if( i_elDiffer == null ) return;

	i_elDiffer.textContent = '';
	if( tasks.length && reports.length )
	{
		i_elDifferDiv.style.display = 'block';
		stcs_ShowDifference({"el":i_elDiffer,"tasks":tasks,"reports":reports});
	}
	else
		i_elDifferDiv.style.display = 'none';
}

function stcs_ShowTable( i_args)
{
	var i_el = i_args.el;
	var i_data = i_args.data;
	var i_total_duration = i_args.total_duration;
	var i_draw_bars = i_args.draw_bars;

	i_data.sort( function(a,b){if(a.duration<b.duration)return 1});

	var elTb = document.createElement('table');
	i_el.appendChild( elTb);
	elTb.classList.add('statistics');

	var elTr = document.createElement('tr');
	elTb.appendChild( elTr);

	var elTh = document.createElement('th');
	elTr.appendChild( elTh);
	elTh.textContent = 'D';
	elTh.title = 'Duration';

	var elTh = document.createElement('th');
	elTr.appendChild( elTh);
	elTh.textContent = 'Tags';
	elTh.title = 'Tags';

	var elTh = document.createElement('th');
	elTr.appendChild( elTh);
	elTh.textContent = 'Artists';
	elTh.title = 'Artists';

	for( var r = 0; r < i_data.length; r++)
	{
		var elTr = document.createElement('tr');
		elTb.appendChild( elTr);

		var elTd = document.createElement('td');
		elTr.appendChild( elTd);
		elTd.classList.add('duration');
		elTd.textContent = i_data[r].duration;
		elTd.title = '/8 = ' + (i_data[r].duration/8.0).toPrecision(2);

		var elTd = document.createElement('td');
		elTr.appendChild( elTd);
		var txt = '';
		for( var t = 0; t < i_data[r].tags.length; t++)
		{
			if( t ) txt += ',';
			txt += ' ' + c_GetTagTitle( i_data[r].tags[t]);
		}
		elTd.textContent = txt;
		if( i_data[r].tags.length == 1 )
			elTd.title = c_GetTagTip( i_data[r].tags[0]);

		var elTd = document.createElement('td');
		elTr.appendChild( elTd);
		var txt = '';
		for( var a = 0; a < i_data[r].artists.length; a++)
		{
			if( a ) txt += ',';
			txt += ' ' + c_GetUserTitle( i_data[r].artists[a]);
		}
		elTd.textContent = txt;
	}

	var elTr = document.createElement('tr');
	elTb.appendChild( elTr);

	var elTd = document.createElement('th');
	elTr.appendChild( elTd);
	elTd.textContent = i_total_duration;
	elTd.title = '/8 = ' + (i_total_duration/8.0).toPrecision(2);

	var elTd = document.createElement('th');
	elTr.appendChild( elTd);
	elTd.textContent = 'total';

	if( i_draw_bars !== true ) return;

	var elBarsDiv = document.createElement('div');
	i_el.appendChild( elBarsDiv);
	elBarsDiv.classList.add('bars');

	var width_coeff = 100 / i_data[0].duration;

	for( var i = 0; i < i_data.length; i++)
	{
		var rect_w = i_data[i].duration * width_coeff;
		var info = Math.round( 100.0 * i_data[i].duration / i_total_duration ) + '% ';
		info += c_GetTagTitle( i_data[i].tags[0]);
		var tooltip = 'Duration: ' + i_data[i].duration + ' ( /8=' + (i_data[i].duration/8.0).toPrecision(2) + ' )';

		var elBar = document.createElement('div');
		elBarsDiv.appendChild( elBar);
		elBar.classList.add('bar');
		elBar.style.position = 'relative';

		var elBarRect = document.createElement('div');
		elBar.appendChild( elBarRect);
		elBarRect.classList.add('rect');
		elBarRect.style.width =  rect_w + '%';
		elBarRect.title = tooltip;

		var elBarInfo = document.createElement('div');
		elBar.appendChild( elBarInfo);
		elBarInfo.classList.add('info');
		elBarInfo.textContent = info;
		elBarInfo.style.position = 'absolute';
		elBarInfo.style.top = '-2px';
		elBarInfo.title = tooltip;
		if( rect_w < 50 )
			elBarInfo.style.left = rect_w + '%';
		else
			elBarInfo.style.right = ( 100 - rect_w ) + '%';
	}
}

function stcs_ShowDifference( i_args)
{
	var i_el = i_args.el;
	var i_tasks = i_args.tasks;
	var i_reports = i_args.reports;

	var alltags = {};

	for( var i = 0; i < i_tasks.length; i++)
	{
		var tag = i_tasks[i].tags[0];
		alltags[tag] = {};
		alltags[tag].tsk_dur = i_tasks[i].duration;
	}

	for( var i = 0; i < i_reports.length; i++)
	{
		var tag = i_reports[i].tags[0];
		if( alltags[tag] == null ) alltags[tag] = {};
		alltags[tag].rep_dur = i_reports[i].duration;
	}

	var data = [];
	var differ_max = 1;
	var differ_min = 1;
	for( var tag in alltags )
	{
		var obj = alltags[tag];
		obj.tag = tag;
		obj.differ = 0;

		if( obj.tsk_dur && obj.rep_dur )
		{
			obj.differ = obj.rep_dur / obj.tsk_dur;

			if( obj.differ > 1 )
			if( obj.differ > differ_max )
				differ_max = obj.differ;
			if( obj.differ < differ_min )
				differ_min = obj.differ;
		}
		else if( obj.rep_dur )
			obj.differ = -1;


		data.push( obj);
	}

	var elBarsDiv = document.createElement('div');
	i_el.appendChild( elBarsDiv);
	elBarsDiv.classList.add('bars');

	data.sort( function(a,b)
	{
		if( b.differ == -1 ) return 1;
		if( a.differ == -1 ) return -1;
		if( a.differ < b.differ ) return 1;
	});

	var width_r = 50 / ( differ_max - 1 );
	var width_l = 50 / ( 1 - differ_min );

//console.log('dn=' + differ_min + ' dx=' + differ_max);
//console.log('wr=' + width_r + ' wl=' + width_l);

	for( var i = 0; i < data.length; i++ )
	{
		var info = data[i].differ.toPrecision(2);

		var tooltip = 'Duration: ';

		var r = 0; var g = 250; var b = 0; var a = .3;

		var elBar = document.createElement('div');
		elBarsDiv.appendChild( elBar);
		elBar.classList.add('bar');
		elBar.style.position = 'relative';

		var elBarRect = document.createElement('div');
		elBar.appendChild( elBarRect);
		elBarRect.classList.add('rect');
		elBarRect.style.position = 'absolute';

		var elBarInfo = document.createElement('div');
		elBar.appendChild( elBarInfo);
		elBarInfo.classList.add('info');
		elBarInfo.style.position = 'absolute';
		elBarInfo.style.top = '-2px';

		if( data[i].differ == -1 )
		{
//			elBarRect.style.display = 'none';
			elBarRect.style.left = '50%';
			elBarRect.style.right = '0';
			elBarInfo.style.right = '50%';
			elBarInfo.style.color = 'rgb(150,0,0)';
			r = 250; g = 0; a = .1;
			info = c_GetTagTitle( data[i].tag) + ' (no tasks)';
			tooltip += 'Reports ' + data[i].rep_dur;
		}
		else if( data[i].differ == 0 )
		{
//			elBarRect.style.display = 'none';
			elBarRect.style.left = '0';
			elBarRect.style.right = '50%';
			elBarInfo.style.left = '50%';
			elBarInfo.style.color = 'rgb(0,0,250)';
			b = 250; g = 0; a = .1;
			info = c_GetTagTitle( data[i].tag) + ' (no reports)';
			tooltip += 'Tasks ' + data[i].tsk_dur;
		}
		else if( data[i].differ < 1 )
		{
			elBarRect.style.right = '50%';
			elBarRect.style.left = (50 - width_l * ( 1 - data[i].differ )) + '%';
			elBarInfo.style.left = '50%';

			b = (1-data[i].differ) / (1-differ_min);
			if( b < .5 )
				b = 2 * b;
			else
			{
				b = 1;
				g = Math.round( 250.0 * ( .5 - .5*b ));
			}
			b = Math.round(250.0 * b);

			info += ' ' + c_GetTagTitle( data[i].tag);
			tooltip += 'Tasks ' + data[i].tsk_dur + ' / ' + data[i].rep_dur + ' Reports';
		}
		else
		{
			elBarRect.style.left = '50%';
			elBarRect.style.right = (50 + width_r * ( 1 - data[i].differ )) + '%';
			elBarInfo.style.right = '50%';

			r = (data[i].differ-1) / (differ_max-1);
			if( r < .5 )
				r = 2 * r;
			else
			{
				r = 1;
				g = Math.round( 250.0 * ( .5 - .5*r ));
			}
			r = Math.round(250.0 * r);

			info += ' ' + c_GetTagTitle( data[i].tag);
			tooltip += 'Tasks ' + data[i].tsk_dur + ' / ' + data[i].rep_dur + ' Reports';
		}
//console.log('rgba('+r+','+g+','+b+',.3)');
		elBarRect.style.background = 'rgba('+r+','+g+','+b+','+a+')';
		elBarInfo.textContent = info;
		elBarInfo.title = tooltip;
		elBarRect.title = tooltip;
	}
}

