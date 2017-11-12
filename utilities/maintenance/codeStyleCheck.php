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

$filesToCheckJS = array();
$filesToCheckJS[] = PROJECT_PATH . "afanasy/browser/" . WILDCARD . ".js";

// TODO
$filesToCheckSource = array();

// TODO
$filesToCheckHeader = array();

$jsFiles = glob("{" . implode(",", $filesToCheckJS) . "}", GLOB_BRACE);
// $sourceFiles = glob("{" . implode(",", $filesToCheckSource) . "}", GLOB_BRACE);
// $headerFiles = glob("{" . implode(",", $filesToCheckHeader) . "}", GLOB_BRACE);

printStartGroup('RUNNING CHECKS ON JS FILES');
foreach ($jsFiles as $i => $filePath) {
	$fileName = basename($filePath);
	$folderName = basename(str_replace($fileName, '', $filePath));
	$status = array();
	$success = true;
	$sourceContent = file_get_contents($filePath);

	if ($success) $success = checkFileHeader($filePath, $sourceContent, $status);
	if ($success) $success = checkClang($sourceContent, $status, 'js');

	if ($success && !isDryRun() && count($status) > 0) {
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
	if (preg_match($headerRegex, $fileContent, $matchHeader)) {
		$fileHeader = $matchHeader[1];
		// $fileSourceCode = $matchHeader[2];
		$modificationString = getModificationInterval($filePath);

		if (strpos($fileHeader, '<yMMMMMMMMMMMMMMy>') === false) {
			// not a recent proper file header, try other known formats
            		$status[] = errorString('header unknown!');
		} else {
			$regexParseHeader = '/^[\s\S]+\*\s*\'\s*\'([\s\S]*?)\.*\s*\*\/([\s\S]+)$/';
			if (preg_match($regexParseHeader, $fileContent, $matchHeaderNew)) {
				$newFileContent = getFileHeader($matchHeaderNew[1], $modificationString) . $matchHeaderNew[2];
				if ($newFileContent != $fileContent) {
					$status[] = noticeString('header has changed!');
					$fileContent = $newFileContent;
				}
				return true;
			} else {
				$status[] = errorString('header unknown!');
			}
		}

	} else {
		$status[] = errorString('No header found!');
	}
	return false;
}

function checkClang(&$fileContent, &$status, $extension)
{
	$contentsBefore = $fileContent;

	$filepathTemp = PROJECT_PATH . '/.tmp.' . $extension;
	file_put_contents($filepathTemp, $fileContent);
	shell_exec(CLANG_FORMAT_EXEC_PATH . ' -i --style=file ' . escapeshellarg($filepathTemp));
	$fileContent = file_get_contents($filepathTemp);
	unlink($filepathTemp); // nothing to see here :)

	if ($contentsBefore != $fileContent) {
		$status[] = noticeString('checkClang changed');
	}
	return true;
}

function getFileHeader($description, $modificationString)
{
	$out = <<<EOT
/** '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                                      Copyright © __date__ by The CGRU team
 *    '          '
 * __description__
 * ....................................................................................................... */
EOT;
	$lines = explode(PHP_EOL, $description);
	$nonEmptyLines = array();
	$firstFound = false;
	foreach ($lines as $line) {
		$l = trim($line);
		if (!$firstFound && ($l == '*' || empty($l))) continue;
		$regexCommentStart = '/^\*\s?(.*)$/';
		if (preg_match($regexCommentStart, $l, $match)) {
			$l = $match[1];
		}
		$nonEmptyLines[] = rtrim(($firstFound ? ' * ' : '') . $l);
		$firstFound = true;
	}
	if ($nonEmptyLines[count($nonEmptyLines) - 1] == ' *') {
		unset($nonEmptyLines[count($nonEmptyLines) - 1]);
	}

	$description = implode(PHP_EOL, $nonEmptyLines);
	return str_replace(array('__description__', '__date__'),
		array($description, $modificationString), $out);
}

?>

