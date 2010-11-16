<?php
include_once('fns_db.php');
include_once('fns_time.php');

function getColors( $colordelta, $shift )
{
   static $COUNTER = 0;
   static $CR = 255;
   static $CG = 0;
   static $CB = 0;
   static $maxcolor = 200;
   static $mincolor = 20;

   if( $shift == 0 )
   {
      $COUNTER = 0;
      $CR = 255;
      $CG = 0;
      $CB = 0;
   }

   $cr = $CR / (($COUNTER+1)%2+1);
   $cg = $CG / (($COUNTER+1)%2+1);
   $cb = $CB / (($COUNTER+1)%2+1);
   if( $cr > $maxcolor ) $cr = $maxcolor;
   if( $cr < $mincolor ) $cr = $mincolor;
   if( $cg > $maxcolor ) $cg = $maxcolor;
   if( $cg < $mincolor ) $cg = $mincolor;
   if( $cb > $maxcolor ) $cb = $maxcolor;
   if( $cb < $mincolor ) $cb = $mincolor;

   if( $CB == 255 )
   {
      $CG -= $colordelta;
   }
   else
   {
      $CG += $colordelta;
      if( $CG > 255 )
      {
         $CG = 255;
         $CR -= $colordelta;
         if( $CR < 0 )
         {
            $CR = 0;
            $CB += $colordelta;
            if( $CB > 255 ) $CB = 255;
         }
      }
   }
   $COUNTER++;

   return array( $cr, $cg, $cb);
}

$size=$_GET['size'];
if( $size == '') $size = 600;
else $size = (int)$size;
$type=$_GET['type'];

//$im = @imagecreatetruecolor( $size*4/3, $size) or die("Cannot Initialize new GD image stream");
//imageantialias( $im, true);
//imagealphablending( $im, true);
$im = @imagecreate( $size*4/3, $size) or die("Cannot Initialize new GD image stream");
$bg = imagecolorallocate($im, 0, 0, 0);
$bg = imagecolortransparent($im, $bg);
imagefill( $im, 0, 0, $bg);

$title_color = imagecolorallocate( $im, 0,  32, 0);
$title_posx = $size/2;
$title_posy = 0;
switch ($type)
{
case 'jobsnum':
   $query="SELECT username AS name,sum(1) AS value FROM stat_jobs GROUP BY username;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Jobs Number', $title_color);
   break;
case 'jobssumonlinetime':
   $query="SELECT username AS name,sum(time_done-time_started) AS value FROM stat_jobs GROUP BY username;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Jobs Online Time Sum', $title_color);
   break;
case 'jobsavgonlinetime':
   $query="SELECT username AS name,avg(time_done-time_started) AS value FROM stat_jobs GROUP BY username;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Jobs Average Online Time', $title_color);
   break;
case 'jobssumruntime':
   $query="SELECT username AS name,sum(taskssumruntime) AS value FROM stat_jobs GROUP BY username;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Jobs Run Time Sum', $title_color);
   break;
case 'jobsavgruntime':
   $query="SELECT username AS name,avg(taskssumruntime) AS value FROM stat_jobs GROUP BY username;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Jobs Average Run Time', $title_color);
   break;
case 'servicequantity':
   $query="SELECT service AS name,sum(1) AS value FROM stat_blocks GROUP BY service;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Service Type Quantity', $title_color);
   break;
case 'tasksquantity':
   $query="SELECT service AS name,sum(tasksnum) AS value FROM stat_blocks GROUP BY service;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Tasks Quantity', $title_color);
   break;
case 'taskssumruntime':
   $query="SELECT service AS name,sum(taskssumruntime) AS value FROM stat_blocks GROUP BY service;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Tasks Sum Run Time', $title_color);
   break;
case 'tasksavgruntime':
   $query="SELECT service AS name,avg(CASE WHEN tasksdone>0 THEN taskssumruntime/tasksdone ELSE 0 END) AS value FROM stat_blocks GROUP BY service;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Tasks Avg Run Time', $title_color);
   break;
case 'tasksdone':
   $query="SELECT service AS name,round(avg(100*tasksdone/tasksnum),2) AS value FROM stat_blocks GROUP BY service;";
   imagestring( $im, 5, $title_posx, $title_posy,  'Tasks Done', $title_color);
   break;
default:
   imagestring( $im, 5, $title_posx, $title_posy,  "Type '$type' is invalid.", $title_color);
   header("Content-type: image/png");
   imagepng( $im);
   return;
}

$names_array = array();
$values_array = array();
$dbconn = db_connect();
$result = pg_query($query) or die('Query failed: ' . pg_last_error());
$countnames = 0;
$countvalues = 0;

while ( $line = pg_fetch_array( $result, null, PGSQL_ASSOC))
{
   $name  = $line["name"];
   $value = (int) $line["value"];
   switch ($type)
   {
   case 'jobssumruntime':
   case 'taskssumruntime':
      $names_array[$countnames] = "$name-".time_strDHMS($value);
      break;
   case 'jobsavgruntime':
   case 'tasksavgruntime':
      $names_array[$countnames] = "$name-".time_strHMS($value);
      break;
   default:
      $names_array[$countnames] = "$name-$value";
      break;
   }
   $values_array[$countnames] = $value;
   $countvalues += $values_array[$countnames];
   $countnames ++;
}
pg_free_result($result);

$lineEX = array();
$lineEY = array();

$center_x = $size/4 + $size/2;
$center_y = $size/2;
$radius_x = $size*9/10;
$radius_y = $radius_x*9/10;
$valuesum = 0;
$previous_angle = 0;
$colordelta = 4 * 255/($countnames+1);
for ( $n = 0; $n < $countnames; $n++)
{
   $name  = $names_array[  $n ];
   $value = $values_array[ $n ];

   $angle = (int)($value*360/$countvalues);
   if( $n == ($countnames-1)) $angle = 360 - $previous_angle;

   $reverce_as = 360 - ($previous_angle + $angle);
   $reverce_ae = 360 - $previous_angle;
   $previous_angle += $angle;

   list ( $cr, $cg, $cb) = getColors( $colordelta, $n );
   $color = imagecolorallocate( $im, $cr, $cg, $cb);


//   if( $angle > 0 ) imagefilledarc( $im, $center_x, $center_y, $radius_x, $radius_y, $reverce_as, $reverce_ae, $color, IMG_ARC_PIE);
   if( $angle == 0 ) $reverce_ae++;
   imagefilledarc( $im, $center_x, $center_y, $radius_x, $radius_y, $reverce_as, $reverce_ae, $color, IMG_ARC_PIE);
//   imagestring( $im, 5, 150, ($n)*$size/$countnames, $angle, $color);

   $lineEX[$n] = $center_x + .4*$radius_x*cos(.5*deg2rad($reverce_as+$reverce_ae));
   $lineEY[$n] = $center_y + .4*$radius_y*sin(.5*deg2rad($reverce_as+$reverce_ae));

   $valuesum += $value;
}

$linehorizlen = $size/4;
$middlenum = $countnames/2;
for( $n = 0; $n < $countnames; $n++)
{
   list ( $cr, $cg, $cb) = getColors( $colordelta, $n );
   $color = imagecolorallocate( $im, .5*$cr, .5*$cg, .5*$cb);
   $ty = ($n)*$size/$countnames;
   imagestring( $im, 5, 0, $ty, $names_array[$n], $color);

   $line_x = 0;
   $line_y = $ty+17;
   $color = imagecolorallocate( $im, 0, 60, 0);
   $horizlen = $linehorizlen + $size*pow(((float) abs($middlenum-$n) / $countnames * 2), 2);
   imageline( $im, $line_x, $line_y, $line_x+$horizlen, $line_y, $color);
   imageline( $im, $line_x+$horizlen, $line_y, $lineEX[$n], $lineEY[$n], $color);
}

header("Content-type: image/png");
imagepng( $im);
//imagedestroy( $im);

?>
