<?php

include ('fns_all.php');

class Afanasy
{

public function Display()
{
    $action=$_GET['action'];
    if( $action == '') $action = 'stat';

    $this->DisplayHeader();

    switch ($action)
    {
    case 'jobs':
        display_jobs();
        break;
    case 'task':
        display_task();
        break;
    case 'tasks':
        display_tasks();
        break;
    case 'users':
        display_users();
        break;
    case 'renders':
        display_renders();
        break;
    case 'user':
        display_user();
        break;
    case 'stat':
        display_stat();
        break;
    case 'stat_chart':
        display_stat_chart();
        break;
    case 'cabinet':
        display_cabinet();
        break;
    case 'login':
        $this->DisplayLogin();
        break;
    case 'logout':
        $this->DisplayLogout();
        break;
    case 'AddAdmin':
        $this->AddAdmin();
        break;
    case 'AddAdmin_register':
        $this->AddAdmin_register();
        break;
    default:
        echo "action '<b><i>$action</i></b>' is not supported.";
        break;
    }

    $this->DisplayFooter();
}

public function AddAdmin()
{
    $dbconn = db_connect();
    $query="SELECT name FROM users;";
    $result = pg_query($query) or die('Query failed: ' . pg_last_error());

    echo '<h3>AddAdmin<h3>';
    echo '<form method="post" action="index.php?action=AddAdmin_register">';
    echo '<table>';
    echo '<tr><td>User:</td>';
    echo '<td><select name="user">';
    while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
    {
        echo '<option value="';
        echo $line["name"];
        echo '">';
        echo $line["name"];
    }
    echo '</select></td>';
    echo '<tr><td>Password:</td>';
    echo '<td><input type="password" name="password"></td></tr>';
    echo '<tr><td colspan="2" align="center">';
    echo '<input type="submit" value="Registrer"></td></tr>';
    echo '</table></form>';
}

public function AddAdmin_register()
{
    if ( false == isset($_POST['user'])) continue;
    if ( false == isset($_POST['password'])) continue;

    $user = $_POST['user'];
    $password = $_POST['password'];
    $dbconn = db_connect();
    $query = "SELECT flags FROM users WHERE name='$user';";
    $result = pg_query($query) or die('Query failed: ' . pg_last_error());
    $line = pg_fetch_array( $result, null, PGSQL_ASSOC);
    $flags = $line["flags"];
    pg_free_result($result);
    $flags = userSetAdmin( $flags);
    $query = "UPDATE users SET password='".sha1($password)."', flags=$flags WHERE name='$user';";
    $result = pg_query($query) or die('Query failed: ' . pg_last_error());
    pg_free_result($result);
    echo "User '$user' is admin now.</br>";
}

public function DisplayLogin()
{
    $dbconn = db_connect();

    $query="SELECT COUNT(*) FROM users WHERE password <> '';";
    $result = pg_query($query) or die('Query failed: ' . pg_last_error());
    $line = pg_fetch_array( $result, null, PGSQL_ASSOC);
    $passcount = (int)($line['count']);
    pg_free_result($result);
    if( $passcount < 1)
    {
        $query="SELECT COUNT(*) FROM users;";
        $result = pg_query($query) or die('Query failed: ' . pg_last_error());
        $line = pg_fetch_array( $result, null, PGSQL_ASSOC);
        $userscount = (int)($line['count']);
        if( $userscount < 1)
        {
            echo '<h3>No users in database found. Login is impossible.</h3>';
        }
        else
        {
            echo '<h3>No users with password</h3>';
            echo '<p><a href="index.php?action=AddAdmin">create an administrator</a></p>';
        }
        pg_free_result($result);
        return;
    }

    echo '<h3>Login<h3>';
    echo '<form method="post" action="index.php">';
    echo '<table>';
    echo '<tr><td>User:</td>';
    echo '<td><input type="text" name="user">';
    echo '<tr><td>Password:</td>';
    echo '<td><input type="password" name="password"></td></tr>';
    echo '<tr><td colspan="2" align="center">';
    echo '<input type="submit" value="Login"></td></tr>';
    echo '</table></form>';
}

public function DisplayLogout()
{
    if($this->old_user!='')
    {
        echo 'Logged out <b> '.$this->old_user.' </b> <br />';
        $this->old_user='';
    }
    else
    {
        // if they weren't logged in but came to this page somehow
        echo 'You were not logged in, and so have not been logged out.<br />'; 
    }
}

private $old_user;
public function DisplayHeader()
{
    session_start();
    //echo "session_start() ";
    if( $_GET['action'] == 'logout')
    {
        $this->old_user = $_SESSION['valid_user'];
        unset($_SESSION['valid_user']);
        session_destroy();
    }
    else if (isset($_POST['user']) && isset($_POST['password']))
    {
        // if the user has just tried to log in
        $user = $_POST['user'];
        $password = $_POST['password'];

        $dbconn = db_connect();
        $query = "SELECT count(*) FROM users WHERE name='$user' AND password='".sha1($password)."';";
        $result = pg_query($query) or die('Query failed: ' . pg_last_error());
        $line = pg_fetch_array( $result, null, PGSQL_ASSOC);
        if((int)($line['count']))
        {
            // if they are in the database register the user id
            $_SESSION['valid_user'] = $user;
        }
    }
    ?>
    <html>
    <head>
    <title>Afanasy Web Visor</title>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <link rel="shortcut icon" href="../doc/images/icon.png" type="image/png">
    <link href="styles.css" rel="stylesheet" type="text/css">
    </head>
    <body>

    <table width="50%" align="center"><tr align="center">
        <td><img src="images/star01.png" alt="★" /></td>
        <td><img src="images/afanasy.png" border="0" alt="AFANASY" /></td>
        <td><img src="images/star02.png" alt="★" /></td>
    </tr></table>

    <table width="100%" align="center" cellpadding="0" cellspacing="0">
    <tr>
    <td width="200px">
    <div style="width:100%;height:40px;background:url('images/header_left.png')" />
    </td>
    <td>
    <div style="width:100%;height:40px;background:url('images/header.png') repeat-x" />
    </td>
    <td width="200px">
    <div style="width:100%;height:40px;background:url('images/header_right.png')" />
    </td>
    </tr>
    </table>

    <table width="90%" height="80%" border="1" align="center">
    <tr align="center" height="50">
    <td width=10%>
        <a href="../../" ><img src="images/cgru.png" alt="CGRU" border="0" width="100" height="40"/></a>
    </td>
    <td width=10%>
    <?php
    //echo 'user='.$_POST['user'].' password='.$_POST['password'].'</br>';
    if (isset($_SESSION['valid_user']))
        echo '<a href="index.php?action=logout"><img src="images/logout.png" alt="Logout" border="0"/></a>';
    else
        echo '<a href="index.php?action=login" ><img src="images/login.png"  alt="Login"  border="0"/></a>';
    ?>
    </td>
    <?php
    $action=$_GET['action'];
    if( $action == '') $action = 'stat';
    $row_actions = array( 0 => 'users', 1 => 'jobs', 2 => 'renders', 3 => 'stat');
    $row_images  = array( 0 => 'users.png', 1 => 'jobs.png', 2 => 'renders.png', 3 => 'statistics.png');
    for( $i = 0; $i < 4; $i++)
    {
        $current = 0;
        if( $action == $row_actions[$i]) $current = 1;

        echo '   ';
        echo '<td width=10%';
        if( $current ) echo ' style="background-color:#D0D080"';
        echo '>';
        if( false == $current )
        {
            echo '<a href="index.php?action=';
            echo $row_actions[$i];
            echo '">';
        }
        echo '<img src="images/';
        echo $row_images[$i];
        echo '" alt="';
        echo $row_images[$i];
        echo '" border="0"/>';
        if( false == $current ) echo '</a>';

        echo '</td>';
        echo "\n";
    }
    ?>
    </tr>
    <tr><td colspan=6 align=center><p>

    <?php
} // function DisplayHeader

public function DisplayFooter()
{
    ?>
    </p></td></tr>
    <tr align="center" height=50>
    <td colspan=6><p>
    <?php
        if (isset($_SESSION['valid_user']))
        {
            echo '<font color="#904010">★</font>';
            echo '<a href="index.php?action=cabinet"><b> '.$_SESSION['valid_user'].' </b></a>';
            echo '<font color="#904010">★</font>';
        }
        else
        {
            if (isset($user))
            {
                // if they've tried and failed to log in
                echo 'Invalid login';
            }
            else
            {
                // they have not tried to log in yet or have logged out
                echo ' (guest) ';
            }
        }
    ?>
    </p></td>
    </tr>
    </table>

    <table width="100%" align="center" cellpadding="0" cellspacing="0">
    <tr>
    <td width="200px">
       <div style="width:100%;height:40px;background:url('images/footer_left.png')" />
    </td>
    <td>
       <div style="width:100%;height:40px;background:url('images/footer.png') repeat-x" />
    </td>
    <td width="200px">
       <div style="width:100%;height:40px;background:url('images/footer_center.png')" />
    </td>
    <td>
       <div style="width:100%;height:40px;background:url('images/footer.png') repeat-x" />
    </td>
    <td width="200px">
       <div style="width:100%;height:40px;background:url('images/footer_right.png')" />
    </td>
    </tr>
    </table>

    </body>
    </html>
    <?php
} // function DisplayFooter

} // class Afanasy
?>
