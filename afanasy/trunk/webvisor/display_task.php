<?php

function display_task()
{
$action=$_GET['action'];
if( $action == '') $action = 'task';

$jid_str=$_GET['jid'];
if( $jid_str == '')
{
    echo 'Job ID is not specified.';
    return;
}
$jid = (int)$jid_str;
if( $jid <= 0)
{
    echo 'Invalid Job ID = "'.$jid_str.'"';
    return;
}
$bid_str=$_GET['bid'];
if( $bid_str == '')
{
    echo 'Block ID is not specified.';
    return;
}
$bid = (int)$bid_str;
if( $bid < 0)
{
    echo 'Invalid Block ID = "'.$bid_str.'"';
    return;
}
$tid_str=$_GET['tid'];
if( $tid_str == '')
{
    echo 'Task ID is not specified.';
    return;
}
$tid = (int)$tid_str;
if( $tid < 0)
{
    echo 'Invalid Task ID = "'.$tid.'"';
    return;
}

$taskaction=$_POST['taskaction'];
echo $taskaction;

echo '<form method="post" action="index.php?action=task&jid='.$jid.'&bid='.$bid.'&tid='.$tid.'">';
echo "<br/>\n";
echo '<table align="center" width="99%"><tr align="center"><td>';
echo '<input type="text" name="taskaction" value="restart">';
echo '</td><td><input type="submit" value="Restart"/>';
echo '<td></tr></table>';
echo '</form>';


// Connect to database:
$dbconn = db_connect();

// Get task progress from database:
$query='SELECT * FROM progress WHERE id_job='.$jid_str.' AND id_block='.$bid.' AND id_task='.$tid.';';
$resprogress = pg_query($query) or die('Query failed: ' . pg_last_error());
$lineprogress = pg_fetch_array( $resprogress, null, PGSQL_ASSOC);

$state = $lineprogress["state"];
$starts_count = $lineprogress["starts_count"];
$errors_count = $lineprogress["errors_count"];
$time_started = $lineprogress["time_started"];
$time_done = $lineprogress["time_done"];

pg_free_result($resprogress);

// Display task progress information:
echo '<h3>Task Progress Information</h3>';
echo '<table border="1">';
echo '<tr align="center"><td style="padding:5px">Progress</td><td style="padding:5px">Log</td></tr>';
echo '<tr>';
echo '<td style="padding:10px">';
    echo 'Task state = <b>'.stateToString( $state).'</b><br/>';
    echo 'Starts count = <b>'.$starts_count.'</b><br/>';
    echo 'Errors count = <b>'.$errors_count.'</b><br/>';
    if( $starts_count > 0 )
    {
        echo 'Time started = <b>'.date('D, j M Y G:i:s', $time_started).'</b><br/>';
        if( stateIsDone( $state ))
            echo 'Time done = <b>'.date('D, j M Y G:i:s', $time_done).'</b><br/>';
        if( $time_done > 0 )
            echo 'Runnig time = <b>'.time_strDHMS($time_done - $time_started).'</b><br/>';
    }
echo '</td>';
echo '<td style="padding:10px">';
    $descriptorspec = array(
       0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
       1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
       2 => array("file", "/tmp/error-output.txt", "a") // stderr is a file to write to
    );
    $cmd = '/cgru/afanasy/trunk/bin/afcmd tlog '.$jid_str.' '.$bid_str.' '.$tid_str.' 0';
    $cwd = '/cgru/afanasy/trunk/bin/';
    $process = proc_open( $cmd, $descriptorspec, $pipes, $cwd);
    fclose($pipes[0]);
    $output = stream_get_contents($pipes[1]);
    fclose($pipes[1]);
    echo str_replace("\n","<br/>\n",$output);
    if( proc_close($process) != 0 ) echo '<br/>Error onexecuting:<br/>'.$cmd;
echo '</td>';
echo '</tr>';
echo '</table>';

echo '<h3>Task Last Session Output</h3>';
echo '<table border="1">';
echo '<tr>';
echo '<td style="padding:10px">';
$descriptorspec = array(
   0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
   1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
   2 => array("file", "/tmp/error-output.txt", "a") // stderr is a file to write to
);
$cmd = '/cgru/afanasy/trunk/bin/afcmd tout '.$jid_str.' '.$bid_str.' '.$tid_str.' 0';
$cwd = '/cgru/afanasy/trunk/bin/';
$process = proc_open( $cmd, $descriptorspec, $pipes, $cwd);
fclose($pipes[0]);
$output = stream_get_contents($pipes[1]);
fclose($pipes[1]);
echo str_replace("\n","<br/>\n",$output);
if( proc_close($process) != 0 ) echo '<br/>Error onexecuting:<br/>'.$cmd;
echo '</td>';
echo '</tr>';
echo '</table>';

}
?>
