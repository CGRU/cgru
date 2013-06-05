<?php

$GuestSites = array('cgru.info','127.0.0.1');
$CONF = array();
$CONF['AUTH_RULES'] = false;
if( false !== array_search( $_SERVER['SERVER_NAME'], $GuestSites))
{
	$CONF['AUTH_RULES'] = true;
}

umask(0000);

$HT_AccessFileName = '.htaccess';
$HT_GroupsFileName = '.htgroups';
$HT_DigestFileName = '.htdigest';

$FileMaxLength = 100000;

$Digest = false;
$UserID = null;
$Groups = null;

$SkipFiles = array( '.', '..', $HT_AccessFileName, $HT_GroupsFileName, $HT_DigestFileName);
$GuestCanCreate = array('status.json','comments.json');
$GuestCanEdit = array('comments.json');

$Out = array();
$Recv = array();

# Decode input:
if( isset($_POST['upload_path']))
	upload( $_POST['upload_path'], $Out);
else
{
	$Recv = json_decode( $HTTP_RAW_POST_DATA, true);
	if( is_null( $Recv ))
	{
		$Recv = json_decode( base64_decode( $HTTP_RAW_POST_DATA), true);
	}
	if( is_null( $Recv ))
	{
		$Recv = array();
		$Out['error'] = 'Can`t decode request.';
	}
}

# Authenticate:
if( $CONF['AUTH_RULES'] )
{
	if( isset( $Recv['digest']))
	{
		$Digest = $Recv['digest'];
		if( false == http_digest_validate( $Out))
		{
			$Digest = false;
			$Out['auth_status'] = 'Wrong credentials.';
			$Out['auth_error'] = true;
		}
		$Out['nonce'] = md5(rand());
	}
}	
else
{
	$Digest = http_digest_parse();
}
if( $Digest !== false )
{
	global $UserID;
	if( $Digest['username'] != 'null' )
		$UserID = $Digest['username'];
}

# Process response:
if( array_key_exists('walkdir', $Recv))
{
	$Out['walkdir'] = array();
	foreach( $Recv['walkdir'] as $dir)
	{
		$rem = array('../','../','..');
		$dir = str_replace( $rem, '', $dir);
		$walkdir = array();
		walkDir( $Recv, $dir, $walkdir, 0);
		array_push( $Out['walkdir'], $walkdir);
	}
}
else if( array_key_exists('afanasy', $Recv))
{
	afanasy( $Recv, $Out);
}
else if( count( $Recv))
{
	foreach( $Recv as $key => $args )
	{
		$func = "jsf_$key";
		if( function_exists($func))
			$func( $args, $Out);
//		else
//			$Out['error'] = 'Function "'.$key.'" does not exist.';
	}
}

# Write response:
if( false == is_null( $Out))
	echo json_encode( $Out);

# Functions:
function jsf_start( $i_arg, &$o_out)
{
	global $CONF, $FileMaxLength;
	$o_out['upload_max_filesize'] = ini_get('upload_max_filesize');
	$o_out['post_max_size'] = ini_get('post_max_size');
	$o_out['memory_limit'] = ini_get('memory_limit');
	if( $fHandle = fopen('version.txt','r'))
	{
		$o_out['version'] = fread( $fHandle, $FileMaxLength);
		fclose($fHandle);
	}
	$o_out['name'] = $_SERVER['SERVER_NAME'];
	$o_out['software'] = $_SERVER['SERVER_SOFTWARE'];
	$o_out['remote_address'] = $_SERVER['REMOTE_ADDR'];
	$o_out['php_version'] = phpversion();
	foreach( $CONF as $key => $val ) $o_out[$key] = $val;
	if( $CONF['AUTH_RULES']) $o_out['nonce'] = md5(rand());
}

function jsf_initialize( $i_arg, &$o_out)
{
	global $UserID, $Groups, $HT_AccessFileName, $HT_GroupsFileName;

	$configs = array();
	readConfig('config_default.json', $configs); 
	$o_out['config'] = $configs;

	if( $UserID != null )
	{
		if(( false == is_file( $HT_AccessFileName)) && ( is_file('htaccess_example')))
		{
			if( copy('htaccess_example', $HT_AccessFileName))
				error_log('HT access file copied.');
			else
				error_log('Unable to copy htaccess file.');
		}
		if( false == is_file( $HT_GroupsFileName))
		{
			$Groups = array();
			$Groups['admins'] = array( $UserID);
			jsf_writegroups( $Groups, $o_out);
			if( array_key_exists('error', $o_out)) return;
			error_log('HT Groups file created with "'.$UserID.'" in "admins".');
		}
	}

	processUser( $o_out);
	if( array_key_exists('error', $o_out)) return;

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
		if( isset( $obj['title'])) $user['title'] = $obj['title'];
		if( isset( $obj['role'])) $user['role'] = $obj['role'];

		if( isset( $obj['avatar']) && strlen($obj['avatar']))
			$user['avatar'] = $obj['avatar'];
		else if( isset( $obj['email']))
			$user['avatar'] = 'http://www.gravatar.com/avatar/'.md5( strtolower( trim($obj['email'])));

		$o_out['users'][$obj['id']] = $user;
	}

	if( isAdmin( $out)) $o_out['admin'] = true;
}

function processUser( &$o_out)
{
	global $UserID;

	$dirname = 'users';
	if( false == is_dir( $dirname ))
		mkdir( $dirname);

	if( $UserID == null ) return;

	$filename = $dirname.'/'.$UserID.'.json';
	$user = array();

	$editobj = array();
	$editobj['add'] = true;
	$editobj['file'] = $filename;

	if( false == is_file( $filename))
	{
		$user['id'] = $UserID;
		$user['channels'] = array();
		$user['news'] = array();
		$user['ctime'] = time();

		$editobj['object'] = $user;
		$out = array();
		jsf_editobj( $editobj, $out);
	}

	readObj( $filename, $user);

	if( array_key_exists('error', $user))
	{
		$o_out['error'] = $user['error'];
		return;
	}

	$user['rtime'] = time();
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

function http_digest_parse()
{
	if( false == isset( $_SERVER['PHP_AUTH_DIGEST']))
		return false;

	// protect against missing data
	$needed_parts = array('nonce'=>1, 'nc'=>1, 'cnonce'=>1, 'qop'=>1, 'username'=>1, 'uri'=>1, 'response'=>1);
	$data = array();
	$keys = implode('|', array_keys($needed_parts));

	preg_match_all('@(' . $keys . ')=(?:([\'"])([^\2]+?)\2|([^\s,]+))@', $_SERVER['PHP_AUTH_DIGEST'], $matches, PREG_SET_ORDER);

	foreach ($matches as $m) {
		$data[$m[1]] = $m[3] ? $m[3] : $m[4];
		unset($needed_parts[$m[1]]);
	}

	return $needed_parts ? false : $data;
}

function http_digest_validate( &$o_out)
{
	global $Digest, $HT_DigestFileName, $FileMaxLength;

	if( $Digest === false ) return false;

	if( false == is_file( $HT_DigestFileName))
	{
		$o_out['error'] = 'HT digest file does not exist.';
		error_log( $o_out['error']);
		return false;
	}

	$data = null;
	if( $fHandle = fopen( $HT_DigestFileName, 'r'))
	{
		$data = fread( $fHandle, $FileMaxLength);
		fclose($fHandle);
	}
	else
	{
		$o_out['error'] = 'Can`t open HT digest file.';
		error_log( $o_out['error']);
		return false;
	}
	
	$data = explode("\n", $data);
	$founded = false;
	foreach( $data as $line )
		if( strpos( $line, $Digest['username']) === 0)
		{
			$data = $line;
			$founded = true;
			break;
		}

	if( false == $founded )
	{
		$o_out['error'] = 'Wrong!';
		return false;
	}

	$data = explode(':', $data);
	if( count($data) != 3 )
	{
		$o_out['error'] = 'Invalid HT digest entry.';
		error_log( $o_out['error']);
		return false;
	}

	$data = $data[2];
	$hget = md5( $_SERVER['REQUEST_METHOD'].':'.$Digest['uri']);
	$valid_response = md5( $data.':'.$Digest['nonce'].':'.$Digest['nc'].':'.$Digest['cnonce'].':'.$Digest['qop'].':'.$hget);
/*
error_log( 'PHP_AUTH_DIGEST = '.$_SERVER['PHP_AUTH_DIGEST']);
error_log( 'REQUEST_METHOD = '.$_SERVER['REQUEST_METHOD']);
error_log( 'Digest = '.json_encode($Digest));
error_log( 'htdigest = '.$data);
error_log( 'hash = '.md5($Digest['username'].':'.$i_arg['realm'].':www'));
error_log( 'response = '.$Digest['response']);
error_log( 'valid_response = '.$valid_response);
*/
	if( $Digest['response'] == $valid_response)
		return true;

	$o_out['error'] = 'Wrong!';
	return false;
}

function jsf_login( $i_arg, &$o_out)
{
	global $CONF, $Digest;

	if( isset( $_SERVER['PHP_AUTH_DIGEST']))
	{
		if( $Digest === false )
		{
			$o_out['error'] = 'Wrong Digest!';
			//$o_out['PHP_AUTH_DIGEST'] = $_SERVER['PHP_AUTH_DIGEST'];
		}
		else if( $Digest['username'] != 'null')
		{
			//$o_out['PHP_AUTH_DIGEST'] = $_SERVER['PHP_AUTH_DIGEST'];
			if( http_digest_validate( $o_out))
				return;
		}
	}
	else if( $CONF['AUTH_RULES'])
	{
		if( false == isset( $i_arg['digest']))
			$o_out['error'] = 'No digest in login object.';
		else
		{
			$Digest = $i_arg['digest'];
			if( http_digest_validate( $o_out))
				$o_out['status'] = 'success';
		}
		return;
	}

	header('HTTP/1.1 401 Unauthorized');
	header('WWW-Authenticate: Digest realm="'.$i_arg['realm'].'",qop="auth",nonce="'.md5(rand()).'"');
	#die('Text to send if user hits Cancel button');
	//$o_out['PHP_AUTH_DIGEST'] = $_SERVER['PHP_AUTH_DIGEST'];
}

function jsf_logout( $i_arg, &$o_out)
{
	header('HTTP/1.1 401 Unauthorized');
}

function skipFile( $file)
{
	global $SkipFiles;
	if( in_array( $file, $SkipFiles))
		return true;
	return false;
}

function htaccessFolder( $i_folder)
{
	global $UserID, $Groups;

	if( $i_folder == '.' ) return true;
//	if( $UserID == null ) return true;

	$out = array();
	readGroups( $out);
	if( array_key_exists('error', $out))
	{
		error_log( $out['error']);
		return false;
	}
	if( is_null( $Groups))
	{
		error_log('Groups are null.');
		return false;
	}

	$args = array();
	$args['path'] = $i_folder;
	permissionsGet( $args, $out);
	if( array_key_exists('error', $out))
	{
		error_log( $out['error']);
		return false;
	}
	if( array_key_exists('valid_user', $out))
	{
		if( $UserID == null )
			return false;
		return true;
	}

	if(( count( $out['users']) == 0 ) && ( count( $out['groups']) == 0 )) return null;

	if( $UserID == null )
		return false;

	foreach( $out['groups'] as $grp )
		if( array_key_exists( $grp, $Groups))
			if( in_array( $UserID, $Groups[$grp]))
				return true;

	if( in_array( $UserID, $out['users'])) return true;

	return false;
}

function htaccessPath( $i_path)
{
//return true;
//error_log('Checking access path "'.$i_path.'"');
	if( is_file( $i_path)) $i_path = dirname( $i_path);
	if( false == is_dir( $i_path))
	{
		error_log('htaccessPath: no such directory: '.$i_path);
		return false;
	}

	$folders = explode('/', $i_path);
	$path = null;
	$paths = array();
	foreach( $folders as $folder)
	{
		if( $path != null )
			$path = $path.'/'.$folder;
		else
			$path = $folder;
		array_push( $paths, $path);
	}
	$paths = array_reverse( $paths);
	foreach( $paths as $path)
	{
//error_log('Checking access folder "'.$path.'"');
		$access = htaccessFolder($path);
		if( $access === false ) return false;
		if( $access === true  ) return true;
	}

	return true;
}

function walkDir( $i_recv, $i_dir, &$o_out, $i_depth)
{
	global $FileMaxLength;

	if( $i_depth > $i_recv['depth'] ) return;

	if( false == is_dir( $i_dir))
	{
		$o_out['error'] = 'No such folder.';
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

	$access = false;
	$denied = true;
	if( htaccessPath( $i_dir))
	{
		$access = true;
		$denied = false;
	}
	else
	{
		$rufiles = array('rules');
		$o_out['denied'] = true;
	}

	if( $handle = opendir( $i_dir))
	{
		$o_out['folders'] = array();
		$o_out['files'] = array();

		while (false !== ( $entry = readdir( $handle)))
		{
			if( skipFile( $entry)) continue;
//			if( $entry == '.') continue;
//			if( $entry == '..') continue;
			$path = $i_dir.'/'.$entry;
			if( $access && ( false == is_dir( $path)))
			{
				if( is_file( $path))
				{
					$fileObj = array();
					$fileObj['name'] = $entry;
					$fileObj['size'] = filesize( $path);
					$fileObj['mtime'] = filemtime( $path);
					array_push( $o_out['files'], $fileObj);
				}
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

						if( $access )
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
							$rudata = fread( $fHandle, $FileMaxLength);
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

			if(( $i_recv['showhidden'] == false ) && is_file("$path/.hidden")) continue;
			if( $denied ) continue;
			if( false === htaccessFolder( $path)) continue;

			$folderObj = array();
			$folderObj['name'] = $entry;
			$folderObj['mtime'] = filemtime( $path);

			if( $rufolder && $lookahead )
				foreach( $lookahead as $sfile )
				{
					$sfilepath = $path.'/'.$rufolder.'/'.$sfile.'.json';
					if( is_file( $sfilepath))
					{
						if( $fHandle = fopen( $sfilepath, 'r'))
						{
							$data = fread( $fHandle, $FileMaxLength);
							fclose( $fHandle);
							mergeObjs( $folderObj, json_decode( $data, true));
						}
					}
				}

			if( $i_depth < $i_recv['depth'] )
				walkDir( $i_recv, $path, $folderObj, $i_depth+1);

			array_push( $o_out['folders'], $folderObj);

		}
		closedir($handle);
	}
}

function readConfig( $i_file, &$o_out)
{
	global $FileMaxLength;

	if( false == is_file( $i_file))
		return;

	if( $fHandle = fopen( $i_file, 'r'))
	{
		$data = fread( $fHandle, $FileMaxLength);
		fclose($fHandle);
		$o_out[$i_file] = json_decode( $data, true);
		if( array_key_exists( 'include', $o_out[$i_file]['cgru_config']))
			foreach( $o_out[$i_file]['cgru_config']['include'] as $file )
				readConfig( $file, $o_out);
	}
}

function jsf_getfile( $i_file, &$o_out)
{
	global $FileMaxLength;

	if( false == is_file( $i_file))
	{
		$o_out['error'] = 'No such file '.$i_file;
		return;
	}
	if( false == htaccessPath( $i_file))
	{
		$o_out['error'] = 'Permissions denied';
		return;
	}

	if( $fHandle = fopen( $i_file, 'r'))
	{
		echo fread( $fHandle, $FileMaxLength);
		fclose($fHandle);
		$o_out = null;
	}
	else
		$o_out['error'] = 'Unable to load file '.$i_file;
}

function jsf_readobj( $i_file, &$o_out)
{
	if( false == is_file( $i_file))
	{
		$o_out['error'] = 'No such file: '.$i_file;
		return;
	}
	if( false == htaccessPath( $i_file))
	{
		$o_out['error'] = 'Permissions denied';
		return;
	}
	readObj( $i_file, $o_out);
}
function readObj( $i_file, &$o_out)
{
	global $FileMaxLength;

	if( false == is_file( $i_file))
	{
		$o_out['error'] = 'No such file '.$i_file;
		return;
	}

	if( $fHandle = fopen( $i_file, 'r'))
	{
		$data = fread( $fHandle, $FileMaxLength);
		fclose($fHandle);
		$o_out = json_decode( $data, true);
	}
	else
		$o_out['error'] = 'Unable to load file '.$i_file;
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
	global $UserID, $FileMaxLength, $GuestCanCreate, $GuestCanEdit;

	if( $UserID == null )
	{
		if( is_file( $i_edit['file']))
		{
			if( false === array_search( basename( $i_edit['file']), $GuestCanEdit))
			{
				$o_out['error'] = 'Guests are not allowed to edit here.';
				return;
			}
		}
		else
		{
			if( false === array_search( basename( $i_edit['file']), $GuestCanCreate))
			{
				$o_out['error'] = 'Guests are not allowed here.';
				return;
			}
		}
	}

	$mode = 'w+';
	if( is_file( $i_edit['file'])) $mode = 'r+';
	if( false == is_dir( dirname($i_edit['file'])))
		mkdir( dirname($i_edit['file']), 0777, true);
	if( $fHandle = fopen( $i_edit['file'], $mode))
	{
		$data = fread( $fHandle, $FileMaxLength);
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
	global $UserID;
	if( $UserID == null )
	{
		$o_out['error'] = 'Guests are not allowed to run commands.';
		return;
	}

	$o_out['cmdexec'] = array();
	foreach( $i_obj['cmds'] as $cmd)
	{
		$rem = array('../','../','..','&','|','>','<');
		$cmd = str_replace( $rem, '', $cmd);
		$out = shell_exec("./$cmd");
		$obj = json_decode( $out, true);
		if( false == is_null( $obj )) $out = $obj;
		array_push( $o_out['cmdexec'], $out);
	}
}

function afanasy( $i_obj, &$o_out)
{
	global $UserID;
	if( $UserID == null )
	{
		$o_out['error'] = 'Guests are not allowed to send jobs.';
		return;
	}

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
	global $UserID;

	$filename = $i_save['file'];
	$dirname = dirname($filename);

	if( $UserID == null )
	{
		if( is_file( $filename) || ( basename( $filename) != 'body.html'))
		{
			$o_out['error'] = 'Guests are not allowed to save files.';
			return;
		}
	}

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

	$fHandle = fopen( $filename, 'wb');
	if( false === $fHandle )
	{
		$o_out['error'] = 'Unable to open file for writing '.$filename;
		return;
	}

	$data = $i_save['data'];
	if( array_key_exists('type', $i_save))
	{
		if( $i_save['type'] == 'base64' ) $data = base64_decode( $data);
	}

	fwrite( $fHandle, $data);
	fclose( $fHandle );
}

function jsf_makenews( $i_news, &$o_out)
{
	global $UserID, $FileMaxLength;
/*
	if( $UserID == null )
	{
		$o_out['error'] = 'Guests are not allowed to make news.';
		return;
	}
*/
	$users = array();	
	if( $fHandle = opendir('users'))
	{
		while (false !== ( $uEntry = readdir( $fHandle)))
		{			
			if( strrpos( $uEntry,'.json') === false ) continue;
			if( $uHandle = fopen( 'users/'.$uEntry, 'r'))
			{
				$udata = fread( $uHandle, $FileMaxLength);
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

	$ignore_own = false;
	if( isset( $i_news['ignore_own']))
	{
		if( $i_news['ignore_own'])
			$ignore_own = true;
		unset( $i_news['ignore_own']);
	}

	$o_out['users'] = array();

	foreach( $users as &$user )
	{
		if( $ignore_own && ( $i_news['user'] == $user['id']))
			continue;

		foreach( $user['channels'] as $channel )
		{
			if( strpos( $i_news['path'], $channel['id'] ) === 0 )
			{
				array_push( $o_out['users'], $user['id']);

				$i_news['user'] = $i_news['user'];

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
	global $Groups, $UserID;
	if( is_null( $UserID ))
	{
		$o_out['error'] = 'Access denied.';
		return false;
	}

	if( is_null( $Groups))
	{
		$out = array();
		readGroups( $out);
		if( array_key_exists('error', $out))
		{
			$o_out['error'] = $out['error'];
			return false;
		}
		if( is_null( $Groups))
		{
			$o_out['error'] = 'Error reading groups.';
			return false;
		}
	}

	if( false == array_key_exists('admins', $Groups))
	{
		$o_out['error'] = 'No admins group founded.';
		return false;
	}

	if( in_array( $UserID, $Groups['admins']))
		return true;

	$o_out['error'] = 'Access denied.';
	return false;
}

function jsf_htdigest( $i_recv, &$o_out)
{
	global $FileMaxLength, $UserID, $HT_DigestFileName;
	if( false == isAdmin( $o_out)) return;

	$user = $i_recv['user'];

	$data = '';
	if( $fHandle = fopen( $HT_DigestFileName, 'r'))
	{
		$data = fread( $fHandle, $FileMaxLength);
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

	array_push( $new_lines, $i_recv['digest']);

	$data = implode("\n", $new_lines)."\n";

	if( $fHandle = fopen( $HT_DigestFileName, 'w' ))
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
	global $FileMaxLength, $HT_DigestFileName;
	if( false == isAdmin( $o_out)) return;

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

	$data = '';
	if( $fHandle = fopen( $HT_DigestFileName, 'r'))
	{
		$data = fread( $fHandle, $FileMaxLength);
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

	if( $fHandle = fopen( $HT_DigestFileName, 'w' ))
	{
		fwrite( $fHandle, $data );
		fclose( $fHandle );
	}
	else $o_out['error'] = 'Unable to write into the file.';
}

function jsf_getallusers( $i_args, &$o_out)
{
	global $FileMaxLength;

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
		if( strrpos( $entry,'.json') !== ( strlen($entry) - 5 )) continue;

		if( $fHandle = fopen( "users/$entry", 'r'))
		{
			$user = json_decode( fread( $fHandle, $FileMaxLength), true);
			if( false == is_null( $user ))
				$o_out['users'][$user['id']] = $user;
			fclose($fHandle);
		}
	}
	closedir($dHandle);
}

function jsf_getallgroups( $i_args, &$o_out)
{
	global $Groups;
	if( false == isAdmin( $o_out)) return;
	$o_out['groups'] = $Groups;
}

function readGroups( &$o_out)
{
	global $Groups, $FileMaxLength, $HT_GroupsFileName;
	if( false == is_null( $Groups)) return;

	if( false == is_file( $HT_GroupsFileName))
	{
		$o_out['error'] = 'HT Groups file does not exist.';
		return;
	}
	$fHandle = fopen( $HT_GroupsFileName, 'r');
	if( $fHandle === false )
	{
		$o_out['error'] = 'Unable to open groups file.';
		return;
	}

	$Groups = array();

	$data = fread( $fHandle, $FileMaxLength);
	$lines = explode("\n", $data);
	foreach( $lines as $line )
	{
		if( strlen($line) < 3 ) continue;
		$fields = explode(':', $line);
		if( count( $fields) == 0 ) continue;
		if( strlen( $fields[0]) < 1 ) continue;
		$Groups[$fields[0]] = array();
		if( count( $fields) < 2 ) continue;
		$users = explode(' ', $fields[1]);
		foreach( $users as $user)
			if( strlen( $user) > 0 )
				array_push( $Groups[$fields[0]], $user);
	}
	fclose($fHandle);
}

function jsf_writegroups( $i_groups, &$o_out)
{
	if( false == isAdmin( $o_out)) return;

	global $Groups, $HT_GroupsFileName;

	$Groups = $i_groups;
/*
	if( false == is_file( $HT_GroupsFileName))
	{
		$o_out['error'] = 'HT Groups file does not exist.';
		return;
	}
*/
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

function jsf_permissionsset( $i_args, &$o_out)
{
	global $FileMaxLength, $HT_AccessFileName;
	if( false == isAdmin( $o_out)) return;

	if( false == array_key_exists('groups', $i_args)) $i_args['groups'] = array();
	if( false == in_array('admins', $i_args['groups'])) array_unshift( $i_args['groups'], 'admins');

	$lines = array();
	array_push( $lines, 'Require group '.implode(' ', $i_args['groups']));
	if( array_key_exists('users', $i_args) && count( $i_args['users']))
	{
		array_push( $lines, 'Require user '.implode(' ', $i_args['users']));
		array_push( $lines, 'Satisfy Any');
	}

	$data = implode("\n", $lines)."\n";
//error_log($data);return;

	$htaccess = $i_args['path'].'/'.$HT_AccessFileName;
	if( $fHandle = fopen( $htaccess, 'w' ))
	{
		fwrite( $fHandle, $data );
		fclose( $fHandle );
	}
	else
		$o_out['error'] = 'Unable to write into the file.';
}

function jsf_permissionsclear( $i_args, &$o_out)
{
	global $HT_AccessFileName;
	if( false == isAdmin( $o_out)) return;

	$htaccess = $i_args['path'].'/'.$HT_AccessFileName;
	if( false === is_file( $htaccess))
	{
		$o_out['error'] = 'Can`t find the file.';
		return;
	}

	unlink( $htaccess);

	if( is_file( $htaccess)) $o_out['error'] = 'Can`t remove the file.';
}

function jsf_permissionsget( $i_args, &$o_out)
{
	if( false == isAdmin( $o_out)) return;
	permissionsGet( $i_args, $o_out);

}
function permissionsGet( $i_args, &$o_out)
{
	global $FileMaxLength, $HT_AccessFileName;

	$o_out['groups'] = array();
	$o_out['users'] = array();

	if( false == is_dir( $i_args['path'] ))
	{
		$o_out['error'] = 'No such directory.';
		return;
	}

	$htaccess = $i_args['path'].'/'.$HT_AccessFileName;
	if( false === is_file( $htaccess)) return;

	$fHandle = fopen( $htaccess, 'r');
	if( $fHandle === false )
	{
		$o_out['error'] = 'Can`t open the file.';
		return;
	}
	$data = fread( $fHandle, $FileMaxLength);
	fclose( $fHandle);

	$founded = false;
	$lines = explode("\n", $data);
	foreach( $lines as $line )
	{
		if( strlen( $line) <= 1 ) continue;
		$words = explode(' ', $line);
		if( $words[0] != 'Require' ) continue;

		unset($words[0]);
//error_log( implode(' ',$words));
		if( $words[1] == 'group' )
		{
			unset($words[1]);
			foreach( $words as $group) array_push( $o_out['groups'], $group);
			$founded = true;
		}
		else if( $words[1] == 'user' )
		{
			unset($words[1]);
			foreach( $words as $user) array_push( $o_out['users'], $user);
			$founded = true;
		}
		else if( $words[1] == 'valid-user' )
		{
			$o_out['valid_user'] = true;
			$founded = true;
		}
	}
/*	if( false == $founded )
	{
		$o_out['error'] = 'Unable to find users or groups in the file.';
		error_log( $htaccess);
		error_log( $data);
		return;
	}*/
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
	global $FileMaxLength;

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

		$founded = false;
		$rufolder = "$path/".$i_args['rufolder'];
		if( is_dir( $rufolder)) $founded = true;
		if( $founded && htaccessPath( $i_path)) $founded = true;

		if( $founded && array_key_exists('status', $i_args ))
		{
			$founded = false;
			$rufile = "$rufolder/status.json";
			if( is_file( $rufile))
			{
				if( $fHandle = fopen( $rufile, 'r'))
				{
					$obj = json_decode( fread( $fHandle, $FileMaxLength), true);
						if( searchStatus( $i_args['status'], $obj))
							$founded = true;
					fclose($fHandle);
				}
			}
		}

		if( $founded && array_key_exists('body', $i_args ))
		{
			$founded = false;
			$rufile = "$rufolder/body.html";
			if( is_file( $rufile))
			{
				if( $fHandle = fopen( $rufile, 'r'))
				{
					$data = fread( $fHandle, $FileMaxLength);
					fclose($fHandle);
					if( mb_stripos( $data, $i_args['body'], 0, 'utf-8') !== false )
						$founded = true;
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
					$obj = json_decode( fread( $fHandle, $FileMaxLength), true);
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

	$parms = array('finish','statmod','bodymod');
	foreach( $parms as $parm )
		if( $founded && array_key_exists( $parm, $i_args))
		{
			$founded = false;
			$val = $parm;
			if( $parm == 'statmod' ) $val = 'mtime';
			else if( $parm == 'bodymod' ) $val = 'body';
//error_log('checking '.$parm.' : '.$val);
			if( array_key_exists( $val, $i_obj['status']))
			{
				if( $val == 'body' ) $val = $i_obj['status']['body']['mtime'];
				else $val = $i_obj['status'][$val];
				$val = ($val - time()) / ( 60 * 60 * 24 );
				if( ($val >= $i_args[$parm][0]) &&
					($val <= $i_args[$parm][1]) )
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

function upload( $i_path, &$o_out)
{
	$o_out['path'] = $i_path;

	if( false == is_dir($i_path))
	{
		if( false == mkdir( $i_path, 0777, true))
		{
			$o_out['error'] = 'Unable to create directory';
		}
	}
	if( false == is_writable($i_path))
		$o_out['error'] = 'Destination not writeable';

	$o_out['files'] = array();

	foreach( $_FILES as $key => $file)
	{
		$fileObj = array();
		$fileObj['path'] = $i_path;
		$fileObj['name'] = $_FILES[$key]['name'];

		if( isset( $o_out['error']))
		{
			$fileObj['error'] = $o_out['error'];
		}
		else if( $_FILES[$key]['error'] != UPLOAD_ERR_OK )
		{
			$fileObj['error'] = 'Error upload';
		}
		else if( false == is_uploaded_file($_FILES[$key]['tmp_name']))
		{
			$fileObj['error'] = 'Invalid upload';
		}
		else
		{
			$path = $i_path.'/'.$_FILES[$key]['name'];
			$path_orig = $path; $i = 1;
			while( is_file( $path)) $path = $path_orig.'-'.$i++;
			if( false == move_uploaded_file( $_FILES[$key]['tmp_name'], $path))
			{
				$fileObj['error'] = 'Can`t save upload';
			}
		}

		array_push( $o_out['files'], $fileObj);
	}
}

function jsf_sendmail( $i_args, &$o_out)
{
	$addr = $i_args['address'];
	if( strpos( $addr, '@' ) === false )
		$addr = implode('@', json_decode( base64_decode( $addr)));
	if( mail( $addr, $i_args['subject'], $i_args['body'], $i_args['headers']))
		$o_out['status'] = 'email sent.';
	else
		$o_out['error'] = 'email was not sent.';
}

?>

