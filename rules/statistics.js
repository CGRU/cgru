function stcs_Show( i_args)
{
//console.log( JSON.stringify( i_args));
	var i_statuses = i_args.statuses;
	var i_elReports = i_args.elReports;
	var i_elReportsDiv = i_args.elReportsDiv;
	var i_elTasks = i_args.elTasks;
	var i_elTasksDiv = i_args.elTasksDiv;
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

		var elTd = document.createElement('td');
		elTr.appendChild( elTd);
		var txt = '';
		for( var t = 0; t < i_data[r].tags.length; t++)
		{
			if( t ) txt += ',';
			txt += ' ' + c_GetTagTitle( i_data[r].tags[t]);
		}
		elTd.textContent = txt;

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

	var elTd = document.createElement('th');
	elTr.appendChild( elTd);
	elTd.textContent = 'total';

	if( i_draw_bars !== true ) return;

	var elBarsDiv = document.createElement('div');
	i_el.appendChild( elBarsDiv);
	elBarsDiv.classList.add('bars');

	var width = 80;
	var height = 100;
	var height_coeff = height / i_data[0].duration;
	var height_text = 20;
	height += height_text * 2;

	for( var i = 0; i < i_data.length; i++)
	{
		var rect_h = i_data[i].duration * height_coeff;

		var elBar = document.createElement('div');
		elBarsDiv.appendChild( elBar);
		elBar.classList.add('bar');
		elBar.style.cssFloat = 'left';
		elBar.style.position = 'relative';
		elBar.style.width = width + 'px';
		elBar.style.height = height + 'px';

		var elBarRect = document.createElement('div');
		elBar.appendChild( elBarRect);
		elBarRect.classList.add('rect');
		elBarRect.style.position = 'absolute';
		elBarRect.style.bottom = height_text + 'px';
		elBarRect.style.width = width + 'px';
		elBarRect.style.height = rect_h + 'px';

		var elBarName = document.createElement('div');
		elBar.appendChild( elBarName);
		elBarName.classList.add('name');
		elBarName.textContent = c_GetTagTitle( i_data[i].tags[0]);
		elBarName.style.position = 'absolute';
		elBarName.style.bottom = '0';
		elBarName.style.width = width + 'px';

		var elBarValue = document.createElement('div');
		elBar.appendChild( elBarValue);
		elBarValue.classList.add('value');
		elBarValue.textContent = i_data[i].duration;
		elBarValue.style.position = 'absolute';
		elBarValue.style.width = width + 'px';
		elBarValue.style.bottom = height_text + rect_h + 'px';
	}
}

