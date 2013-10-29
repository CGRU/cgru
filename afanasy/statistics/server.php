<?php

$Out = array();
$Recv = array();

# Decode input:
$Recv = json_decode( $HTTP_RAW_POST_DATA, true);
if( is_null( $Recv ))
{
	$Recv = array();
	$Out['error'] = 'Can`t decode request.';
}

# Launch function(s):
foreach( $Recv as $key => $args )
{
	$func = "$key";
	if( function_exists($func))
		$func( $args, $Out);
	else
		$Out['error'] = 'Function "'.$key.'" does not exist.';
}

# Write response:
if( false == is_null( $Out))
	echo json_encode( $Out);


# Functions:

function init( $i_arg, &$o_out)
{
	$o_out['post_max_size'] = ini_get('post_max_size');
	$o_out['memory_limit'] = ini_get('memory_limit');
	$o_out['name'] = $_SERVER['SERVER_NAME'];
	$o_out['software'] = $_SERVER['SERVER_SOFTWARE'];
	$o_out['remote_address'] = $_SERVER['REMOTE_ADDR'];
	$o_out['php_version'] = phpversion();
}


function getjobs( $i_arg, &$o_out)
{
	$time_min = null;
	$time_max = null;
	$order_u = 'sumruntime';
	$order_s = 'taskssumruntime';

	if( isset( $i_args['time_min'])) $time_min = $i_args['time_min'];
	if( isset( $i_args['time_max'])) $time_min = $i_args['time_max'];
	if( isset( $i_args['order_u'])) $order_u = $i_args['order_u'];
	if( isset( $i_args['order_s'])) $order_s = $i_args['order_s'];

	$dbconn = db_connect();
	$query="SELECT min(time_done) AS time_done FROM statistics WHERE time_done > 0;";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
	$time_begin = $line["time_done"];
	pg_free_result($result);
	if( $time_begin == '') $time_begin = 0;

	if( is_null( $time_min)) $time_min = $time_begin;
	if( is_null( $time_max)) $time_max = time();

	$o_out['time_min'] = $time_min;
	$o_out['time_max'] = $time_max;
	$o_out['tables'] = array();

	// Users:
	$o_out['tables']['users'] = array();
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
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		# Get user favorite service:
		$sub_query="
SELECT service,
 sum(taskssumruntime) AS sumruntime
 FROM statistics
 WHERE username='".$line["username"]."'
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
		$line['service_name'] = $sub_name;
		$line['service_percent'] = $sub_favourite/$sub_total * 100;

		$o_out['tables']['users'][] = $line;
	}
	pg_free_result($result);

	// Services:
	$o_out['tables']['services'] = array();
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
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
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
		$line['user_name'] = $sub_name;
		$line['user_percent'] = $sub_favourite/$sub_total * 100;

		$o_out['tables']['services'][] = $line;
	}
	pg_free_result($result);
}

function db_connect()
{
	return pg_connect('host=localhost dbname=afanasy user=afadmin password=AfPassword') or die('Could not connect: ' . pg_last_error());
}

?>

