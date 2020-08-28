.. _afanasy-pools:

=====
Pools
=====

Pool consists of renders and other child pools.
It designed to manipulate properties (abilities) of group of render clients.


Creation
========

You can add child pool to any parent pool using GUI.
Root pool will be automatically created.


Attributes
==========

name
----
Pool name represents its path.

parent
------
Pool parent name (path).

time_creation
-------------
Time, when pool was created.

pools_num
---------
Number of direct child pools.

pools_total
-----------
Total number of child and pools with grand childs.

renders_num
-----------
Number of renders in this pool.

renders_total
-------------
Total number of renders in this pool and renders in all child pools.

run_tasks
---------
Number of total running tasks of all renders in the pool.

run_capacity
------------
Total capacity of all running tasks in the pool.

task_start_finish_time
----------------------
Time when first task was started or last task was finished.


Editable Parameters
===================

annotation
----------
Just some informative string that will be shown in GUI.

capacity_host
-------------
Pool hosts capacity.

max_tasks_host
--------------
Pool hosts maximum running tasks.

power_host
----------
Pool hosts *power*.
This is just any custom integer.
Job can filter renders for some minimum power.

properties_host
---------------
Pool hosts *properties*.
This is just any custom string.
Job can filter renders for matches this string.

new_nimby
---------
New render will be registered in *NIMBY* state.
Useful when new host was created, Afanasy installed, but render software is not.
Afanasy can be used to install render software.
*Maintenance* job can ignore *NIMBY* and *PAUSED* render state.
*NIMBY* state can turned off automatically.

new_paused
----------
New render will be registered in *PAUSED* state for maintenance purposes.

sick_errors_count
-----------------
Number of errors from different users render considered as *SICK*.
On any error render remembers task job user and counts them.
On any success task finish this count will be reset.
*RENDER_SICK* event can be used to notify admin that some machine can't render.

services
--------
Services names list that pool renders can run.

services_disabled
-----------------
Disabled services names list that pool renders can not run.
If some parent pool allows to run some service, you can disallow to run in child pool.
Also it is useful for temporary service disabling,
to not to delete service and remember that it is just disabled for some time.


Flags
=====
