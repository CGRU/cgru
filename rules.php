<?php

$HT_AccessFileName = '.htaccess';
$HT_GroupsFileName = '.htgroups';

$RuleMaxLength = 100000;
$UserName = null;

if( isset($_SERVER['PHP_AUTH_USER']))
{
	global $UserName;

	$UserName = $_SERVER['PHP_AUTH_USER'];
}

function htaccessFolder( $i_folder)
{
	global $RuleMaxLength, $HT_AccessFileName, $UserName;

	if( $UserName == null ) return true;

	$htaccess = $i_folder.'/'.$HT_AccessFileName;
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
//		sort( $o_out['folders']);
//		sort( $o_out['files']);
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

function jsf_readobj( $i_file, &$o_out)
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
function jsf_editobj( $i_edit, &$o_out)
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

function jsf_cmdexec( $i_obj, &$o_out)
{
	$o_out['cmdexec'] = array();
	foreach( $i_obj['cmds'] as $cmd)
	{
		$rem = array('../','../','..','&','|','>','<');
		$cmd = str_replace( $rem, '', $cmd);
		array_push( $o_out['cmdexec'], shell_exec("./$cmd"));
	}
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

function jsf_save( $i_save, &$o_out)
{
	$filename = $i_save['file'];
	$dirname = dirname($filename);

	$o_out['save'] = $filename;

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
		fwrite( $fHandle, base64_decode( $i_save['data']));
		fclose( $fHandle );
		return;
	}

	$o_out['error'] = 'Unable to open file for writing '.$filename;
}

$recv = json_decode( $HTTP_RAW_POST_DATA, true);
if( is_null($recv))
	$recv = json_decode( base64_decode( $HTTP_RAW_POST_DATA), true);

$out = array();
umask(0000);
if( array_key_exists('walkdir', $recv))
{
	$out['walkdir'] = array();
	foreach( $recv['walkdir'] as $dir)
	{
		$rem = array('../','../','..');
		$dir = str_replace( $rem, '', $dir);
		$walkdir = array();
		walkDir( $recv, $dir, $walkdir, 0);
		array_push( $out['walkdir'], $walkdir);
	}
}
else if( array_key_exists('afanasy', $recv))
{
	afanasy( $recv, $out);
}
else if( count( $recv))
{
	foreach( $recv as $key => $args )
	{
		$func = "jsf_$key";
		if( function_exists($func))
			$func( $args, $out);
		else
			$out['error'] = 'Function "'.$key.'" does not exist.';
	}
}

echo json_encode( $out);

function jsf_initialize( $i_arg, &$o_out)
{
	global $RuleMaxLength;

	$configs = array();
	readConfig('config_default.json', $configs); 
	$o_out['config'] = $configs;

	processUser( $o_out);
	if( array_key_exists('error', $o_out))
	{
		$o_out['error'] = $out['error'];
		return;
	}

	$out = array();
	jsf_getallusers( null, $out);
	if( array_key_exists('error', $out))
	{
		$o_out['error'] = $out['error'];
		return;
	}

	$o_out['users'] = array();
	foreach( $out['users'] as $obj)
	{
		$user = array();
		$user['id'] = $obj['id'];
		if( isset( $obj['role'])) $user['role'] = $obj['role'];
		if( isset( $obj['title'])) $user['title'] = $obj['title'];
		$o_out['users'][$obj['id']] = $user;
	}

	if( $fHandle = fopen('version.txt','r'))
	{
		$o_out['version'] = fread( $fHandle, $RuleMaxLength);
		fclose($fHandle);
	}
}

function processUser( &$o_out)
{
	global $UserName;

	if( $UserName == null ) return;

	$dirname = 'users';
	$filename = $dirname.'/'.$UserName.'.json';
	$user = array();

	$editobj = array();
	$editobj['add'] = true;
	$editobj['file'] = $filename;

	if( false == is_file( $filename))
	{
		$user['id'] = $UserName;
		$user['channels'] = array();
		$user['news'] = array();
		$user['ctime'] = time();

		if( false == is_dir( $dirname ))
			$user['role'] = 'admin';
		else
			$user['role'] = 'user';

		$editobj['object'] = $user;
		$out = array();
		jsf_editobj( $editobj, $out);
	}

	jsf_readobj( $filename, &$user);

	if( array_key_exists('error', $user))
	{
		$o_out['error'] = $user['error'];
		return;
	}

	$user['rtime'] = time();
//	$user['title'] = $i_user['title'];
	$editobj['object'] = $user;
	$out = array();
	jsf_editobj( $editobj, $out);
	if( array_key_exists('error', $out))
	{
		$o_out['error'] = $out['error'];
		return;
	}

	$o_out['user'] = $user;
}

function jsf_makenews( $i_news, &$o_out)
{
	global $UserName, $RuleMaxLength;

	if( $UserName == null )
	{
		$o_out['error'] = 'No authentificated user founded.';
		return;
	}

	$users = array();	
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
				array_push( $users, $uobj);
			}
		}
		closedir($fHandle);
	}

	if( count( $users) == 0 )
	{
		$o_out['error'] = 'No users founded.';
		return;
	}

	$o_out['users'] = array();

	foreach( $users as &$user )
	{
		foreach( $user['channels'] as $channel )
		{
			if( strpos( $i_news['path'], $channel['id'] ) === 0 )
			{
				array_push( $o_out['users'], $user['id']);

				$i_news['user'] = $UserName;

				$has_event = false;
				foreach( $user['news'] as &$event )
				{
					if( $event['path'] == $i_news['path'])
					{
						$has_event = true;
						$event = $i_news;
						break;
					}
				}

				if( false == $has_event )
					array_push( $user['news'], $i_news);

				$filename = 'users/'.$user['id'].'.json';
				if( $fHandle = fopen( $filename, 'w'))
				{
					fwrite( $fHandle, json_encode( $user));
					fclose($fHandle);
				}
//error_log('New wrote in '.$filename);

				break;
			}
		}
	}
}

function isAdmin( &$o_out)
{
	global $RuleMaxLength, $UserName;
	if( is_null( $UserName )) return false;
	$user = array();
	jsf_readobj( "users/$UserName.json", &$user);
	if( array_key_exists( 'role', $user))
		if( $user['role'] == 'admin')
			return true;

	$o_out['error'] = 'Access denied.';
	return false;
}

function jsf_htdigest( $i_recv, &$o_out)
{
	global $RuleMaxLength, $UserName;

	# Only admin can change or set password
	if( false == isAdmin( $o_out))
		return;

	$htdigest_file = '.htdigest';

	$user = $i_recv['user'];

	# Construct md5 hash
	$p = $i_recv['p'];
	$hash = md5("$user:RULES:$p");
	$new_line = "$user:RULES:$hash";

//error_log($new_line);

	$data = '';
	if( $fHandle = fopen( $htdigest_file, 'r'))
	{
		$data = fread( $fHandle, $RuleMaxLength);
		fclose($fHandle);
	}
	$old_lines = explode("\n", $data);
	$new_lines = array();
	foreach( $old_lines as $line )
	{
		$values = explode(':', $line);
		if( count( $values ) == 3 )
		{
			if( $values[0] == $user )
				$o_out['status'] = 'User "'.$user.'" updated.';
			else
			{
				$o_out['status'] = 'User "'.$user.'" set.';
				array_push( $new_lines, $line);
			}
		}
	}

	array_push( $new_lines, $new_line);

	$data = implode("\n", $new_lines)."\n";

	if( $fHandle = fopen( $htdigest_file, 'w' ))
	{
		fwrite( $fHandle, $data );
		fclose( $fHandle );
	}
	else
	{
		$o_out['error'] = 'Unable to write into the file.';
	}
//error_log($data);
}

function jsf_deleteuser( $i_user_id, &$o_out)
{
	global $RuleMaxLength;

	if( false == isAdmin( $o_out))
		return;

	$dHandle = opendir('users');
	if( $dHandle === false )
	{
		$o_out['error'] = 'Can`t open users folder.';
		return;
	}

	while (false !== ( $entry = readdir( $dHandle)))
	{
		if( false === is_file("users/$entry")) continue;
		if( $entry != "$i_user_id.json" ) continue;
		unlink("users/$entry");
		break;
	}
	closedir($dHandle);

	$htdigest_file = '.htdigest';
	$data = '';
	if( $fHandle = fopen( $htdigest_file, 'r'))
	{
		$data = fread( $fHandle, $RuleMaxLength);
		fclose($fHandle);
	}
	else
	{
		$o_out['error'] = 'Unable to read the file.';
		return;
	}
	$old_lines = explode("\n", $data);
	$new_lines = array();
	foreach( $old_lines as $line )
	{
		$values = explode(':', $line);
		if( count( $values ) == 3 )
		{
			if( $values[0] != $i_user_id )
				array_push( $new_lines, $line);
		}
	}
	$data = implode("\n", $new_lines)."\n";

	if( $fHandle = fopen( $htdigest_file, 'w' ))
	{
		fwrite( $fHandle, $data );
		fclose( $fHandle );
	}
	else $o_out['error'] = 'Unable to write into the file.';
}

function jsf_getallusers( $i_args, &$o_out)
{
	global $RuleMaxLength;

	$dHandle = opendir('users');
	if( $dHandle === false )
	{
		$o_out['error'] = 'Can`t open users folder.';
		return;
	}

	$o_out['users'] = array();

	while (false !== ( $entry = readdir( $dHandle)))
	{
		if( false === is_file("users/$entry")) continue;
		if( strrpos( $entry,'.json') === false ) continue;

		if( $fHandle = fopen( "users/$entry", 'r'))
		{
			$user = json_decode( fread( $fHandle, $RuleMaxLength), true);
			$o_out['users'][$user['id']] = $user;
			fclose($fHandle);
		}
	}
	closedir($dHandle);
}

function jsf_getallgroups( $i_args, &$o_out)
{
	global $RuleMaxLength, $HT_GroupsFileName;
	if( false == is_file( $HT_GroupsFileName))
	{
		$o_out['error'] = 'Groups file does not exist.';
		return;
	}

	if( $fHandle = fopen( $HT_GroupsFileName, 'r'))
	{
		$o_out['groups'] = array();
		$data = fread( $fHandle, $RuleMaxLength);
		$lines = explode("\n", $data);
		foreach( $lines as $line )
		{
			if( strlen($line) < 3 ) continue;
			$fields = explode(':', $line);
			if( count( $fields) == 0 ) continue;
			if( strlen( $fields[0]) < 1 ) continue;
			$o_out['groups'][$fields[0]] = array();
			if( count( $fields) < 2 ) continue;
			$users = explode(' ', $fields[1]);
			foreach( $users as $user)
				if( strlen( $user) > 0 )
					array_push( $o_out['groups'][$fields[0]], $user);
		}
		fclose($fHandle);
	}
	else
	{
		$o_out['error'] = 'Unable to open groups file.';
		return;
	}
}

function jsf_creategroup( $i_args, &$o_out)
{
	jsf_getallgroups( null, $o_out);
	if( isset( $o_out['error'])) return;

	$new_group = $i_args;
	$groups = $o_out['groups'];
	if( array_key_exists( $new_group, $groups))
	{
		$o_out['error'] = "Group $new_group already exists.";
		return;
	}
	$groups[$new_group] = array();
	writeGroups( $groups, $o_out);
}

function jsf_deletegroup( $i_args, &$o_out)
{
	jsf_getallgroups( null, $o_out);
	if( isset( $o_out['error'])) return;

	$del_group = $i_args;
	$groups = $o_out['groups'];
	if( false == array_key_exists( $del_group, $groups))
	{
		$o_out['error'] = "Group $del_group does not exist.";
		return;
	}
	unset( $groups[$del_group]);
	writeGroups( $groups, $o_out);
}

function writeGroups( $i_groups, &$o_out)
{
	global $HT_GroupsFileName;
	if( false == is_file( $HT_GroupsFileName))
	{
		$o_out['error'] = 'Groups file does not exist.';
		return;
	}

	$data = '';
	foreach( $i_groups as $group => $users )
		$data = $data."$group:".implode(' ',$users)."\n";

	if( $fHandle = fopen( $HT_GroupsFileName, 'w'))
	{
		fwrite( $fHandle, $data );
		fclose( $fHandle);
	}
	else
	{
		$o_out['error'] = 'Unable to write in groups file.';
		return;
	}
}

function jsf_usersadd( $i_args, &$o_out)
{
	global $RuleMaxLength;
	if( false == isAdmin( $o_out))
		return;

	$htaccess = $i_args['path'].'/.htaccess';

	$htaccess_read = $htaccess;
	if( false === is_file( $htaccess_read ))
	$htaccess_read = '.htaccess';
	if( false === is_file( $htaccess_read ))
	{
		$o_out['error'] = 'Can`t find access file.';
		return;
	}

	$fHandle = fopen( $htaccess_read, 'r');
	if( $fHandle === false )
	{
		$o_out['error'] = 'Can`t read access file.';
		return;
	}
	$data = fread( $fHandle, $RuleMaxLength);
	fclose( $fHandle);

	$old_lines = explode("\n", $data);
	$new_lines = array();
	$users = array();
	$founded = false;
	foreach( $old_lines as $line )
	{
		if( strlen($line) <= 1 ) continue;
		$users = explode(' ', $line);

		if( $users[0] == 'Require' )
		{
			if( $users[1] == 'valid-user')
				$users = array('Require','user');
			foreach( $users as $user)
				if( $user == $i_args['id'])
				{
					$o_out['error'] = 'User "'.$i_args['id'].'" aready exists.';
					return;
				}
			$founded = true;
			break;
		}
		array_push( $new_lines, $line);
	}
	if( false == $founded )
	{
		$o_out['error'] = 'Unable to find users in the file.';
		return;
	}

	array_push( $users, $i_args['id']);
	array_push( $new_lines, implode(' ', $users));

	$data = implode("\n", $new_lines)."\n";
//error_log($data);

	if( $fHandle = fopen( $htaccess, 'w' ))
	{
		fwrite( $fHandle, $data );
		fclose( $fHandle );
	}
	else
		$o_out['error'] = 'Unable to write into the file.';
}

function jsf_usersdel( $i_args, &$o_out)
{
	global $RuleMaxLength;
	if( false == isAdmin( $o_out))
		return;

	$htaccess = $i_args['path'].'/.htaccess';

	if( false === is_file( $htaccess ))
	{
		$o_out['error'] = 'Can`t find access file.';
		return;
	}

	$fHandle = fopen( $htaccess, 'r');
	if( $fHandle === false )
	{
		$o_out['error'] = 'Can`t read access file.';
		return;
	}
	$data = fread( $fHandle, $RuleMaxLength);
	fclose( $fHandle);

	$old_lines = explode("\n", $data);
	$new_lines = array();
	$users = array();
	$founded = false;
	foreach( $old_lines as $line )
	{
		if( strlen($line) <= 1 ) continue;
		$words = explode(' ', $line);

		if(( $words[0] == 'Require') && ( $words[1] == 'user'))
		{
			foreach( $words as $user)
				if( $user == $i_args['id'])
					$founded = true;
				else
					array_push( $users, $user);
			continue;
		}
		array_push( $new_lines, $line);
	}
	if( false == $founded )
	{
		$o_out['error'] = 'Unable to find user in the file.';
		return;
	}

	array_push( $new_lines, implode(' ', $users));

	$data = implode("\n", $new_lines)."\n";
//error_log($data);return;

	if( $fHandle = fopen( $htaccess, 'w' ))
	{
		fwrite( $fHandle, $data );
		fclose( $fHandle );
	}
	else
		$o_out['error'] = 'Unable to write into the file.';
}

function jsf_usersclear( $i_args, &$o_out)
{
	if( false == isAdmin( $o_out))
		return;

	$htaccess = $i_args['path'].'/.htaccess';
	if( false === is_file( $htaccess))
	{
		$o_out['error'] = 'Can`t find the file.';
		return;
	}

	unlink( $htaccess);

	if( is_file( $htaccess))
	{
		$o_out['error'] = 'Can`t remove the file.';
		return;
	}
}

function jsf_usersget( $i_args, &$o_out)
{
	global $RuleMaxLength;
	if( false == isAdmin( $o_out))
		return;

	$o_out['users'] = array();
	$htaccess = $i_args['path'].'/.htaccess';
	if( false === is_file( $htaccess)) return;
	$fHandle = fopen( $htaccess, 'r');
	if( $fHandle === false )
	{
		$o_out['error'] = 'Can`t open the file.';
		return;
	}
	$data = fread( $fHandle, $RuleMaxLength);
	fclose( $fHandle);

	$lines = explode("\n", $data);
	$users = array();
	foreach( $lines as $line )
	{
		if( strlen($line) <= 1 ) continue;
		$words = explode(' ', $line);
//error_log( implode(' ',$words));
		if( $words[0] == 'Require' )
		{
			if( $words[1] == 'user' )
			{
				unset($words[0]);
				unset($words[1]);
				foreach( $words as $user) array_push( $users, $user);
			}
			$founded = true;
			break;
		}
	}
	if( false == $founded )
	{
		$o_out['error'] = 'Unable to find users in the file.';
		return;
	}

	$o_out['users'] = $users;
}

function jsf_search( $i_args, &$o_out)
{
	if( false == array_key_exists('path', $i_args))
	{
		$o_out['error'] = 'Search path is not specified.';
		return;
	}

	if( false == array_key_exists('depth', $i_args))
		$i_args['depth'] = 1;

	$path = $i_args['path'];
	$rem = array('../','../','..');
	$path = str_replace( $rem, '', $path);
	if( false == is_dir( $path))
	{
		$o_out['error'] = "Search path '$path' does not exist.";
		return;
	}

	$o_out['search'] = $i_args;
	$o_out['result'] = array();

	searchFolder( $i_args, $o_out, $path, 0);
}

function searchFolder( &$i_args, &$o_out, $i_path, $i_depth)
{
	global $RuleMaxLength;

	$i_depth++;
	if( $i_depth > $i_args['depth'] ) return;

	if( false == is_dir( $i_path)) return;
	$dHandle = opendir($i_path);
	if( $dHandle === false ) return;

	while (false !== ( $entry = readdir( $dHandle)))
	{
		if( $entry == '.') continue;
		if( $entry == '..') continue;

		$path = "$i_path/$entry";
		if( false == is_dir( $path)) continue;

		$rufolder = "$path/".$i_args['rufolder'];
		if( false == is_dir( $rufolder)) continue;

		$founded = true;

		if( $founded && array_key_exists('status', $i_args ))
		{
			$founded = false;
			$rufile = "$rufolder/status.json";
			if( is_file( $rufile))
			{
				if( $fHandle = fopen( $rufile, 'r'))
				{
					$obj = json_decode( fread( $fHandle, $RuleMaxLength), true);
						if( searchStatus( $i_args['status'], $obj))
							$founded = true;
					fclose($fHandle);
				}
			}
		}

		if( $founded && array_key_exists('comment', $i_args ))
		{
			$founded = false;
			$rufile = "$rufolder/comments.json";
			if( is_file( $rufile))
			{
				if( $fHandle = fopen( $rufile, 'r'))
				{
					$obj = json_decode( fread( $fHandle, $RuleMaxLength), true);
						if( searchComment( $i_args['comment'], $obj))
							$founded = true;
					fclose($fHandle);
				}
			}
		}

		if( $founded )
			array_push( $o_out['result'], $path);

		if( $i_depth < $i_args['depth'] )
			searchFolder( $i_args, $o_out, $path, $i_depth);
	}

	closedir($dHandle);
}

function searchStatus( &$i_args, &$i_obj)
{
	if( false == is_array( $i_obj)) return false;
	if( false == array_key_exists('status', $i_obj)) return false;

	$founded = true;

	if( $founded && array_key_exists('ann', $i_args))
	{
		$founded = false;
		if( array_key_exists('annotation', $i_obj['status']))
			if( mb_stripos( $i_obj['status']['annotation'], $i_args['ann'], 0, 'utf-8') !== false )
				$founded = true;
		if( array_key_exists('description', $i_obj['status']))
			if( mb_stripos( $i_obj['status']['description'], $i_args['ann'], 0, 'utf-8') !== false )
				$founded = true;
	}

	if( $founded && array_key_exists('artists', $i_args))
	{
		$founded = false;
		if( array_key_exists('artists', $i_obj['status']))
			foreach( $i_args['artists'] as $artist )
				if( in_array( $artist, $i_obj['status']['artists']))
					$founded = true;
	}

	if( $founded && array_key_exists('tags', $i_args))
	{
		$founded = false;
		if( array_key_exists('tags', $i_obj['status']))
			foreach( $i_args['tags'] as $tag )
				if( in_array( $tag, $i_obj['status']['tags']))
					$founded = true;
	}

	if( $founded && array_key_exists('percent', $i_args))
	{
		$founded = false;
		if( array_key_exists('progress', $i_obj['status']))
			if( ($i_obj['status']['progress'] >= $i_args['percent'][0]) &&
				($i_obj['status']['progress'] <= $i_args['percent'][1]) )
					$founded = true;
	}

	if( $founded && array_key_exists('finish', $i_args))
	{
		$founded = false;
		if( array_key_exists('finish', $i_obj['status']))
		{
			$days = ($i_obj['status']['finish'] - time()) / ( 60 * 60 * 24 );
			if( ($days >= $i_args['finish'][0]) &&
				($days <= $i_args['finish'][1]) )
					$founded = true;
		}
	}

	return $founded;
}

function searchComment( &$i_args, &$i_obj)
{
	if( false == is_array( $i_obj)) return false;
	if( false == array_key_exists('comments', $i_obj)) return false;
	foreach( $i_obj['comments'] as &$comment )
		if( array_key_exists('text', $comment))
			if( mb_stripos( $comment['text'], $i_args, 0, 'utf-8') !== false )
				return true;
	return false;
}

?>

