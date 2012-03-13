<?php

function display_stat_chart()
{
$type=$_GET['type'];
$size=$_GET['size'];
$action=$_GET['action'];
if( $action == '') $action = 'stat_chart';
if( $size == '') $size = 600;
else $size = (int)$size;

if( $type == '')
{
   echo 'No type provided.';
   return;
}

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
