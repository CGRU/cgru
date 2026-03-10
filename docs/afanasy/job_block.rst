=========
Job Block
=========

Afanasy Job Block keeps the same parameters for all its tasks.
When task is generated (when it ready to run) it take such parameters as working folder, capacity, service from block.

Blocks can be *numeric* (most blocks in Afanasy are numeric).
Numeric blocks does not have tasks at all.
Such block keeps parameters for all its tasks itself.
For example it has a command as

``render -s @#@ -e @#@"``

where ``@#@`` will be replaced with start and end frames for each task.

.. code-block:: python

   import af

   job = af.Job(job_name)

   block = af.Block(name, service)

   job.blocks.append(block)


Attributes
==========

name
----
``af.Block(name, service)``

``af.Block.setName()``

Each Block has an unique name.
If new Block added to Job which the name already exists, Job change it's name by adding a number.
Blocks dependence bases on their names and depend masks to match it.

tasks_num
---------
The number of tasks in block.

frame_first
-----------
``af.Block.setNumeric(start, end, pertask, inc)``

First block frame.

frame_last
----------
``af.Block.setNumeric(start, end, pertask, inc)``

Last block frame.

frames_inc
----------
``af.Block.setNumeric(start, end, pertask, inc)``

In various software also known as frames step, jump, by frame.
You can use it if you want to render only each third frame, for example.

frames_per_task
---------------
``af.Block.setNumeric(start, end, pertask, inc)``

``af.Block.setFramesPerTask(int)``

Number of frames in each task.
When block is not numeric (has tasks with individual commands) used to compute blocks per task dependency.
Negative values means sub-frame dependency.
For example you can render tiles or generate shadows in one block and generate mantra 'ifd' files in another with 1 frame per task.
And if task has 4 shadows or 2x2 tiles you can set -4 frames per task for depended block.

Editable Parameters
===================

tasks_name
----------
``af.Block.setTasksName(str)``

Block tasks names pattern.

- *Block - numeric, pattern - not empty*: generated task will fill this pattern with its first and last frames numbers.
- *Block - numeric, pattern - empty*: tasks will take first_frame - last_frame name.
- *Block - not numeric, pattern - not empty*: task fill this pattern with it name.
- *Block - not numeric, pattern - empty*: task simply take its name.

sequential
----------
``af.Block.setSequential(int)``

By default, sequential is 1, tasks will be solved from the first to the last one by one.
If this parameter is -1, tasks will be solved from the last to the first one by one.
If this parameter is greater than 1 or less than -1, 10 for example, tasks with every 10 frame will be solved at first, than other tasks.
If -10, every 10 frame but from the end.
Important thing that task frame is used for sequential calculation, not task number.
If sequential is 0, always middle task will be solved.
For example if frame range is 1-100, tasks solving order will be: 1,100,50,25,75 and so on.

service
-------
``af.Block.setService(str)``

The name of a block tasks service type.

parser
------
``af.Block.setParser(str)``

Block tasks output parser name.

working_directory
-----------------
``af.Block.setWorkingDirectory(str)``

Tasks process working directory.

environment
-----------
``af.Block.setEnv(name, value)``

Tasks process extra environment.
Each task in a block will automatically get the following environment variables:

.. code-block:: shell

   AF_JOB_ID = tasks job id
   AF_BLOCK_ID = tasks block id
   AF_TASK_ID = tasks task id


command_post
------------
``af.Block.setCmdPost(str)``

Like job "command_post" but for each block.
Working directory of this command will be this block working folder.

capacity
--------
``af.Block.setCapacity(int)``

Task checks available capacity on render to run on it.
Capacity can be static (by default) and variable - base value and coefficients:

capacity_coeff_min
------------------
capacity_coeff_max
------------------
``af.Block.setVariableCapacity(min, max)``

Block can generate tasks with capacity*coefficient to fit free render capacity.
Task commands will be searched for the special string to replace it with capacity coefficient number.
This command replacement performs render by service python class instance.
Any service can describe own rule for this replacement by implementation of base class method.
By default, base service class performs command.replace('@AF_CAPACITY@', str(capacity)).
You can specify number of CPUs to use for your applications (if it supports it by command line arguments).

multihost_min
-------------
multihost_max
-------------
multihost_max_wait
------------------
multihost_master_on_slave
-------------------------
multihost_service
-----------------
multihost_service_wait
----------------------
``af.Block.setMultiHost(min, max, wait, master_on_slave=False, service=None, service_wait=-1)``

A singe block task can run on several hosts.
You can specify minimum and maximum number of hosts that task can take.
Time in seconds to wait for maximum hosts.
Whether the master host will be in slaves list.
For example if task took 'r00', 'r01', 'r02', 'r03', 'r04' hosts master command will be executed on 'r00' and 'r00' will be in slaves list too.
If tasks has multihost service you can't enable this parameter, because only one command can be executed master or slave.
Command to execute on slaves hosts, if it is empty, no service will be executed.
Time in seconds to wait for master execution after slaves execution.

max_running_tasks
-----------------
``af.Block.setMaxRunningTasks(int)``

Maximum number of tasks block can run on the same time.

max_running_tasks_per_host
--------------------------
``af.Block.setMaxRunTasksPerHost(int)``

Maximum number of tasks block can run on the same time on the same host.

hosts_mask
----------
``af.Block.setHostsMask(str)``

Block run only on Renders which host name matches this mask.

hosts_mask_exclude
------------------
``af.Block.setHostsMaskExclude(str)``

Block can not run on renders which host name matches this mask.

pools
-----
``af.Block.setPools(dict)``

Pools is a string and number pairs (map<string,int>).
Each pair represents pool name string and pool priority number.
If block pools are set they override job pools for this block.

depend_mask
-----------
``af.Block.setDependMask(str)``

Block will wait other job blocks which name matches this mask.

tasks_depend_mask
-----------------
``af.Block.setTasksDependMask(str)``

Block task will wait other job blocks task which name matches this mask.

errors_retries
--------------
``af.Block.setErrorsRetries(int)``

Number of task errors to retry it automatically. Value '-1' means take this value from user settings.

errors_avoid_host
-----------------
``af.Block.setErrorsAvoidHost(int)``

Maximum number of errors on same host.
Block begins to avoid render host name if number of errors on it greater or equal this value.
Zero value means no limit.
Value '-1' means take this value from user settings.

errors_task_same_host
---------------------
``af.Block.setErrorsTaskSameHost(int)``

Maximum number of errors for task on same host.
Task begin to avoid this host name of errors on it greater or equal this value.
Zero value means no limit.
Value '-1' means take this value from user settings.

errors_forgive_time
-------------------
``af.Block.setErrorsForgiveTime(int)``

Time form last error to forgive error host (reset it's errors count).
Zero value means no forgive.
Value '-1' means take this value from user settings.

task_max_run_time
-----------------
``af.Block.setTaskMaxRunTime(seconds)``

Task maximum time to run.
After this time task will be set to error (and may be automatically restarted according to Error Retries value).
If this value equals or less than zero, no task run time limit exists.

task_min_run_time
-----------------
``af.Block.setTaskMinRunTime(seconds)``

Task minimum time to run.
If task will finished with success for a time less this value, it will be treated as an error.
If this value equals or less than zero, this limit will be disabled.

task_progress_change_timeout
----------------------------
``af.Block.setTaskProgressChangeTimeout(seconds)``

If running task progress (percentage) will be the same for this time, task will be stopped with error.
If this value equals or less that zero, no such limit exists.

The default value can be set by ``af_task_progress_change_timeout`` config variable.
It is a server side variable, you can ask server to reload config without restarting by ``afcmd cload`` command.
See `Configuration <../configuration/configuration.html>`_ and `afcmd <../afanasy/afcmd.html>`_ sections.

need_power
----------
``af.Block.setNeedPower(int)``

Minimum render host power needed.
It's custom host parameter can be set by pool.

need_memory
-----------
``af.Block.setNeedMemory(int)``

Minimum render host free memory needed in megabytes.

need_gpu_mem_mb
---------------
``af.Block.setNeedGPUMemGB(float)``

Minimum render host GPU free memory needed in gigabytes.
The function will convert it to integer megabytes.

need_cpu_freq_mgz
-----------------
``af.Block.setNeedCPUFreqGHz(float)``

Minimum render host CPU frequency in gigahertz.
The function will convert it to integer megahertz.

need_cpu_cores
--------------
``af.Block.setNeedCPUCores(int)``

Minimum render host CPU cores number.

need_cpu_freq_cores
-------------------
``af.Block.setNeedCPUFreqCores(float)``

Minimum render host CPU frequency * cores in gigahertz.
The function will convert it to integer megahertz.

need_hdd
--------
``af.Block.setNeedHDD(int)``

Minimum render host free disk space needed in gigabytes.

need_properties
---------------
``af.Block.setNeedProperties(str)``

A mask to much render host properties to run on it.
It's a custom host parameter can be set by pool.

command
-------
``af.Block.setCommand(str)``

Tasks command pattern.
When block produces a task it calculates an unique command from this pattern and other parameters,
depend on block type - numeric or string, replacing @#@ pattern with a number.
Padding is specified by the number of "#" symbols between "@" symbols.

String: block seek for "@#@" string in command and replace it by another string get from Task Command.

Example:

command: ``myrender some.scene -camera @#@``

arguments = ``['sun','sky','front','side','bottom']``

Result:

1st task command: ``myrender some.scene -camera sun``

2nd task command: ``myrender some.scene -camera sky``

3rd task command: ``myrender some.scene -camera front``

If block command is empty Task Command is simply used.

Numeric block calculates first and last frame for the task
according to task number, Frame First, Frame Last, Frame per Host and Frame Increment values.
Each of @#@ pairs will be replaced with the start and end numbers.

Examples:

command: ``myrender some.scene -s @#@ -e @#@``

frame_first: 1, frame_last: 10, frames_per_task: 4

Result:

1st task command: ``myrender some.scene -s 1 -e 4``

2nd task command: ``myrender some.scene -s 5 -e 8``

3rd task command: ``myrender some.scene -s 9 -e 10``

command: ``myrender something.@####@.obj``

frame_first: 1, frame_last: 10, frames_per_task: 1

Result:

1st task command: ``myrender something.0001.obj``

2nd task command: ``myrender something.0002.obj``

last task command: ``myrender something.0010.obj``

You can check numbers filling by command:

.. code-block:: bash

    afcmd numcmd service frame_start frame_end command

files[]
-------
``af.Block.setFiles(str[])``

Each task can have result file(s) pattern.
Result file name will be constructed from this pattern by the same method as described before.
Some another application, for example watch GUI, can execute your favorite image viewer program [file] and preview result frame.

Python function will extend an existing files array.

Example (numeric):

Block files: ``images/back.@####@.exr``

Preview command: ``nuke -v @ARG@``

Result for 57 frame: ``nuke -v images/back.0057.exr``

Example (not numeric):

Block files: ``images/back.@#@.exr``

Task files: ``0057``

Preview command: ``nuke -v @ARG@``

Result: ``nuke -v images/back.0057.exr``

Task can have several files for preview, for example when several render passes or a stereo images pair.

If block is not numeric and block view command is empty only task view command is used.

Watch will execute command in a task block working directory.


Flags
=====

numeric
-------
``1 << 0``

Numeric

varcapacity
-----------
``1 << 1``

``af.Block.setVariableCapacity(min, max)``

multihost
---------
``1 << 2``

``af.Block.setMultiHost(min, max, wait, master_on_slave=False, service=None, service_wait=-1)``

masteronslave
-------------
``1 << 3``

``af.Block.setMultiHost(min, max, wait, master_on_slave=False, service=None, service_wait=-1)``

dependsubtask
-------------
``1 << 4``

``af.Block.setDependSubTask()``

For tasks with several frames calculate sub task dependence.
Useful for simulation and render when not all frames simulated.

skipthumbnails
--------------
``1 << 5``

``af.Block.skipThumbnails()``

Do not try to generate any thumbnails.

skipexistingfiles
-----------------
``1 << 6``

``af.Block.skipExistingFiles( size_min = -1, size_max = -1)``

AfRneder can check files on client just before task start (in a Python service class initialization). It can skip task command launch if file(s) are exist. If size_min or(and) size_max are positive, it will check size too. Block(task) files parameter should be set properly.

checkrenderedfiles
------------------
``1 << 7``

``af.Block.checkRenderedFiles(size_min = -1, size_max = -1)``

AfRneder can check files on client just after task finish (in a Python service class).
It can set task as error if file(s) are not exist.
If size_min or(and) size_max are positive, it will check size too.
Block(task) files parameter should be set properly.

slavelostignore
---------------
``1 << 8``

``af.Block.setSlaveLostIgnore()``

On a slave host missing, multi-host task will not restart. It will just ignore this.


State
=====

==================== ======= ===
Ready                ``RDY`` Block is ready to produce a task.
Running              ``RUN`` Block has running tasks.
Done                 ``DON`` All block tasks are done (or some skipped).
Error                ``ERR`` Block has some error tasks.
Skipped              ``SKP`` Some block tasks are skipped.
Waiting Dependencies ``WD``  Block waits some other blocks.
==================== ======= ===

