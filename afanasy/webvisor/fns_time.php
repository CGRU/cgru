<?php

function time_strHMS( $time )
{
   $hours   = (int)floor( $time / 3600);
   $minutes = (int)floor(($time - $hours*3600) / 60);
   $seconds = (int)floor($time - $hours*3600 - $minutes*60);
   $str = "$hours:";
   if( $minutes < 10) $str = "{$str}0";
   $str = "$str$minutes:";
   if( $seconds < 10) $str = "{$str}0";
   $str = "$str$seconds";
   return $str;
}

function time_strDHMS( $time )
{
   $days = (int)floor( $time / (3600*24));
   $str = "";
   if( $days > 1 ) $str = "$days days, ";
   return $str.time_strHMS( $time - $days * (3600*24));
}

?>
