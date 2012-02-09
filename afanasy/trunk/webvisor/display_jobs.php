<?php

function display_jobs()
{
$action = $_GET['action'];
if( $action == '') $action = 'jobs';

$uid = $_GET['uid'];
$uid = (int)$uid;

$sort = $_GET['sort'];
switch($sort)
{
case "name": break;
case "user": $sort = 'username'; break;
case "done": $sort = 'time_done'; break;
case "duration": break;
case "state": break;
default: $sort = 'time_creation';
}

$order = (int)$_GET['order'];
if( $order == 0) $order_str = 'DESC';
else
{
   $order_str = 'ASC';
   $order = 1;
}
$sort = " ORDER BY $sort $order_str";

$dbconn = db_connect();

$query = "SELECT name,id FROM users;";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
{
    $uname_uid[$line["name"]] = $line["id"];
    $uid_uname[$line["id"]] = $line["name"];
}
if( isset($_GET["user"]))
{
    $user = $_GET["user"];
    $uidstr = " WHERE username='$user'";
    $uid = $uname_uid[$user];
}
else if( $uid > 0 )
{
    if( isset($uid_uname[$uid]))
        $uidstr = " WHERE username='$uid_uname[$uid]'";
    else
    {
        $uidstr = '';
        $uid = 0;
    }
}
else $uidstr = '';

$query = '
SELECT name,username,state,time_creation,time_started,time_done,time_wait,id,
sum(time_done-time_started) AS duration FROM jobs'.$uidstr.' GROUP BY
 jobs.name,jobs.username,jobs.state,jobs.time_creation,jobs.time_started,jobs.time_done,jobs.time_wait,jobs.id
'.$sort.';';

$result = pg_query($query) or die('Query failed: ' . pg_last_error());

echo "<table border=1 width=99%><caption>Current Jobs:</caption>\n";

echo "<tr align=center>\n";

echo "\t\t<td>";
echo '#';
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&uid=$uid&sort=name&order=".(1-$order)."'>Name</a></b>";
echo "</td>\n";

if( $uid < 1)
{
   echo "\t\t<td>";
   echo "<b><a href='index.php?action=$action&uid=$uid&sort=user&order=".(1-$order)."'>User</a></b>";
   echo "</td>\n";
}

//echo "\t\t<td>";
//echo "<b><a href='index.php?action=$action&uid=$uid&sort=created&order=".(1-$order)."'>Time Created</a></b>";
//echo "</td>\n";

//echo "\t\t<td>";
//echo "<b><a href='index.php?action=$action&uid=$uid&sort=started&order=".(1-$order)."'>Time Started</a></b>";
//echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&uid=$uid&sort=done&order=".(1-$order)."'>Time Done</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&uid=$uid&sort=duration&order=".(1-$order)."'>Duration</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&uid=$uid&sort=state&order=".(1-$order)."'>State</a></b>";
echo "</td>\n";

//echo "\t\t<td>";
//echo 'Wait';
//echo "</td>\n";

echo "\t\t<td>";
echo 'id';
echo "</td>\n";

echo "\t</tr>\n";

$count = 1;

if(isset($_SESSION['valid_user'])) $user_session = $_SESSION['valid_user'];

$timeformat = 'M j D G:i';

while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
{
   $state = $line["state"];
   $state_str = stateToStr( $state);
   $style = stateToStyle( $state);
   $time_started = $line["time_started"];
   $time_done = $line["time_done"];
   if( $style != '') $style = ' class="state_'.$style.'"';

   $markOn  = '';
   $markOff = '';
   $username = $line["username"];
   if( isset($user_session) && ($user_session == $username) && ($uid < 1))
   {
      $markOn  = '<b>';
      $markOff = '</b>';
   }
   if( stateIsRunning( $state))
   {
      $markOn  = $markOn.'<i>';
      $markOff = '</i>'.$markOff;
   }

   echo "\t<tr$style>\n";

   echo "\t\t<td>$markOn";
   echo $count++;
   echo "$markOff</td>\n";

   echo "\t\t<td>$markOn";
   echo "<a href='index.php?action=tasks&jid=".$line["id"]."'>".$line["name"]."</a>";
   echo "$markOff</td>\n";

   if( $uid < 1)
   {
      echo "\t\t<td>$markOn";
      if( isset($uname_uid[$username]))
         echo "<a href='index.php?action=$action&uid=".$uname_uid[$username]."'>$username</a>";
      else echo "<i>$username</i>";
      echo "$markOff</td>\n";
   }

//   echo "\t\t<td align=center>$markOn";
//   echo date( $timeformat, $line["time_creation"]);
//   echo "$markOff</td>\n";

//   echo "\t\t<td align=center>$markOn";
//   if( $time_started )
//      echo date( $timeformat, $time_started);
//   else
//      echo 'not started';
//   echo "$markOff</td>\n";

   echo "\t\t<td align=center>$markOn";
   if( stateIsDone($state) )
      echo date( $timeformat, $time_done);
   else if( $time_started) echo 'running';
   else echo 'not started';
//   echo '<br/>'.date( $timeformat, $time_started);
   echo "$markOff</td>\n";

   echo "\t\t<td align=center>$markOn";
   if( stateIsDone($state)) echo time_strHMS($line["duration"]);
   else if( $time_started ) echo time_strHMS( time() - $time_started);
   else echo '-';
   echo "$markOff</td>\n";

   echo "\t\t<td>$markOn";
   echo $state_str;
   echo "$markOff</td>\n";

//   echo "\t\t<td>$markOn";
//   if( stateIsWaittime($state)) echo time_strHMS( $line["time_wait"] - time());
//   echo "$markOff</td>\n";

   echo "\t\t<td>$markOn";
   echo $line["id"];
   echo "$markOff</td>\n";

   echo "\t</tr>\n";
}
echo "</table>\n";

pg_free_result($result);
}

?>
