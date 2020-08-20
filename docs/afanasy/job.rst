===
Job
===

Afanasy Job can have one or more blocks.
Blocks have tasks.
Blocks are needed to store some same parameters for all tasks it consists of.
For example all block tasks have the same working directory, capacity, service and parser type.
But commands are differ.

Jobs are created by Python API by some submission script:

.. code-block:: python

   import af

   job = af.Job(job_name)


Attributes
==========

name
----
``af.Job(str)``

``af.Job.setName(str)``

Each job has an unique name.
If new a job comes to server with the name which already exists, server change it's name by adding a number.
Jobs dependences bases on their names and depend mask to match it.
Afanasy uses standard Regular Expressions.
The same expressions are in Python, Perl, JavaScript, PHP and other languages.

user_name
---------
``af.Job.setUserName(str)``

User name who has created the Job.
Python API constructs a job with the current user name.

host_name
---------
Host name where job was created.

time_creation
-------------
Time when the job was created.

time_started
------------
Time when the job was started (produced the first task).

time_done
---------
Time when the job was done (last task finished).

description
-----------
``af.Job.setDescription(str)``

Any custom description. For statistics database QSL queries only.

blocks[]
--------
``af.Job.blocks[] (array)``

Job consists of block(s).


Editable Parameters
===================

priority
--------
``af.Job.setPriority(int)``

Job with a greater priority will run first.

max_running_tasks
-----------------
``af.Job.setMaxRunningTasks(int)``

Maximum number of running tasks at the same time.

max_running_tasks_per_host
--------------------------
``af.Job.setMaxRunTasksPerHost(int)``

Maximum number of running tasks at the same time at the same host.

hosts_mask
----------
``af.Job.setHostsMask(str)``

Job run only on renders which host name matches this mask.

hosts_mask_exclude
------------------
``af.Job.setHostsMaskExclude(str)``

Job can not run on renders which host name matches this mask.

pools
-----
``af.Job.setPools(dict)``

Pools is a string and number pairs (map<sting,int>).
Each pair represents pool name string and pool priority number.

depend_mask
-----------
``af.Job.setDependMask(str)``

Job will wait other user jobs which name matches this mask.

depend_mask_global
------------------
``af.Job.setDependMaskGlobal(str)``

Job will wait other jobs from any user which name matches this mask.

time_wait
---------
``af.Job.setWaitTime(seconds)``

Time to wait to start a job.

ppa
---
``af.Job.setPPApproval()``

Preview Pending Approval parameter plays role only when job block(s) has a non-sequential tasks solving.
When PPA is turned on, job renders only non-sequential tasks (for example just each 10 frame).
Then job state falls into PPA and it stops to solve any tasks.
Artist can check each 10 job frames.
And, depending on the results, continue job or not.
To continue job, you can turn PPA parameter off.

maintenance
-----------
``af.Job.setMaintenance()``

Job will run on tasks which name matches render name.
Useful for "Maintenance" jobs, when you want some command run only once on each render.
For example you can install software this way.

ignorenimby
-----------
``af.Job.setIgnoreNimby()``

Job tasks will run on render even it has "Nimby" state.
Useful for "Maintenance" jobs.

ignorepaused
------------
``af.Job.setIgnorePaused()``

Job tasks will run on render even it has "Paused" state.
Useful for "Maintenance" jobs.

need_os
-------
``af.Job.setNeedOS(str)``

``af.Job.setNativeOS()``

Job will run only on hosts which name contains this mask.
Python *setNativeOS()* function will automatically set needed OS the same that it run.

need_properties
---------------
``af.Job.setNeedProperties(str)``

Job will run only on hosts with custom properties contains this mask. It's custom host parameter can be defined in farm description.

command_pre
-----------
``af.Job.setCmdPre(str)``

Command to execute on job registration.
Note, that this command is executed by server, and not from tasks working directory.
Use absolute paths here or even transfer paths if you server has another file system than renders.
If somebody executes 'sleep 1000', other commands execution (and jobs registration) will be delayed on 1000 seconds (only delayed, not lost).
Try not use Pre Command at all.
You always can create one more task(block) and make other tasks(blocks) depend on it.

command_post
------------
``af.Job.setCmdPost(str)``
Command executed on job deletion.
Usually used to delete temporary render scene.
This commands are executed on render farm hosts by special system job.
Working directory of such system task will be the first block working folder.


time_life
---------
``af.Job.setTimeLife(seconds)``

Maximum job age in seconds.
When job age becomes greater then life time if will be automatically deleted in any case.
It is useful for some technical jobs to prevent their amount rise.
User can set default Life Time value for all its jobs.

annotation
----------
``af.Job.setAnnotation(str)``

Job annotation.
Does not influence anything.
This string will be shown in a GUI item.

report
------
Job annotation.
Does not influence anything.
This string will be shown in a GUI item.
It should be set from a task parser: self.report


State
=====

======================== ======= ===
Ready                    ``RDY`` Job is ready to produce a task.
Running                  ``RUN`` Job has running tasks.
Done                     ``DON`` All job tasks are done (may be some skipped).
Error                    ``ERR`` Job has some error tasks.
Skipped                  ``SKP`` Some job tasks are skipped.
Waiting Dependencies     ``WD``  Job waits some other jobs to be done.
Waiting Time             ``WT``  Job waits some time to start.
Preview Pending Approval ``PPA`` Job has rendered all non-sequential tasks and waits approval.
Offline                  ``OFF`` Flag to server not to solve a job.
======================== ======= ===

