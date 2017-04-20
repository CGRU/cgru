<?php

ini_set('upload_max_filesize', '16G');
ini_set('post_max_size', '16G');
ini_set('memory_limit', '16G');
ini_set('max_input_time', 36000);
ini_set('max_execution_time', 600);


$GuestSites = array('rules.cgru.info','127.0.0.1');
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

$FileMaxLength = 1000000;

$Digest = false;
$UserID = null;
$Groups = null;

$SkipFiles = array( '.', '..', $HT_AccessFileName, $HT_GroupsFileName, $HT_DigestFileName);
$GuestCanCreate = array('status.json','comments.json');
$GuestCanEdit = array('comments.json');

$Out = array();
$Recv = array();

# Encode "Pretty" JSON data:
if( false == defined('JSON_PRETTY_PRINT')) define('JSON_PRETTY_PRINT', 0);
function jsonEncode( &$i_obj)
{
	if( phpversion() >= "5.3" )
		return json_encode( $i_obj, JSON_PRETTY_PRINT);
	else
		return json_encode( $i_obj);
}

function _flock_( &$i_handle, $i_type)
{
//	flock( $i_handle, $i_type);
}

$InputData = file_get_contents('php://input');

# Decode input:
if( isset($_POST['upload_path']))
	upload( $_POST['upload_path'], $Out);
else
{
	$Recv = json_decode( $InputData, true);
	if( is_null( $Recv ))
	{
		$Recv = json_decode( base64_decode( $InputData), true);
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
	$o_out['max_input_time'] = ini_get('max_input_time');
	$o_out['max_execution_time'] = ini_get('max_execution_time');
	if( $fHandle = fopen('version.txt','r'))
	{
		$o_out['version'] = fread( $fHandle, $FileMaxLength);
		fclose($fHandle);
	}
	$o_out['name'] = $_SERVER['SERVER_NAME'];
	$o_out['software'] = $_SERVER['SERVER_SOFTWARE'];
	$o_out['php_version'] = phpversion();
	foreach( $CONF as $key => $val ) $o_out[$key] = $val;
	if( $CONF['AUTH_RULES']) $o_out['nonce'] = md5(rand());

	$o_out['client_ip'] = get_client_ip();
}

function get_client_ip()
{
	$ipaddress = '';
	if( isset($_SERVER['HTTP_CLIENT_IP']))
		$ipaddress = $_SERVER['HTTP_CLIENT_IP'];
	else if( isset($_SERVER['HTTP_X_FORWARDED_FOR']))
		$ipaddress = $_SERVER['HTTP_X_FORWARDED_FOR'];
	else if( isset($_SERVER['HTTP_X_FORWARDED']))
		$ipaddress = $_SERVER['HTTP_X_FORWARDED'];
	else if( isset($_SERVER['HTTP_FORWARDED_FOR']))
		$ipaddress = $_SERVER['HTTP_FORWARDED_FOR'];
	else if( isset($_SERVER['HTTP_FORWARDED']))
		$ipaddress = $_SERVER['HTTP_FORWARDED'];
	else if( isset($_SERVER['REMOTE_ADDR']))
		$ipaddress = $_SERVER['REMOTE_ADDR'];
	else
		$ipaddress = 'UNKNOWN';
	return $ipaddress;
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

	processUser( $i_arg, $o_out);
	if( array_key_exists('error', $o_out)) return;

	$out = array();
	getallusers( $out);
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

		// We do not send all users props to each user.
		$props = array('title','role','states','disabled','signature');
		foreach( $props as $prop )
			if( isset( $obj[$prop])) $user[$prop] = $obj[$prop];

		if( isset( $obj['avatar']) && strlen($obj['avatar']))
			$user['avatar'] = $obj['avatar'];
		else if( isset( $obj['email']) && strlen($obj['email']))
			$user['avatar'] = 'http://www.gravatar.com/avatar/'.md5( strtolower( trim($obj['email'])));

		$o_out['users'][$obj['id']] = $user;
	}

	if( isAdmin( $out)) $o_out['admin'] = true;
}

function processUser( $i_arg, &$o_out)
{
	global $UserID;

	$dirname = 'users';
	if( false == is_dir( $dirname ))
		mkdir( $dirname);

	if( $UserID == null ) return;

	$filename = $dirname.'/'.$UserID.'.json';
	$user = array();

	if( is_file( $filename))
		readObj( $filename, $user);
	if( array_key_exists('error', $user))
		$o_out['error'] = $user['error'];

	if( false == isset( $user['id']      )) $user['id']       = $UserID;
	if( false == isset( $user['channels'])) $user['channels'] = array();
	if( false == isset( $user['news']    )) $user['news']     = array();
	if( false == isset( $user['ctime']   )) $user['ctime']    = time();
	$user['rtime'] = time();

	processUserIP( $i_arg, $user);

	// Delete nulls from some arrays:
	$arrays = array('news','bookmarks','channels');
	foreach( $arrays as $arr)
		for( $i = 0; $i < count($user[$arr]); )
			if( is_null( $user[$arr][$i]))
				array_splice( $user[$arr], $i, 1);
			else $i++;

	if( false == writeUser( $user))
	{
		$o_out['error'] = 'Can`t write current user';
		return;
	}

	$o_out['user'] = $user;
}

function processUserIP( $i_arg, &$o_user)
{
	$ip = get_client_ip();

	if( false == isset( $o_user['ips'] ))
		$o_user['ips'] = array();

	for( $i = 0; $i < count($o_user['ips']); )
		if(( false == isset($o_user['ips'][$i]['ip'])) || ( $o_user['ips'][$i]['ip'] == $ip ))
			array_splice( $o_user['ips'], $i, 1);
		else
			$i++;

	$rec = array();
	$rec['ip'] = $ip;
	$rec['time'] = time();
	$rec['url'] = $i_arg['url'];

	array_unshift( $o_user['ips'], $rec);

	$o_user['ips'] = array_slice( $o_user['ips'], 0, 10);
}

function writeUser( &$i_user)
{
	$filename = 'users/'.$i_user['id'].'.json';

	if( $fHandle = fopen( $filename, 'w'))
	{
		flock( $fHandle, LOCK_EX);
		fwrite( $fHandle, jsonEncode( $i_user));
		flock( $fHandle, LOCK_UN);
		fclose( $fHandle);
		return true;
	}

	return false;
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
		flock( $fHandle, LOCK_SH);
		$data = fread( $fHandle, $FileMaxLength);
		flock( $fHandle, LOCK_UN);
		fclose($fHandle);
	}
	else
	{
		$o_out['error'] = 'Can`t open HT digest file.';
		error_log( $o_out['error']);
		return false;
	}
	
	$data = explode("\n", $data);
	$found = false;
	foreach( $data as $line )
		if( strpos( $line, $Digest['username']) === 0)
		{
			$data = $line;
			$found = true;
			break;
		}

	if( false == $found )
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
/* Javascript logout used, PHP way does not work
function jsf_logout( $i_arg, &$o_out)
{
	header('HTTP/1.1 401 Unauthorized');
	session_destroy();
}
*/
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

		$walk = null;
		$walk_file = $i_dir.'/'.$rufolder.'/walk.json';
		if( is_file( $walk_file))
		if( $wHandle = fopen( $walk_file, 'r'))
		{
//error_log($path);
			$wdata = fread( $wHandle, $FileMaxLength);
//error_log($wdata);
			$walk = json_decode( $wdata, true);
			fclose($wHandle);
		}

		while (false !== ( $entry = readdir( $handle)))
		{
			if( skipFile( $entry)) continue;
			$path = $i_dir.'/'.$entry;

			if( $access && ( false == is_dir( $path)))
			{
				if( is_file( $path))
				{
					$fileObj = array();
					if( false == is_null( $walk) && isset( $walk['files']) && isset( $walk['files'][$entry]))
						$fileObj = $walk['files'][$entry];
					$fileObj['name'] = $entry;
					$st = stat( $path);
					$fileObj['size'] = $st['size'];
					$fileObj['mtime'] = $st['mtime'];
					$fileObj['space'] = $st['blocks'] * 512;
					array_push( $o_out['files'], $fileObj);
				}
				continue;
			}

			if(( $entry == $rufolder ) && is_dir( $path))
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

						if( is_null( $rufiles)) continue;
						$found = false;
						foreach( $rufiles as $rufile )
							if( strpos( $ruentry, $rufile ) === 0 )
							{
								$found = true;
								break;
							}
						if( false == $found ) continue;

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
				continue;
			}

			if(( $i_recv['showhidden'] == false ) && is_file("$path/.hidden")) continue;
			if( $denied ) continue;
			if( false === htaccessFolder( $path)) continue;

			$folderObj = array();

			if( is_array( $walk))
				if( array_key_exists('folders', $walk))
					if( array_key_exists( $entry, $walk['folders']))
						$folderObj = $walk['folders'][$entry];

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
		_flock_( $fHandle, LOCK_SH);
		echo fread( $fHandle, $FileMaxLength);
		_flock_( $fHandle, LOCK_UN);
		fclose($fHandle);
		$o_out = null;
	}
	else
		$o_out['error'] = 'Unable to load file '.$i_file;
}

function jsf_getobjects( $i_args, &$o_out)
{
	global $FileMaxLength;

	if( false == isset( $i_args['file']))
	{
		$o_out['error'] = 'File is not set.';
		return;
	}
	if( false == isset( $i_args['objects']))
	{
		$o_out['error'] = 'Object is not set.';
		return;
	}

	$file = $i_args['file'];
	$objects = $i_args['objects'];

	if( false == is_file( $file))
	{
		$o_out['error'] = 'No such file '.$file;
		return;
	}
	if( false == htaccessPath( $file))
	{
		$o_out['error'] = 'Permissions denied';
		return;
	}

	if( $fHandle = fopen( $file, 'r'))
	{
		_flock_( $fHandle, LOCK_SH);

		$data = json_decode( fread( $fHandle, $FileMaxLength), true);
		foreach( $objects as $object )
		{
			if( false == is_null($data ) && isset($data[$object]))
				$o_out[$object] = $data[$object];
			else
				$o_out[$object] = null;
		}

		_flock_( $fHandle, LOCK_UN);
		fclose($fHandle);
	}
	else
		$o_out['error'] = 'Unable to load file '.$file;
}

function jsf_makefolder( $i_args, &$o_out)
{
	$dirname = $i_args['path'];
	mkdir( $dirname, 0777, true);
	if( false == is_dir( $dirname))
	{
		$o_out['error'] = 'Unable to create directory '.$dirname;
		return;
	}
	$o_out['makefolder'] = $dirname;
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
		_flock_( $fHandle, LOCK_SH);
		$data = fread( $fHandle, $FileMaxLength);
		_flock_( $fHandle, LOCK_UN);
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
function pushArray( &$o_obj, &$i_edit, $io_depth = 0 )
{
//error_log('pushArray: '.json_encode($i_edit));
	if( is_null( $i_edit) || is_null( $o_obj))
		return false;

	if( false == is_array( $o_obj))
		return false;

	$id = 'id';
	if( array_key_exists('keyname', $i_edit))
		$id = $i_edit['keyname'];

	// If id is provided, we should operate with an array with this id
	if(( array_key_exists( $id, $i_edit) == false ) || ( array_key_exists( $id, $o_obj) && ( $o_obj[$id] == $i_edit[$id])))
	{
		if( array_key_exists( $i_edit['pusharray'], $o_obj) && is_array( $o_obj[$i_edit['pusharray']]))
		{
			$pusharray = &$o_obj[$i_edit['pusharray']];
			$offset = count( $pusharray); // Default index for new item - push at the end

			// Delete any other items with the same ids as input objects:
			for( $e = 0; $e < count( $i_edit['objects']); $e++)
				if( array_key_exists( $id, $i_edit['objects'][$e] ))
					for( $i = 0; $i < count( $pusharray); $i++)
						if( array_key_exists( $id, $pusharray[$i]))
							if( $pusharray[$i][$id] == $i_edit['objects'][$e][$id])
								array_splice( $pusharray, $i, 1);

			// Search for an index to insert before:
			if( array_key_exists( 'id_before', $i_edit))
				for( $i = 0; $i < count( $pusharray); $i++)
					if( array_key_exists( $id, $pusharray[$i]))
						if( $pusharray[$i][$id] == $i_edit['id_before'])
							$offset = $i;

//error_log('id_before = '.$i_edit['id_before']);
//error_log('offset = '.$offset);
			array_splice( $pusharray, $offset, 0, $i_edit['objects']);

			return true;
		}
	}

	// Search for an array deeper:
	foreach( $o_obj as &$obj )
		if( pushArray( $obj, $i_edit, $io_depth + 1 ))
			return true;

	// Create an array in the object root, if it was not founded:
	if( $io_depth == 0 )
	{
		$o_obj[$i_edit['pusharray']] = array();
		return pushArray( $o_obj, $i_edit, 1 );
	}

	return false;
}
function delArray( &$o_obj, $i_edit)
{
	//error_log('o_obj:'.json_encode($o_obj));
	//error_log('i_edit:'.json_encode($i_edit));

	if( false == is_array( $o_obj))
		return;

	// Iterate object to search delarray
	foreach( $o_obj as $name => &$arr )
	{
		if( false == is_array( $arr))
			continue;

		if( count( $arr) == 0 )
			continue;

		if( $i_edit['delarray'] === $name)
		{
			//error_log( $i_edit['delarray'].'<>'.$name);

			// Iteate delarray
			for( $i = 0; $i < count( $arr); )
			{
				// Array member to delete should be an array too
				if( false == is_array( $arr[$i]))
				{
					$i++;
					continue;
				}
				/*if( is_null( $arr[$i]))
				{
					array_splice( $arr, $i, 1);
					continue;
				}*/

				//error_log('ckecking:'.json_encode($arr[$i]));

				$deleted = false;
				// Iterate objects to delete:
				foreach( $i_edit['objects'] as $delobj )
				{
					$allkeysequal = true;
					// Iterate all provided keys to found a member to delete
					foreach( $delobj as $key => $val )
					{
						if( array_key_exists( $key, $arr[$i]))
							if( $arr[$i][$key] == $val )
								continue;

						$allkeysequal = false;
						break;
					}

					if( $allkeysequal )
					{
						//error_log('unsetting:'.json_encode($arr[$i]));
						array_splice( $arr, $i, 1);
						$deleted = true;
						break;
					}
				}

				if( false == $deleted )
					$i++;
			}
		}

		// Recursively going deeper to find delarray:
		delArray( $o_obj[$name], $i_edit);
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
		_flock_( $fHandle, LOCK_EX);
		$data = fread( $fHandle, $FileMaxLength);
		$obj = json_decode( $data, true);

		if( is_null( $obj))
			$obj = array();

		if( array_key_exists('add', $i_edit) && ( $i_edit['add'] == true ))
			mergeObjs( $obj, $i_edit['object']);
		else if( array_key_exists('pusharray', $i_edit))
			pushArray( $obj, $i_edit);
		else if( array_key_exists('replace', $i_edit) && ( $i_edit['replace'] == true ))
			foreach( $i_edit['objects'] as $newobj )
				replaceObject( $obj, $newobj);
		else if( array_key_exists('delarray', $i_edit))
			delArray( $obj, $i_edit);

//error_log('obj:'.json_encode($obj));
		rewind( $fHandle);
		ftruncate( $fHandle, 0);
		fwrite( $fHandle, jsonEncode( $obj));
		_flock_( $fHandle, LOCK_UN);
		fclose($fHandle);
		$o_out['status'] = 'success';
		$o_out['object'] = $obj;
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
	$header = 'AFANASY '.strlen($data).' JSON';

	fwrite( $socket, $header.$data);

	$len = -1;
	$data = '';
	while( true)
	{
		$part = fread( $socket, 4096);
		if( $part == FALSE ) break;
		$data = $data.$part;

		// Try to parse header and find length
		if(( $len == -1 ) && ( strpos( $part,'AFANASY') == 0 ) && ( strpos( $part,'JSON') !== FALSE ))
		{
			$part = substr( $part, 8); // cut 'AFANASY'
			$part = substr( $part, 0, strpos( $part,' ')); // cut to the first space
			$len = intval($part);
		}

		if(( $len != -1 ) && ( strlen($data) >= $len ))
			break;
	}

	fclose( $socket);

	// Cut header:
	$data = substr( $data, strpos( $data,'JSON')+4);

	$o_out['response'] = json_decode( $data, true);
	$o_out['satus'] = 'success';
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

	_flock_( $fHandle, LOCK_EX);
	fwrite( $fHandle, $data);
	_flock_( $fHandle, LOCK_UN);
	fclose( $fHandle );
}

function jsf_makenews( $i_args, &$o_out)
{
	// Read all users:
	$users = array();	
	getallusers( $users);
	if( array_key_exists('error', $users))
	{
		$o_out['error'] = $users['error'];
		return;
	}
	$users = $users['users'];
	if( count( $users) == 0 )
	{
		$o_out['error'] = 'No users found.';
		return;
	}

	$users_changed = array();

	foreach( $i_args['news_requests'] as $request)
	{
		$ids = makenews( $request, $users, $o_out);
		if( isset( $o_out['error']))
			return;

		foreach( $ids as $id)
			if( false == in_array( $id, $users_changed))
				array_push( $users_changed, $id);
	}

	if( isset( $i_args['bookmarks']))
		foreach( $i_args['bookmarks'] as $bm)
		{
			$ids = makebookmarks( $bm, $users, $o_out);
			if( isset( $o_out['error']))
				return;

			foreach( $ids as $id)
				if( false == in_array( $id, $users_changed))
					array_push( $users_changed, $id);
		}

	// Write changed users:
	foreach( $users_changed as $id)
		writeUser( $users[$id]);

	$o_out['users'] = $users_changed;
}
function makenews( $i_args, &$io_users, &$o_out)
{
	global $FileMaxLength;

	$news = $i_args['news'];

	// Ensure that news has a path:
	if( false == array_key_exists('path', $news ))
	{
		$o_out['error'] = 'News can`t be without a path.';
		return;
	}

	$path = $news['path'];

	// Path should not be an empty string:
	if( strlen($path) == 0 )
	{
		$o_out['error'] = 'Path is an empty string.';
		return;
	}

	// Ensure that the first path character is '/':
	if( $path[0] != '/' )
		$path = '/'.$path;

	// Ensure that path last character is not '/' (if path is not just '/' root):
	if(( $path != '/') && ( $path[strlen($path-1)] == '/'))
		$path = substr( $path, 0, $strlen($path)-1);

	// Process recent for current and each parent folders till root:
	for( $i = 0; $i <= 100; $i++)
	{
		// Simple loop check:
		if( $i >= 100 )
		{
			error_log('News: Recent path loop.');
			break;
		}

//error_log('path='.$path);
		$rarray = array();

		// Get existing recent:
		$rfile = $i_args['root'].$path.'/'.$i_args['rufolder'].'/'.$i_args['recent_file'];
		if( is_file( $rfile))
		if( $rhandle = fopen( $rfile, 'r'))
		{
			_flock_( $rhandle, LOCK_SH);
			$rdata = fread( $rhandle, $FileMaxLength);
			_flock_( $rhandle, LOCK_UN);
			fclose( $rhandle);
			$rarray = json_decode( $rdata, true);
			if( is_null( $rarray))
				$rarray = array();
			$count = count( $rarray);
			if( $count)
			{
				if( $i )
				{
					// Remove all news with the same path from all parent folders,
					// so folder has only one recent from each child:
					for( $j = 0; $j < $count; $j++ )
						if( $rarray[$j]['path'] == $news['path'])
						{
							array_splice( $rarray, $j, 1);
							$count = count( $rarray);
							$j--;
						}
				}
				else
				{
					// Remove latest recent news if it is the same:
					if( ( $rarray[0]['path']  == $news['path']  ) &&
						( $rarray[0]['title'] == $news['title'] ) &&
						( $rarray[0]['user']  == $news['user']  ) )
							array_splice( $rarray, 0, 1);
				}
				while( count( $rarray) >= $i_args['recent_max'])
					array_pop( $rarray);
			}
		}

		// Add new recent:
		array_unshift( $rarray, $news);

		// Save recent:
		if( false == is_dir( dirname( $rfile)))
			mkdir( dirname( $rfile));
		if( $rhandle = fopen( $rfile, 'w'))
		{
			_flock_( $rhandle, LOCK_EX);
			fwrite( $rhandle, jsonEncode( $rarray));
			_flock_( $rhandle, LOCK_UN);
			fclose($rhandle);
		}

		// Exit cycle if path is root:
		if( strlen( $path) == 0 ) break;

		// Set path to parent folder:
		$path_prev = $path;
		$path = substr( $path, 0, strrpos( $path, '/'));
		// Stop cycle if can't go to parent folder:
		if( $path == $path_prev ) break;
	}


	// Process users subsriptions:

	// User may be does not want to receive own news:
	$ignore_own = false;
	if( isset( $i_args['ignore_own']) && $i_args['ignore_own'] )
		$ignore_own = true;

    // Get subscribed users:
    $sub_users = array();
	$changed_users = array();
	foreach( $io_users as &$user )
	{
		// If this is news owner:
		if( $news['user'] == $user['id'] )
		{
			// Store last news time:
			$user['ntime'] = time();
			if( false == in_array( $user['id'], $changed_users))
				array_push( $changed_users, $user['id']);

			// If user does not want to receive own news:
			if( $ignore_own )
				continue;
		}

		if( array_key_exists( 'artists', $news))
			if( in_array( $user['id'], $news['artists']))
			{
				if( false == in_array( $user['id'], $sub_users))
					array_push( $sub_users, $user['id']);
				continue;
			}


		if( array_key_exists( 'channels', $user))
			foreach( $user['channels'] as $channel )
				if( strpos( $news['path'], $channel['id'] ) === 0 )
				{
					if( false == in_array( $user['id'], $sub_users))
						array_push( $sub_users, $user['id']);
					break;
				}
	}


    // Add news and write files:
    foreach( $sub_users as $id)
    {
		$user = &$io_users[$id];

		// Add uid to changed array:
		if( false == in_array( $user['id'], $changed_users))
			array_push( $changed_users, $user['id']);

		// Delete older news with the same path:
		for( $i = 0; $i < count($user['news']); $i++)
		{
			if( $user['news'][$i]['path'] == $news['path'])
			{
				array_splice( $user['news'], $i, 1);
				break;
			}
		}

		// Add news to the beginning of array:
		array_unshift( $user['news'], $news);

		// Delete news above the limit:
		$limit = $i_args['limit'];
		if( array_key_exists('news_limit', $user))
			if( $user['news_limit'] > 0 )
				$limit = $user['news_limit'];

		while( count( $user['news']) > $limit)
			array_pop( $user['news']);

		// Send emails
		if( array_key_exists('email', $user) == false ) continue;
		if( array_key_exists('email_news', $user) == false ) continue;
		if( $user['email_news'] != true ) continue;

		$mail = array();
		$mail['from_title'] = $i_args['email_from_title'];
		$mail['address'] = $user['email'];
		$mail['subject'] = $i_args['email_subject'];
		$mail['body'] = $i_args['email_body'];

		$out = array();
		jsf_sendmail( $mail, $out);
	}

	return $changed_users;
}
function makebookmarks( $i_bm, &$io_users, &$o_out)
{
	global $UserID;

	$changed_users = array();
	foreach( $io_users as &$user )
	{
		if( false == isset( $user['bookmarks']))
			$user['bookmarks'] = array();

		// Try to find existing bookmark index with the same path:
		$bm_index = -1;
		for( $i = 0; $i < count($user['bookmarks']); $i++)
		{
			if( is_null( $user['bookmarks'][$i]))
				continue;

			if( $user['bookmarks'][$i]['path'] == $i_bm['path'])
			{
				$bm_index = $i;
				break;
			}
		}

		// Bookmark with the same path does not exist:
		if( $bm_index == -1 )
		{
			// Check whether the bookmark is needed:
			if( is_null( $i_bm['status'])) continue;
			if( false == isset( $i_bm['status']['artists'])) continue;
			if( false == in_array( $user['id'], $i_bm['status']['artists'])) continue;
			if( isset( $i_bm['status']['progress']) && ($i_bm['status']['progress'] >= 100)) continue;
			if( isset( $i_bm['status']['flags']) && in_array('omit', $i_bm['status']['flags'])) continue;

			// Initialize parameters:
			$i_bm['cuser'] = $UserID;
			$i_bm['ctime'] = time();
		}
		else
		{
			// Copy paramters:
			$i_bm['cuser'] = $user['bookmarks'][$i]['cuser'];
			$i_bm['ctime'] = $user['bookmarks'][$i]['ctime'];

			// Delete existing bookmark:
			array_splice( $user['bookmarks'], $i, 1);
		}

		$i_bm['muser'] = $UserID;
		$i_bm['mtime'] = time();

		array_push( $user['bookmarks'], $i_bm);
		array_push( $changed_users, $user['id']);
	}

	return $changed_users;
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
		$o_out['error'] = 'No admins group found.';
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

	$user = $i_recv['user'];

	// Not admin can change only own password,
	//    if he has special state "passwd".
	// Admin can change any user password.
	if( false == isAdmin( $o_out))
	{
		if( is_null( $UserID))
		{
			$o_out['error'] = 'Guests can`t change any password';
			return;
		}

		if( $user != $UserID )
		{
			$o_out['error'] = 'User can`t change other user password';
			return;
		}

		// Check "passwd" state:
		$out = array();
		getallusers( $out);
		if( array_key_exists('error', $out))
		{
			$o_out['error'] = $out['error'];
			return;
		}
		if( false == array_key_exists( $user, $out['users']))
		{
			$o_out['error'] = 'User "'.$user.'" not founded.';
			return;
		}
		$uobj = $out['users'][$user];
		if(( false == array_key_exists('states', $uobj))
			|| ( false === array_search('passwd', $uobj['states'])))
		{
			$o_out['error'] = 'You are not allowed to change password.';
			return;
		}
	}

	$data = '';
	if( $fHandle = fopen( $HT_DigestFileName, 'r'))
	{
		_flock_( $fHandle, LOCK_SH);
		$data = fread( $fHandle, $FileMaxLength);
		_flock_( $fHandle, LOCK_UN);
		fclose($fHandle);
	}

	// Construct new lines w/o our user (if it exists):
	$o_out['status'] = 'User "'.$user.'" set.';
	$o_out['user'] = $user;
	$old_lines = explode("\n", $data);
	$new_lines = array();
	foreach( $old_lines as $line )
	{
		$values = explode(':', $line);
		if( count( $values ) == 3 )
		{
			if( $values[0] == $user )
			{
				// Just skip old line with our user:
				$o_out['status'] = 'User "'.$user.'" updated.';
			}
			else
			{
				// Store line with other user:
				array_push( $new_lines, $line);
			}
		}
	}
	// Add our user to the end:
	array_push( $new_lines, $i_recv['digest']);

	$data = implode("\n", $new_lines)."\n";

	if( $fHandle = fopen( $HT_DigestFileName, 'w' ))
	{
		_flock_( $fHandle, LOCK_EX);
		fwrite( $fHandle, $data );
		_flock_( $fHandle, LOCK_UN);
		fclose( $fHandle );
	}
	else
	{
		$o_out['error'] = 'Unable to write into the file.';
	}
//error_log($data);
}

function jsf_disableuser( $i_args, &$o_out)
{
	global $FileMaxLength, $HT_DigestFileName;
	if( false == isAdmin( $o_out)) return;

	$uid = $i_args['uid'];

	$dHandle = opendir('users');
	if( $dHandle === false )
	{
		$o_out['error'] = 'Can`t open users folder.';
		return;
	}
	while (false !== ( $entry = readdir( $dHandle)))
	{
		if( false === is_file("users/$entry")) continue;
		if( $entry != "$uid.json" ) continue;

		// If user new object provided, we write it.
		// This needed to just disable user and not to loose its settings.
		if( array_key_exists('uobj', $i_args))
		{
			if( $fHandle = fopen("users/$uid.json",'w'))
			{
				flock( $fHandle, LOCK_EX);
				fwrite( $fHandle, jsonEncode( $i_args['uobj']));
				flock( $fHandle, LOCK_UN);
				fclose($fHandle);
				$o_out['status'] = 'success';
			}
			else
				$out['error'] = 'Unable to write "'.$uid.'" user file';
		}
		else
		{
			// Delete user file and loose all its data:
			unlink("users/$entry");
		}
		break;
	}
	closedir($dHandle);

	$data = '';
	if( $fHandle = fopen( $HT_DigestFileName, 'r'))
	{
		flock( $fHandle, LOCK_SH);
		$data = fread( $fHandle, $FileMaxLength);
		flock( $fHandle, LOCK_UN);
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
			if( $values[0] != $uid )
				array_push( $new_lines, $line);
		}
	}
	$data = implode("\n", $new_lines)."\n";

	if( $fHandle = fopen( $HT_DigestFileName, 'w' ))
	{
		flock( $fHandle, LOCK_EX);
		fwrite( $fHandle, $data );
		flock( $fHandle, LOCK_UN);
		fclose( $fHandle );
	}
	else $o_out['error'] = 'Unable to write into the file.';
}

function jsf_getallusers( $i_args, &$o_out)
{
	if( false == isAdmin( $o_out)) return;
	getallusers( $o_out);
}
function getallusers( &$o_out)
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
			flock( $fHandle, LOCK_SH);
			$user = json_decode( fread( $fHandle, $FileMaxLength), true);
			flock( $fHandle, LOCK_UN);
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

	flock( $fHandle, LOCK_SH);
	$data = fread( $fHandle, $FileMaxLength);
	flock( $fHandle, LOCK_UN);
	fclose($fHandle);

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
		flock( $fHandle, LOCK_EX);
		fwrite( $fHandle, $data );
		flock( $fHandle, LOCK_UN);
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
	}

	$data = implode("\n", $lines)."\n";
//error_log($data);return;

	$htaccess = $i_args['path'].'/'.$HT_AccessFileName;
	if( $fHandle = fopen( $htaccess, 'w' ))
	{
		_flock_( $fHandle, LOCK_EX);
		fwrite( $fHandle, $data );
		_flock_( $fHandle, LOCK_UN);
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
		$o_out['error'] = 'No permissions settings founded.';
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
	_flock_( $fHandle, LOCK_SH);
	$data = fread( $fHandle, $FileMaxLength);
	_flock_( $fHandle, LOCK_UN);
	fclose( $fHandle);

	$found = false;
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
			$found = true;
		}
		else if( $words[1] == 'user' )
		{
			unset($words[1]);
			foreach( $words as $user) array_push( $o_out['users'], $user);
			$found = true;
		}
		else if( $words[1] == 'valid-user' )
		{
			$o_out['valid_user'] = true;
			$found = true;
		}
	}
/*	if( false == $found )
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

		$found = false;
		$rufolder = "$path/".$i_args['rufolder'];
		if( is_dir( $rufolder)) $found = true;
		if( $found && htaccessPath( $i_path)) $found = true;

		if( $found && array_key_exists('status', $i_args ))
		{
			$found = false;
			$rufile = "$rufolder/status.json";
			if( is_file( $rufile))
			{
				if( $fHandle = fopen( $rufile, 'r'))
				{
					$obj = json_decode( fread( $fHandle, $FileMaxLength), true);
						if( searchStatus( $i_args['status'], $obj))
							$found = true;
					fclose($fHandle);
				}
			}
		}

		if( $found && array_key_exists('body', $i_args ))
		{
			$found = false;
			$rufile = "$rufolder/body.html";
			if( is_file( $rufile))
			{
				if( $fHandle = fopen( $rufile, 'r'))
				{
					$data = fread( $fHandle, $FileMaxLength);
					fclose($fHandle);
					if( mb_stripos( $data, $i_args['body'], 0, 'utf-8') !== false )
						$found = true;
				}
			}
		}

		if( $found && array_key_exists('comment', $i_args ))
		{
			$found = false;
			$rufile = "$rufolder/comments.json";
			if( is_file( $rufile))
			{
				if( $fHandle = fopen( $rufile, 'r'))
				{
					$obj = json_decode( fread( $fHandle, $FileMaxLength), true);
					if( searchComment( $i_args['comment'], $obj))
						$found = true;
					fclose($fHandle);
				}
			}
		}

		if( $found )
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

	$found = true;

	if( $found && array_key_exists('ann', $i_args))
	{
		$found = false;
		if( array_key_exists('annotation', $i_obj['status']))
			if( mb_stripos( $i_obj['status']['annotation'], $i_args['ann'], 0, 'utf-8') !== false )
				$found = true;
	}

	if( $found && array_key_exists('artists', $i_args))
	{
		$found = false;
		if( array_key_exists('artists', $i_obj['status']) && count($i_obj['status']['artists']))
		{
			foreach( $i_args['artists'] as $artist )
				if( in_array( $artist, $i_obj['status']['artists']))
					$found = true;
		}
		else if( in_array('_null_', $i_args['artists']))
			$found = true;
	}

	if( $found && array_key_exists('tags', $i_args))
	{
		$found = false;
		if( array_key_exists('tags', $i_obj['status']))
			foreach( $i_args['tags'] as $tag )
				if( in_array( $tag, $i_obj['status']['tags']))
					$found = true;
	}

	if( $found && array_key_exists('percent', $i_args))
	{
		$found = false;
		if( array_key_exists('progress', $i_obj['status']))
			if( ($i_obj['status']['progress'] >= $i_args['percent'][0]) &&
				($i_obj['status']['progress'] <= $i_args['percent'][1]) )
					$found = true;
	}

	$parms = array('finish','statmod','bodymod');
	foreach( $parms as $parm )
		if( $found && array_key_exists( $parm, $i_args))
		{
			$found = false;
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
						$found = true;
			}
		}

	return $found;
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
			$basename = $_FILES[$key]['name'];
			$path = $i_path.'/'.$basename;

			// If such file already exists, we rename the upload:
			$dot = strrpos( $basename, '.');
			$i = 1;
			while( is_file( $path))
			{
				if( $dot )
				{
					$base = substr( $basename, 0, $dot);
					$ext = substr( $basename, $dot);
					$path = $i_path.'/'.$base.'-'.$i.$ext;
				}
				else
					$path = $i_path.'/'.$basename.'-'.$i;
				$i++;
			}

			// Move uploaded file to the desired place:
			if( false == move_uploaded_file( $_FILES[$key]['tmp_name'], $path))
			{
				$fileObj['error'] = 'Can`t save upload';
			}

			$fileObj['filename'] = $path;
		}

		array_push( $o_out['files'], $fileObj);
	}
}

function jsf_sendmail( $i_args, &$o_out)
{
	$addr = $i_args['address'];
	if( strpos( $addr, '@' ) === false )
		$addr = implode('@', json_decode( base64_decode( $addr)));

	$from_title = 'CGRU';
	$from_address = 'noreply@cgru.info';
	if( array_key_exists('from_title', $i_args)) $from_title = $i_args['from_title'];
	if( array_key_exists('from_address', $i_args)) $from_address = $i_args['from_address'];

	$headers = '';
	$headers = $headers."MIME-Version: 1.0\r\n";
	$headers = $headers."Content-type: text/html; charset=utf-8\r\n";
//	$headers = $headers."To: <'+i_address+'>\r\n";
	$headers = $headers."From: $from_title <$from_address>\r\n";

	$subject = $i_args['subject'];

	$body = '';
	$body = $body.'<html><body>';
	$body = $body.'<div style="background:#DFA; color:#020; margin:8px; padding:8px; border:2px solid #070; border-radius:9px;">';
	$body = $body.$subject;
	$body = $body.'<div style="background:#FFF; color:#000; margin:8px; padding:8px; border:2px solid #070; border-radius:9px;">';
	$body = $body.$i_args['body'];
	$body = $body.'</div><a href="cgru.info" style="padding:10px;margin:10px;" target="_blank">CGRU</a>';
	$body = $body.'</div></body></html>';

	if( mail( $addr, $subject, $body, $headers))
		$o_out['status'] = 'email sent.';
	else
		$o_out['error'] = 'email was not sent.';
}

?>

