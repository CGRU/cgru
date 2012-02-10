<?php

function display_task()
{

if( false == $_SESSION['valid_user'])
{
    echo 'Only registered users can watch tasks.';
    return;
}

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
echo '<h3><a href="index.php?action=tasks&jid='.$jid_str.'">Goto Job</a></h3>';
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

$afcmd_dir = dirname( getcwd()).'/bin';
$afcmd = $afcmd_dir.'/afcmd';

// Restart task request:
if( isset($_POST['taskaction']))
{
    if($_POST['taskaction'] == 'restart')
    {
        echo launchAfCmd('trestart '.$jid_str.' '.$bid_str.' '.$tid_str);
        echo '<h2>Restarting Task...</h2>';
        echo '<h3><a href="index.php?action=task&jid='.$jid.'&bid='.$bid.'&tid='.$tid.'">Refresh</a></h3>';
        return;
    }
}

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
    echo launchAfCmd('tlog '.$jid_str.' '.$bid_str.' '.$tid_str.' 0');
echo '</td>';
echo '</tr>';
echo '</table>';

echo '<h3>Task Last Session Output</h3>';
echo '<table border="1">';
echo '<tr>';
echo '<td style="padding:10px">';
echo launchAfCmd('tout '.$jid_str.' '.$bid_str.' '.$tid_str.' 0');
echo '</td>';
echo '</tr>';
echo '</table>';

// Restart task form:
echo "\n<br/>\n";
echo '<form method="post" action="index.php?action=task&jid='.$jid.'&bid='.$bid.'&tid='.$tid.'">';
echo '<input type="text" hidden=1 name="taskaction" value="restart">';
echo '<input type="submit" value="Restart Task"/>';
echo '</form>';

}
?>
