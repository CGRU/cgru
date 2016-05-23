table_columns = {};
table_columns.name     = {'label':'Name'};
table_columns.picture  = {'label':'Picture'};
table_columns.state    = {'label':'State'};
table_columns.info     = {'label':'Info'};
table_columns.comments = {'label':'Comments'};
table_columns.tasks    = {'label':'Tasks'};
table_columns.duration = {'label':'Dur'};
table_columns.price    = {'label':'Price'};

table_args = null;
table_doc = null;
table_shots = null;
table_shot_num = null;
table_shot = null;
table_functions = null;
table_function_num = null;

table_params = {};
table_params.picture  = {'type':'bool','default':true,'width':'20%'};
table_params.state    = {'type':'bool','default':true,'width':'20%'};
table_params.tasks    = {'type':'bool','default':true,'width':'20%'};
table_params.duration = {'type':'bool','default':true,'width':'20%'};
table_params.price    = {'type':'bool','default':true,'width':'20%'};

function table_Export( i_args)
{
	var wnd = new cgru_Window({'name':'shots_table_export','title':'Export Shots Table'});
	wnd.m_args = i_args;

	gui_Create( wnd.elContent, table_params, []);

	var elExport = document.createElement('div');
	wnd.elContent.appendChild( elExport);
	elExport.textContent = 'Export';
	elExport.classList.add('button');
	elExport.m_wnd = wnd;
	elExport.onclick = function(e){ table_ExportDo( e.currentTarget.m_wnd);}
}

function table_ExportDo( i_wnd)
{
	args = i_wnd.m_args;
	args.params = gui_GetParams( i_wnd.elContent, table_params);
	table_Start( args);
}

function table_Start( i_args)
{
	//console.log( JSON.stringify( i_args));
	//console.log( i_args);

	// Initializations:
	table_shot_num = 0;
	table_function_num = -1;
	table_functions = [];


	// Arguments:
	table_args = i_args;
	table_shots = i_args.shots;
	var table_show = {};
	for( var c in table_columns )
	{
		table_show[c] = true;
		if( i_args.params[c] === false )
			table_show[c] = false;
	}


	// Prepare input data:
	for( var s = 0; s < table_shots.length; s++)
	{
		var shot = table_shots[s];

		if( shot.status == null )
			shot.status = {};

		if( shot.status.flags ==  null )
			shot.status.flags = [];

		if( shot.status.flags.indexOf('omit') != -1 )
			shot.omit = true;
	}


	// Prepare document title:
	var title = g_CurPath();
	title = title.replace(/^\//,'');
	title = title.replace(/\/$/,'');
	title = title.replace(/\//g,'_');


	// Open new window
	var wnd = window.open( null, title/*, 'location=no,scrollbars=yes,resizable=yes'/*,menubar=no'*/);
	if( wnd == null )
	{
		g_Error('Can`t open new browser window.');
		return;
	}
	table_doc = wnd.document;


	// Write headers:
	var lines = ['<!DOCTYPE html>',
		'<html><head><title>' + title + '</title>',
		'</head><body>',
		'<style type="text/css">',
		'table {',
			'border-collapse: collapse;',
			'width: 100%;',
			'}',
		'tr {',
			'page-break-inside:avoid;',
			'height: 50px;',
			'}',
		'td, th {',
			'text-align: center;',
			'border: 1px solid rgba(0,0,0,.5);',
			'padding: 4px;',
			'overflow: hidden;',
			'height: 50px;',
			'}',
		'</style>'];
	table_WriteLines( lines);
	table_Write('<h3>' + title + '</h3>');


	// Summary:
	var progress = 0;
	var omits = 0;
	var duration = 0;
	var price = 0;
	for( var s = 0; s < table_shots.length; s++)
	{
		var shot = table_shots[s];

		if( shot.omit )
		{
			omits ++;
			continue;
		}

		if( shot.status.progress && ( shot.status.progress > 0 ))
			progress += shot.status.progress;

		if( shot.status.duration && ( shot.status.duration > 0 ))
			duration += shot.status.duration;

		if( shot.status.price && ( shot.status.price > 0 ))
			price += shot.status.price;
	}
	progress = Math.floor( progress / ( table_shots.length - omits ));
	if( price    == 0 ) table_show.price = false;
	if( duration == 0 ) table_show.duration = false;

	var lines = ['<table>',
		'<tr>',
			'<th>Count</th>',
			'<th>Progress</th>',
			'<th>Duration</th>',
			'<th>Price</th>',
		'</tr>',
		'<tr>',
			'<td>' + ( table_shots.length - omits ) + (omits?(' ( +' + omits + ' omits)'):'') + '</td>',
			'<td>' + progress + '%</td>',
			'<td>' + duration + '</td>',
			'<td>' + price + '</td>',
		'</tr>',
		'</table>'];
	table_WriteLines( lines);


	// Write shots table caption:
	table_Write('<table>');
	ln = '<tr>';
	for( c in table_columns )
	{
		if( false == table_show[c] )
			continue;

		ln += '<th>';
		ln += table_columns[c].label;
		ln += '</th>';
	}
	ln += '</tr>';
	table_Write( ln);


	// Start write shots table:
	table_Write('<tr>');
	for( var c in table_columns )
	{
		if( false == table_show[c] )
			continue;

		table_functions.push( window['table_Gen_' + c]);
	}
	table_Function();
}

function table_WriteLines( i_lines)
{
	for( var l = 0; l < i_lines.length; l++)
		table_Write( i_lines[l]);
}
function table_Write( i_data, i_td)
{
	if( i_td)
		i_data = '<td>' + i_data + '</td>';

	table_doc.writeln( i_data);
}

function table_Finish()
{
	table_Write('</table>');
	table_Write('</body></html>');

	table_doc.close();
}

function table_Function()
{
	table_function_num++;
	if( table_function_num >= table_functions.length )
	{
		table_Write('</tr>');
		table_function_num = 0;
		table_shot_num++;

		if( table_shot_num >= table_shots.length )
		{
			table_Finish();
			return;
		}

		table_Write('<tr>');
	}

//console.log('table_Function: f=' + table_function_num + ' s=' + table_shot_num);
	table_functions[table_function_num]( table_shots[table_shot_num]);
//	setTimeout( table_functions[table_function_num]( table_shots[table_shot_num]), 100);
}

function table_Gen_name( i_shot)
{
	var name = c_PathBase( i_shot.path);

	table_Write( name, true);

	table_Function();
}

function table_Gen_picture( i_shot)
{
	var url = c_GetRuFilePath(RULES.thumbnail.filename, i_shot.path);
	//console.log(url);
	var xhr = new XMLHttpRequest();
	xhr.responseType = 'blob';
	xhr.onload = function()
	{
		if( xhr.status != 200 )
		{
			table_PictureReceived( null);
			return;
		}

		var reader = new FileReader();
		reader.onloadend = function()
		{
			table_PictureReceived( reader.result);
		}
		reader.readAsDataURL( xhr.response);
	}
	xhr.open('GET', url);
	xhr.send();
}
function table_PictureReceived( i_data)
{
	//console.log( i_data);
	var img = '';

	if( i_data )
		img = '<img width="200px" src="' + i_data + '">';

	table_Write( img, true);

	table_Function();
}

function table_Gen_state( i_shot)
{
	var st = '';
	if( i_shot.status.progress && ( i_shot.status.progress > 0 ))
		st += i_shot.status.progress + '%';

	table_Write( st, true);

	table_Function();
}

function table_Gen_info( i_shot)
{
	var path = i_shot.path;
	path = RULES.root + path + '/' + RULES.rufolder + '/' + u_body_filename;
	n_GetFile({'path':path,'func':table_BodyReceived,'info':'table_body','shot':i_shot,'parse':false});
}
function table_BodyReceived( i_data, i_args)
{
	var info = '';
	if( i_args.shot.status.annotation )
		info += i_args.shot.status.annotation;

	if( i_data.indexOf('"error"') == -1 )
	{
		var body = i_data.replace(/<(?:.|\n)*?>/gm, '');

		info += '<div style="text-align: left">'
		info += body;
		info += '</div>'
	}

	table_Write( info, true);

	table_Function();
}

function table_Gen_comments( i_shot)
{
	var cpath = c_GetRuFilePath( cm_file, i_shot.path);
	n_GetFile({'path':cpath,'func':table_CommentsReceived,'info':'table_comments','parse':true});
}
function table_CommentsReceived( i_data, i_args)
{
	var text = '';

	if(( i_data.error == null ) && ( i_data.comments))
	{
		var cm_array = [];
		for( key in i_data.comments )
		{
			i_data.comments[key].key = key;
			cm_array.push( i_data.comments[key]);
		}

		if( cm_array.length )
		{
			cm_array.sort( function(a,b){if(a.key<b.key)return -1;if(a.key>b.key)return 1;return 0;});
			text = cm_array[0].text;
			text = text.replace(/<(?:.|\n)*?>/gm, '');
		}
	}

//text = JSON.stringify( i_data);
	table_Write( text, true);

	table_Function();
}

function table_Gen_tasks( i_shot)
{
	var tasks = '';

	table_Write( tasks, true);

	table_Function();
}

function table_Gen_duration( i_shot)
{
	var dur = '';
	if( i_shot.status.duration )
		dur += i_shot.status.duration;

	table_Write( dur, true);

	table_Function();
}

function table_Gen_price( i_shot)
{
	var price = '';
	if( i_shot.status.price )
		price += i_shot.status.price;

	table_Write( price, true);

	table_Function();
}

