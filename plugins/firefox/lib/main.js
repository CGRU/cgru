const { Cc, Ci } = require('chrome');
const ContextMenu = require('context-menu');
const { get, set, exists } = Cc['@mozilla.org/process/environment;1'].getService( Ci.nsIEnvironment);
const RunTime = require('runtime');

Info = function( i_msg) { console.log('CGRU: '+i_msg);}

Info('OS = "'+RunTime.OS+'"');
var CmdShell = ['/bin/bash','-c'];
if( RunTime.OS == 'WINNT' )
{
	CmdShell = [ get('ComSpec'),'/c'];
	Info('USER = "'+get('USERNAME')+'"');
}
else
{
	Info('USER = "'+get('USER')+'"');
}
Info('CmdShell = "'+CmdShell[0]+' '+CmdShell[1]+'"');

var menuItem = ContextMenu.Item({
	label: "Run",
	context: ContextMenu.SelectorContext('.cmdexec'),
	contentScript: 'self.on("click", function ( node, data) {' +
                 '  var cmdexec = node.getAttribute("cmdexec");' +
                 '  if( cmdexec ) {' +
                 '  cmdexec = JSON.parse( cmdexec);' +
                 '  for( var i = 0; i < cmdexec.length; i++ ) self.postMessage( cmdexec[i]);' +
                 '} else' +
                 '  self.postMessage( node.textContent);' +
                 '});',

onMessage: function ( i_cmd) {

    Info('Executing: '+i_cmd);

	var file = Cc["@mozilla.org/file/local;1"].createInstance( Ci.nsILocalFile);
	file.initWithPath( CmdShell[0]);

	var process = Cc["@mozilla.org/process/util;1"].createInstance( Ci.nsIProcess);
	process.init( file);
	process.run( false, [ CmdShell[1], i_cmd], 2);
}
});
