=======
Houdini
=======

.. warning:: Documentation is outdated.

Afanasy is represented by a special multi-functional ROP.
You can connect several other ROPs to Afanasy ROP to render.
You can connect several Afanasy ROPs to Afanasy ROP for a job with a complex denendencies.

Afanasy ROP
===========

- Submit
    Generate a job and send it to server.
- Start Paused
    Send a job in off-line state.

General
-------

.. figure:: images/houdini_afrop_general.png

	Afanasy ROP General tab

- Job Name
    Afanasy job name.
- Connected Nodes Are Independent
    Allow run the same frames of all connected nodes at the same time.
- Valid Frame Range:
    - Render Any Frame
        Use frame range form downstream node. Or render current frame if no range in network defined.
    - Render Frame Range
        Render this specified frame range.
    - Render Frame Range Only (Strict)
        Render this specified frame range. Other ROPs will wait this whole frame range rendered.
- Frames Per Task
    Number of frames in each task.
- Allow Sub-Task Dependence
    Tasks can wait other tasks to be done partially.
- Render With Take
    Specify take to render.
- Ignore ROP Inputs
    Do not execute input ROPs.

Parameters
----------

.. figure:: images/houdini_afrop_parameters.png

	Afanasy ROP Parameters tab

- Platform
    OS type the job can launch tasks on:
    ``any`` - any OS,
    ``Native`` - the same as the script was launched on.
- Enable Extended Parameters
    To switch ON/OFF it fast.
- Depend Mask
    Same user jobs names pattern to wait to be done to start
    (empty value means not to wait any job).
- Global Depend Mask
    Same as Depend Mask, but waits for a jobs from any user.
- Priority
    Job order in user jobs list
    (``-1`` means to use default value).
- Maximum Running Tasks
    Maximum tasks job can run at the same time
    (``-1`` means no limit).
- Capacity
    Tasks capacity value (``-1`` means use default value).
    Render must have enough free capacity to run it.
- Hosts Mask
    Hosts names pattern where job can run on
    (empty value means that job can run on host with any name).
- Hosts Mask Exclude
    Hosts names pattern where job can not run on.

Separate Render
---------------

Separate Render allows to separate render process on IFD files generation and render it by ``mantra``.
It can give several advantages on some *heavy* scenes.

Separate render generates a job which can:

- Render images locally in temporary folder and copy whole image after successful render.
  It can save your network traffic as render do not send small portions of an image during render process.
- Generate IFD file locally and then render it in separate process but in the same task.
  It can save render memory.
- Split one frame on tiles to render them simultaneously.
  So you can increase speed of one frame render.
  And also it can reduce memory needed to render a frame.
- Cleanup rendered IFD files and joined tiles images.

.. figure:: images/houdini_afrop_separate.png

	Afanasy ROP Separate Render tab

- Enable Separate Render
	Turn this feature on.
- Run ROP
	Run ROP to generate files to render.
	Houdini will generate IFD files for mantra.
- Read Parameters from ROP
	Read files to generate and images to render parameters from specified ROP.
- Render Arguments
	Arguments for render command.
	Usually files and may be some other options.
- Files
	Files to generate.
- Images
	Images which render will produce. Needed for tile render, AfWatch preview/thumbnails.
- Use Temp Images Folder
	Render locally in temp directory and then copy entire image in destination folder.
- Delete ROP Files
	Delete ROP generated files (IFD) after render.
- Tile Render
	Enable rendering tiles and then combine them.
- Divisions
	Tiles divisions.
- Join Render Stages
	Generate IFD files and render in the same task.
	In this case IFD files will be generated to local temporary folder.
	It can save and memory usage and network traffic.

Custom Command
--------------

Run any custom command.
For example you can render IFD files using ``mantra`` command,
generate a preview movie with ``ffmpeg``. 

.. figure:: images/houdini_afrop_command.png

	Afanasy ROP Custom Command tab

- Add Custom Command
	Add custom command tasks block to a job.
- Name
	Tasks block name.
	If empty the first word of the command will be used.
- Command
	The command.
- Service
	Tasks block service.
	If empty the first word of the command will be used.
- Files
	Some files you can point to use in command.
- Preview
	Specify result picture here to enable tasks preview.
- Prefix with $AF_CMD_PREFIX
	Add ``$AF_CMD_PREFIX`` environment variable value to the beginning of the command.
	This may be needed for some software (environment) setup.


Examples
========

Simple
------

Just connect ``afanasy`` ROP to your render ROP. 

.. figure:: images/houdini_simple_network.png

	Simple Network

.. figure:: images/houdini_simple_job.png

	Simple Job

.. figure:: images/houdini_simple_tasks.png

	Simple Job Tasks

The job consists of single tasks block.
Each task represents a frame or several number of frames, specified in *Frames Per Task* parameter.

Command Render
--------------

You can send any custom command to your farm.
Usually you need separate IFD files generation and run ``mantra`` as a standalone process to render.

.. figure:: images/houdini_command_network.png

	Command Network

.. figure:: images/houdini_command_job.png

	Command Job

.. figure:: images/houdini_command_tasks.png

	Command Job Tasks

This job consists of two blocks of tasks.
The first block produced by ``mantra_ifd`` node, with ``Disk File`` parameter turned on.
Next block runs ``mantra`` with ``files`` parameter pointing to the generated files.

Tile Render
-----------

You can split single image to render on several hosts.
Each host (task) will produce a *tile* - some part of an image.
Tiles will be combined in a single image.

.. figure:: images/houdini_tilerender_network.png

	Tile Render Network

.. figure:: images/houdini_tilerender_job.png

	Tile Render Job

.. figure:: images/houdini_tilerender_tasks.png

	Tile Render Job Tasks

Tile job consists of three blocks:

- Generate
	Generate IFD files.
- Render
	Render tiles with ``mantra`` standalone process.
- Join
	Join tiles to assemble an image.
	If tiles were successfully joined they will be removed.
	At the end of this stage, IFD will be removed, if it was asked.

Houdini native ``itilestitch`` tool is used to join tiles. 


Sub Task Dependence
-------------------

This option is designed to start to render simulation without waiting the whole simulation is finished.

.. figure:: images/houdini_subtask_network.png

	Sub-Task Dependence Network

.. figure:: images/houdini_subtask_job.png

	Sub-Task Dependence Job

.. figure:: images/houdini_subtask_tasks.png

	Sub-Task Dependence Job Tasks

The first block of a job is a simulation.
It consists of a single task (*Frames Per Task* parameter is set to the whole frame range).
The second block set to wait the first one with sub-task dependence.
So it begins to render as first frames of a simulation completed, while the simulation task is still running. 


Complex
-------

You can construct a complex Afanasy ROP network to construct a complex job.

.. figure:: images/houdini_complex_network.png

	Complex Network

.. figure:: images/houdini_complex_job.png

	Complex Job

This job consists of a simulation with sub-task dependence.
Two caches waiting the simulation, but cat run independently from each other.
Mantra tile render which produces three blocks which wait all the cache.
Two blocks for preview which can run independently but wait tile render tasks.
One to convert EXR files to JPEGs and one to generate a preview movie form EXRs. 


Setup
=====

CGRU setup should be sourced before.
To do this you can source setup.sh script in CGRU root folder.
Afanasy houdini operator library and Python module are located in:

``cgru/plugins/houdini``

You should add this folder ``HOUDINI_OTLSCAN_PATH`` and ``PYTHONPATH`` environment variables.

Houdini setup example (*bash*):

.. code-block:: bash

	# Setup CGRU
	cd /opt/cgru
	source ./setup.sh

	# Setup CGRU houdini scripts location:
	export HOUDINI_CGRU_PATH="${CGRU_LOCATION}/plugins/houdini"

	# Append OTL scan path with afanasy.otl location:
	export HOUDINI_OTLSCAN_PATH="${HOUDINI_CGRU_PATH}:&"

	# Append Python path with afanasy submission script:
	export PYTHONPATH="${HOUDINI_CGRU_PATH}:${PYTHONPATH}"


If you avoid sourcing ``cgru/setup.sh`` see :ref:`manual-environment-setup`.

