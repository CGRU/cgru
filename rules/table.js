function table_Export( i_args)
{
console.log( JSON.stringify( i_args));
//console.log( i_args);

	var wnd = window.open( null, 'Shots Table', 'location=no,scrollbars=yes,resizable=yes,menubar=no');
	if( wnd == null )
	{
		g_Error('Can`t open new browser window.');
		return;
	}
/*
	wnd.document.writeln('<!DOCTYPE html>');
	wnd.document.writeln('<html><head><title>'+"Shots Table"+'</title>');
	wnd.document.writeln('</head><body>');

	wnd.document.writeln('</body></html>');
*/
	var ln = [];
	ln.push('<!DOCTYPE html>');
	ln.push('<html><head><title>'+"Shots Table"+'</title>');
	ln.push('</head><body>');
	ln.push('<style type="text/css">');
	ln.push('table {' +
			'border-collapse: collapse;' +
			'width: 100%;' +
			'}'); +
	ln.push('td { text-align: center; border: 1px solid rgba(0,0,0,.5); padding: 4px; min-height: 20px; }');
		/*

.shot_files {
clear: both;
}
.shot_files .filesview {
margin: 10px;
border: 1px solid #777;
border-radius: 2px;
}

#shot_process_div > div {
float:left;
font-size: 12px;
margin:2px 4px;
}

.shot_empty_result { float: left; padding: 2px 6px; }

*/
	ln.push('</style>');
	ln.push('<table>');
	ln.push('<tr>');

	ln.push('<th>Name</th>');
	ln.push('<th>%</th>');
	ln.push('<th>Body</th>');
	ln.push('<th>Comments</th>');
	ln.push('<th>Price</th>');
	ln.push('</tr>');

	for( var s = 0; s < i_args.shots.length; s++)
	{
		ln.push('<tr>');

		var sln = table_ShotExport( i_args.shots[s]);

		for( var l = 0; l < sln.length; l++)
		{
			ln.push('<td>');

			ln.push(sln[l]);

			ln.push('</td>');
		}

		ln.push('</tr>');
	}

	ln.push('</table>');
	ln.push('</body></html>');

	for( var l = 0; l < ln.length; l++)
		wnd.document.writeln(ln[l]);

	wnd.document.close();
}

function table_ShotExport( i_shot)
{
	var name = c_PathBase( i_shot.path);

	var progress = '';

	var body = '';

	var comments = '';

	var price = '';

	var ln = [];
	ln.push( name);
	ln.push( progress);
	ln.push( body);
	ln.push( comments);
	ln.push( price);

	return ln;
}

