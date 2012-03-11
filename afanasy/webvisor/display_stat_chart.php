<?php

function display_stat_chart()
{
$type=$_GET['type'];
$size=$_GET['size'];
$action=$_GET['action'];
if( $action == '') $action = 'stat_chart';
if( $size == '') $size = 600;
else $size = (int)$size;

echo '<p><b>';
switch ($type)
{
case 'jobsnum':
   echo 'User / Jobs Number';
   break;
case 'jobssumruntime':
   echo 'User / Jobs Total Run Time';
   break;
case 'jobsavgruntime':
   echo 'User / Jobs Average Run Time';
   break;
case 'usertasksnum':
   echo 'User / Total Tasks Quantity';
   break;
case 'usertasksavg':
   echo 'User / Average Tasks Quantity';
   break;
case 'servicequantity':
   echo 'Service / Quantity';
   break;
case 'tasksquantity':
   echo 'Service / Tasks Quantity';
   break;
case 'tasksquantityavg':
   echo 'Service / Tasks Average Quantity';
   break;
case 'taskssumruntime':
   echo 'Service / Tasks Total Run Time';
   break;
case 'tasksavgruntime':
   echo 'Service / Tasks Average Run Time';
   break;
case 'tasksdone':
   echo 'Service / Tasks Done';
   break;
case '':
   echo 'No type provided.';
   return;
default:
   echo "Type '$type' is invalid.</b></p>";
   return;
}
echo '</b></p>';

echo "<img src='image_chart.php?type=$type&size=$size'>";

$size_delta = 100;
$sizeless   = $size - $size_delta;
$sizemore   = $size + $size_delta;
if( $sizeless < 200 ) $sizeless = 200 ;
if( $sizemore > 2000) $sizeless = 2000;
echo "<table height=50 width=$size><tr align=center>";
echo "<td width=50%><a href='index.php?action=$action&type=$type&size=$sizeless'> << </a></td>";
echo "<td width=50%><a href='index.php?action=$action&type=$type&size=$sizemore'> >> </a></td>";
echo '</tr></table>';
}
?>
