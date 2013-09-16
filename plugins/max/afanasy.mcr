Macroscript Afanasy
ButtonText:"Afanasy..."
category:"CGRU"
toolTip:"Render by Afanasy..."
(
-- Set render time type to "User specified range"
rendTimeType = 3

-- Set initial parameters:
local jobname = maxFileName
local startFrame = rendStart.frame as integer
local endFrame = rendEnd.frame as integer
local byFrame = rendNThFrame
local taskFrameNumber = 1

persistent global AfOutputImage
persistent global AfUseSceneWDir
persistent global AfWorkingDirectory
persistent global AfPriority
persistent global AfMaxHosts
persistent global AfCapacity
persistent global AfDependMask
persistent global AfGlobalMask
persistent global AfHostsMask
persistent global AfExcludeHosts
persistent global AfSaveTempScene
persistent global AfStartPaused

if AfPriority           == undefined then AfPriority           = -1
if AfMaxHosts           == undefined then AfMaxHosts           = -1
if AfCapacity           == undefined then AfCapacity           = -1
if AfOutputImage        == undefined then AfOutputImage        = ""
if AfWorkingDirectory   == undefined then AfWorkingDirectory   = ""
if AfDependMask         == undefined then AfDependMask         = ""
if AfGlobalMask         == undefined then AfGlobalMask         = ""
if AfHostsMask          == undefined then AfHostsMask          = ""
if AfExcludeHosts       == undefined then AfExcludeHosts       = ""
if AfSaveTempScene      == undefined then AfSaveTempScene      = false
if AfStartPaused        == undefined then AfStartPaused        = false
if AfUseSceneWDir       == undefined then AfUseSceneWDir       = false


-- Get scene cameras:
local CameraNames = #("")
for cam in cameras do
(
   if cam.isTarget then continue
   append CameraNames cam.name
)

-- Get batch render views:
local BatchViewNames = #("", "all")
for i = 1 to batchRenderMgr.numViews do
(
   local batchview = batchRenderMgr.getView i
   append BatchViewNames batchview.name
)

-- Delete old dialog:
if (AfanasyDialog != undefined) do
(
   try DestroyDialog AfanasyDialog
   catch ()
)   

rollout AfanasyDialog "Afanasy"
(
-- Job name:
   edittext jobnameControl "Job Name" text:jobname
-- Frame range:
   spinner startFrameControl "Start Frame" range:[0,99999,startFrame] type:#integer scale:1 toolTip:"First frame to render."
   spinner endFrameControl "End Frame" range:[0,99999,endFrame] type:#integer scale:1 toolTip:"Last Frame to render."
   spinner byFrameControl "By Frame" range:[1,999,byFrame] type:#integer scale:1 toolTip:"Render every Nth frame."
   spinner taskFrameNumberControl "Frames Per Task" range:[1,999,taskFrameNumber] type:#integer scale:1 toolTip:"Number of frames in one task."
-- Cameras:
   dropdownlist cameraControl "Override Camera" items:CameraNames toolTip:"Override render camera."
-- Output image:
   label outputImageLabel "Override Output Image:"
   edittext outputImageControl text:AfOutputImage toolTip:"Specify output image."
-- Working directory:
   checkbox useSceneWDirControl "Use scene folder as WDir" checked:AfUseSceneWDir toolTip:"Use scene folder as working directory."
   label workingDirectoryLabel "Specify Working Directory:"
   edittext workingDirectoryControl text:AfWorkingDirectory toolTip:"Specify working directory."
-- Batch views:
   dropdownlist batchControl "Render Batch View" items:BatchViewNames toolTip:"Render batch view."
-- Priority:
   spinner priorityControl "Priority" range:[-1,99,AfPriority] type:#integer scale:1 toolTip:"Job order."
-- Maximum hosts:
   spinner maxHostsControl "Max Hosts" range:[-1,9999,AfMaxHosts] type:#integer scale:1 toolTip:"Maximum number of hosts job can run on."
-- Capacity:
   spinner capacityControl "Capacity" range:[-1,999999,AfCapacity] type:#integer scale:1 toolTip:"Job tasks capacity."
-- Depend mask:
   edittext dependMaskControl "Depend Mask" text:AfDependMask toolTip:"Jobs to wait names pattern (same user)."
-- Global Depend mask:
   edittext globalMaskControl "Global Depend" text:AfGlobalMask toolTip:"Jobs to wait names pattern (all users)."
-- Hosts mask:
   edittext hostsMaskControl "Hosts Mask" text:AfHostsMask toolTip:"Hosts names pattern job can run on."
-- Exclude hosts:
   edittext excludeHostsControl "Exclude Hosts" text:AfExcludeHosts toolTip:"Hosts names pattern job can not run on."
-- Save temporarry scene:
   checkbox useTempControl "Save Temporary Scene" checked:AfSaveTempScene toolTip:"Save scene to temporary file before render."
-- Render button:
   button renderButton "Render" toolTip:"Start Render Proces."
-- Start job paused:
   checkbox pauseControl "Start Job Paused" checked:AfStartPaused toolTip:"Send job paused."

   on batchControl selected i do
   (
      local restore = true
      if i > 2 then
      (
         local numview = i - 2
         local batchview = batchRenderMgr.getView numview
         if batchview.overridePreset then
         (
            startFrameControl.value = batchview.startFrame
            endFrameControl.value = batchview.endFrame
            restore = false
         )
      )
      if restore then
      (
         startFrameControl.value = startFrame
         endFrameControl.value = endFrame
      )
   )
   
   on renderButton pressed do
   (
-- Set render time type to "User specified range"
      rendTimeType = 3

-- Save scene:
      checkForSave()

-- Get Parameters:
      AfPriority           = priorityControl.value
      AfMaxHosts           = maxHostsControl.value
      AfCapacity           = capacityControl.value
      AfOutputImage        = outputImageControl.text
      AfWorkingDirectory   = workingDirectoryControl.text
      AfDependMask         = dependMaskControl.text
      AfGlobalMask         = globalMaskControl.text
      AfHostsMask          = hostsMaskControl.text
      AfExcludeHosts       = excludeHostsControl.text
      AfSaveTempScene      = useTempControl.checked
      AfStartPaused        = pauseControl.checked

      AfUseSceneWDir       = useSceneWDirControl.checked

-- Check Parameters:
      if AfWorkingDirectory[AfWorkingDirectory.count] == "\\" then
         AfWorkingDirectory = substring AfWorkingDirectory 1 (AfWorkingDirectory.count-1)

-- Create command:
      local cmd = "\"" + systemTools.getEnvVariable("AF_ROOT") + "\\python\\" + "afjob.py\""
      cmd += " \"" + maxFilePath + maxFileName + "\""
      cmd += " " + (startFrameControl.value as string)
      cmd += " " + (endFrameControl.value as string)
      cmd += " -fpt " + (taskFrameNumberControl.value as string)
      cmd += " -by " + (byFrameControl.value as string)
      if jobnameControl.text != "" then cmd += " -name \"" + jobnameControl.text + "\""
      if AfPriority > -1 then cmd += " -priority " + (AfPriority as string)
      if AfMaxHosts > -1 then cmd += " -maxhosts " + (AfMaxHosts as string)
      if AfCapacity > -1 then cmd += " -capacity " + (AfCapacity as string)
      if AfDependMask         != "" then cmd += " -depmask \""   + AfDependMask        + "\""
      if AfGlobalMask         != "" then cmd += " -depglbl \""   + AfGlobalMask        + "\""
      if AfHostsMask          != "" then cmd += " -hostsmask \"" + AfHostsMask         + "\""
      if AfExcludeHosts       != "" then cmd += " -hostsexcl \"" + AfExcludeHosts      + "\""
      if AfOutputImage        != "" then cmd += " -output \""    + AfOutputImage       + "\""
      if AfWorkingDirectory   != "" then cmd += " -pwd \""       + AfWorkingDirectory  + "\""
      else if AfUseSceneWDir == true then (
         local folder = maxFilePath
         if folder[folder.count] == "\\" then folder = substring folder 1 (folder.count-1)
         cmd += " -pwd \"" + folder + "\""
      )
      if AfStartPaused == true then cmd += " -pause"
      if AfSaveTempScene == true then (
         cmd += " -tempscene"
         cmd += " -deletescene"
      )
      if cameraControl.selection > 1 then cmd += " -node \"" + cameraControl.selected + "\""
      if batchControl.selection > 1 then
      (
         cmd += " -take \"" + batchControl.selected + "\""
         if batchControl.selection > 2 then
         (
            local batchview = batchRenderMgr.getView(batchControl.selection-2)
            local image = batchview.outputFilename
            if image != "" then cmd += " -image \"" + image + "\""
         )
      )
      else
      (
         if rendOutputFilename != "" then cmd += " -image \"" + rendOutputFilename + "\""
      )

-- Prepare  command:
      cmd = "python " + cmd
      format "-- %\n" cmd

-- Prepare command output file
      local outputfile = "afanasy_submint_max.txt"
      local tempdir = systemTools.getEnvVariable("TMP")
      if tempdir == undefined then tempdir = systemTools.getEnvVariable("TEMP")
      if tempdir == undefined then tempdir = "c:\\temp"
      local outputfile = tempdir + "\\" + outputfile

-- Launch command with redirected output:
      HiddenDOSCommand (cmd + " > " + outputfile + " 2>&1") ExitCode:&status

-- Output error if bad command exit status:
      if status != 0 then
      (
         format "-- %\n" "Error:"
         local outtext = openfile outputfile
         while not eof outtext do
         (
            local str = readLine outtext
            format "-- %\n" str
         )
         close outtext
      )
   )
)
CreateDialog  AfanasyDialog style:#(#style_titlebar, #style_border, #style_sysmenu,#style_minimizebox,#style_sunkenedge)
)
