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
	}

	$o_out['readdir'] = $out;
}

$recv = json_decode( $HTTP_RAW_POST_DATA, true);
$out = array();
if( $recv['readdir'])
{
	listDir($recv, $out); 
}

echo json_encode( $out);

?>

