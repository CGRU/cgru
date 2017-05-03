function stcs_Show( i_args)
{
	var i_statuses     = i_args.statuses;
	var i_elReports    = i_args.elReports;
	var i_elReportsDiv = i_args.elReportsDiv;
	var i_elTasks      = i_args.elTasks;
	var i_elTasksDiv   = i_args.elTasksDiv;
	var i_elDiffer     = i_args.elDiffer;
	var i_elDifferDiv  = i_args.elDifferDiv;
	var i_main_atrists = i_args.main_artists;
	var i_draw_bars    = i_args.draw_bars;

	if( i_elTasks == null ) return;
	if( i_elReports == null ) return;

	i_elTasks.textContent = '';
	var tasks_types = {};
	var tasks_artists = {};

	i_elReports.textContent = '';
	var reps_types = {};
	var reps_artists = {};

	for( var s = 0; s < i_statuses.length; s++)
	{
		if( i_statuses[s] == null ) continue;

		// Get progress from status:
		var done = false;
		var progress = 0;
		if( i_statuses[s].progress )
		{
			progress = i_statuses[s].progress;
			if( i_statuses[s].progress >= 100 )
				done = true;
		}

		// Tasks collection:
		if( i_statuses[s].tasks )
		for( var t = 0; t < i_statuses[s].tasks.length; t++)
		{
			var task = i_statuses[s].tasks[t];

			// If task has no own progress,
			// we set task progress from status progress:
			if( task.done == null )
				task.done = done;
			if( task.progress == null )
				task.progress = progress;

			if(( task.duration == null ) || ( task.duration <= 0 ))
				task.duration = 1;
			if(( task.tags == null ) || ( task.tags.length == 0 ))
				task.tags = ['other'];
			if(( task.artists == null ) || ( task.tags.artists == 0 ))
				task.artists = ['other'];

			if( i_statuses.length == 1 )
			{
				tasks_types[t] = task;
				tasks_artists[t] = task;
				continue;
			}

			for( var g = 0; g < task.tags.length; g++)
			{
				var tag = task.tags[g];

				if( tasks_types[tag] == null )
					tasks_types[tag] = {"tags":[tag],"duration":0,"done":0,"progress":0,"price":null,"artists":[]};

				if( task.artists && task.artists.length )
				for( var a = 0; a < task.artists.length; a++)
					if( tasks_types[tag].artists.indexOf( task.artists[a]) == -1 )
						tasks_types[tag].artists.push( task.artists[a]);

				tasks_types[tag].duration += task.duration;
				if( task.done )
					tasks_types[tag].done += task.duration;
				if( task.progress )
					tasks_types[tag].progress += task.duration * task.progress;

				if( task.price )
				{
					if( tasks_types[tag].price == null )
						tasks_types[tag].price = 0;
					tasks_types[tag].price += task.price;
				}
			}

			for( var a = 0; a < task.artists.length; a++)
			{
				var artist = task.artists[a];

				if( tasks_artists[artist] == null )
					tasks_artists[artist] = {"artists":[artist],"duration":0,"done":0,"progress":0,"price":0,"tags":[]};

				if( task.tags && task.tags.length )
				for( var g = 0; g < task.tags.length; g++)
					if( tasks_artists[artist].tags.indexOf( task.tags[g]) == -1 )
						tasks_artists[artist].tags.push( task.tags[g]);

				tasks_artists[tag].duration += task.duration;
				if( task.done )
					tasks_artists[tag].done += task.duration;
				if( task.progress )
					tasks_artists[tag].progress += task.duration * task.progress;

				if( task.price )
				{
					if( tasks_artists[tag].price == null )
						tasks_artists[tag].price = 0;
					tasks_artists[tag].price += task.price;
				}
			}
		}

		// Reports collection:
		if( i_statuses[s].reports )
		for( var r = 0; r < i_statuses[s].reports.length; r++)
		{
			var report = i_statuses[s].reports[r];

			if(( report.duration == null ) || ( report.duration <= 0 ))
				report.duration = 1;
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

			var rartist;
			var rartist;
			if( reps_artists[report.artist])
			{
				rartist = reps_types[report.artist];
			}
			else
			{
				rtype = {};
				rtype.duration = 0;
				rtype.artists = [report.artist];
				rtype.tags = report.tags;
				reps_artists[report.artist] = rtype;
			}

			rtype.duration += report.duration;

			for( var t = 0; t < report.tags.length; t++)
				if( rtype.tags.indexOf( report.tags[t]) == -1 )
					rtype.tags.push( report.tags[t]);
		}
//console.log( JSON.stringify( report));
	}


	// Tasks show:
	var tasks_t = []; var tasks_a = [];
	for( var ttype in tasks_types   ) tasks_t.push( tasks_types[ttype]);
	for( var ttype in tasks_artists ) tasks_a.push( tasks_artists[ttype]);
	if( tasks_t.length )
	{
		i_elTasksDiv.style.display = 'block';
		if( tasks_t.length )
			stcs_ShowTable({"el":i_elTasks,"data":tasks_t,"draw_bars":i_draw_bars,"main":"tags"});
		if( tasks_a.length && i_main_atrists )
			stcs_ShowTable({"el":i_elTasks,"data":tasks_a,"draw_bars":i_draw_bars,"main":"artists"});
	}
	else
		i_elTasksDiv.style.display = 'none';


//console.log( JSON.stringify( reps_artists));
	// Reports show:
	var reports_t = []; var reports_a = [];
	for( var rtype in reps_types   ) reports_t.push( reps_types[rtype]);
	for( var rtype in reps_artists ) reports_a.push( reps_artists[rtype]);
	if( reports_t.length || reports_a.length )
	{
		i_elReportsDiv.style.display = 'block';
		if( reports_t.length )
			stcs_ShowTable({"el":i_elReports,"data":reports_t,"draw_bars":i_draw_bars,"main":"tags"});
		if( reports_a.length && i_main_atrists )
			stcs_ShowTable({"el":i_elReports,"data":reports_a,"draw_bars":i_draw_bars,"main":"artists"});
	}
	else
		i_elReportsDiv.style.display = 'none';


	if( i_elDiffer == null ) return;

	i_elDiffer.textContent = '';
	if( tasks_t.length && reports_t.length )
	{
		i_elDifferDiv.style.display = 'block';
		stcs_ShowDifference({"el":i_elDiffer,"tasks":tasks_t,"reports":reports_t});
	}
	else
		i_elDifferDiv.style.display = 'none';
}

function stcs_ShowTable( i_args)
{
	var i_el = i_args.el;
	var i_data = i_args.data;
	var i_draw_bars = i_args.draw_bars;
if( i_data[0] == null ) console.log( JSON.stringify( i_args));
	var has_price = i_data[0].price != null;

	var sort = 'duration';
	if( has_price && ( i_args.main == 'artists' )) sort = 'price';
	i_data.sort( function(a,b){if(a[sort]<b[sort])return 1});

	var elTb = document.createElement('table');
	i_el.appendChild( elTb);
	elTb.classList.add('statistics');

	var elTr = document.createElement('tr');
	elTb.appendChild( elTr);

	var elThD = document.createElement('th');
	elTr.appendChild( elThD);
	elThD.textContent = 'D';
	elThD.title = 'Duration';

	if( has_price )
	{
		var elTh = document.createElement('th');
		elTh.textContent = 'P';
		elTh.title = 'Price';
		if( sort =='price' )
			elTr.insertBefore( elTh, elThD);
		else
			elTr.appendChild( elTh);
	}

	var elThT = document.createElement('th');
	elTr.appendChild( elThT);
	elThT.textContent = 'Tags';
	elThT.title = 'Tags';

	var elTh = document.createElement('th');
	elTh.textContent = 'Artists';
	elTh.title = 'Artists';
	if( sort == 'price' )
		elTr.insertBefore( elTh, elThD);
	else if( i_args.main == 'artists')
		elTr.insertBefore( elTh, elThT);
	else
		elTr.appendChild( elTh);

	var summs = {"duration":0,"done":0,"progress":0,"price":0};
	for( var r = 0; r < i_data.length; r++)
	{
		var elTr = document.createElement('tr');
		elTb.appendChild( elTr);

		var elTdD = document.createElement('td');
		elTr.appendChild( elTdD);
		elTdD.classList.add('duration');
		var text = c_NumToStr( i_data[r].duration);
		if( i_data[r].done  )
			text += ' / ' + c_NumToStr( i_data[r].done);
		elTdD.textContent = text;

		if( has_price )
		{
			var elTd = document.createElement('td');
			elTd.classList.add('price');
			elTd.textContent = i_data[r].price;
			elTd.title = '/1000 = ' + (i_data[r].price/1000.0).toPrecision(2);

			if( i_args.main == 'artists' )
				elTr.insertBefore( elTd, elTdD);
			else
				elTr.appendChild( elTd);
		}

		var elTdT = document.createElement('td');
		elTr.appendChild( elTdT);
		var txt = '';
		for( var t = 0; t < i_data[r].tags.length; t++)
		{
			if( t ) txt += ',';
			txt += ' ' + c_GetTagTitle( i_data[r].tags[t]);
		}
		elTdT.textContent = txt;
		if( i_data[r].tags.length == 1 )
			elTdT.title = c_GetTagTip( i_data[r].tags[0]);

		var elTd = document.createElement('td');
		var txt = '';
		for( var a = 0; a < i_data[r].artists.length; a++)
		{
			if( a ) txt += ',';
			txt += ' ' + c_GetUserTitle( i_data[r].artists[a]);
		}
		elTd.textContent = txt;
		if( sort == 'price' )
			elTr.insertBefore( elTd, elTdD);
		else if( i_args.main == 'artists')
			elTr.insertBefore( elTd, elTdT);
		else
			elTr.appendChild( elTd);

		summs.duration += i_data[r].duration;
		summs.done     += i_data[r].done;
		summs.progress += i_data[r].progress;
		summs.price    += i_data[r].price;
	}

	var elTr = document.createElement('tr');
	elTb.appendChild( elTr);

	var elTdD = document.createElement('th');
	elTr.appendChild( elTdD);
	var text = c_NumToStr( summs.duration);
	if( summs.done )
		text += ' / ' + c_NumToStr( summs.done);
	elTdD.textContent = text;

	var elTdT = document.createElement('th');
	elTdT.textContent = 'total';

	if( has_price )
	{
		var elTdP = document.createElement('th');
		elTdP.textContent = summs.price;
		elTdP.title = '/1000 = ' + (summs.price/1000.0).toPrecision(2);
		elTr.appendChild( elTdP);
		if( i_args.main == 'artists' )
		{
			elTr.insertBefore( elTdP, elTdD);
			elTr.insertBefore( elTdT, elTdD);
		}
		else
		{
			elTr.appendChild( elTdP);
			elTr.appendChild( elTdT);
		}
	}

	if( i_draw_bars !== true ) return;


	var elBarsDiv = document.createElement('div');
	i_el.appendChild( elBarsDiv);
	elBarsDiv.classList.add('bars');

	var width_coeff = 100 / i_data[0][sort];

	for( var i = 0; i < i_data.length; i++)
	{
		var rect_w = i_data[i][sort] * width_coeff;
		var info = Math.round( 100.0 * i_data[i][sort] / summs[sort] ) + '% ';
		var key = i_data[i][i_args.main][0];
		if( i_args.main == 'artists' )
			info += c_GetUserTitle( key);
		else
			info += c_GetTagTitle( key);

		var elBar = document.createElement('div');
		elBarsDiv.appendChild( elBar);
		elBar.classList.add('bar');
		elBar.style.position = 'relative';

		var elBarRect = document.createElement('div');
		elBar.appendChild( elBarRect);
		elBarRect.classList.add('rect');
		elBarRect.style.width =  rect_w + '%';

		if(( sort == 'duration') && i_data[i]['duration'] && i_data[i]['done'])
		{
			var elBarRectDone = document.createElement('div');
			elBarRect.appendChild( elBarRectDone);
			elBarRectDone.classList.add('rect_done');
			elBarRectDone.style.width =  Math.round( 100.0 * i_data[i]['done'] / i_data[i]['duration']) + '%';
		}

		var elBarInfo = document.createElement('div');
		elBar.appendChild( elBarInfo);
		elBarInfo.classList.add('info');
		elBarInfo.textContent = info;
		elBarInfo.style.position = 'absolute';
		elBarInfo.style.top = '-2px';
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
				g = Math.round( 250.0 * ( 1 - 2*(b-.5) ));
				b = 1;
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
				g = Math.round( 250.0 * ( 1 - 2*(r-.5) ));
				r = 1;
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

