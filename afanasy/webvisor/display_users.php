<?php

function display_users()
{
$action=$_GET['action'];
if( $action == '') $action = 'users';

$order=$_GET['order'];
switch($order)
{
case "name": break;
case "priority": break;
case "jobsnum": break;
case "jobsrunning": break;
default: $order = 'jobsnum';
}

$dbconn = db_connect();

$query="
SELECT   users.name,users.priority,
         sum(CASE WHEN jobs.id>0 THEN 1 ELSE 0 END) AS jobsnum,
         sum(CASE WHEN jobs.state&2>0 THEN 1 ELSE 0 END) AS jobsrunning,
         users.id
      FROM users LEFT JOIN jobs ON jobs.username=users.name
      GROUP BY users.name,users.id,users.priority
      ORDER BY $order DESC;";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());

echo "<table border=1 width=99%><caption>Known Users:</caption>\n";

echo "<tr align=center>\n";

echo "\t\t<td>";
echo '#';
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order=name'>Name</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order=priority'>Priority</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order=jobsnum'>Jobs Quantity</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order=jobsrunning'>Running Jobs</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo 'id';
echo "</td>\n";

echo "\t</tr>\n";

$count = 1;

if(isset($_SESSION['valid_user'])) $user = $_SESSION['valid_user'];

while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
{
   $boldOn  = '';
   $boldOff = '';
   $username = $line["name"];
   $uid = $line["id"];
   if( isset($user) && ($user==$username))
   {
      $boldOn  = '<b>';
      $boldOff = '</b>';
   }

   $style = 'empty';
   if( $line["jobsrunning"] > 0) $style = 'running';
   $style = ' class="state_'.$style.'"';

   echo "\t<tr$style>\n";

   echo "\t\t<td>$boldOn";
   echo $count++;
   echo "$boldOff</td>\n";

   echo "\t\t<td>$boldOn";
   echo "<a href='index.php?action=jobs&uid=$uid'>$username</a>";
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo $line["priority"];
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo $line["jobsnum"];
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo $line["jobsrunning"];
   echo "$boldOff</td>\n";

   echo "\t\t<td>$boldOn";
   echo $line["id"];
   echo "$boldOff</td>\n";

   echo "\t</tr>\n";
}
echo "</table>\n";

pg_free_result($result);
}

?>
