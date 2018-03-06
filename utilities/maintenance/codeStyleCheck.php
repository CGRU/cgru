#!/usr/bin/env php
#
# this file checks misc source code files and applies automated code refactoring
# requires clang-format, git and php (apt-get install clang-format php5-cli git)
#
# on default this script runs dry,
# it will try to parse all files and prints problems inside the files
# this should always be run first, to see if any issues occur
# if you invoke this script with "nondry" as cli argument it will write changes to the project files
#

<?php
require_once(dirname(__FILE__) . '/common.inc.php');

printStart();

define('WILDCARD', '*');
define('LINE_LENGTH', 110);

$jsFiles = getAutoFormatFiles(array('js'));
$sourceFiles = getAutoFormatFiles(array('c', 'cpp', 'h', 'hpp'));

printStartGroup('RUNNING CHECKS ON JS FILES');
foreach ($jsFiles as $i => $filePath)
{
	$fileName = basename($filePath);
	$folderName = basename(str_replace($fileName, '', $filePath));
	$status = array();
	$success = true;
	$sourceContent = file_get_contents($filePath);

	if ($success) $success = checkFileHeader($filePath, $sourceContent, $status);
	if ($success) $success = checkClang($filePath, $sourceContent, $status);
	if ($success) $success = checkComment($sourceContent, $status);

	if ($success && !isDryRun() && count($status) > 0)
	{
		file_put_contents($filePath, $sourceContent);
	}
	printResultLine($folderName . '/' . $fileName, $success, $status, $i / count($jsFiles));
}
printEndGroup();

printStartGroup('RUNNING CHECKS ON SOURCE AND HEADER FILES');
foreach ($sourceFiles as $i => $filePath)
{
	$fileName = basename($filePath);
	$folderName = basename(str_replace($fileName, '', $filePath));
	$status = array();
	$success = true;
	$sourceContent = file_get_contents($filePath);

	if ($success) $success = checkFileHeader($filePath, $sourceContent, $status);
	if ($success) $success = checkClang($filePath, $sourceContent, $status);
	if ($success) $success = checkComment($sourceContent, $status);

	if ($success && !isDryRun() && count($status) > 0)
	{
		file_put_contents($filePath, $sourceContent);
	}
	printResultLine($folderName . '/' . $fileName, $success, $status, $i / count($jsFiles));
}
printEndGroup();
printFinish();
exit;


function checkFileHeader($filePath, &$fileContent, &$status)
{
	$headerRegex = '/^(\/\*\*?[\s\S]*?\*\/)([\s\S]*)$/';
	$modificationString = getModificationInterval($filePath);
	if (preg_match($headerRegex, $fileContent, $matchHeader))
	{
		$fileHeader = $matchHeader[1];
		// $fileSourceCode = $matchHeader[2];

		if (strpos($fileHeader, '<yMMMMMMMMMMMMMMy>') === false)
		{
			// not a recent proper file header, try other known formats
			$status[] = errorString('header unknown!');
		} else
		{
			$regexParseHeader = '/\.\.\ \*\/\n\n\/\*([\S\s]+?)\*\/([\S\s]+)$/';
			if (preg_match($regexParseHeader, $fileContent, $matchHeaderNew))
			{
				$newFileContent = getFileHeader($matchHeaderNew[1], $modificationString)
					. PHP_EOL . PHP_EOL . trim($matchHeaderNew[2]) . PHP_EOL;
				if ($newFileContent != $fileContent)
				{
					$status[] = noticeString('header has changed!');
					$fileContent = $newFileContent;
				}
				return true;
			} else
			{
				$status[] = errorString('header unknown!');
			}
		}

	} else
	{
		$status[] = noticeString('No header found, inserting dummy header');
		$fileContent = getFileHeader(basename($filePath) . ' - TODO: description', $modificationString) . PHP_EOL . PHP_EOL . trim($fileContent);
		return true;
	}
	return false;
}

function checkClang($filePath, &$fileContent, &$status)
{
	$contentsBefore = $fileContent;
	$extension = substr($filePath, strrpos($filePath, '.') + 1);
	$filepathTemp = PROJECT_PATH . '/.tmp.' . $extension;
	file_put_contents($filepathTemp, $fileContent);
	shell_exec(CLANG_FORMAT_EXEC_PATH . ' -i --style=file ' . escapeshellarg($filepathTemp));
	$fileContent = file_get_contents($filepathTemp);
	unlink($filepathTemp); // nothing to see here :)

	if ($contentsBefore != $fileContent)
	{
		$status[] = noticeString('checkClang changed');
	}
	return true;
}

function checkComment(&$fileContent, &$status)
{
	
	$contentsBefore = $fileContent;
	// superheading	
	$search = array(
		'/\/\/\s*####+\n\/\/\s*####+\s*([a-zA-Z0-9-_ ,;]+)\s*####+\n\/\/\s*####+/',
		'/\/\*\s*####+\n\s*####+\s*\[\s*([a-zA-Z0-9-_ ,;]+)\s*\]\s*####+\n\s*####+\s\*\//',
	);
	$fileContent = preg_replace_callback(
		$search,
		function ($match)
		{
			$title = trim($match[1]);
			return '/* ' . str_repeat('#', LINE_LENGTH - 3) . PHP_EOL
				. '################### [ ' . $title . ' ] #####' . str_repeat('#', (LINE_LENGTH - 30) - strlen($title)) . PHP_EOL
				. str_repeat('#', LINE_LENGTH - 3) . ' */';
		},
		$fileContent
	);
	// subheading	
	$search = '/\/\*\s*-{3}-*\s*\[\s*([a-zA-Z0-9-_ ,;]+)\s*\]\s*-{3}-*\s*\*\//';
	$fileContent = preg_replace_callback(
		$search,
		function ($match)
		{
			$title = trim($match[1]);
			return '/* ---------------- [ ' . $title . ' ] -----' . str_repeat('-', (LINE_LENGTH - 34) - strlen($title)) . ' */';
		},
		$fileContent
	);
	if ($contentsBefore != $fileContent)
	{
		$status[] = noticeString('checkComment changed');
	}
	return true;
}

function getFileHeader($description, $modificationString)
{
	$out = <<<EOT
/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	__description__
*/
EOT;
	return str_replace(array('__description__', '__date__'),
		array(trim($description), $modificationString), $out);
}

function getAutoFormatFiles($extensions)
{
	// all changed files since introduction of autoformat in commit b078a1 (includes uncommited changes)
	$files = shell_exec('git diff --name-only b078a1');
	$fileListAll = explode(PHP_EOL, $files);
	$fileList = array();
	foreach ($fileListAll as $file)
	{
		$extension = substr($file, strrpos($file, '.') + 1);
		if (in_array($extension, $extensions))
			$fileList[] = PROJECT_PATH . $file;
	}
	return $fileList;
}


?>

