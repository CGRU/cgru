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
case "usertasksnum": break;
case "usertasksavg": break;
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
case "tasksquantityavg": break;
case "taskssumruntime": break;
case "tasksavgruntime": break;
case "tasksdone": break;
default: $order_s = 'taskssumruntime';
}

$dbconn = db_connect();

$time_min=$_GET['time_min'];
$time_max=$_GET['time_max'];
$query="SELECT min(time_done) AS time_done FROM statistics WHERE time_done > 0;";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
$time_begin = $line["time_done"];
pg_free_result($result);
if( $time_begin == '') $time_begin = 0;

if( $time_min != '') $time_min = strtotime($time_min);
else $time_min = $time_begin;
if( $time_max != '') $time_max = strtotime($time_max);
else $time_max = time();

echo '<form method="get" action="index.php">';
echo "<br/>\n";
echo '<table align="center" width="99%"><tr align="center"><td>';
echo 'From Date: ';
echo '<input type="text" name="time_min" value="'.date( 'Y-m-j', $time_min).'">';
#echo '<br/>time_min='.$time_min;
echo '</td><td>';
echo 'To Date: ';
echo '<input type="text" name="time_max" value="'.date( 'Y-m-j', $time_max).'">';
#echo '<br/>time_max='.$time_max;
echo '</td><td><input type="submit" value="Refresh Dates"/>';
echo '<td></tr></table>';
echo '</form>';

echo '<h3>Users:</h3>';

$query="
SELECT username,
 sum(1) AS numjobs,
 sum(taskssumruntime) AS sumruntime,
 avg(taskssumruntime) AS avgruntime,
 sum(tasksnum) AS usertasksnum,
 avg(tasksnum) AS usertasksavg
 FROM statistics
 WHERE time_done BETWEEN $time_min and $time_max
 GROUP BY username ORDER BY $order_u DESC;
";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
echo "<table border=1 width=99%>\n";

echo "<tr align=center>\n";

echo "\t\t<td>";
echo '#';
echo "</td>\n";

echo "\t\t<td>";
if( $order_u == 'username' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=username&order_s=$order_s'>User Name</a>";
if( $order_u == 'username' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_u == 'numjobs' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=numjobs&order_s=$order_s'>Job Blocks<br/>Quantity</a>";
if( $order_u == 'numjobs' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_u == 'sumruntime' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=sumruntime&order_s=$order_s'>Sum Run Time</a>";
if( $order_u == 'sumruntime' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_u == 'avgruntime' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=avgruntime&order_s=$order_s'>Average<br/>Run Time</a>";
if( $order_u == 'avgruntime' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_u == 'usertasksnum' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=usertasksnum&order_s=$order_s'>Task<br/>Quantity</a>";
if( $order_u == 'usertasksnum' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_u == 'usertasksavg' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=usertasksavg&order_s=$order_s'>Average<br/>Quantity</a>";
if( $order_u == 'usertasksavg' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
echo "<b>Favorite Service</b>";
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
   echo time_strDHMS($line["sumruntime"]);
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo time_strHMS($line["avgruntime"]);
   echo "$boldOff</td>\n$boldOff";

   echo "\t\t<td align=center>$boldOn";
   echo $line["usertasksnum"];
   echo "$boldOff</td>\n$boldOff";

   echo "\t\t<td align=center>$boldOn";
   echo sprintf("%01.0f", $line["usertasksavg"]);
   echo "$boldOff</td>\n$boldOff";

   # Get user favorite service:
   $sub_query="
   SELECT service,
   sum(taskssumruntime) AS sumruntime
   FROM statistics
   WHERE username='".$username."'
   AND time_done BETWEEN $time_min and $time_max
   GROUP BY service ORDER BY sumruntime DESC;
   ";
   $sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
   $sub_total = 0;
   $sub_favourite = 0;
   $sub_name = 0;
   while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
   {
      if( $sub_favourite < $sub_line["sumruntime"])
      {
         $sub_favourite = $sub_line["sumruntime"];
         $sub_name = $sub_line["service"];
      }
      $sub_total += $sub_line["sumruntime"];
   }
   pg_free_result($sub_result);
   echo "\t\t<td align=center>$boldOn";
   echo $sub_name.": ".sprintf("%01.0f%%", $sub_favourite/$sub_total * 100);
   echo "$boldOff</td>\n$boldOff";

   echo "\t</tr>\n";
}
pg_free_result($result);

echo "<tr align=center>\n";
echo "<td></td>\n";
echo "<td></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=jobsnum'>Chart</a></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=jobssumruntime'>Chart</a></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=jobsavgruntime'>Chart</a></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=usertasksnum'>Chart</a></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=usertasksavg'>Chart</a></td>\n";
echo "</tr>\n";

$query="
SELECT
 sum(1) AS numjobs,
 sum(taskssumruntime) AS sumruntime,
 avg(taskssumruntime) AS avgruntime,
 sum(tasksnum) AS usertasksnum,
 avg(tasksnum) AS usertasksavg
 FROM statistics
 WHERE time_done BETWEEN $time_min and $time_max
;
";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
echo "<tr align=center>\n";
echo "<td></td>\n";
echo "<td><i>total</i></td>\n";
echo "<td><i>".$line["numjobs"]."</i></td>\n";
echo "<td><i>".time_strDHMS($line["sumruntime"])."</i></td>\n";
echo "<td><i>".time_strHMS($line["avgruntime"])."</i></td>\n";
echo "<td><i>".$line["usertasksnum"]."</i></td>\n";
echo "<td><i>".sprintf("%01.0f", $line["usertasksavg"])."</i></td>\n";
pg_free_result($result);

# Get favorite service:
$sub_query="
SELECT service,
 sum(taskssumruntime) AS sumruntime
 FROM statistics
 WHERE time_done BETWEEN $time_min and $time_max
 GROUP BY service ORDER BY sumruntime DESC;
";
$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
$sub_total = 0;
$sub_favourite = 0;
$sub_name = 0;
while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
{
   if( $sub_favourite < $sub_line["sumruntime"])
   {
      $sub_favourite = $sub_line["sumruntime"];
      $sub_name = $sub_line["service"];
   }
   $sub_total += $sub_line["sumruntime"];
}
pg_free_result($sub_result);
echo "<td>".$sub_name.": ".sprintf("%01.0f%%", $sub_favourite/$sub_total * 100)."</td>\n";

echo "</tr>\n";
echo "</table>\n";


echo '<h3>Services:</h3>';
//CASE WHEN jobs.id>0 THEN 1 ELSE 0 END
//sum(tasksdone) AS tasksdone
$query="
SELECT service,
 sum(1) AS servicequantity,
 sum(tasksnum) AS tasksquantity,
 sum(tasksnum)/sum(1) AS tasksquantityavg,
 sum(taskssumruntime) AS taskssumruntime,
 avg(CASE WHEN tasksdone>0 THEN taskssumruntime/tasksdone ELSE 0 END) AS tasksavgruntime,
 round(avg(100*tasksdone/tasksnum),2) AS tasksdone
 FROM statistics
 WHERE time_done BETWEEN $time_min and $time_max
 GROUP BY service ORDER BY $order_s DESC;
";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
echo "<table border=1 width=99%>\n";

echo "<tr align=center>\n";

echo "\t\t<td>";
echo '#';
echo "</td>\n";

echo "\t\t<td>";
if( $order_s == 'service' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=$order_u&order_s=service'>Service</a>";
if( $order_s == 'service' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_s == 'servicequantity' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=$order_u&order_s=servicequantity'>Service<br/>Quantity</a>";
if( $order_s == 'servicequantity' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_s == 'tasksquantity' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=$order_u&order_s=tasksquantity'>Tasks<br/>Quantity</a>";
if( $order_s == 'tasksquantity' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_s == 'tasksquantityavg' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=$order_u&order_s=tasksquantityavg'>Average<br/>Quantity</a>";
if( $order_s == 'tasksquantityavg' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_s == 'taskssumruntime' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=$order_u&order_s=taskssumruntime'>Sum Run Time</a>";
if( $order_s == 'taskssumruntime' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_s == 'tasksavgruntime' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=$order_u&order_s=tasksavgruntime'>Average<br/>Run Time</a>";
if( $order_s == 'tasksavgruntime' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
if( $order_s == 'tasksdone' ) echo '<b>';
echo "<a href='index.php?action=$action&order_u=$order_u&order_s=tasksdone'>Done</a>";
if( $order_s == 'tasksdone' ) echo '</b>';
echo "</td>\n";

echo "\t\t<td>";
echo "<b>Favorite User</b>";
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
   echo sprintf("%01.0f", $line["tasksquantityavg"]);
   echo "</td>\n";

   echo "\t\t<td align=center>";
   echo time_strDHMS($line["taskssumruntime"]);
   echo "</td>\n";

   echo "\t\t<td align=center>";
   echo time_strHMS($line["tasksavgruntime"]);
   echo "</td>\n";

   echo "\t\t<td align=center>";
   echo sprintf("%01.0f%%", $line["tasksdone"]);
   echo "</td>\n";

   # Get service favorite user:
   $sub_query="
   SELECT username,
   sum(taskssumruntime) AS sumruntime
   FROM statistics
   WHERE service='".$line["service"]."'
   AND time_done BETWEEN $time_min and $time_max
   GROUP BY username ORDER BY sumruntime DESC;
   ";
   $sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
   $sub_total = 0;
   $sub_favourite = 0;
   $sub_name = 0;
   while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
   {
      if( $sub_favourite < $sub_line["sumruntime"])
      {
         $sub_favourite = $sub_line["sumruntime"];
         $sub_name = $sub_line["username"];
      }
      $sub_total += $sub_line["sumruntime"];
   }
   pg_free_result($sub_result);
   echo "\t\t<td align=center>";
   echo $sub_name.": ".sprintf("%01.0f%%", $sub_favourite/$sub_total * 100);
   echo "</td>\n";

   echo "\t</tr>\n";
}
pg_free_result($result);

echo "<tr align=center>\n";
echo "<td/>\n";
echo "<td></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=servicequantity'>Chart</a></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=tasksquantity'>Chart</a></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=tasksquantityavg'>Chart</a></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=taskssumruntime'>Chart</a></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=tasksavgruntime'>Chart</a></td>\n";
echo "<td><a href='index.php?action=stat_chart&type=tasksdone'>Chart</a></td>\n";
echo "</tr>\n";

$query="
SELECT
 sum(1) AS servicequantity,
 sum(tasksnum) AS tasksquantity,
 sum(tasksnum)/sum(1) AS tasksquantityavg,
 sum(taskssumruntime) AS taskssumruntime,
 avg(CASE WHEN tasksdone>0 THEN taskssumruntime/tasksdone ELSE 0 END) AS tasksavgruntime,
 round(avg(100*tasksdone/tasksnum),2) AS tasksdone
 FROM statistics
 WHERE time_done BETWEEN $time_min and $time_max;
";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
echo "<tr align=center>\n";
echo "<td/>\n";
echo "<td><i>total</i></td>\n";
echo "<td><i>".$line["servicequantity"]."</i></td>\n";
echo "<td><i>".$line["tasksquantity"]."</i></td>\n";
echo "<td><i>".$line["tasksquantityavg"]."</i></td>\n";
echo "<td><i>".time_strDHMS($line["taskssumruntime"])."</i></td>\n";
echo "<td><i>".time_strHMS($line["tasksavgruntime"])."</i></td>\n";
echo "<td><i>".sprintf("%01.0f", $line["tasksdone"])."%</i></td>\n";


# Get favorite user:
$sub_query="
SELECT username,
 sum(taskssumruntime) AS sumruntime
 FROM statistics
 WHERE time_done BETWEEN $time_min and $time_max
 GROUP BY username ORDER BY sumruntime DESC;
";
$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
$sub_total = 0;
$sub_favourite = 0;
$sub_name = 0;
while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
{
   if( $sub_favourite < $sub_line["sumruntime"])
   {
      $sub_favourite = $sub_line["sumruntime"];
      $sub_name = $sub_line["username"];
   }
   $sub_total += $sub_line["sumruntime"];
}
pg_free_result($sub_result);
echo "\t\t<td align=center>";
echo $sub_name.": ".sprintf("%01.0f%%", $sub_favourite/$sub_total * 100);
echo "</td>\n";


echo "</tr>\n";
$taskssumruntime = $line["taskssumruntime"];
pg_free_result($result);

echo "</table>\n";

echo "<br/>\n";

// Find the earliest statistics record day and average tasks number:
echo "<table width=99%>\n";
echo "<tr align=center>\n";
echo "<td width=\"50%\">\n";
echo '<p><i>Average farm usage = '.sprintf("%01.2f",$taskssumruntime/($time_max-$time_min)).' tasks</i></p>';
echo "</td>\n";
echo "<td width=\"50%\">\n";
if( $time_begin != 0 ) echo '<p><i>The earliest statistics date: '.date( 'j F Y', $time_begin).'</i></p>';
echo "</td width=\"50%\">\n";
echo "</tr>\n";
echo "</table>\n";
}

?>
