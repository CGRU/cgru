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
	var tasks_duration = 0;
	var tasks_count = 0;

	i_elReports.textContent = '';
	var reps_types = {};
	var reps_duration = 0;
	var reps_count = 0;

	for( var s = 0; s < i_statuses.length; s++)
	{
		if( i_statuses[s] == null ) continue;

		if( i_statuses[s].tasks )
		for( var t = 0; t < i_statuses[s].tasks.length; t++)
		{
			var task = i_statuses[s].tasks[t];

			var el = document.createElement('div');
			i_elTasks.appendChild( el);
			el.classList.add('task');

			var elDur = document.createElement('div');
			el.appendChild( elDur);
			elDur.classList.add('duration');
			elDur.textContent = task.duration;

			if( task.tags && task.tags.length )
			{
				var elTags = document.createElement('div');
				el.appendChild( elTags);
				elTags.classList.add('tags');

				for( var g = 0; g < task.tags.length; g++)
				{
					var elTag = document.createElement('div');
					elTags.appendChild( elTag);
					elTag.classList.add('tag');
					elTag.textContent = c_GetTagTitle( task.tags[g]);
				}
			}

			if( task.artists && task.artists.length )
			{
				var elTags = document.createElement('div');
				el.appendChild( elTags);
				elTags.classList.add('artists');

				for( var g = 0; g < task.artists.length; g++)
				{
					var elTag = document.createElement('div');
					elTags.appendChild( elTag);
					elTag.classList.add('tag');
					elTag.textContent = c_GetUserTitle( task.artists[g]);
				}
			}

			tasks_duration += task.duration;
			tasks_count++;
		}

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
					reps_types[report.tags[t]] = rtype;
				}

				rtype.duration += report.duration;

				if( rtype.artists.indexOf( report.artist) == -1 )
					rtype.artists.push( report.artist);

			}

			reps_duration += report.duration;
			reps_count++;
		}
//console.log( JSON.stringify( report));
	}

	if( tasks_count )
	{
		var el = document.createElement('div');
		i_elTasks.appendChild( el);
		el.textContent = 'Total Duration: ' + tasks_duration;
		i_elTasksDiv.style.display = 'block';
	}
	else
		i_elTasksDiv.style.display = 'none';

	for( var rtype in reps_types )
	{
		var el = document.createElement('div');
		i_elReports.appendChild( el);
		var info = c_GetTagTitle( rtype) + ': ' + reps_types[rtype].duration;
		for( var a = 0; a < reps_types[rtype].artists.length; a++)
		{
			if( a ) info += ',';
			info += ' ' + c_GetUserTitle( reps_types[rtype].artists[a]);
		}
		el.textContent = info;
	}

	if( reps_count )
	{
		var el = document.createElement('div');
		i_elReports.appendChild( el);
		el.textContent = 'Total Duration: ' + reps_duration;
		i_elReportsDiv.style.display = 'block';
	}
	else
		i_elReportsDiv.style.display = 'none';

}

