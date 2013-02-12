<?php

$AccessFileName = '.htaccess';
$RuleMaxLength = 100000;
$UserName = null;

if( isset($_SERVER['PHP_AUTH_USER']))
{
	global $UserName;

	$UserName = $_SERVER['PHP_AUTH_USER'];
}

function htaccessFolder( $i_folder)
{
	global $RuleMaxLength, $AccessFileName, $UserName;

	if( $UserName == null ) return true;

	$htaccess = $i_folder.'/'.$AccessFileName;
//error_log('checking file '.$htaccess);
	if( is_file( $htaccess))
	{
		if( $fHandle = fopen( $htaccess, 'r'))
		{
			$data = fread( $fHandle, $RuleMaxLength);
			fclose( $fHandle);

			$pos = strrpos( $data, 'Require user ');
			if( $pos !== false )
			{
				$data = substr( $data, $pos+13);
//error_log('checking string1['.$pos.':]: '.$data);
				$end = strpos( $data, PHP_EOL);
//error_log('checking string2['.$pos.':'.$end.']: '.substr( $data, 0, $end));
				if( $end !== false )
				{
					$list = explode(' ', substr( $data, 0, $end));
					foreach( $list as $user )
					{
//error_log('checking user: "'.$user.'"');
						if( $user == $UserName )
							return true;
					}
					return false;
				}
			}
		}
	}

	return true;
}

function htaccessPath( $i_path)
{
//error_log('Checking access path "'.$i_path.'"');
	$folders = explode('/', $i_path);
	$path = null;
	foreach( $folders as $folder)
	{
		if( $path != null )
			$path = $path.'/'.$folder;
		else
			$path = $folder;
//error_log('Checking access folder "'.$path.'"');
		if( false == htaccessFolder($path))
			return false;
	}

	return true;
}

function walkDir( $i_recv, $i_dir, &$o_out, $i_depth)
{
	global $RuleMaxLength;

	if( $i_depth > $i_recv['depth'] ) return;

	if( false == is_dir( $i_dir))
	{
		$o_out['error'] = 'No such folder.';
		return;
	}

	if( false == htaccessPath($i_dir))
	{
		$o_out['error'] = 'Access denied.';
		return;
	}

	$rufolder = null;
	if( array_key_exists('rufolder', $i_recv))
		$rufolder = $i_recv['rufolder'];
	$rufiles = null;
	if( array_key_exists('rufiles', $i_recv))
		$rufiles = $i_recv['rufiles'];
	$lookahead = null;
	if( array_key_exists('lookahead', $i_recv))
		$lookahead = $i_recv['lookahead'];

	if( $handle = opendir( $i_dir))
	{
		$o_out['folders'] = array();
		$o_out['files'] = array();

		while (false !== ( $entry = readdir( $handle)))
		{
			if( $entry == '.') continue;
			if( $entry == '..') continue;
			$path = $i_dir.'/'.$entry;
			if( false == is_dir( $path))
			{
				array_push( $o_out['files'], $entry);
				continue;
			}

			if( $entry == $rufolder )
			{
				$o_out['rufiles'] = array();
				$o_out['rules'] = array();

				if( $rHandle = opendir( $path))
				{
					while (false !== ( $ruentry = readdir( $rHandle)))
					{
						if( $ruentry == '.') continue;
						if( $ruentry == '..') continue;

						array_push( $o_out['rufiles'], $ruentry);

						if( strrpos( $ruentry,'.json') === false ) continue;

						$founded = false;
						foreach( $rufiles as $rufile )
							if( strpos( $ruentry, $rufile ) === 0 )
							{
								$founded = true;
								break;
							}
						if( false == $founded ) continue;

						if( $fHandle = fopen( $path.'/'.$ruentry, 'r'))
						{
							$rudata = fread( $fHandle, $RuleMaxLength);
							$ruobj = json_decode( $rudata, true);
							$o_out['rules'][$ruentry] = $ruobj;
							fclose($fHandle);
						}
					}
					closedir($rHandle);
					sort( $o_out['rufiles']);
					ksort($o_out['rules']);
				}
			}

			if( false == htaccessFolder( $path))
				continue;

			$folderObj = array();
			$folderObj['name'] = $entry;

			if( $rufolder && $lookahead )
				foreach( $lookahead as $sfile )
				{
					$sfilepath = $path.'/'.$rufolder.'/'.$sfile.'.json';
					if( is_file( $sfilepath))
					{
						if( $fHandle = fopen( $sfilepath, 'r'))
						{
							$data = fread( $fHandle, $RuleMaxLength);
							fclose( $fHandle);
							mergeObjs( $folderObj, json_decode( $data, true));
						}
					}
				}

			if( $i_depth < $i_recv['depth'] )
				walkDir( $i_recv, $path, &$folderObj, $i_depth+1);

			array_push( $o_out['folders'], $folderObj);

		}
		closedir($handle);
		sort( $o_out['folders']);
		sort( $o_out['files']);
	}
}

function readConfig( $i_file, &$o_out)
{
	global $RuleMaxLength;

	if( $fHandle = fopen( $i_file, 'r'))
	{
		$data = fread( $fHandle, $RuleMaxLength);
		fclose($fHandle);
		$o_out[$i_file] = json_decode( $data, true);
		if( array_key_exists( 'include', $o_out[$i_file]['cgru_config']))
			foreach( $o_out[$i_file]['cgru_config']['include'] as $file )
				readConfig( $file, $o_out);
	}
}

function readObj( $i_file, &$o_out)
{
	global $RuleMaxLength;

	if( $fHandle = fopen( $i_file, 'r'))
	{
		$data = fread( $fHandle, $RuleMaxLength);
		fclose($fHandle);
		$o_out = json_decode( $data, true);
	}
	else
	{
		$o_out['status'] = 'error';
		$o_out['error'] = 'Unable to load file '.$i_file;
	}
}

function mergeObjs( &$o_obj, $i_obj)
{
//error_log('mergeObjs: i='.json_encode($i_obj));
	if( is_null( $i_obj) || is_null( $o_obj)) return;
	foreach( $i_obj as $key => $val )
	{
		if( array_key_exists( $key, $o_obj) && is_array( $val) && is_array( $o_obj[$key]))
		{
/*			if( is_int( key($o_obj[$key])) && is_int( key($val)))
			{
				foreach( $val as $v )
					array_push( $o_obj[$key], $v);
				continue;
			}*/
			if( is_string( key($o_obj[$key])) && is_string( key($val)))
			{
				mergeObjs( $o_obj[$key], $val);
				continue;
			}
		}
		$o_obj[$key] = $val;
	}
}
function pushArray( &$o_obj, $i_edit)
{
//error_log('pushArray: '.json_encode($i_edit));
	if( is_null( $i_edit) || is_null( $o_obj)) return;
	if( false == is_array( $o_obj)) return;
	if( array_key_exists('id', $o_obj) && ( $o_obj['id'] == $i_edit['id']))
		if( array_key_exists( $i_edit['pusharray'], $o_obj) && is_array( $o_obj[$i_edit['pusharray']]))
		{
			foreach( $i_edit['objects'] as $obj )
				array_push( $o_obj[$i_edit['pusharray']], $obj);
			return;
		}
	foreach( $o_obj as &$obj ) pushArray( $obj, $i_edit);
}
function delObject( &$o_obj, $i_obj)
{
//error_log('obj:'.json_encode($o_obj));
//error_log('delobj:'.json_encode($i_obj));
	if( is_null( $i_obj) || is_null( $o_obj)) return;
	foreach( $o_obj as $name => $obj )
	{
		if( is_array( $obj) && count( $obj))
		{
//error_log('ckecking:'.json_encode($o_obj[$name]));
			$allkeysequal = true;
			foreach( $i_obj as $key => $val )
			{
				if( array_key_exists( $key, $obj))
					if( $obj[$key] == $val )
						continue;

				$allkeysequal = false;
				break;
			}
			if( $allkeysequal )
			{
//error_log('unsetting:'.json_encode($o_obj[$name]));
				if( is_int( $name))
					array_splice( $o_obj, $name, 1);
				else
					unset( $o_obj[$name]);
//error_log('unset:'.json_encode($o_obj));
			}
			else
				delObject( $o_obj[$name], $i_obj);
		}
	}
}
function replaceObject( &$o_obj, $i_obj)
{
	if( false == is_array( $o_obj))
		return;

	foreach( $o_obj as &$obj)
		replaceObject( $obj, $i_obj);

	if( array_key_exists('id', $o_obj) && ( $o_obj['id'] == $i_obj['id'] ))
		foreach( $i_obj as $key => $val )
			if( $key != 'id' )
				$o_obj[$key] = $val;

//static $replaceObjectCount = 0;
//error_log('replace:'.$replaceObjectCount.' '.json_encode( $o_obj));
//$replaceObjectCount++;
//if( $replaceObjectCount > 5 ) return;

//	if( array_key_exists( $i_attr, $o_obj))
//		$o_obj[$i_attr] = $i_obj;
}
function editObj( $i_edit, &$o_out)
{
	global $RuleMaxLength;

	$mode = 'w+';
	if( file_exists( $i_edit['file'])) $mode = 'r+';
	if( false == is_dir( dirname($i_edit['file'])))
		mkdir( dirname($i_edit['file']));
	if( $fHandle = fopen( $i_edit['file'], $mode))
	{
		$data = fread( $fHandle, $RuleMaxLength);
		$obj = json_decode( $data, true);
		if( is_null( $obj)) $obj = array();
		if( array_key_exists('add', $i_edit) && ( $i_edit['add'] == true ))
			mergeObjs( $obj, $i_edit['object']);
		else if( array_key_exists('pusharray', $i_edit))
			pushArray( $obj, $i_edit);
		else if( array_key_exists('replace', $i_edit) && ( $i_edit['replace'] == true ))
			foreach( $i_edit['objects'] as $newobj )
				replaceObject( $obj, $newobj);
		else if( array_key_exists('delobj', $i_edit) && ( $i_edit['delobj'] == true ))
			foreach( $i_edit['objects'] as $delobj )
				delObject( $obj, $delobj);
//error_log('obj:'.json_encode($obj));
		rewind( $fHandle);
		ftruncate( $fHandle, 0);
		fwrite( $fHandle, json_encode( $obj));
		fclose($fHandle);
		$o_out['status'] = 'success';
	}
	else
	{
		$o_out['status'] = 'error';
		$o_out['error'] = 'Can`t write to '.$i_edit['file'];
	}
}

function cmdExec( $i_obj, &$o_out)
{
	$o_out['cmdexec'] = array();
	foreach( $i_obj['cmds'] as $cmd)
		array_push( $o_out['cmdexec'], shell_exec($cmd));
}

function afanasy( $i_obj, &$o_out)
{
	$socket = fsockopen( $i_obj['address'], $i_obj['port'], $errno, $errstr);
	if( !$socket)
	{
		$o_out['satus'] = 'error';
		$o_out['error'] = $errstr;
		return;
	}

	$obj = array();
	$obj['job'] = $i_obj['job'];
	$data = json_encode( $obj);

	$header = '[ * AFANASY * ]';
	$header = $header.' '.$i_obj['magick_number'];
	$header = $header.' '.$i_obj['sender_id'];
	$header = $header.' '.strlen($data);
	$header = $header.' JSON';

	fwrite( $socket, $header.$data);
	fclose( $socket);

	$o_out['satus'] = 'success';
//	$o_out['header'] = $header;
}

function save( $i_recv, &$o_out)
{
	$filename = $i_recv['save'];
	$dirname = dirname($filename);

	$o_out['file'] = $filename;

	if( false == is_dir( $dirname))
	{
		mkdir( $dirname, 0777, true);
		if( false == is_dir( $dirname))
		{
			$o_out['error'] = 'Unable to create directory '.$dirname;
			return;
		}		
	}

	if( $fHandle = fopen( $filename, 'wb'))
	{
		fwrite( $fHandle, base64_decode( $i_recv['data']));
		fclose( $fHandle );
		return;
	}

	$o_out['error'] = 'Unable to open file for writing '.$filename;
}

$recv = json_decode( $HTTP_RAW_POST_DATA, true);
$out = array();
umask(0000);
if( array_key_exists('walkdir', $recv))
{
	$out['walkdir'] = array();
	foreach( $recv['walkdir'] as $dir)
	{
		$dir = str_replace('../','', $dir);
		$dir = str_replace('/..','', $dir);
		$dir = str_replace('..','', $dir);
		$walkdir = array();
		walkDir( $recv, $dir, $walkdir, 0);
		array_push( $out['walkdir'], $walkdir);
	}
}
else if( array_key_exists('readobj', $recv))
{
	readObj( $recv['readobj'], $out); 
}
else if( array_key_exists('cmdexec', $recv))
{
	cmdExec( $recv['cmdexec'], $out); 
}
else if( array_key_exists('editobj', $recv))
{
	editObj( $recv['editobj'], $out); 
}
else if( array_key_exists('readconfig', $recv))
{
	$configs = array();
	readConfig( $recv['readconfig'], $configs); 
	$out['config'] = $configs;
	processUser( $out);
}
else if( array_key_exists('news', $recv))
{
	makeNews( $recv['news'], $out);
}
else if( array_key_exists('save', $recv))
{
	$save = array();
	save( $recv, $save);
	$out['save'] = $save;
}
else if( array_key_exists('afanasy', $recv))
{
	afanasy( $recv, $out);
}

echo json_encode( $out);

function processUser( &$o_out)
{
	global $UserName;

	if( $UserName == null ) return;

//	$o_out['user_name'] = $UserName;

	$filename = 'users/'.$UserName.'.json';
	$user = array();

	$editobj = array();
	$editobj['add'] = true;
	$editobj['file'] = $filename;

	if( false == is_file( $filename))
	{
		$user['id'] = $UserName;
		$user['subscribe'] = array();
		$user['events'] = array();
		$user['ctime'] = time();

		$editobj['object'] = $user;
		$out = array();
		editObj( $editobj, $out);
	}

	readObj( $filename, &$user);

	if( array_key_exists('error', $user))
	{
		error_log( $user['error']);
		return;
	}

	$user['rtime'] = time();
	$editobj['object'] = $user;
	$out = array();
	editObj( $editobj, $out);

	$o_out['user'] = $user;
}

function makeNews( $i_news, &$o_out)
{
	global $UserName, $RuleMaxLength;

	if( $UserName == null )
	{
		$o_out['error'] = 'No authentificated user founded.';
		return;
	}

	$users_all = array();	
	if( $fHandle = opendir('users'))
	{
		while (false !== ( $uEntry = readdir( $fHandle)))
		{			
			if( strrpos( $uEntry,'.json') === false ) continue;
			if( $uHandle = fopen( 'users/'.$uEntry, 'r'))
			{
				$udata = fread( $uHandle, $RuleMaxLength);
				fclose( $uHandle);
				$uobj = json_decode( $udata, true);
				if( is_null( $uobj)) continue;
				array_push( $users_all, $uobj);
			}
		}
		closedir($fHandle);
	}

	if( count( $users_all) == 0 )
	{
		$o_out['error'] = 'No users founded.';
		return;
	}

	$users = array();
	foreach( $users_all as $user )
	{
		foreach( $uses['subscribe'] as $subscribe )
		{
			if( $i_news['path'] == $subscribe['id'] )
			{
				array_push( $users_sbs, $user);
				break;
			}
		}
	}

	$o_out['users'] = array();
	foreach( $users as &$user )
	{
		array_push( $o_out['users'], $user['id']);
	}
}

?>

