cd %0\..

if not exist project\Render_Pictures mkdir project\Render_Pictures

..\..\software_setup\start_softimage.cmd %CD%\project\Scenes\scene.scn
