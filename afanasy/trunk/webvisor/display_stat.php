<?php

function display_stat()
{
$action=$_GET['action'];
if( $action == '') $action = 'stat';

$order_u=$_GET['order_u'];
switch($order_u)
{
case "username": break;
case "numjobs": break;
case "sumonlinetime": break;
case "avgonlinetime": break;
case "sumruntime": break;
case "avgruntime": break;
default: $order_u = 'sumruntime';
}

$order_s=$_GET['order_s'];
switch($order_s)
{
case "service": break;
case "servicequantity": break;
case "tasksquantity": break;
case "taskssumruntime": break;
case "tasksavgruntime": break;
case "tasksdone": break;
default: $order_s = 'taskssumruntime';
}

echo '<h3>Users:</h3>';

$dbconn = db_connect();

$query="
SELECT username,
sum(1) AS numjobs,
sum(time_done-time_started) AS sumonlinetime,
avg(time_done-time_started) AS avgonlinetime,
sum(taskssumruntime) AS sumruntime,
avg(taskssumruntime) AS avgruntime
FROM statistics GROUP BY username ORDER BY $order_u DESC;
";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
echo "<table border=1 width=99%>\n";

echo "<tr align=center>\n";

echo "\t\t<td>";
echo '#';
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=username&order_s=$order_s'>User Name</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=numjobs&order_s=$order_s'>Num Jobs</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=sumonlinetime&order_s=$order_s'>Sum Online Time</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=avgonlinetime&order_s=$order_s'>Avg Online Time</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=sumruntime&order_s=$order_s'>Sum Run Time</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=avgruntime&order_s=$order_s'>Avg Run Time</a></b>";
echo "</td>\n";

echo "\t</tr>\n";

$count = 1;

if(isset($_SESSION['valid_user'])) $user = $_SESSION['valid_user'];

while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
{
   $boldOn  = '';
   $boldOff = '';
   $username = $line["username"];
   if( isset($user) && ($user==$username))
   {
      $boldOn  = '<b>';
      $boldOff = '</b>';
   }

   echo "\t<tr>\n";

   echo "\t\t<td>$boldOn";
   echo $count++;
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo $username;
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo $line["numjobs"];
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo time_strDHMS($line["sumonlinetime"]);
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo time_strHMS($line["avgonlinetime"]);
   echo "$boldOff</td>\n$boldOff";

   echo "\t\t<td align=center>$boldOn";
   echo time_strDHMS($line["sumruntime"]);
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo time_strHMS($line["avgruntime"]);
   echo "$boldOff</td>\n$boldOff";

   echo "\t</tr>\n";
}
pg_free_result($result);

echo '<tr align=center>';
echo "<td></td>";
echo "<td></td>";
echo "<td><a href='index.php?action=stat_chart&type=jobsnum'>Chart</a></td>";
echo "<td><a href='index.php?action=stat_chart&type=jobssumonlinetime'>Chart</a></td>";
echo "<td><a href='index.php?action=stat_chart&type=jobsavgonlinetime'>Chart</a></td>";
echo "<td><a href='index.php?action=stat_chart&type=jobssumruntime'>Chart</a></td>";
echo "<td><a href='index.php?action=stat_chart&type=jobsavgruntime'>Chart</a></td>";
echo '</tr>';

$query="
SELECT
sum(1) AS numjobs,
sum(time_done-time_started) AS sumonlinetime,
avg(time_done-time_started) AS avgonlinetime,
sum(taskssumruntime) AS sumruntime,
avg(taskssumruntime) AS avgruntime
FROM statistics;
";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
echo '<tr align=center>';
echo "<td></td>";
echo "<td><i>total</i></td>";
echo "<td><i>".$line["numjobs"]."</i></td>";
echo "<td><i>".time_strDHMS($line["sumonlinetime"])."</i></td>";
echo "<td><i>".time_strHMS($line["avgonlinetime"])."</i></td>";
echo "<td><i>".time_strDHMS($line["sumruntime"])."</i></td>";
echo "<td><i>".time_strHMS($line["avgruntime"])."</i></td>";
echo '</tr>';
pg_free_result($result);

echo "</table>\n";


echo '<h3>Services:</h3>';
//CASE WHEN jobs.id>0 THEN 1 ELSE 0 END
//sum(tasksdone) AS tasksdone
$query="
SELECT service,
sum(1) AS servicequantity,
sum(tasksnum) AS tasksquantity,
sum(taskssumruntime) AS taskssumruntime,
avg(CASE WHEN tasksdone>0 THEN taskssumruntime/tasksdone ELSE 0 END) AS tasksavgruntime,
round(avg(100*tasksdone/tasksnum),2) AS tasksdone
FROM statistics GROUP BY service ORDER BY $order_s DESC;
";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
echo "<table border=1 width=99%>\n";

echo "<tr align=center>\n";

echo "\t\t<td>";
echo '#';
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=$order_u&order_s=service'>Service</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=$order_u&order_s=servicequantity'>Service Quantity</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=$order_u&order_s=tasksquantity'>Tasks Quantity</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=$order_u&order_s=taskssumruntime'>Sum Run Time</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=$order_u&order_s=tasksavgruntime'>Avg Run Time</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order_u=$order_u&order_s=tasksdone'>Done</a></b>";
echo "</td>\n";

echo "\t</tr>\n";

$count = 1;

while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
{
   echo "\t<tr>\n";

   echo "\t\t<td>";
   echo $count++;
   echo "</td>\n";

   echo "\t\t<td>";
   echo $line["service"];
   echo "</td>\n";

   echo "\t\t<td align=center>";
   echo $line["servicequantity"];
   echo "</td>\n";

   echo "\t\t<td align=center>";
   echo $line["tasksquantity"];
   echo "</td>\n";

   echo "\t\t<td align=center>";
   echo time_strDHMS($line["taskssumruntime"]);
   echo "</td>\n";

   echo "\t\t<td align=center>";
   echo time_strHMS($line["tasksavgruntime"]);
   echo "</td>\n";

   echo "\t\t<td align=center>";
   echo $line["tasksdone"]."%";
   echo "</td>\n";

   echo "\t</tr>\n";
}
pg_free_result($result);

echo '<tr align=center>';
echo "<td/>";
echo "<td></td>";
echo "<td><a href='index.php?action=stat_chart&type=servicequantity'>Chart</a></td>";
echo "<td><a href='index.php?action=stat_chart&type=tasksquantity'>Chart</a></td>";
echo "<td><a href='index.php?action=stat_chart&type=taskssumruntime'>Chart</a></td>";
echo "<td><a href='index.php?action=stat_chart&type=tasksavgruntime'>Chart</a></td>";
echo "<td><a href='index.php?action=stat_chart&type=tasksdone'>Chart</a></td>";
echo '</tr>';

$query="
SELECT
sum(1) AS servicequantity,
sum(tasksnum) AS tasksquantity,
sum(taskssumruntime) AS taskssumruntime,
avg(CASE WHEN tasksdone>0 THEN taskssumruntime/tasksdone ELSE 0 END) AS tasksavgruntime,
round(avg(100*tasksdone/tasksnum),2) AS tasksdone
FROM statistics;
";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
echo '<tr align=center>';
echo "<td/>";
echo "<td><i>total</i></td>";
echo "<td><i>".$line["servicequantity"]."</i></td>";
echo "<td><i>".$line["tasksquantity"]."</i></td>";
echo "<td><i>".time_strDHMS($line["taskssumruntime"])."</i></td>";
echo "<td><i>".time_strHMS($line["tasksavgruntime"])."</i></td>";
echo "<td><i>".$line["tasksdone"]."%</i></td>";
echo '</tr>';
$taskssumruntime = $line["taskssumruntime"];
pg_free_result($result);

echo "</table>\n";

// Find the earliest record to display statistics start time:
$query="
SELECT min(time_started) AS time_started FROM statistics WHERE time_started > 0;";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
echo '<p><b><i>First job date: '.date( 'j F Y', $line["time_started"]).'</i></b></p>';
echo '<p><i>Farm average usage = '.sprintf("%01.2f",$taskssumruntime/(time()-$line["time_started"])).' hosts</i></p>';
pg_free_result($result);
}

?>
