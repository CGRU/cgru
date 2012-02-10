<?php

function display_tasks()
{
$action=$_GET['action'];
if( $action == '') $action = 'tasks';

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

$sort = $_GET['sort'];
switch($sort)
{
    case "state": break;
    case "number": $sort = 'id_task'; break;
    case "starts": $sort = 'starts_count'; break;
    case "errors": $sort = 'errors_count'; break;
    case "started": $sort = 'time_started'; break;
    case "done": $sort = 'time_done'; break;
    case "run": $sort = 'duration'; break;
    default: $sort = 'id_task';
}

$order = (int)$_GET['order'];
if( $order == 0) $order_str = 'ASC';
else
{
    $order_str = 'DESC';
    $order = 1;
}
$sort = " ORDER BY $sort $order_str";

// Connect to database:
$dbconn = db_connect();

// Get job from database:
$query='SELECT * FROM jobs WHERE id='.$jid_str.';';
$resjob = pg_query($query) or die('Query failed: ' . pg_last_error());
$linejob = pg_fetch_array( $resjob, null, PGSQL_ASSOC);

echo '<h3><a href="index.php?action=jobs&user='.$linejob["username"].'">Goto user <b><i>\''.$linejob["username"].'\'</i></b> jobs.</a></h3>';

// Display job information:
echo '<p>';
echo 'Job['.$jid_str.'] = "<b>'.$linejob["name"].'</b>":';
echo '<br/>';
echo '</p>';
pg_free_result($resjob);


// Get job blocks from database:
$query='SELECT * FROM blocks WHERE id_job='.$jid_str.' ORDER BY id_block;';
//echo $query.'<br/>';
$resblocks = pg_query($query) or die('Query failed: ' . pg_last_error());
while( $lineblock = pg_fetch_array( $resblocks, null, PGSQL_ASSOC))
{
    // Display block information:
    $bid_str = (string)$lineblock["id_block"];
    echo '<p>';
    echo 'Block['.$bid_str.']:';
    echo ' name="<b>'.$lineblock["name"].'</b>"';
    echo '<br/>Command = "<b>'.$lineblock["command"].'</b>"';
    echo '<br/>Files = "<b>'.$lineblock["files"].'</b>"';
    echo '<br/>Working Directory = "<b>'.$lineblock["wdir"].'</b>"';
    if($lineblock["cmd_view"] != '') echo '<br/>View="<b>'.$lineblock["cmd_view"].'</b>"';
    echo '</p>';

$query = '
SELECT starts_count,errors_count,state,time_started,time_done,id_block,id_task,
sum(time_done-time_started) AS duration FROM progress WHERE id_job='.$jid_str.' AND id_block='.$bid_str.' GROUP BY
progress.starts_count,progress.errors_count,progress.state,progress.time_started,progress.time_done,progress.id_block,progress.id_task
'.$sort.';';

    $resprogress = pg_query($query) or die('Query failed: ' . pg_last_error());

    echo "<table width=\"99%\" border=\"1\">\n";
    echo "<tr>\n";

    echo "\t\t<td>";
    echo "<b><a href='index.php?action=$action&jid=$jid&sort=number&order=".(1-$order)."'>#</a></b>";
    echo "</td>\n";

    echo "\t\t<td>";
    echo "<b><a href='index.php?action=$action&jid=$jid&sort=starts&order=".(1-$order)."'>Starts</a></b>";
    echo "</td>\n";

    echo "\t\t<td>";
    echo "<b><a href='index.php?action=$action&jid=$jid&sort=started&order=".(1-$order)."'>Started Time</a></b>";
    echo "</td>\n";

    echo "\t\t<td>";
    echo "<b><a href='index.php?action=$action&jid=$jid&sort=done&order=".(1-$order)."'>Done Time</a></b>";
    echo "</td>\n";

    echo "\t\t<td>";
    echo "<b><a href='index.php?action=$action&jid=$jid&sort=errors&order=".(1-$order)."'>Errors</a></b>";
    echo "</td>\n";

    echo "\t\t<td>";
    echo "<b><a href='index.php?action=$action&jid=$jid&sort=state&order=".(1-$order)."'>State</a></b>";
    echo "</td>\n";

    echo "\t\t<td>";
    echo "<b><a href='index.php?action=$action&jid=$jid&sort=run&order=".(1-$order)."'>Runing Time</a></b>";
    echo "</td>\n";

    echo "</tr>\n";

    // Display tasks information:
    $tasks_count = 0;
    $done_count = 0;
    $sumruntime = 0;
    while( $linetask = pg_fetch_array( $resprogress, null, PGSQL_ASSOC))
    {
        $state = $linetask["state"];
        $style = stateToStyle( $state);
        if( $style != '') $style = ' class="state_'.$style.'"';

        echo "<tr$style>\n";

        $time_started = $linetask["time_started"];
        $time_done = $linetask["time_done"];
        $time_run = $linetask["duration"];

        if( $_SESSION['valid_user'])
        {
            // Show task number with a link to task for registered users only:
            echo "\t<td><a href=\"index.php?action=task&jid=".$jid_str."&bid=".$bid_str."&tid=".$linetask["id_task"]."\">";
            echo $linetask["id_task"]."</a></td>\n";
        }
        else
        {
            echo "\t<td>".$linetask["id_task"]."</td>\n";
        }

        echo "\t<td>".$linetask["starts_count"]."</td>\n";

        echo "\t<td>";
        if( $time_started) echo date('D, j M Y G:i:s', $time_started);
        else echo '-';
        echo "</td>\n";

        echo "\t<td>";
        if( $time_done && stateIsDone($state)) echo date('D, j M Y G:i:s', $time_done);
        else echo '-';
        echo "</td>\n";

        echo "\t<td>".$linetask["errors_count"]."</td>\n";

        echo "\t<td>";
        if( stateIsError($state)) echo '<b>ERROR</b>';
        else if( stateIsRunning($state)) echo '<i>running</i>';
        else if( stateIsSkipped($state)) echo 'skipped';
        else if( stateIsDone($state)) echo 'DONE';
        else echo '-';
        echo "</td>\n";

        echo "\t<td>";
        if( stateIsDone($state))
        {
            if( $time_started && $time_done)
            {
                echo time_strHMS($time_run);
                $sumruntime += $time_run;
            }
            else echo '-';
            $done_count++;
        }
        else echo '-';
        echo "</td>\n";

        echo "</tr>\n";
        $tasks_count++;
    }
    echo "</table>\n";

    echo "Tasks Count = <b>$tasks_count</b>";
    echo ", Total Running Time = <b>".time_strHMS($sumruntime)."</b>";
    if( $done_count > 0) echo ", Average Task Running Time = <b>".time_strHMS($sumruntime/$done_count)."</b>\n";
}
pg_free_result($resblocks);
}
?>
