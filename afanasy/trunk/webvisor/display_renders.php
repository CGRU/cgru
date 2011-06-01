<?php

function display_renders()
{
$action=$_GET['action'];
if( $action == '') $action = 'renders';

$order=$_GET['order'];
switch($order)
{
case "name": break;
case "priority": break;
case "username": break;
case "state": break;
case "capacity": break;
default: $order = 'priority';
}

$dbconn = db_connect();

$query = "SELECT name,id FROM users;";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
{
   $uname_uid[$line["name"]] = $line["id"];
   $uid_uname[$line["id"]] = $line["name"];
}
pg_free_result($result);

$query="SELECT * FROM renders ORDER BY $order DESC, name DESC;";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());

echo "<table border=1 width=99%><caption>Known Hosts:</caption>\n";

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
echo "<b><a href='index.php?action=$action&order=priority'>Capacity</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order=username'>User Name</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "<b><a href='index.php?action=$action&order=state'>State</a></b>";
echo "</td>\n";

echo "\t\t<td>";
echo "id";
echo "</td>\n";

echo "\t</tr>\n";

$SOnline = 1<<0;
$Snimby  = 1<<1;
$SNIMBY  = 1<<2;
$SBusy   = 1<<3;

$count = 1;

if(isset($_SESSION['valid_user'])) $user = $_SESSION['valid_user'];

while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
{
   $state = $line["state"];
   $style = renderStyle( $state);
   if( $style != '') $style = ' class="state_'.$style.'"';

   $boldOn  = '';
   $boldOff = '';
   $username = $line["username"];
   if( isset($user) && ($user==$username))
   {
      $boldOn  = '<b>';
      $boldOff = '</b>';
   }

   echo "\t<tr$style>\n";

   echo "\t\t<td>$boldOn";
   echo $count++;
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo $line["name"];
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo $line["priority"];
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo $line["capacity"];
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   if( isset($uname_uid[$username]))
      echo "<a href='index.php?action=jobs&uid=".$uname_uid[$username]."'>$username</a>";
   else echo "<i>$username</i>";
   if(renderIsNIMBY($state)) echo ' (NIMBY)';
   else if(renderIs_nimby($state)) echo ' (nimby)';
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   if(renderIsOnline($state))
   {
      if(renderIsBusy($state)) echo 'BUSY';
      else echo 'free';
   }
   else echo '<i>offline</i>';
   echo "$boldOff</td>\n";

   echo "\t\t<td align=center>$boldOn";
   echo $line["id"];
   echo "$boldOff</td>\n";

   echo "\t</tr>\n";
}
echo "</table>\n";

pg_free_result($result);
}

?>
