<?php

function display_cabinet()
{
if(isset($_SESSION['valid_user']) == false)
{
   echo 'Your are not logged in.';
   return;
}
$user_session = $_SESSION['valid_user'];

$action=$_GET['action'];
if( $action == '') $action = 'cabinet';
$operation=$_GET['operation'];

$dbconn = db_connect();
$query="SELECT count(*) FROM users WHERE name='$user_session' AND administrator=TRUE;";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
$admin = (int) $line["count"];
pg_free_result($result);

switch ($operation)
{
case '': break;
case 'changepassword':
   $password = $_POST['password'];
   $dbconn = db_connect();
   $query = "UPDATE users SET password='".sha1($password)."', administrator=TRUE WHERE name='$user_session';";
   $result = pg_query($query) or die('Query failed: ' . pg_last_error());
   pg_free_result($result);
   echo 'Password Changed.</br>';
   break;
case 'setnullpasswords':
   if( $admin == 0 ) break;
   $dbconn = db_connect();
   $query = "UPDATE users SET password='".sha1('')."', administrator=FALSE WHERE password  IS NULL;";
   $result = pg_query($query) or die('Query failed: ' . pg_last_error());
   pg_free_result($result);
   echo 'Empty Passwords Set To "".</br>';
   break;
default:
   echo "operation $operation is not supported.<br/>";
   break;
}
echo '<table>';

echo '<tr align="center"><td>';
echo '<h3>New Password<h3>';
echo "<form method='post' action='index.php?action=$action&operation=changepassword'><table>";
echo '<tr><td><input type="password" name="password"></td></tr>';
echo '<tr><td colspan="2" align="center"><input type="submit" value="Change"></td></tr>';
echo '</table></form>';
echo '</td></tr>';

if( $admin == 0)
{
   echo '</table>';
   return;
}

echo '<tr align="center"><td>Administration:</td></tr>';

echo '<tr align="center"><td>';
echo "<a href='index.php?action=$action&operation=setnullpasswords'>Set NULL Passwords.</a>";
echo '</td></tr>';

echo '</table>';
}
?>
