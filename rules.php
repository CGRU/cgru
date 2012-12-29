<?php

function listDir( $i_readdir, &$o_out)
{
	$out = array();
	$dir = $i_readdir['readdir'];
	$rules = $i_readdir['rules'];
	$out['dir'] = $dir;

	$dir = str_replace('../','', $dir);
	$dir = str_replace('/..','', $dir);
	$dir = str_replace('..','', $dir);

	if( false == is_dir( $dir))
	{
		$out['error'] = 'No such folder.';
	}
	else if( $handle = opendir( $dir))
	{
		$out['folders'] = array();
		$out['files'] = array();
		$out['rufiles'] = array();
		$out['rules'] = array();
		$numdir = 0;
		$numfile = 0;
		while (false !== ( $entry = readdir( $handle)))
		{
			if( $entry == '.') continue;
			if( $entry == '..') continue;
			$path = $dir.'/'.$entry;
			if( false == is_dir( $path))
			{
				$out['files'][$numfile++] = $entry;
				continue;
			}

			$out['folders'][$numdir++] = $entry;

			if( $entry != $rules ) continue;

			$numrufile = 0;
			if( $rHandle = opendir( $path))
			{
				while (false !== ( $entry = readdir( $rHandle)))
				{
					if( $entry == '.') continue;
					if( $entry == '..') continue;
					$out['rufiles'][$numrufile++] = $entry;
					if( strrpos( $entry,'.json') == strlen($entry)-5)
					{
						if( $fHandle = fopen( $path.'/'.$entry, 'r'))
						{
							$rudata = fread( $fHandle, 1000000);
							$ruobj = json_decode( $rudata, true);
							$out['rules'][$entry] = $ruobj;
							fclose($fHandle);
						}
					}
				}
				closedir($rHandle);
				ksort($out['rules']);
			}
		}
		closedir($handle);
		sort( $out['folders']);
		sort( $out['files']);
		sort( $out['rufiles']);
	}

	$o_out['readdir'] = $out;
}

function walkDir( $i_readdir, &$o_out)
{
	$o_out['folder'] = 'qwerty';
}

function readConfig( $i_file, &$o_out)
{
	if( $fHandle = fopen( $i_file, 'r'))
	{
		$data = fread( $fHandle, 1000000);
		fclose($fHandle);
		$o_out[$i_file] = json_decode( $data, true);
		if( array_key_exists( 'include', $o_out[$i_file]['cgru_config']))
			foreach( $o_out[$i_file]['cgru_config']['include'] as $file )
				readConfig( $file, $o_out);
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

$recv = json_decode( $HTTP_RAW_POST_DATA, true);
$out = array();
if( array_key_exists('readdir', $recv))
{
	listDir($recv, $out);
}
else if( array_key_exists('walkdir', $recv))
{
	$walkdir = array();
	walkDir( $recv['walkdir'], $walkdir);
	$out['walkdir'] = $walkdir;
}
else if( array_key_exists('readconfig', $recv))
{
	$configs = array();
	readConfig( $recv['readconfig'], $configs); 
	$out['config'] = $configs;
}
else if( array_key_exists('afanasy', $recv))
{
	afanasy( $recv, $out);
}

echo json_encode( $out);

?>

