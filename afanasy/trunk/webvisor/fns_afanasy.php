<?php

$STATE_READY    = 1<<0;
$STATE_RUNNING  = 1<<1;
$STATE_WAITDEP  = 1<<2;
$STATE_WAITTIME = 1<<3;
$STATE_DONE     = 1<<4;
$STATE_ERROR    = 1<<5;
$STATE_STDOUT   = 1<<6;
$STATE_STDERR   = 1<<7;
$STATE_SKIPPED  = 1<<8;
$STATE_OFFLINE  = 1<<9;

$STATE_ROnline = 1<<0;
$STATE_Rnimby  = 1<<1;
$STATE_RNIMBY  = 1<<2;
$STATE_RBusy   = 1<<3;

$FLAGS_UAdmin  = 1<<0;

function stateIsReady( $state )
{
   global $STATE_READY;
   return $state & $STATE_READY;
}

function stateIsRunning( $state )
{
   global $STATE_RUNNING;
   return $state & $STATE_RUNNING;
}

function stateIsWaitdep( $state )
{
   global $STATE_WAITDEP;
   return $state & $STATE_WAITDEP;
}

function stateIsWaittime( $state )
{
   global $STATE_WAITTIME;
   return $state & $STATE_WAITTIME;
}

function stateIsDone( $state )
{
   global $STATE_DONE;
   return $state & $STATE_DONE;
}

function stateIsError( $state )
{
   global $STATE_ERROR;
   return $state & $STATE_ERROR;
}

function stateIsStdout( $state )
{
   global $STATE_STDOUT;
   return $state & $STATE_STDOUT;
}

function stateIsStderr( $state )
{
   global $STATE_STDERR;
   return $state & $STATE_STDERR;
}

function stateIsSkipped( $state )
{
   global $STATE_SKIPPED;
   return $state & $STATE_SKIPPED;
}

function stateIsOffline( $state )
{
   global $STATE_OFFLINE;
   return $state & $STATE_OFFLINE;
}

function stateToStr( $state)
{
   $state_str = '';
   if( stateIsReady(    $state)) $state_str = $state_str.' RDY';
   if( stateIsRunning(  $state)) $state_str = $state_str.' RUN';
   if( stateIsDone(     $state)) $state_str = $state_str.' DON';
   if( stateIsError(    $state)) $state_str = $state_str.' ERR';
   if( stateIsWaitdep(  $state)) $state_str = $state_str.' WD';
   if( stateIsWaittime( $state)) $state_str = $state_str.' WT';
   if( stateIsSkipped(  $state)) $state_str = $state_str.' SKP';
   if( stateIsOffline(  $state)) $state_str = $state_str.' OFF';
   return $state_str;
}

function stateToString( $state)
{
   $state_str = '';
   if( stateIsReady(    $state)) $state_str = $state_str.' Ready';
   if( stateIsRunning(  $state)) $state_str = $state_str.' Running';
   if( stateIsDone(     $state)) $state_str = $state_str.' Done';
   if( stateIsError(    $state)) $state_str = $state_str.' Error';
   if( stateIsWaitdep(  $state)) $state_str = $state_str.' Waiting Dependences';
   if( stateIsWaittime( $state)) $state_str = $state_str.' Waiting Time';
   if( stateIsSkipped(  $state)) $state_str = $state_str.' Skipped';
   if( stateIsOffline(  $state)) $state_str = $state_str.' Offline';
   if( $state_str == '') $state_str = ' None';
   return $state_str;
}

function stateToStyle( $state)
{
   $style = 'empty';
   if(      stateIsError(    $state)) $style = 'error';
   else if( stateIsOffline(  $state)) $style = 'offline';
   else if( stateIsRunning(  $state)) $style = 'running';
   else if( stateIsWaittime( $state)) $style = 'waittime';
   else if( stateIsWaitdep(  $state)) $style = 'waitdep';
   else if( stateIsSkipped(  $state)) $style = 'offline';
   else if( stateIsDone(     $state)) $style = 'done';
   return $style;
}

function renderIsOnline( $state )
{
   global $STATE_ROnline;
   return $state & $STATE_ROnline;
}

function renderIs_nimby( $state )
{
   global $STATE_Rnimby;
   return $state & $STATE_Rnimby;
}

function renderIsNIMBY( $state )
{
   global $STATE_RNIMBY;
   return $state & $STATE_RNIMBY;
}

function renderIsBusy( $state )
{
   global $STATE_RBusy;
   return $state & $STATE_RBusy;
}

function userIsAdmin( $flags )
{
   global $FLAGS_UAdmin;
   return $flags & $FLAGS_UAdmin;
}

function userSetAdmin( $flags )
{
   global $FLAGS_UAdmin;
   return $flags | $FLAGS_UAdmin;
}

function renderStyle( $state)
{
   $style = '';
   if(    ! renderIsOnline( $state)) $style = 'offline';
   else if( renderIsBusy(   $state)) $style = 'running';
   else if( renderIsNIMBY(  $state) || renderIs_nimby( $state)) $style = 'nimby';
   return $style;
}

?>
