<?php

function launchCmd( $cmd, $wdir )
{
    $output = '';

    // Prepare file descriptors:
    $descriptorspec = array(
       0 => array("pipe", "r"), // stdin is a pipe that the child will read from
       1 => array("pipe", "w"), // stdout is a pipe that the child will write to
       2 => array("pipe", "w")  // stderr is a file to write to
    );

    // Launch command:
    $process = proc_open( $cmd, $descriptorspec, $pipes, $wdir);
    fclose($pipes[0]);

    // Get output:
    $output = stream_get_contents($pipes[1]);
    fclose($pipes[1]);
    $output = $output.'<br/>';
    $output = $output.stream_get_contents($pipes[2]);
    fclose($pipes[2]);

    // Replace new line to html style:
    $output = str_replace("\n","<br/>\n",$output);

    // Check return status:
    if( proc_close($process) != 0 ) echo '<br/>Error executing:<br/>'.$cmd.'<br/>';

    return $output;
}

function launchAfCmd( $args )
{
    $wdir = dirname( getcwd()).'/bin';
    $cmd = $wdir.'/afcmd '.$args;
    return launchCmd( $cmd, $wdir );
}

?>
