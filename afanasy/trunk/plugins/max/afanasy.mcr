Macroscript Afanasy
ButtonText:"Afanasy..."
category:"CGRU"
toolTip:"Render by Afanasy..."
(
global ver = "Afanasy"

-- Delete old windows:
if (AfanasyDialog != undefined) do (
   try DestroyDialog AfanasyDialog
   catch ()
)   

function createCommand useTemp taskFrameNumber pause= (

   local startFramei = 1
   local endFramei = 1
   local byFramei = rendNThFrame
   local taskFrameNumberi = taskFrameNumber as integer

   --1 - A single frame. 
   --2 - The active time segment. 
   --3 - The user specified range. 
   --4 - The user specified frame pickup string (for example "1,3,5-12"). 
   
   if rendTimeType == 1 then(
      startFramei = currentTime.frame as integer
      endFramei = currentTime.frame as integer
   )
   if rendTimeType == 2 then(
      startFramei = animationRange.start.frame as integer
      endFramei = animationRange.end.frame as integer
   )
   if rendTimeType == 3 then(
      startFramei = rendStart.frame as integer
      endFramei = rendEnd.frame as integer
   )
   if rendTimeType == 4 then(
      format "--Warning: Can render frame range only.%\n" ""
      startFramei = rendStart.frame as integer
      endFramei = rendEnd.frame as integer
   )

   local cmd = "afjob.py "
   cmd += maxFilePath + maxFileName
   cmd += " " + (startFramei as string)
   cmd += " " + (endFramei as string)
   cmd += " -fpt " + (taskFrameNumberi as string)
   cmd += " -by " + (rendNThFrame as string)
   if (rendOutputFilename != '') then (
      cmd += " -image " + rendOutputFilename
   )
   if (useTemp == true) then (
      cmd += " -tempscene"
      cmd += " -deletescene"
   )
   if (pause == true) then (
      cmd += " -pause"
   )
   return cmd
)	

rollout AfanasyDialog ver
(
   checkbox useTempControl "Save Temporary Scene" pos:[16,10] width:152 height:20 checked:false toolTip:"Save scene to temporary file before render."
   spinner taskFrameNumberControl "Frames Per Task" pos:[70,40]  width:60 height:19 range:[1,999,1] type:#integer scale:1 toolTip:"Number of frames in one task"
   button render "Render" pos:[16,70] width:64 height:24 toolTip:"Start Render Proces"
   button close "Close" pos:[80,70] width:64 height:24 toolTip:"Close window"
   checkbox pauseControl "Start Job Paused" pos:[16,100] checked:false toolTip:"Send job paused."

   on render pressed do
   (
      local useTemp = useTempControl.checked 
      local taskFrameNumber = taskFrameNumberControl.value
      local pause = pauseControl.checked 

      -- Save scene:
      max file save

      -- Create command:
      local cmd = createCommand useTemp taskFrameNumber pause

      -- If errors, empty command generated and return:
      if (cmd == "") then (
         return false
      )

      -- Prepare  command:
      cmd = systemTools.getEnvVariable("AF_ROOT") + "\\python\\" + cmd
      cmd = "python " + cmd
      format "%\n" cmd

      -- Launch command:
      --DOSCommand ("start \"" + cmd + "\"")
      DOSCommand (cmd)

      -- Close dialog:
      DestroyDialog AfanasyDialog
   )

   on close pressed do	(
      try DestroyDialog AfanasyDialog
      catch ()
   )
)
CreateDialog  AfanasyDialog style:#(#style_titlebar, #style_border, #style_sysmenu,#style_minimizebox,#style_sunkenedge)
)
