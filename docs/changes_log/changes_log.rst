============
Changes Log
============

If Afanasy **network protocol changed**, it became incompatible with old one.

- The first number in the version means some significant changes in the project.
  Probably you should reconfigure Afanasy.
- The second number means major changes that caused compatibility lost.
  You should upgrade all clients and server at once.
- The third number versions are fully compatible.
  You can upgrade only one client or server for changes.



v3.4.0
======

*2024.03.26 NOT RELEASED*

- AfWatch Delete Done Jobs confirm dialog.

- AfRender can get CPU temperature and hardware info by Python modules. AfWatch can show it.

- AfRender Python service class skip task proceeded by a special function (variable). Empty command is threated not as skipped but as error now. As empty command can happen on some error in service class.


.. _changes-log-latest:


v3.3.1
======

2023.03.12

- Afanasy service can be configured to limit task post command running time.
  Task post commands are used to generate thumbnails.
  By default ``af_task_post_limit_sec`` config variable stands for it, and it is 16 seconds.
  Later (on limit exceeded) task post command will be killed.

- Afanasy can be configured not to cut domain names from user and host names.
  New parameters ``af_render_cut_domain_name`` and ``af_user_cut_domain_name`` added to control this.
  The default behaviour is the same as before, domain names will be cut.

- Afanasy statistics database tables got new rows.
  Job blocks got job serial. Tasks run got job serial, block and task ids.
  See `database schema <../afanasy/server.html#database-schema>`_.

- Nuke Afanasy Gizmo can set extra
  `environment <../software/nuke.html#environment>`_
  variables that will be added to task process.
  You use it to store Nuke location and version.

- Houdini Afanasy ROP can set extra
  `environment <../software/houdini.html#environment>`_
  variables that will be added to task process.
  You use it to store Houdini location and version.

- AfWatch can edit job block environment variables.

- AfWatch now uses ``open_folder_cmd`` config variable to open folders.

- Afanasy server will not allow to change a pool of a busy render.

- Afanasy server branches action added ``delete_done_jobs``. You can delete all done jobs from branch from AfWatch.

- Afanasy jobs branch can be paused.

- Afanasy user can be paused.

- Server annoying error log removed on a running job deletion.

- Rules Python API started.


v3.3.0
======

2022.07.25

- AfRender can send to server `GPU resources <../afanasy/render.html#gpu-string>`_:
  utilization, temperature, memory total and used.

  .. note::

    This feature is sponsored by SMF Animation Studio, LLC

  For now, just NVIDIA is supported.

- New job block need parameters:
  `GPU Memory <../afanasy/job_block.html#need-gpu-mem-mb>`_,
  `CPU Frequency <../afanasy/job_block.html#need-cpu-freq-mgz>`_,
  `CPU Cores <../afanasy/job_block.html#need-cpu-cores>`_ and
  `CPU Cores*Frequency <../afanasy/job_block.html#need-cpu-freq-cores>`_.

- AfWatch and WebGUI shows some summary info, when several jobs are selected.

- AfWatch got and "Administration" menu to switch to super user mode with password.

- AfWatch warning and error messages and super user mode label highlighted to attract more attention.
  When selection is not allowed, nodes list displays a warning message.

- Now parser checks whether the task progress was changed.
  Later server had to compare previous and new percentage value.
  And this behaviour was build-in.
  Now you can override it by parser.
  A new `progress_changed <../afanasy/render.html#self-progress-changed-false-true>`_ variable for it was added.
  And by default, progress is changed if task produced any output.

- New task state flag `Wait Dependencies <../afanasy/job_task.html#state>`_ appeared.
  Now tasks that wait some other tasks will be marked with this flag.
  Tasks with this flag are skipped during job solving.
  So now task dependencies will not wait the last frame of unbroken sequence.

- **Bugfixes:**

    - AfServer and job block with no task crash fixed.
      The crash was very rare, another block with tasks should follow a block without tasks.


v3.2.2
======

2022.02.09

- AfServer ``JOB_CREATED`` event added.

- Afanasy config: Parameter of a string array type can be overridden by command arguments or environment by a string contains a comma separated items.

- AfWatch

	- Task window uses mono-space font for task output and log.

	- Scroll step size preferences option. Later scroll size was only by item height.

- Houdini Afanasy ROP:

	- Pre and post submission scripts execution.

	- Separate Mantra rendering: Generate IFD block parameters.

- Houdini Afanasy TOP:

	- "Keep Job On Cancel Cook" option added. You can check tasks outputs, compare different jobs after cooking stop.

	- **Changed to work with Houdini 19 version, will not work with 18.5.**

- Nuke: Submission and render scripts support rendering a movie.

- Afanasy new services and parsers added: *shotgun, ftrack*.

- **Bugfixes:**

	- AfServer:

		- Change running job owner or branch fixed.

		- On start set WARNING tasks to READY.

		- Fixed task progress sending to GUIs mixing task and block numbers.

	- AfWatch: Skipped tasks does not affect job block average running timings.

	- Keeper: Refresh when local render deleted now does not produce an error in server log.

	- Houdini:

		- USD ROP now can works like Alembic ROP. One render session for all frame range.

		- MS Windows launch scripts fixed to work with 19 version.

		- Afanasy ROP:

			- Custom command mode block naming fix.

			- Separate Mantra rendering Auto Tickets fixed.


v3.2.1
======

2021.08.19

- Houdini Afanasy `TOP Scheduler <../software/houdini.html#afanasy-top-scheduler>`_.

- Houdini setup changed. Now ``HOUDINI_PATH=cgru/plugins/houdini`` and *afanasy.otl* moved to *otls/afanasy.hda*.

- Each not numeric task can have an own extra `environment <../afanasy/job_task.html#environment>`_.

- Paths mapping is applied to Block and Task environment.

- AfWatch now understands appending new blocks/tasks to an existing job.

- AfWatch can restart error tasks of a specified block.

- AfWatch Preferences menu got Reset Windows Geometry item.

- AfWatch can hide branch jobs and pool renders.

- Pools got some operations that will be applied to all renders in it: ejects tasks, launch command, exit renders and delete renders.

- MS Windows release moved on MSVC 2019, Python 3.9.6 and Qt 5.15.2.

	.. warning::

		MS Windows 7 is no longer supported, as Python 3.9 dropped it.

- **Bugfixes**
	- AfWatch jobs list right panel folders buttons refresh fix on a new job selection.
	- AfWatch jobs list switching from admin mode, other users jobs appear fixed.

v3.2.0
======

2021.04.18

**New Afanasy network protocol version.**

- Such config parameters as
  *af_render_heartbeat_sec*,
  *af_render_up_resources_period*,
  *af_render_zombietime* and
  *af_render_exit_no_task_time*
  moved to pool parameters:
  `heartbeat_sec <../afanasy/pools.html#heartbeat-sec>`_,
  `resources_update_period <../afanasy/pools.html#resources-update-period>`_,
  `zombie_time <../afanasy/pools.html#zombie-time>`_,
  `exit_no_task_time <../afanasy/pools.html#exit-no-task-time>`_.
  On change they will be dispatched to all pool renders.
  This way you can tune farm "on-the-fly". Parameter *af_render_connection_lost_time* removed.
  Now render just uses the same *zombie_time* parameter as server for connection lost.
- Parsers `do <../afanasy/render.html#do>`_ function takes arguments via dictionary.
  I hope that is was the last time we needed to change all parsers classes on a function interface change.
- Parser takes resources string and can return resources string.
  Takes host resources as JSON, that afrender gets for server and GUIs.
  Can return any custom resources, for example peak memory usage or (and) triangles count.
  Server stores this string in statistics database and dispatches to GUIs.
- `RENDER_NO_TASK <../afanasy/server.html#render-no-task>`_ event
  and pool `no_task_event_time <../afanasy/pools.html#no-task-event-time>`_ parameter.
- `RENDER_OVERLOAD <../afanasy/server.html#render-overload>`_ event
  and pool `overload_event_time <../afanasy/pools.html#overload-event-time>`_ parameter.
- AfWatch jobs thumbnails size buttons.
- Server `HTTP configuration <../afanasy/webgui.html#http-server-configuration>`_ added.
  Now it is more easy to make server to serve some custom or even several WebGUIs.

- **Bugfixes**

	- AfWatch Work monitor allows modifications only in admin mode.
	- AfWatch Users and Farm monitors allow selection and current item change only in admin mode.
	- AfWatch Job Block operations fixed to work on MS Windows.
	- AfWatch Job Tasks List window on open task progresses refreshing.
	- AfWatch Jobs List hidden nodes mix on reopen / monitor type or change fixed.

v3.1.1
======

2021.01.31

- AfWatch job item can collapse blocks.
  Useful for UI space economy, especially on jobs with a big blocks count.
  In a View Options menu you can collapse/expand all jobs,
  and set an option to collapse new jobs.
- Houdini submission ROP works with Python 3.
  Now you can choose to download Houdini with internal Python 3.

v3.1.0
======

2020.10.05

**New Afanasy network protocol version.**

- Afanasy pool tickets got maximum hosts property.
  This is mostly needed for licence hosts limits.
  There is a common type of licensing where you can run multiple instances of software on same host, occupying only single license.

v3.0.0
======

2020.09.13

**New Afanasy network protocol version.**

- :ref:`afanasy-pools`
	Now renders are organized in pools hierarchy.
	All farm (services, capacity, limits, ...) settings are configured by pool properties.

	.. warning::

		You will loose your previous farm setup described in **farm.json**.

- :ref:`afanasy-tickets`
	Pools Job blocks got Tickets.
	It is like named capacity.

	You can set root pool NUKE:20 pool tickets to limit Nuke licenses on the entire farm.
	You can set some pool MEM:64 host tickets to limit RAM.
	And set corresponding tickets to your job blocks.

- Render node becomes *Sick*, when it produces errors only from different users.
- ``RENDER_SICK`` and ``RENDER_ZOMBIE`` events.
- AfWatch got side panel to manipulate nodes.
- AfWatch admin mode let you to mark task as DONE w/o SKIP state.
- Block got a server information string.
  Now it used to store last started task host name.
  GUIs show it.
  Useful for a single task blocks, no need to open job to see what host your simulation running on.
- Try this task next.
  You can ask server to solve some task(s) as soon as possible.
  Also you can specify such tasks via Python API on a job submission.
- Each Afanasy node stores running services counts.
  AfWatch shows user and branch items running services.

v2.3.1
======

2019.03.11

- Afanasy
	- Linux packages moved on System D.
	- Windows service.
	- AfterFX `submission <../software/afterfx.html#afterfx#tool-dialog>`__ improvements. More setting appeared.
	- NVIDIA `nvidia-smi <../afanasy/render.html#nvidia-smi>`__ Python custom resource class</a>.
- RULES
	- News, Bookmarks and Recent items display status.
	- Incoming fresh News statuses update folders statuses.
	- Scenes shots filtering mutes not found artists, flags and tags.
	- Bookmarks folders.
- Bugfixes
	- Afanasy:
		- Blender submission fix.
		- Redshift parser fix.
		- Houdini distribute Alembic ROP fix.
		- AfWatch any operation does not affect hidden items.
		- AfWatch setting string parameter JSON value escape added, you can set annotation with quotes.
		- AfRender custom resources meter Python classes fixed to work within Python 3.
		- Multi-host task start and server hung fixed. This bug appeared in 2.3.0.

v2.3.0
======

2018.10.17

**New Afanasy network protocol version.**

- Afanasy:
	- Generally new class :ref:`afanasy-branch` appeared.
	  Now jobs solving is going within branches hierarchy.
	  Branches can represent departments/projects/assets and you can vary their priorities.
	- Job block/task command and files pattern frame replacement is processed in a Python service class.
	  Later it was coded in libafanasy and processed by afserver, and you could not alter it.
	  Now you can use custom frame patterns.
	  AfRender receives a pattern and frame settings (first, last, increment) instead of a ready command/files.
	  This is much more flexible.
	  For example, now in a Python service class we can check all numeric task files for existence and size.
	  And decide to skip task execution if all files are fine.
	- Task `progress change timeout <../afanasy/job_block.html#task-progress-change-timeout>`__  job block parameter.
	- Task `minimum run time <../afanasy/job_block#task-min-run-time>`__ job block parameter.
	- New user max_run_tasks_per_host and jobs_life_time default values are configurable.
	- Some config parameters renamed:
		- ``af_maxrunningtasks`` -> ``af_max_running_tasks``
		- ``filenamesizemax`` -> ``file_name_size_max``
	- Houdini submission:
		- Job Branch, Wait Time and Task Minimum Run Time parameters added.
- RULES:
	- Each playlist item has an own delete button.
	- Shots export to table *frames_num* column added.
- Bugfixes:
	- Afanasy:
		Server - Python API communication invalid JSON answer fixed:
			- Server JSON answer will never contain extra ``A`` character after JSON object finish (latest ``}``).
			- Python API will never try to read JSON data over message size.
	- Server JSON answer *task_files* fixed, now it escape file names.
	  Later when server was running on windows, ``\`` slashes produced JSON syntax error.
	- Server hung on incoming JSON with invalid syntax fixed.

v2.2.3
======

2017.11.02

- Afanasy:
	- Server creates all needed threads for network IO at start.
	  Networking threads pool size is configurable.
	  Later it has to create a thread for each incoming connection.
	  Now server can handle bigger amount of clients, consumes less system resources for it.
	  Later on some systems afserver could even hung when clients count is more than about a hundred.
	  As system can fail to create 100 threads per second for a long time (have no time to free all thread resources every second).
	- Linux server can be configured to use not-blocking network IO based on Linux EPOLL facility.
	  By default Linux will use the same blocking (threading) IO, as on other platforms.
	  Non-blocking IO consumes less system resources and can handle more connections at the same time.
	- AfWatch (libafqt) switched to non-blocking network IO based on Qt Signal-Slot mechanism.
	  Now it works better (less hangs) with afserver that has a big total amount on clients.
	- Farm setup allows new machines can be configured to register paused/nimby.
	  This can be useful for a just born machines to not to produce error tasks.
	  When a new afrender registers, but has not enough software installed yet.
	- You can find for some string in the text in task output/log in AfWatch.
	- Double click needed on a button to skip and restart task in AfWatch task window.
	  This can help to prevent unwanted task restart by a single occasional click.
	- Previewcmd options added: Houdini *Mplay* and *DJV* open source sequence player.
	- Houdini submission:
		- Shorter names for afanasy temporary .hip files.
		- Camera verification for Mantra submission.
		- Support for *Alembic* submission with progress.
		- Support for *Wedge* submission with block per wedge.
- Bugfixes:
	- Afanasy:
		- After server restart, reconnected tasks are not ignored by "Max Run Tasks Per Host" limit.
		- Multi-host task start and server hung fixed.
		  This bug appeared in 2.2.1.
		- MS Windows server tasks state storing fixed.
		  Later, after restart, it run done tasks again.
		  This bug appeared in 2.2.1.
		- Server memory leak fixed.
		- Houdini Current frame submission fixed.

v2.2.2
======

2017.05.21

- RULES:
	- If status progress is 100% all its tasks are considered as done.
- Afanasy:
	- Maya Redshift support.
	- Parser updates: Fusion, Redshift, Arnold, Redline, Rsync.
- Bugfixes:
	- Afanasy:
		- MS Windows AfWatch and jpeg images (thumbnails) reading fixed.
		  Missing Qt5 dll added to the package release archive.
		- AfServer on some Linux distributions can hung when clients number over 100.
		  Fixed - reduced default (configurable) afserver client thread stack size.
		- Parser Error fixed.
		  Later on Parser Error, afrender can ignore its restart from afserver and task update/stop timeouts happen.

v2.2.1
======

2017.01.28

- `Forum <http://forum.cgru.info/>`__.
- Keeper shows machine memory usage in a system tray icon.
- Afanasy:
	- Isotropix :ref:`software-clarisse` support.
	- AfServer solves jobs by running tasks total capacity (by default), not just by running tasks count.
	- Each new job acquires an unique serial number. You can get jobs from server by serial.
	- Linux distributives that has a native Python 3, Qt 5 switched on these new libs version.
	- Qt 4 and Python 2 are still supported. There is no plan to discontinue this support for the near feature.
- RULES:
	- Artists got automatic bookmarks on assigned shots.
- Bugfixes:
	- Afanasy:
		- MS Windows AfServer WebGUI hosting fixed.
	- Blender parser fixed to work with new versions (previous still supported).
	- VRay parser fixed to work with new versions (previous still supported).

v2.2.0
======

2016.11.23

**New Afanasy network protocol version.**

- Afanasy:
	- On server restart it reconnects running renders tasks. New task "WaitForReconnect" state.
	- New job flags designed for "Maintenance" added:
	  *maintenance*, *ignorenimby* and *ignorepaused*.
	  Now you can run some command once on each render (even if it is "Nimby").
	  For example you can install software this way.
	- GUI got "task" window. One place to view and manipulate job block task.
	- New render "Paused" state.
	  It is like "Nimby" but without "Auto Nimby".
	  Only admins can change this state.
	  Designed to disable render permanently while "Auto Nimby" is enabled.
	- Parsers got "tagHTML" function.
	  It designed to mark task output for AfWatch GUI.
	  For example replace terminal escape sequences, highlight errors.
	- AfWatch GUI nodes list has a second sorting parameter.
	- Houdini submission:
    	- Added minimum memory, PPA settings, render temp HIP and wedges support.
    	- *Separate Render* deletes ROP files not after render, but on job deletion (you can re-render w/o re-generation).
    	- Distributed simulations support.
	- Job Block *environment* parameter added. Render can run task process with some extra environment.
	- You can quickly :ref:`override <config-overrides>` any config parameter w/o any file changing.
	- You can enable/disable services by a regular expression.
	  It is useful when you have several *houdini_.** types services.
	- JOB_DELETED event added.
- RULES:
	- Scenes/Shots asset: Export shots to HTML table.
	  You save/send this table. Print to PDF. Open in Exel/Word.
	- Files view: You can colorize and annotate any item.
	- Walk: Calculates and stores disk usage along with total files size.
- Bugfixes:
	- Afanasy:
		- AfServer store folders removal fixed on some modern file systems.
		- AfServer on windows thumbnails serving fixed.
		- AfServer now reset depend state on a job block if it depend mask changes on empty string.
		- AfServer does not send job changed event every cycle if a job block has depend mask.
		- Mac OS X: AfRender memory resources detection fixed.
		- AfRender get CPU frequency each time it measures resources, and stores its maximum.
		  Now most machines can change CPU frequency depending on load.


v2.1.0
======

2016.04.29

**New Afanasy network protocol version**

- Afanasy:
	- Clients does not listen any port (afrender and afwatch).
	  Server does not connects to clients itself.
	  This means that no local network needed any more.
	  Also it increases maximum clients quantity.
	- MS Windows build now compiled with MSVC 2015.
	  You may needed to install
	  `Redistributable 2015 x64 <https://www.microsoft.com/en-us/download/details.aspx?id=48145>`__
	  package to work, if you have some old updated Windows OS.
	  It also it has Python 3.4.4 and Qt 5.6.0 versions.
	- WebGUI can listen job and task outputs.
	- Job got *report* report.
	  It is some important info to show in GUI that can be returned from a task process parser.
	- Job Block got *skipexistingfiles* and *checkrenderedfiles* flags.
	- Service and parser can write to task log.
	  This is useful when you decide to skip a task or mark it as an error from service or parser
	  to explain why you did it.
	- You can ask render to execute custom command and exit (or not) after.
	- Multi-host task can ignore slave host missing.
	  To control this, *slavelostignore* job block flag was added.
	- Wake-On-LAN: Sleep and wake commands are constructed in 'wakeonlan' Python service class.
- RULES:
	- File buffer to move folders/files.
- Bugfixes:
	- Afanasy:
		- AfWatch: Turn off listening job/task fixed.

v2.0.8
======

2015.12.06

- Afanasy:
	- AfWatch desktop notifications.
	- Server waits client have closed network connection first.
	  This way there is no TCP socket TIME_WAIT state on server.
	  It can be needed for a big amount of clients.
- RULES:
	- Shot tasks price.
	- Auxiliary folders.
	- User last entries record: IP, URL and time.
- Bugfixes:
	- Afanasy:
		- Thumbnails double generation fixed.

v2.0.7
======

2015.10.23

- Keeper:
	- Show and change local render user name.
- Afanasy:
	- :ref:`software-fusion` integration.
	- :ref:`software-natron` integration.
	- Job has folders string map parameter. It can be user in GUIs to open some location.
	- Statistics folders table and graph.
- RULES:
	- Player link to the current frame.

v2.0.6
======

2015.07.17

- RULES:
	- Dailies with sound.
	- Deploy shot renaming prefix and find/replace regexp.
- Afanasy:
	- Auto NIMBY and Auto Free now can depend on MEM, SWAP, HDD and Network usage.
- Bugfixes:
	- Afanasy:
		- AfWatch can reset (set to an empty string) job block mask (host, depend).
		- Python parser class appendFile function fixed (old style parsers lost thumbnails in 2.0.5).
		- MacOSX compilation fixed (2.0.5 has compilation errors).

v2.0.5
======

2015.06.30

- RULES:
	- Upload rules.
	  You can describe a rules to upload .mov files in a shot dailies folder and .zip files in results folder.
	  So no artist can upload everything just in a shot, and there is no need to know where shot dailies are located.
	  And a news will be created on upload.
	- Edit body and tasks of a several selected shots.
	- Tasks has prices.
	- You can add scene(s) selected shots to playlist.
- Afanasy:
	- Preview Pending Approval ``PPA`` flag.
	  Now job can render just tasks that described with a sequential parameter (every 10 frame, for example).
	  Then job falls in a PPA state and stops to solve tasks.
	  Artist can check that every 10 frame and unset PPA to continue or delete a job.
	- Sequential behavior slightly changed.
	  Now it renders first and last frames at first, then sequential frames.
	- AfStarter and afjob.py supports Natron.
	- AfRender can generate <a href="afanasy/render#thumbnails" target="_blank">thumbnails</a>
	  while task process is still running.
	  Was designed and now used in dailies creation.
- Bugfixes:
	- Afanasy:
		- User can change his job priority above the default value.
		- Afwatch can show hidden node on some parameter change.
		- Set farm auto nimby parameters to zero (to disable them) and reload farm 'on-the-fly' (afcmd fload) now works.

v2.0.4
======

2015.02.26

- RULES:
	- Permissions to edit tasks, body, playlist, assign artists.
- Afanasy:
	- Job block frame <a href="/afanasy/job#Sequential" target="_blank">sequential</a> new parameter.
	- AfWatch can edit custom data for job and user.
	- You can get farm setup from afserver via json.
	- Server can save json object.
	  This can be useful to edit config or farm setup.
	- WebGUI major changes, but not finished, in progress.
	  New idea is less RMB menus.
	  Actions are buttons on the left control panel,
	  Parameters manipulation is on the right panel.
- Bugfixes:
	- Afanasy:
		- Events service fixed (was broken in 2.0.3).
		- Server memory leak fixed. It was small and rare, probably you did not noticed it.

v2.0.3
======

2014.11.09

- Blender:
	- Blender plugin was completely rewritten.
	  Now there is a CGRU Tools Addon and Afanasy is a part of it.
	  Afanasy now not a Blender render engine.
- RULES:
	- Create Nuke scene in a shot using template.
	  Scene will have good project settings, sources and results (Read and Write nodes).
	- News display filter.
	  You can show/hide/delete specific news (dailies, reports, status, ...).
	  Filter news by a project.
	- Results invalid naming highlighting and tool-tip.
	- Status edit: artists are combined by roles.
	- User states: admin can allow user to change his password,
	  make user not-an-artist to hide him from status edit.
	- Player: show images while loading.
- Afanasy:
	- Afrender calls parser function on task finished in any case, even if there is no new output.
	  This may be needed if want to perform some finalizing actions in your custom parser.
	- Service python class has a function to check task process exit status value.
	  By default (in service.py) zero is considered as a success, any other as an error.
	  But now you can override this function in you custom service.
	- Web GUI supports afrender custom resources monitor.
- Bugfixes:
	- Afanasy:
		- Paths map (mixed os farm) and non-ascii character(s) fixed.
		  Now you can have national characters in paths,
		  but it is not recommended in any case.

v2.0.2
======

2014.08.19

- RULES:
	- Shot tasks and reports statistics.
	- Create and extract archives via Afanasy farm.
	- Put files on FTP via Afanasy farm.
- Afanasy:
	- GUI: Job item ETA.
	- Server:
		- On a new job, server returns its ID.
		- Tasks solving speed limit configurable <a href="/afanasy/server#af_serve_tasks_speed" target="_blank">parameter</a>.
	- Events: Server sends the entire job JSON object to render.
	  You can use any job parameter in an event Python service class.
- Movie Maker (RULES Convert and Dailies):
	- Apple ProRes422 and ProRes444 10-bit codecs presets.
- Bugfixes:
	- Afanasy:
		- Cmd: Send json job and an error message in any case fixed.
		- Watch:
			- Listen task/job output fixed.
			- Zero thumbnails quantity preference and crush fixed.
		- Render: Task output maximum size and output middle truncation invalid characters.
		- Server: Task that reached maximum running time limit takes ERR state.

v2.0.1
======

2014.04.10

- RULES: Convert multiply selected movies or sequences to other movies or sequences.
  You can change codec, fps, resolution and
  convert DPX-es to JPEG-s with a specified colorspace and quality, for example.
  Such calculations will be processed on a farm via Afanasy.
- Movie Maker: AV conversion tool is configurable.
  You can set a custom command or executable path.
  The default is *ffmpeg*.
  Some Linux distributions switched from *ffmpeg* to *avconv*. For now, they are fully compatible.
- Afanasy JSON protocol:
	- Jobs list can be generated providing to server an array of user names.
	- Server configuration and farm setup can be reloaded via JSON message.
- Bugfixes:
	- Movie Maker and RULES thumbnails: EXR and DPX colorspace problem is solved.
	  You need at least ImageMagick >= 6.8.8-8 version for it.
	  EXR bug was in CGRU, DPX bug was in ImageMagick.
- Afanasy Web GUI: Sorting and filtering parameters storing.
- Afanasy Server: Several bugs that can cause hang fixed.
  You should definitely switch to this version as soon as possible.
  It is fully compatible with 2.0.0 (you can just replace afserver binary only).

v2.0.0
======

2014.03.01

- No SQL
	Afanasy server stores state in *json* files in its temporary folder.
	Now SQL stands for statistics only.
	If you does not need statistics you can not to setup SQL at all (or setup it later).

	.. warning::

		Server state will not be stored switching to this version.
		You will loose all jobs, renders and users settings if any.

	*So now on MS Windows OS, Afanasy server does not needs any installation/configuration procedures to work.
	Just run (double click) cgru/start/AFANASY/_afserver.cmd.
	Or drag a link to Startup menu for auto launch at logon.*

- Authentication
	Afanasy *json* protocol has an authentication mechanism.
	It uses `Digest Access Authentication <http://en.wikipedia.org/wiki/Digest_access_authentication>`__ method.
	IP Trust mask allows to skip authentication.
	By default mask allows any IP, and if you did not configured it, you should not notice authentication at all.
	Binary protocol does not have authentication mechanism.
	If IP does not match trust mask and message uses binary protocol (not *json*) - message will be ignored.
	This was designed to use Web GUI not from a local network.
	Note, that it is only authentication mechanism and not data encryption.
	But passwords are not sent in a plain text, and even are not stored in a plain text (see Digest description).

- Python service class got doPost method.
	You can do some post process there.
	If post command requires enough calculation, you can return a list of commands (strings) from this function.
	In this case all that commands will be executed in child subprocesses and output will attached to task output.

- Thumbnails
	If task (block) has files parameter or parser finds images thumbnail will be generated.
	Thumbnails are generated by afrender.
	Python service *doPost* function returns commands for it.
	This commands can be configured.
	Thumbnail files binary data is send by afrender to afserver along with task output.
	Server stores all files that afrender sends on task finish.
	AfWatch and Web GUI can show thumbnails.
	You can get tasks thumbnails from afserver by HTTP GET method.
	Python parser class can find images in task output.
	Python service class can ask parsed images for thumbnails generation.

- Python parser class got mode argument in parse function.
	This argument stands for task subprocess status.
	For example, now parser knows whether the task is running or finished and how it was finished.
	Now if a task has finished with success you can set an error if output does not contain some required result.
- All plugins from *cgru/afanasy/plugins/* moved to *cgru/plugins/*.
	That old plugins location came from SVN age, when Afanasy has branches, tags, trunk. Within Git it is not needed.
	So now there is no mess where to put or find files in *cgru/plugins/* or *cgru/afanasy/plugins*.
- You can add new user via JSON. An example is located in *cgru/examples/json/*.
- WindowsMustDie function configures via general configure system (json files).
  So there is no a special *windowsmustdie.txt* file now.
- Python service class now instance parser class itself.
  So you can exchange information between service and parser classes.
- Python API Block and Task classes *setFiles* method takes an array of string.
  And not a single string where several files are separated with ';'.
  **You should fix your custom submission scripts if any.**
- CGRU Home folder on MS Windows OS moved to *%APPDATA%/cgru/*.
  It is used to keep user personal configuration.
  Previously it was in *%HOMEPATH%/.cgru/* where *%HOMEPATH%* is usually user Documents folder.
- Web GUI is not "beta".
  It is a full functional GUI for Afanasy, that can replace AfWatch (Qt).
- RULES is not "beta".
  But there is still the lack of documentation and lots of things to do.
- There is no *temporary* users.
  Any (each) user in stored in its json file.
  No *af_user_zombietime* variable - time for temporary user to have no jobs to be deleted.
  (Temporary was a user that was not stored in SQL database.)
- Maya
	- No overriding scripts.
	- No auto scripts sourcing.
	- No plugins auto load.
	- No CGRU main menu auto launch.

	Sow now CGRU in Maya is just a set of stand-alone scripts,
	and it does not modify any native Maya interface and workflow.
	This means the lost of some features:

	- No autosave manager.
	- No auto project seek.
	- No Outliner and Channels menus custom items.

	Since Maya 2014 CGRU main menu appears on load *cgru.mll* plugin.
	For auto load, enable it in plugins manager window.
	Or you can source *cgru.mel* from a shelf or *userSetup.mel*.
- Movie Maker
	*ffmpeg* and *convert* binaries are removed from Linux packages.
	There are two reasons for it.
	Modern Linux distributions has various dependences to build and install them,
	so it begin harder and harder to support them in CGRU.
	Also modern Linux distributions already has enough high versions of this products to support EXR and H264.
	If you need to some special version of this binaries, you can to download and build it yourself,
	there is no problems in Linux to compile them manually.

	Debian based packages will have *ffmpeg* and *imagemagick* (*convert*) dependences.
	As all such distributions has them in native repositories (they are usually enough big).

	RPM based packages will not have only *imagemagick* dependency,
	as for *ffmpeg* you need to add some extra repository (native repositories are usually small).
	The exception is AltLinux.

	MS Windows release will continue to contain this executables.

- AfTalk Afanasy chat client was removed from the project.

- Bugfixes:
	- Server hung when a job with no blocks sent.
	- Change job bock (tasks) command (working folder) change from watch GUI.
	- Post command ignore when job json file send with afcmd.
	- Change any user parameter resets jobs solving method to 'order'.

v1.7.0
======

2013.06.05

**New Afanasy network protocol version.**

- WEB GUI (beta).
- RULES (beta).
	It has begun!
- Forum (beta).
  Based on RULES web engine.
- Parser can return running task *activity* string parameter.
  For example Nuke can notify which of stereo views is rendering now.
  Movie Maker notify whether an encoding is started.
  Activity string is shown by GUI in job tasks list window for each task item.
- Render client Nimby can be set to free if computer is idle for some time.
  You can configure it in farm setup.
  Machine considered as idle if CPU busy percentage is less than *idle_cpu* value.
  It is useful for render on workstations that artists left.
- Render client Nimby can be turned on if computer CPU is busy for some time and has no Afanasy task.
  You can configure it in farm setup.
  Machine considered as busy if CPU busy percentage greater than *busy_cpu* value.
- Afanasy server sends to GUI tasks percentage with renders list.
  GUI renders list items show running tasks percentage.
- New system job block - **events**.
  New service - **events**.
  Afanasy server can generate events, on job error, for example.
  Events are pushed to system job as tasks for events block.
  Render farm can process events, send email notifications for example.
- Each afnode has a custom data.
  Afanasy server sends this data to render to service class with a task.
  In Python service class you can do with this data what you want.
  For example user email parameter and events settings are stored in custom data via JSON.
- You can restart all job running tasks from GUI menuitem.
- Archived binaries Python version is 3.3.2.
- Bugfixes:
	- AfWatch: Several blocks selection for some action works.

v1.6.12
=======

2013.03.22

- Afanasy configuration now has parameters to control user ability to change priority:
  *af_perm_user_mod_his_priority* and *af_perm_user_mod_job_priority*.
  By default user can change his own priority and his jobs priority.
  Set this parameters to *false<* and only admin will be allowed to change priorities.
- Movie Maker: Apple ProRes codec presets.
- Tested with Nuke 7 - works fine.
- Bugfixes:
	- Movie Maker: H264 (ffmpeg-libx264) uses 420 pixel format instead of 444 to work on most players.
	- Nuke Submission: Fixed to render Write-nodes inside group.
	- Nuke Render Script: Fixed to render different views in different folders.

v1.6.11
=======

2013.02.15

- Maya users should look at  `meTools for Afanasy <http://meshstudio.blogspot.ru/2013/01/metools-for-afanasy.html>`__. And use it.
- Nuke and Paths Map: Filename filter can be added to always have valid paths on any OS in the same script.
  You can configure to add or not to add it - not to break you potential in-house filters.
- Tested on Windows 8 - works fine.
- Bugfixes:
	- Nuke Submission: Negative frame range fixed.
	- Nuke Render Script: Fixed to render several views in one file (you can write stereo in a singe EXR).
	- AfStarter Blender: Now does not ignore output images parameter.
	- Paths Map: Now works with big files thousands times faster.
	- AfServer: Enable/Disable service fixed (was broken in last versions while json protocol switch).
	- PyQt: Open file dialog fixed to work with old PyQt versions (4.6.2 - CentOS 6).

v1.6.10
=======

2012.12.21

- Bugfixes:
	- AfServer: Creating temp folder it tries to create all parent folders.
	- Keeper: Set Afanasy server fixed.
	- World: No the end, fixed.

v1.6.9
======

2012.12.19

- Cinema4D: Submission switched from *afjob.py* command to Afanasy Python API.
  So there are no issues with *subprocess.Popen* any more.
  Same code works fine on all platforms.
- Bugfixes:
	- AfStarter: Output images browse file button fixed.
	- Cinema4D: Render scene with spaces in path fixed.
	- Keeper: Software setup fixed (select executable dialog).

v1.6.8
======

2012.12.10

- Automatic Wake-On-LAN.
- Bugfixes:
	- Nuke: It does not really use render script when it should not (when there is no paths map or temporary images).

v1.6.7
======

2012.12.03

- All CGRU config files moved to JSON.
  It refers to any Afanasy configuration, farm setup, paths map.
  XML is removed from the project at all.
  Any XML config file will not works.
  AfWatch GUI turning will be reset.

  .. important::

  	You should reconfigure Afanasy.

- Afanasy user 'home' configuration files moved to *HOME/.cgru* from *HOME/.afanasy*.
- One config file can include another file(s).
  Specify a files to be included in "include" string array.
  All include files will be included after all file will be read (not like include directive in most common program languages).
  This is done to override file contents.
  Any next occurrence of a variable with the same name will override previous value.
- Config file can have OS specific section.
  So you can setup different OS-es configs in the same file.
- Paths map setup moved to common config files.
  And you can setup paths map for all OS-es in the same file.
- Some general config parameters, as time format, maximum file name length, command shell, preview commands,
  moved from Afanasy specific config to global CGRU config.
  As they can be used later by other CGRU tools.
- Afanasy on start-up reads CGRU config file and does not tries to find some specific config itself.
  CGRU config file simple includes Afanasy specific config file.
  All Afanasy specific parameters has *af_* prefix now.

v1.6.6
======

2012.09.26

- All Python applications with GUI in CGRU can use and PySide and PyQt.
  At first PySide will be tried to import and than PyQt.
  It means that if you have PySide installed it will be used.
  PySide has LGPL license, PyQt - only GPL.
  So now <b>all components in CGRU has LGPL license</b> or similar.
- New Linux package *afanasy-qtgui* appeared.
  Needed only to remove *libqt* dependence from *afanasy-render* and *afanasy-server* packages
  (to not to install huge Qt on render nodes).
- Bugfixes:
	- ``afcmd uadd`` works fine (is was broken in v1.6.5 - it added users that can't run any tasks)

v1.6.5
======

2012.09.04

- Movie Maker can fake dailies date and time.
- Python Parser class can consider that task is already done and ask render to stop a task.
  AfRender sends to server that it was finished with a success.
- Bugfixes:
	- Movie Maker open/save parameters and non ASCII characters bug fixed, all operations uses UTF-8 encoding.
	- AfStarter and negative frame values (actually the bug was in afjob.py).
	- Negative frame values and numeric commands with padding (afserver generates commands, so it should be restarted).

v1.6.4
======

2012.06.26

- CGRU now has a domain https://cgru.info.
  Soon documentation from sourceforge.net will be removed.
  If you have RSS subscribed, you should resubscribe on http://cgru.info/doc/cgru_rss_feed.xml
- Bugfixes:
	- Python API *af.Block.setHostsMask* and *af.Block.setHostsMaskExclude*
	  methods are back after occasional deletion when switching to JSON.
	- AfStarter maya_mental submission set verbose level for task progress parsing, afjob.py changed for it.
	- AfStarter dialog GUI dialog bug fixed: *first_frame* <= *last_frame check* works correctly.
	- Nuke CGRU menu open/save scene through paths map fixed.


v1.6.3
======

2012.05.07

- Nuke render and submission scripts options added to skip paths map and render to temporary image stages.
  Render hosts (farm) should be updated too to recognize such options, as not only submission script changed.
- Bugfixes:
	- Nuke render just one frame fixed.
	- Depend sub task and depended block frames per task > 1 fixed.

v1.6.2
======

2012.04.23

- API is based on JSON now.
	Python API is the same but no binary module needed, it communicates with server itself by JSON build-in module.
	(All Python API is written on Python language, not on Python C API.)
	You can communicate with Afanasy server within any language/script that can create JSON structures.
	(No libafanasy needed to send and get data, all possible linking problems are in the past.)

	JSON protocol is not finished. Finished only job structure - to remove python binary module dependence to send a job.

- Bugfixes:
	- AfWatch shows tasks with no service icon.
	- AfWatch filtering and sorting nodes when new nodes created and old changed fixed.
	- Houdini render script loads scene within try-catch to pass warning exception.
	- Nuke dailies node can handle tcl expressions, it uses *getEvaluatedValue()* instead of *value()*.
	- Nuke render script changes *root.project_directory* according to OS paths map (for mixed OS-es farm rendering).

v1.6.1
======

2012.03.28

- Tasks can be solved in a not-sequential manner.

  For example 1-10: 0 9 5 2 7 1 3 6 8 4

  This can be needed to catch some error earlier and to calculate average running time more accurate.
- You can hide jobs or renders in AfWatch by some parameter.
  Also you can show only hidden nodes.
  And a new "hidden" parameter was added to every node (job, render) just to hide (and store hidden state).
- Bugfixes:
	- AfWatch can preview tasks of a not-numeric blocks in a task information window (by double click).
	  This is a main reason of this release.

v1.6.0
======

2012.03.22

**New Afanasy network protocol version.**

- New parameter added to configuration *cmd_shell*.
  Render will launch tasks commands with it. Default values are:
  - UNIX: ``/bin/bash -c``
  - MS Windows: ``cmd.exe /c``
- Administrator (super users) can change job owner.
  It can be performed by AfWatch GUI and afcmd CLI.
- You can enable/disable render service via afcmd (CLI).
- AfWatch GUI styles available.
  You can change, copy, modify them, create your own.
  You can set sounds to playback on some events (Job added, finished or got an error).
- Renders list has an ability to change items size.
- No Qt library in Afanasy render client.
  So Qt is used for GUI only now.
- *Magic Number* to filter connections.
- Afanasy server is available for MS Windows OS.
- GitHub https://github.com/cgru CGRU project started.
- Afanasy branches removed from repository.
  Use git for branching.
  As there is no need in branches in project subdirectories structure.
- Bugfixes:
	- Server bug fixed.
	  It could hung on job submission.
	  It was a very rare deadlock bug.
	  I never managed to catch it for 4 years.
	- Keeper hung on new network protocol version fixed.

v1.5.5
======

2012.02.12

- Cinema 4D support.
- Maya Bins release removed.
  Use archive for MS Windows for or Linux to get plugins for Maya.
- Nuke dailies gizmo can encode only (skip convert stage).
- Movie Maker allow user to specify container to encode movie to (mov, avi, ...), through GUI dialog or command line argument.
- Movie Maker can save and load settings, keeps recent jobs options.
- Nuke submit and render scripts can handle write node file expressions.
- Python Class Block - added following functions: setErrorsAvoidHost, setErrorsForgiveTime, setErrorsRetries, setErrorsTaskSameHost.
- Afanasy stand-alone starter has an ability to add some custom arguments to command.
- Server has an acceptable IP Addresses Mask.
  Connections form addresses not matching specified masks will be ignored by server.
- User can set jobs solving method to parallel.
- Afanasy now supports only PostgreSQL database engine.
  QtSql library replaced with native PostgreSQL libpq in libafsql module.
  So there is no Qt in afserver and afcmd applications (as later Qt was removed from libafanasy).
  Do not forget to update you server database connection settings, if you override defaults.
- Bugfixes:
	- Afanasy Starter error message in console fixed, sending a job and with Python 2.x
	  (Fedora Linux raises a warning in system tray in this case).
- Paths map works in lower case mode on windows and only in client -> server direction
  So you can use paths with uppercase letters with UNIX clients and MS Windows.

v1.5.4
======

2011.12.22

- `AltLinux <http://www.altlinux.ru>`__ RPM packages support.
- "Nimby" schedule improved.
  Now if *time begin* > *time end* it assumes that *time end* is tomorrow.
  So now you can set for example for Monday that *time begin* is 14:00 and *time end* is 1:00, and it makes render free at Tuesday 1:00.
- Afanasy stand-alone starter has an ability customize command, preview images and OS needed for render.
- Blender Cycles render engine support.
- Windows version switched on MSVC 10 SP 1.
- Release archives switched on Qt 4.8.0.
- Bugfixes:
	- Error messages in standard output fixed opening Movie Maker and Afanasy Starter dialogs.
	- Afanasy render and server Linux packages post install scripts fixed.
	  On some systems they were unable to create *render* user, if it does not exist.

v1.5.3
======

2011.12.05

- Bugfixes:
	- Home configuration folders and files permissions. Now they writable to all.

v1.5.2
======

2011.12.02

- Movie Maker can decode movie to sequence and add sound to movie from an audio or another movie file with audio.
- Keeper tray icon displays Afanasy local render client status.
- Bugfixes:
	- Keeper AFANASY client operation local host name bug fixed.
	  Bug was, for example, if you are setting NIMBY on "c1" machine,
	  it will be set to all computers with name starts with "c1": "c10","c11","c19" ... 
- Movie Maker fixed to work with a sequence without padding specified ("%d" or single "#" character).
- Houdini submission fixed. Afanasy ROP got a check for a null connection.
  Full path to ROP is used. You can to submit ROPs placed anywhere in a scene, not only from "/out/".

v1.5.1
======

2011.11.14

- **Keeper** - CGRU applications managing program.
- **Afanasy Starter** - Standalone dialog to submit jobs to Afanasy.
- **Adobe After Effects** support.
- Linux packages structure simplified. Some of them removed.
- Server farm setup *clearservces* directive.
- Movie Maker input images and output movie pixel aspect and auto input aspect. Custom aspect cacher.
- Scan Scan input images and output movie pixel aspect and auto input aspect.
  Search path include and exclude patterns.
  Search files older than some date option. Place result relative to the sequence.
- Python 3 full support.
  You can build all Afanasy application with Python 3,
  construct and submit jobs,
  write services and parsers for render clients.
- Release for MS Windows uses Python 3.
  It provided with CGRU.
  You don't need to install and configure Python and PyQt on MS Windows.
  On Linux distributions native python version is used and you should to install native PyQt.
- Bugfixes:
	- SoftImage submit a scene with a spaces in file path.
	- Scan Scan does not try to create a movie just from one file with digits in a filename like a sequence.
	- When block (job) errors avoid host parameter is zero, block (job) does not avoiding any hosts.
	- Mac OS X Afanasy server with client connection error fixed, render client resources collection improvements.
	- Nuke submission frame increment parameter not ignored any more.

v1.5.0
======

2011.08.29

**New Afanasy network protocol version.**

- Houdini submission improvements.
  You can connect several Afanasy and other ROP nodes together to describe a complex job with dependencies between ROPs.
- Block tasks can depend on other block sub-task progress.
- New job parameter *Maximum running tasks per host*.
  The same parameter was added to job block.
- You can override render *Max Tasks* parameter directly from Watch in super user mode.
- New numeric pattern replacement rules.

	.. important::

		You should delete all jobs on server as their tasks commands can be invalid.

		Jobs created by your custom submission scripts probably will generate invalid numeric tasks too.
		But it is very simple to fix them.

		You should to fix your custom submission scripts.

- Server stores renders IP and MAC addresses in a database.
  So you can perform some operations with off-line renders after server restart (for example wake-on-lan).
- On start, server checks all database tables, and adds (removes) needed columns.
- All date/time and frame range parameters are 64bit integers.
- All numeric types has BIGINT SQL type.
- Numeric tasks block "frame increment" (or "by frame") parameter plays role in tasks generation.
  It means that blocks with this parameter grater then one will have less tasks number.

  .. important::

  		You should delete all jobs on server before upgrade to this version.

- Bugfixes:
	- Web Visor statistics average farm usage parameter does not ignore custom dates range.

v1.4.5
======

2011.05.26

- Server tries to reconnect to database when connection failed.
- Python 3 supported by Afanasy module.
  You can construct and send jobs using Python 3.
- Blender 2.5 support.
- Web visor statistics favorite user and favorite service column. You can specify dates to for statistics information tables.
- Autodesk Max, Maya and XSI 2012 support.
- Bugfixes:
	- Render "Division by zero" hung fixed.
	  It was very rare bug but you could catch it after machine sleep (was suspended with stored RAM and running afrender process).

	  .. note::

	  	Only Chuck Norris can divide by zero.

v1.4.4
======

2011.05.07

- Wake-On-LAN
- Render client sends network interfaces information to server (MAC and IP addresses).
- Watch can request information message from server about render client.
- Watch items tool-tips improved.
- Watch renders custom commands can use selected node(s) IP address ("@IP@" string will be replaced with it).
- Watch can set job block parameter for all selected jobs.
- Web-Visor statistics displays total counters row, first record date, services tasks quantity.
- When render can't import task service Python class, it imports services base class called "service".
- Movie Maker temporary images format and quality settings, option to auto correct color space (Linear and Cineon to sRGB).
- Nuke client-server-client paths map interface in a Nuke CGRU main menu.
- Bugfixes:
	- Render busy time calculation corrected (it affects GUI counter only).
	- Watch job tasks list window title - job total percentage fixed.
	- Watch job tasks list - block item tool-tip corrected.
	- Nuke dailies node - job (block) custom capacity not ignored.
	- Nuke afanasy node - "Wait whole frame range rendered" behavior corrected.
	- Client does not try to lookup Afanasy server if direct IP literals specified.
	- MS Windows 7 clients does not try to create Afanasy home folder if it is already exists.
	- Server reload farm setup on-the-fly fixed when new host has less services.
	- Server hung when user tries to restart or skip all job blocks (but not restart entire job menu item) fixed.
	- Listen entire job when some tasks are already running corrected. They begin to sent output too.
	- Fixed ffmpeg presets end-of-line for UNIX. On Linux they cause an error with Windows end-of-line.

v1.4.3
======

2011.04.11

- Bugfixes:
	- Some server memory leaks fixed.
	- Watch listen just one task bug fixed.

v1.4.2
======

2011.04.03

- Added **afcmd** commands to control jobs: start, stop, pause, restart.
- Bugfixes:
	- Lots of errors in Afanasy server log if it was launched without database connection fixed.
	- Watch jobs list stores sorting and filtering settings.
	- Nuke parser bug fixed (error could appear in Nuke 6.2).

v1.4.1
======

2011.03.30

- Farm Services Limits to describe a number of software licenses.
- Movie Maker can draw a logo on an images sequence.
- Watch renders list can sort and filter renders addresses.
- System job commands queue can be cleared by restarting task.
- Technical:
	- Default Python version is 2.7.1. Default Qt version is 4.7.2.
	- XML parser moved from Qt to `RapidXML <http://rapidxml.sourceforge.net>`__.
	  No library needed, it is implemented by headers only.
	- Regular expressions moved from Qt to `POSIX <http://en.wikipedia.org/wiki/Regular_expression>`__,
	  they are almost the same. No library needed. They are in C standard, already realized in GCC and MSVC>=2008SP1.
	- No Qt needed for *libafanasy* and so for *libafapi* and *libpyaf* too.
	  No errors can happen importing Python module in other software using Qt.
	- Windows version moved to static Qt libraries. No errors can happen with various Qt "dll"s in PATH.
	- If parsing is no needed, parser should have an empty string name.
	  Render do not tries to import parser module with an empty name, no error happen.
- Bugfixes:
	- Nuke render script: A try to delete moved temporary image removed.
	- Listen job and task output connection error fixed.
	- Numeric command frame(s) replacement bug fixed.
	  Now it replaces any number of %04d patterns with start and end frame in a cycle.
	  (The bug appears for example on a composite commands: "cmd1 && cmd2" or "cmd1; cmd2".
	  And when one task has several files for preview.)
	- Maya Auto Save Manager history backup filename from date and time construct on MS Windows bug fixed.

v1.4.0
======

2011.02.20

**New Afanasy network protocol version. New Afanasy database schema.**

- Errors forgive time for job tasks <a href="../afanasy/doc/job_block.html#ErrorsForgiveTime">block</a>
  and for <a href="../afanasy/doc/user.html#ErrorsForgiveTime">user</a>.
  It is a time form last host error to exclude it from error hosts list.
- System job
	Now job (and block) post commands are executed on a render farm by a special system job.
	**Your farm hosts must have "system" service to execute job post commands (remove rendered scenes).**
	``afcmd db_sysjobdel`` deletes system job from database.
	Will be needed if system job will have too much changes with new Afanasy version.
- Nuke *dailies* gizmo can be connected to *Read* node.
- Render views list can be customized.
- Job *Life Time* parameter added, for automatic jobs deletion after some time.
- WindowsMustDie
	windows names list can be defined in several files, matched windowsmustdie*.txt mask.
- User can sort jobs in Watch.
- Server does not store deleted jobs logs and tasks outputs.
- Release **bin_pyaf** removed. Modules for various Python versions are in every release now.
- Release **svn** added. It is an export of a repository.
- Bugfixes:
	- Nuke afanasy gizmo: If it creates output folder, it creates recursive all needed folders.
	- Watch job tasks list: Block item error hosts counters corrected.
	- Server stores job order in user list in database, so on server restart user jobs list order restored.
	- When parser on render finds an error, and than rapidly finds a warning, error status may be lost.

v1.3.1
======

2010.12.14

- Movie Maker output file naming customizable rules.
  This rules works for Nuke *dailies* node too.
- Server Farm Setup changed.
  Now host get setup form every matched pattern.
  And in each pattern you can precise host settings.
- Render reboot and shutdown commands can be configured.
- Bugfixes:
	- Watch job tasks window: Task item: Task host name string may overlap task name strings if this strings are long enough.

v1.3.0
======

2010.12.06

**New Afanasy network protocol version. New Afanasy database schema.**

- AfWatch shows services icons, it is common programs icons for users to recognize jobs type.
- Every Afanasy client has compiled revision number, startup version string and sends them to server.
  Most dialogues in CGRU show version, Afanasy GUI also shows clients build revision.
- Nuke *dailies* node to generate movies locally or on Afanasy farm.
- *movgen* service added. It will be used for movies generation: annotate frames, encode, make dailies.
- Bugfixes:
	- *ScanScan* codecs presets search folder.
	- SoftImage *VariRender* changes output folder name for every Framebuffer if folder is specified.
	- Houdini mantra filter (af_separate_render ROP) does not filter null images now (shadows for example).

v1.2.4
======

2010.11.01

- 3D Studio MAX submit to Afanasy scripts.
  MAX Afanasy service and parser.
- Watch can ask and launch a custom command with render items and has more sort&filter parameters. 
- Movie Maker stereo mode, DNxHD codec ffmpeg preset and Utf-8 full support.
- Linear float EXR and logarithm DPX to sRGB conversion bug fixed.
- H264 ffmpeg preset updated: good size&quality and frame navigation on MS Windows QuickTime player.
- Nuke stereo render views in different folders bug fixed.

v1.2.3
======

2010.08.18

- Houdini parsers total percentage calculation bug fixed.

v1.2.2
======

2010.08.17

- Movie Maker works with folders with spaces.
- MS Windows:
	- Afanasy Render prefix commands with *cmd.exe /c*.
	- Afrender kills all child tasks in any case.
	  (There was still some cases when it did not do it. Warning! QtCore4.dll patched, do not use it.)
	- Afrender measures network and disk traffic.
	- Afrender MS Windows version has the same functionality as Linux version.

- Package "afanasy-examples" removed. All examples are in "cgru" package.

v1.2.1
======

2010.08.06

- Afanasy server database communication bug fixed.

v1.2.0
======

2010.08.02

**New Afanasy network protocol version. New Afanasy database schema.**


- Afanasy Python *Custom Resources Meter*.
  You can measure any resource by writing you Python resource meter class.
- Afanasy Python Parsers has a new functionality.
  A parser can produce *warning* to notify user, *error* to stop task with error,
  *bad result* to finish task like with bad exit status (with error in any way).
- Afanasy render client *Windows Must Die* function.
  It finds and kills windows with specified names.
  When process crashes, MS Windows can raise a window with apologizes.
  This can hang process until someone closes the window.
  (AfRender periodically sends WM_CLOSE signal to windows listed in special file.)
- Houdini Separate Render
  ROP to separate Mantra ROP render process on 'ifd' files generation and 'mantra' command render.
  It can also split one frame into tiles and render them simultaneous,
  clean 'ifd' files, clean tiles and render an image in local temporary folder,
  and after successful render copy it to network location
  (it can save network traffic, as host do not often write small portions of an image during calculations).
- Block *Frames Per Task* parameter can be negative.
  Needed for sub-frame dependency.
- Afanasy has an ability to map paths.
  You can setup farm with various platforms clients.
  Submit jobs on Windows or Linux (MacOSX) to render and on Windows and on Linux (MacOSX) clients.
  Every client can have individual a paths map file to translate paths to server and from server.
- Movie Maker works on MS Windows.
  Linux releases has *ffmpeg* binary compiled with *x264* library to encode 'H.264' codec.
  Windows users need to install `ImageMagick <http://www.imagemagick.org>`__, which contains 'ffmpeg' with 'x264'.
- SoftImage XSI submit to Afanasy scripts.
  XSI Afanasy service and parser.
- Lots of bug-fixes for MS Windows platform. Windows version can be called 'beta'.

v1.1.0
======

2010.05.09

**New Afanasy network protocol version.**

- Afanasy supports IPv6.
  Server needs to support new protocol, as it stores client addresses,
  and do not ask name server at every connect (most managers do, alfred too).
- Nuke render script to render images locally in temporary folder and copy completely rendered image
  (it can reduce network traffic).
- Nuke render network: 'afanasy' nodes can be connected to describe 'Write' nodes dependency.
- **Movie Maker** Dialog and command line utility to make movie file from image sequence on Linux.
- RPM build scripts (tested on openSUSE, Fedora, CentOS).
- Windows Afanasy GUI applications does not open terminal.
- Maya 2010 and 2011 support.
- *fbx2clip* utility removed.

v1.0.0
======

2009.12.21

- New project structure.
  Afanasy source code repository contains 'tags', 'branches' and 'trunk'.
- CGRU has 'deb' packages build scripts (for Debian and Ubuntu Linux).

v2009.11.12
===========

- Afanasy project building uses **CMake**  cross-platform build system.
- CGRU environment initialization is much simplified.
  You do not need to edit or create scripts.
  To setup CGRU you need to go in it's root folder and source setup script (like in Houdini now).
  Unix and Windows examples corrected to work the same way.
  (And also total quantity of variables initializing by CGRU setup and needed for correct work is reduced.)

v2009.10.07
===========

- Python class *Job* has a *blocks* array property.
  You can manipulate it in your own way it to fill job with blocks.
- Python class *Block* can be constructed without any job and has a *tasks* array property.
  You can manipulate it in your own way it to fill block with tasks.
- Python class *Task* can be constructed without any block or job.

v2009.09.16
===========

**New network protocol version. New database schema.**

- Watch renders colors customization.
- **Multi Host Tasks** - tasks can run on several hosts.
- Python Class *Block* got *setMultiHost* method to describe multi-host tasks.

v2009.08.24
===========

**New network protocol version.**

- Afanasy Watch GUI can manipulate job blocks parameters without to open job tasks window.
- *afjob.py* supports tasks capacity and capacity coefficients.

v2009.08.20
===========

**New network protocol version. New database schema.**

- Job block capacity can be variable.
- Python Class *Block* got *setVariableCapacity* method to describe variable capacity.
- Job blocks errors solving parameters has '-1' value by default.
  It means to take this parameters from job user settings.
  Watch does not show this default values.

v2009.08.12
===========

**New network protocol version. New database schema.**

- Job block have a rule for generated tasks names.
- Not numeric block can generated tasks with preview.
- Python Class *Block* got *addTask* method to add tasks.
- Python Class *Task* got *Task* - New interface for not numeric blocks.
- Watch shows block generated task by double click on task in job tasks view.

