function stsc_Show( i_args)
{
//console.log( JSON.stringify( i_args));
	var i_statuses = i_args.statuses;
	var i_elReports = i_args.elReports;
	var i_elReportsDiv = i_args.elReportsDiv;
	var i_elTasks = i_args.elTasks;
	var i_elTasksDiv = i_args.elTasksDiv;

	if( i_elTasks == null ) return;
	if( i_elReports == null ) return;

	i_elTasks.textContent = '';
	var tasks = [];
	var tasks_types = {};
	var tasks_duration = 0;

	i_elReports.textContent = '';
	var reports = [];
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
	for( var ttype in tasks_types ) tasks.push( tasks_types[ttype]);
	if( tasks.length )
	{
		i_elTasksDiv.style.display = 'block';
		stsc_ShowTable({"el":i_elTasks,"data":tasks,"total_duration":tasks_duration});
	}
	else
		i_elTasksDiv.style.display = 'none';


	// Reports show:
	for( var rtype in reps_types ) reports.push( reps_types[rtype]);
	if( reports.length )
	{
		i_elReportsDiv.style.display = 'block';
		stsc_ShowTable({"el":i_elReports,"data":reports,"total_duration":reps_duration});
	}
	else
		i_elReportsDiv.style.display = 'none';

}

function stsc_ShowTable( i_args)
{
	var i_el = i_args.el;
	var i_data = i_args.data;
	var i_total_duration = i_args.total_duration;

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
}

