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

function db_connect()
{
	return pg_connect('host=localhost dbname=afanasy user=afadmin password=AfPassword') or die('Could not connect: ' . pg_last_error());
}

function get_jobs_table( $i_arg, &$o_out)
{
	$time_min = null;
	$time_max = null;
	$order_s = 'run_time_sum';
	$order_u = 'run_time_sum';
	$table = 'jobs';

	if( isset( $i_args['time_min'])) $time_min = $i_args['time_min'];
	if( isset( $i_args['time_max'])) $time_min = $i_args['time_max'];
	if( isset( $i_args['order_u'])) $order_u = $i_args['order_u'];
	if( isset( $i_args['order_s'])) $order_s = $i_args['order_s'];

	$dbconn = db_connect();
	$query="SELECT min(time_done) AS time_done FROM $table WHERE time_done > 0;";
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

	// Services:
	$o_out['tables']['services'] = array();
$query="
SELECT service,
 sum(1) AS jobs_quantity,
 sum(tasks_quantity) AS tasks_quantity,
 sum(tasks_quantity)/sum(1) AS tasks_quantity_avg,
 avg(capacity) AS capacity_avg,
 sum(run_time_sum) AS run_time_sum,
 avg(CASE WHEN tasks_done>0 THEN run_time_sum/tasks_done ELSE 0 END) AS run_time_avg,
 avg(tasks_done/tasks_quantity) AS tasks_done_percent
 FROM $table
 WHERE time_done BETWEEN $time_min and $time_max
 GROUP BY service ORDER BY $order_s DESC;
";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		# Get service favorite user:
		$sub_query="
SELECT username,
 sum(1) AS tasks_quantity
 FROM $table
 WHERE service='".$line["service"]."'
 AND time_done BETWEEN $time_min and $time_max
 GROUP BY username ORDER BY tasks_quantity DESC;
";
		$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
		$sub_total = 0;
		$sub_favourite = 0;
		$sub_name = 0;
		while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
		{
			if( $sub_favourite < $sub_line["tasks_quantity"])
			{
				$sub_favourite = $sub_line["tasks_quantity"];
				$sub_name = $sub_line["username"];
			}
			$sub_total += $sub_line["tasks_quantity"];
		}
		pg_free_result($sub_result);
		$line['fav_name'] = $sub_name;
		$line['fav_percent'] = $sub_favourite/$sub_total;

		$o_out['tables']['services'][] = $line;
	}
	pg_free_result($result);

	// Users:
	$o_out['tables']['users'] = array();
	$query="
SELECT username,
 sum(1) AS jobs_quantity,
 sum(tasks_quantity) AS tasks_quantity,
 avg(tasks_quantity) AS tasks_quantity_avg,
 avg(capacity) AS capacity_avg,
 sum(run_time_sum) AS run_time_sum,
 avg(run_time_sum) AS run_time_avg,
 avg(tasks_done/tasks_quantity) AS tasks_done_percent
 FROM $table
 WHERE time_done BETWEEN $time_min and $time_max
 GROUP BY username ORDER BY $order_u DESC;
";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		# Get user favorite service:
		$sub_query="
SELECT service,
 sum(1) AS tasks_quantity
 FROM $table
 WHERE username='".$line["username"]."'
 AND time_done BETWEEN $time_min and $time_max
 GROUP BY service ORDER BY tasks_quantity DESC;
";
		$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
		$sub_total = 0;
		$sub_favourite = 0;
		$sub_name = 0;
		while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
		{
			if( $sub_favourite < $sub_line["tasks_quantity"])
			{
				$sub_favourite = $sub_line["tasks_quantity"];
				$sub_name = $sub_line["service"];
			}
			$sub_total += $sub_line["tasks_quantity"];
		}
		pg_free_result($sub_result);
		$line['fav_name'] = $sub_name;
		$line['fav_percent'] = $sub_favourite/$sub_total;

		$o_out['tables']['users'][] = $line;
	}
	pg_free_result($result);
}

function get_tasks_table( $i_arg, &$o_out)
{
	$time_min = null;
	$time_max = null;
	$order_s = 'service';
	$order_u = 'username';
	$table = 'tasks';

	if( isset( $i_args['time_min'])) $time_min = $i_args['time_min'];
	if( isset( $i_args['time_max'])) $time_min = $i_args['time_max'];
	if( isset( $i_args['order_u'])) $order_u = $i_args['order_u'];
	if( isset( $i_args['order_s'])) $order_s = $i_args['order_s'];

	$dbconn = db_connect();
	$query="SELECT min(time_done) AS time_done FROM $table WHERE time_done > 0;";
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

	// Services:
	$o_out['tables']['services'] = array();
$query="
SELECT service,
 sum(1) AS tasks_quantity,
 avg(capacity) AS capacity_avg,
 sum(time_done-time_started) AS run_time_sum,
 avg(time_done-time_started) AS run_time_avg,
 avg(CASE WHEN errors_count>0 THEN 1 ELSE 0 END) AS error_avg
 FROM $table
 WHERE time_done BETWEEN $time_min and $time_max
 GROUP BY service ORDER BY $order_s DESC;
";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		# Get service favorite user:
		$sub_query="
SELECT username,
 sum(1) AS tasks_quantity
 FROM $table
 WHERE service='".$line["service"]."'
 AND time_done BETWEEN $time_min and $time_max
 GROUP BY username ORDER BY tasks_quantity DESC;
";
		$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
		$sub_total = 0;
		$sub_favourite = 0;
		$sub_name = 0;
		while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
		{
			if( $sub_favourite < $sub_line["tasks_quantity"])
			{
				$sub_favourite = $sub_line["tasks_quantity"];
				$sub_name = $sub_line["username"];
			}
			$sub_total += $sub_line["tasks_quantity"];
		}
		pg_free_result($sub_result);
		$line['fav_name'] = $sub_name;
		$line['fav_percent'] = $sub_favourite/$sub_total;

		$o_out['tables']['services'][] = $line;
	}
	pg_free_result($result);

	// Users:
	$o_out['tables']['users'] = array();
	$query="
SELECT username,
 sum(1) AS tasks_quantity,
 avg(capacity) AS capacity_avg,
 sum(time_done-time_started) AS run_time_sum,
 avg(time_done-time_started) AS run_time_avg,
 avg(CASE WHEN errors_count>0 THEN 1 ELSE 0 END) AS error_avg
 FROM $table
 WHERE time_done BETWEEN $time_min and $time_max
 GROUP BY username ORDER BY $order_u DESC;
";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		# Get user favorite service:
		$sub_query="
SELECT service,
 sum(1) AS tasks_quantity
 FROM $table
 WHERE username='".$line["username"]."'
 AND time_done BETWEEN $time_min and $time_max
 GROUP BY service ORDER BY tasks_quantity DESC;
";
		$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
		$sub_total = 0;
		$sub_favourite = 0;
		$sub_name = 0;
		while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
		{
			if( $sub_favourite < $sub_line["tasks_quantity"])
			{
				$sub_favourite = $sub_line["tasks_quantity"];
				$sub_name = $sub_line["service"];
			}
			$sub_total += $sub_line["tasks_quantity"];
		}
		pg_free_result($sub_result);
		$line['fav_name'] = $sub_name;
		$line['fav_percent'] = $sub_favourite/$sub_total;

		$o_out['tables']['users'][] = $line;
	}
	pg_free_result($result);
}

?>

