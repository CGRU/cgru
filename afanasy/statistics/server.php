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

function init( $i_args, &$o_out)
{
	# Configuration:
	$srv = array();
	$srv['name'] = $_SERVER['SERVER_NAME'];
	$srv['software'] = $_SERVER['SERVER_SOFTWARE'];
	$srv['remote_address'] = $_SERVER['REMOTE_ADDR'];
	$srv['php_version'] = phpversion();
	$srv['post_max_size'] = ini_get('post_max_size');
	$srv['memory_limit'] = ini_get('memory_limit');
	$o_out['server'] = $srv;

	$dbconn = db_connect();

	# Time min and max:
	$time = array();
	$query="SELECT min(time_done) AS time_done FROM jobs WHERE time_done > 0;";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	$line = pg_fetch_array( $result, null, PGSQL_ASSOC);
	$time['time_min'] = (int)$line["time_done"];
	pg_free_result($result);
	if( $time['time_min'] == '') $time['time_min'] = 0;
	$time['time_max'] = time();
	$o_out['time'] = $time;

	# Folders:
	$query="SELECT min(folder) as folder FROM jobs GROUP BY folder ORDER BY folder;";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	$out['folders'] = array();
	while( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
		if( array_key_exists('folder', $line))
			$o_out['folders'][] = $line['folder'];
}

function db_connect()
{
	return pg_connect('host=localhost dbname=afanasy user=afadmin password=AfPassword') or die('Could not connect: ' . pg_last_error());
}

function get_jobs_folders( $i_args, &$o_out)
{
	$table = 'jobs';

	$select   = $i_args['select'];
	$time_min = $i_args['time_min'];
	$time_max = $i_args['time_max'];
	$folder   = rtrim( $i_args['folder'],'/');
	$f_depth  = substr_count( $folder,'/') + 1;
	$order    = 'jobs_quantity';

	$dbconn = db_connect();

	$o_out['select'] = $select;
	$o_out['table']  = array();

	// Select:
// SELECT min(folder),sum(1) FROM tasks GROUP BY (regexp_split_to_array(btrim(folder,'/'),'/'))[2];
$query="
SELECT min(folder) as folder,
 sum(1) AS jobs_quantity,
 sum(tasks_quantity) AS tasks_quantity,
 sum(run_time_sum) AS run_time_sum,
 avg(CASE WHEN tasks_done>0 THEN run_time_sum/tasks_done ELSE 0 END) AS run_time_avg
 FROM $table
 WHERE time_done BETWEEN $time_min and $time_max AND folder LIKE '$folder%'
 GROUP BY (regexp_split_to_array(btrim(folder,'/'),'/'))[$f_depth]
 ORDER BY $order DESC;
";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		$where = $line[$select];
		$names = explode('/', $where);
		$names = array_slice( $names, 0, $f_depth + 1);
		$where = implode('/', $names);

		# Get folder favorite service:
		$sub_query="
SELECT service,
 sum(1) AS quantity
 FROM jobs
 WHERE folder LIKE '$where%'
 AND time_done BETWEEN $time_min and $time_max
 GROUP BY service ORDER BY quantity DESC;
";
//error_log($sub_query);
		$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
		$sub_total = 0;
		$sub_favourite = 0;
		$sub_name = 0;
		while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
		{
			if( $sub_favourite < $sub_line['quantity'])
			{
				$sub_favourite = $sub_line['quantity'];
				$sub_name = $sub_line['service'];
			}
			$sub_total += $sub_line['quantity'];
		}
		pg_free_result($sub_result);
		$line['fav_service'] = $sub_name;
		$line['fav_service_percent'] = $sub_favourite/$sub_total;

		# Get folder favorite user:
		$sub_query="
SELECT username,
 sum(1) AS quantity
 FROM jobs
 WHERE folder LIKE '$where%'
 AND time_done BETWEEN $time_min and $time_max
 GROUP BY username ORDER BY quantity DESC;
";
//error_log($sub_query);
		$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
		$sub_total = 0;
		$sub_favourite = 0;
		$sub_name = 0;
		while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
		{
			if( $sub_favourite < $sub_line['quantity'])
			{
				$sub_favourite = $sub_line['quantity'];
				$sub_name = $sub_line['username'];
			}
			$sub_total += $sub_line['quantity'];
		}
		pg_free_result($sub_result);
		$line['fav_user'] = $sub_name;
		$line['fav_user_percent'] = $sub_favourite/$sub_total;

		$o_out['table'][] = $line;
	}
	pg_free_result($result);
}

function get_jobs_table( $i_args, &$o_out)
{
	$order_s = 'run_time_sum';
	$order_u = 'run_time_sum';
	$table = 'jobs';

	$select   = $i_args['select'];
	$favorite = $i_args['favorite'];
	$time_min = $i_args['time_min'];
	$time_max = $i_args['time_max'];
	$folder   = rtrim( $i_args['folder'],'/');

	if( isset( $i_args['order_u'])) $order_u = $i_args['order_u'];
	if( isset( $i_args['order_s'])) $order_s = $i_args['order_s'];

	$dbconn = db_connect();

	$o_out['select']   = $select;
	$o_out['favorite'] = $favorite;
	$o_out['table'] = array();

	// Select:
$query="
SELECT $select,
 sum(1) AS jobs_quantity,
 sum(tasks_quantity) AS tasks_quantity,
 sum(tasks_quantity)/sum(1) AS tasks_quantity_avg,
 avg(capacity) AS capacity_avg,
 sum(run_time_sum) AS run_time_sum,
 avg(CASE WHEN tasks_done>0 THEN run_time_sum/tasks_done ELSE 0 END) AS run_time_avg,
 avg(tasks_done/tasks_quantity) AS tasks_done_percent
 FROM $table
 WHERE
 time_done BETWEEN $time_min and $time_max
 AND
 folder LIKE '$folder%'
 GROUP BY $select ORDER BY $order_s DESC;
";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		# Get service favorite user:
		$sub_query="
SELECT $favorite,
 sum(1) AS quantity
 FROM $table
 WHERE
 $select='".$line[$select]."'
 AND
 time_done BETWEEN $time_min and $time_max
 AND
 folder LIKE '$folder%'
 GROUP BY $favorite
 ORDER BY quantity DESC;
";
//error_log($sub_query);
		$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
		$sub_total = 0;
		$sub_favourite = 0;
		$sub_name = 0;
		while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
		{
			if( $sub_favourite < $sub_line['quantity'])
			{
				$sub_favourite = $sub_line['quantity'];
				$sub_name = $sub_line[$favorite];
			}
			$sub_total += $sub_line['quantity'];
		}
		pg_free_result($sub_result);
		$line['fav_name'] = $sub_name;
		$line['fav_percent'] = $sub_favourite/$sub_total;

		$o_out['table'][] = $line;
	}
	pg_free_result($result);
}

function get_tasks_folders( $i_args, &$o_out)
{
	$table = 'tasks';

	$select   = $i_args['select'];
	$time_min = $i_args['time_min'];
	$time_max = $i_args['time_max'];
	$folder   = rtrim( $i_args['folder'],'/');
	$f_depth  = substr_count( $folder,'/') + 1;
	$order = 'tasks_quantity';

	$dbconn = db_connect();

	$o_out['select'] = $select;
	$o_out['table']  = array();

$query="
SELECT min($select) as $select,
 sum(1) AS tasks_quantity,
 avg(capacity) AS capacity_avg,
 sum(time_done-time_started) AS run_time_sum,
 avg(time_done-time_started) AS run_time_avg,
 avg(error) AS error_avg
 FROM $table
 WHERE time_done BETWEEN $time_min and $time_max AND folder LIKE '$folder%'
 GROUP BY (regexp_split_to_array(btrim(folder,'/'),'/'))[$f_depth]
 ORDER BY $order DESC;
";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		$where = $line[$select];
		$names = explode('/', $where);
		$names = array_slice( $names, 0, $f_depth + 1);
		$where = implode('/', $names);

		# Get folder favorite service:
		$sub_query="
SELECT service,
 sum(1) AS quantity
 FROM tasks
 WHERE folder LIKE '$where%'
 AND time_done BETWEEN $time_min and $time_max
 GROUP BY service ORDER BY quantity DESC;
";
//error_log($sub_query);
		$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
		$sub_total = 0;
		$sub_favourite = 0;
		$sub_name = 0;
		while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
		{
			if( $sub_favourite < $sub_line['quantity'])
			{
				$sub_favourite = $sub_line['quantity'];
				$sub_name = $sub_line['service'];
			}
			$sub_total += $sub_line['quantity'];
		}
		pg_free_result($sub_result);
		$line['fav_service'] = $sub_name;
		$line['fav_service_percent'] = $sub_favourite/$sub_total;

		# Get folder favorite user:
		$sub_query="
SELECT username,
 sum(1) AS quantity
 FROM tasks
 WHERE folder LIKE '$where%'
 AND time_done BETWEEN $time_min and $time_max
 GROUP BY username ORDER BY quantity DESC;
";
//error_log($sub_query);
		$sub_result = pg_query($sub_query) or die('Query failed: ' . pg_last_error());
		$sub_total = 0;
		$sub_favourite = 0;
		$sub_name = 0;
		while ( $sub_line = pg_fetch_array( $sub_result, null, PGSQL_ASSOC))
		{
			if( $sub_favourite < $sub_line['quantity'])
			{
				$sub_favourite = $sub_line['quantity'];
				$sub_name = $sub_line['username'];
			}
			$sub_total += $sub_line['quantity'];
		}
		pg_free_result($sub_result);
		$line['fav_user'] = $sub_name;
		$line['fav_user_percent'] = $sub_favourite/$sub_total;

		$o_out['table'][] = $line;
	}
	pg_free_result($result);
}

function get_tasks_table( $i_args, &$o_out)
{
	$time_min = null;
	$time_max = null;
	$order_s = 'run_time_sum';
	$order_u = 'run_time_sum';
	$table = 'tasks';

	$select   = $i_args['select'];
	$favorite = $i_args['favorite'];
	$time_min = $i_args['time_min'];
	$time_max = $i_args['time_max'];
	$folder   = $i_args['folder'];

	if( isset( $i_args['order_u'])) $order_u = $i_args['order_u'];
	if( isset( $i_args['order_s'])) $order_s = $i_args['order_s'];

	$dbconn = db_connect();

	$o_out['select']   = $select;
	$o_out['favorite'] = $favorite;

	$o_out['table'] = array();
$query="
SELECT $select,
 sum(1) AS tasks_quantity,
 avg(capacity) AS capacity_avg,
 sum(time_done-time_started) AS run_time_sum,
 avg(time_done-time_started) AS run_time_avg,
 avg(error) AS error_avg
 FROM $table
 WHERE time_done BETWEEN $time_min and $time_max AND folder LIKE '$folder%'
 GROUP BY $select ORDER BY $order_s DESC;
";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		# Get service favorite user:
		$sub_query="
SELECT $favorite,
 sum(1) AS tasks_quantity
 FROM $table
 WHERE $select='".$line[$select]."'
 AND time_done BETWEEN $time_min and $time_max AND folder LIKE '$folder%'
 GROUP BY $favorite ORDER BY tasks_quantity DESC;
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
				$sub_name = $sub_line[$favorite];
			}
			$sub_total += $sub_line["tasks_quantity"];
		}
		pg_free_result($sub_result);
		$line['fav_name'] = $sub_name;
		$line['fav_percent'] = $sub_favourite/$sub_total;

		$o_out['table'][] = $line;
	}
	pg_free_result($result);
}

function get_tasks_folders_graph( $i_args, &$o_out)
{
	$table = 'tasks';

	$select   = $i_args['select'];
	$time_min = $i_args['time_min'];
	$time_max = $i_args['time_max'];
	$interval = $i_args['interval'];
	$folder   = rtrim( $i_args['folder'],'/');
	$f_depth  = substr_count( $folder,'/') + 1;
	$order    = 'quantity';

	$o_out['time_min'] = $time_min;
	$o_out['time_max'] = $time_max;
	$o_out['interval'] = $interval;
	$o_out['select'] = $select;
	$o_out['graph'] = array();
	$o_out['table'] = array();

	$dbconn = db_connect();

	// Query whole time interval table:
	$query="
SELECT min(folder) as folder,
 sum(1) AS quantity
 FROM $table
 WHERE time_done BETWEEN $time_min and $time_max AND folder LIKE '$folder%'
 GROUP BY (regexp_split_to_array(btrim(folder,'/'),'/'))[$f_depth]
 ORDER BY quantity DESC;
";
//error_log($query);
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		$o_out['table'][] = $line;
	}

	// Query graph (a table per time interval):
	$time = $time_min;
	while( $time <= $time_max )
	{
		$cur_time_min = $time;
		$cur_time_max = $time + $interval;
		$o_out['graph'][$time] = array();

		$query="
SELECT min(folder) as folder,
 sum(1) AS quantity
 FROM $table
 WHERE time_done BETWEEN $cur_time_min and $cur_time_max AND folder LIKE '$folder%'
 GROUP BY (regexp_split_to_array(btrim(folder,'/'),'/'))[$f_depth]
 ORDER BY quantity DESC;
";
//error_log($query);
		$result = pg_query($query) or die('Query failed: ' . pg_last_error());
		while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
		{
			$fname = $line[$select];
			$fname = explode('/', $fname);
			$fname = array_slice( $fname, $f_depth, $f_depth + 1);
			if( count( $fname))
				$fname = $fname[0];
			else
				$fname = '';

			$o_out['graph'][$time][$fname] = $line;
		}
		pg_free_result($result);

		$time += $interval;
	}
}

function get_tasks_graph( $i_args, &$o_out)
{
	$table = 'tasks';

	$time_min = $i_args['time_min'];
	$time_max = $i_args['time_max'];
	$interval = $i_args['interval'];
	$folder   = $i_args['folder'];

	$select = $i_args['select'];

	$o_out['time_min'] = $time_min;
	$o_out['time_max'] = $time_max;
	$o_out['interval'] = $interval;
	$o_out['select'] = $select;
	$o_out['graph'] = array();
	$o_out['table'] = array();

	$dbconn = db_connect();

	// Query whole time interval table:
	$query="
SELECT $select,
 sum(1) AS quantity
 FROM $table
 WHERE time_done BETWEEN $time_min and $time_max AND folder LIKE '$folder%'
 GROUP BY $select ORDER BY quantity DESC;
";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
	{
		$o_out['table'][] = $line;
	}

	// Query graph (a table per time interval):
	$time = $time_min;
	while( $time <= $time_max )
	{
		$cur_time_min = $time;
		$cur_time_max = $time + $interval;
		$o_out['graph'][$time] = array();

		$query="
SELECT $select,
 sum(1) AS quantity
 FROM $table
 WHERE time_done BETWEEN $cur_time_min and $cur_time_max AND folder LIKE '$folder%'
 GROUP BY $select ORDER BY quantity DESC;
";
		$result = pg_query($query) or die('Query failed: ' . pg_last_error());
		while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
		{
			$o_out['graph'][$time][$line[$select]] = $line;
		}
		pg_free_result($result);

		$time += $interval;
	}
}

function folder_delete( $i_args, &$o_out)
{
	$folder = $i_args['folder'];

	$dbconn = db_connect();


	$query="DELETE FROM jobs WHERE folder LIKE '$folder'";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	$o_out['jobs'] = array();
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
		$o_out['jobs'][] = $line;

	$query="DELETE FROM tasks WHERE folder LIKE '$folder'";
	$result = pg_query($query) or die('Query failed: ' . pg_last_error());
	$o_out['tasks'] = array();
	while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
		$o_out['tasks'][] = $line;
}

?>

