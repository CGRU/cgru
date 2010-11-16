<?php

function db_connect()
{
   return pg_connect('host=afanasy dbname=afanasy user=afadmin password=AfPassword') or die('Could not connect: ' . pg_last_error());
}

?>
