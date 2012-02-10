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
$query="SELECT count(*) FROM users WHERE name='$user_session' AND flags <> 0;";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
$admin = (int) $line["count"];
pg_free_result($result);

switch ($operation)
{
case '': break;

case 'changepassword':
    echo 'Changing password:<br/>';

    if( false == isset($_POST['password']))
    {
        echo 'Error: Password is not set.<br/>';
        break;
    }

    $password = $_POST['password'];

    if( $password == '')
    {
        echo 'Error: Password is empty.<br/>';
        break;
    }

    $dbconn = db_connect();
    $query = "UPDATE users SET password='".sha1($password)."' WHERE name='$user_session';";
    $result = pg_query($query) or die('Query failed: ' . pg_last_error());
    pg_free_result($result);
    echo 'Password Changed.</br>';
    break;

case 'setuserpassword':
    if( $admin == 0 ) break;

    echo 'Setting user password:<br/>';

    if( false == isset($_POST['user']))
    {
        echo 'Error: User is not set.<br/>';
        break;
    }
    if( false == isset($_POST['password']))
    {
        echo 'Error: Password is not set.<br/>';
        break;
    }

    $user = $_POST['user'];
    $password = $_POST['password'];

    if( $user == '')
    {
        echo 'Error: User empty.<br/>';
        break;
    }
    if( $password == '')
    {
        echo 'Error: Password is empty.<br/>';
        break;
    }

    $dbconn = db_connect();
    $query = "UPDATE users SET password='".sha1($password)."' WHERE name='$user';";
    $result = pg_query($query) or die('Query failed: ' . pg_last_error());
    pg_free_result($result);

    echo "User '$user' set.</br>\n";
    break;

default:
    echo "operation $operation is not supported.<br/>";
    break;
}
echo '<table>';

// Change password:
echo '<tr align="center"><td>';
echo '<h3>New Password<h3>';
echo "<form method='post' action='index.php?action=$action&operation=changepassword'><table>";
echo '<tr><td><input type="password" name="password"></td></tr>';
echo '<tr><td colspan="2" align="center"><input type="submit" value="Change"></td></tr>';
echo '</table></form>';
echo '</td></tr>';
echo '</table>';

if( $admin == 0 ) return;

echo "<br/>\n";

// Set selected user password:
$query="SELECT name FROM users;";
$result = pg_query($query) or die('Query failed: ' . pg_last_error());

echo '<table>';
echo '<tr align="center"><td><h2>Administration:</h2></td></tr>';
echo '<tr align="center"><td>';
echo "<form method='post' action='index.php?action=$action&operation=setuserpassword'><table>";
echo '<h3>Set User Password<h3>';
echo '<select name="user">';
while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
{
    echo '<option value="';
    echo $line["name"];
    echo '">';
    echo $line["name"];
}
echo '</select>';
echo '<input type="password" name="password">';
echo '<input type="submit" value="Apply">';
echo '</form>';
echo '</td></tr>';
echo '</table>';

pg_free_result($result);

}
?>
