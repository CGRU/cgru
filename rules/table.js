table_columns = {};
table_columns.name     = {'label':'Name'};
table_columns.picture  = {'label':'Picture'};
table_columns.state    = {'label':'State'};
table_columns.info     = {'label':'Info'};
table_columns.comments = {'label':'Comments'};
table_columns.tasks    = {'label':'Tasks'};
table_columns.duration = {'label':'D'};
table_columns.price    = {'label':'P'};

table_args = null;
table_doc = null;
table_shots = null;
table_shot_num = null;
table_shot = null;
table_functions = null;
table_function_num = null;

function table_Export( i_args)
{
	//console.log( JSON.stringify( i_args));
	//console.log( i_args);

	// Arguments:
	table_args = i_args;
	table_shots = i_args.shots;

	// Prepare input data:
	for( var s = 0; s < table_shots.length; s++)
	if( table_shots[s].status == null )
		table_shots[s].status = {};

	// Initializations:
	table_shot_num = 0;
	table_function_num = -1;
	table_functions = [];
	for( var c in table_columns )
		table_functions.push( window['table_Gen_' + c]);

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
		'td {',
			'text-align: center;',
			'border: 1px solid rgba(0,0,0,.5);',
			'padding: 4px;',
			'overflow: hidden;',
			'height: 50px;',
			'}',
		'</style>',
		'<table>'];
	table_WriteLines( lines);

	// Write shots table caption:
	ln = '<tr>';
	for( c in table_columns )
	{
		ln += '<th>';
		ln += table_columns[c].label;
		ln += '</th>';
	}
	ln += '</tr>';
	table_Write( ln);

	// Start write shots table:
	table_Write('<tr>');
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
		var reader = new FileReader();
		reader.onloadend = function()
		{
			table_PictureReceived( reader.result);
		}
		reader.readAsDataURL( xhr.response);
		xhr.response;
	}
	xhr.open('GET', url);
	xhr.send();
}
function table_PictureReceived( i_data)
{
	//console.log( i_data);
	var img = '<img src="' + i_data + '" width="200px">';

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
		info += '<div style="text-align: left">'
		info += i_data.replace(/\<\s*br\s*\/?\s*\>/g,' ');
		info += '</div>'
	}

	table_Write( info, true);

	table_Function();
}

function table_Gen_comments( i_shot)
{
	var cm = '';

	table_Write( cm, true);

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

