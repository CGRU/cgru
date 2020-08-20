====
User
====

User is created on a new job from user that does not exist.

Or you can create user manually using CLI:

.. code-block:: bash

    afcmd uadd username

Attributes
==========

name
----
Each user should have an unique name.
If some artists has the same login name, you can use ``AF_USERNAME`` environment variable to override it.

host_name
---------
Host name where the last job was send from.

jobs_num
--------
The number of jobs the user has.

running_jobs_num
----------------
The number of currently running jobs.

running_tasks_num
-----------------
The number of currently running tasks.

time_register
-------------
Time when the user was registered.

time_activity
-------------
The last user activity time.


Editable Parameters
===================

priority
--------
User with greater priority can have more running tasks number (get more render hosts).

``need = pow( 1.1, priority) / (running_tasks_num + 1.0)``

Each priority point gives 10% hosts bonus (running tasks number).

max_running_tasks
-----------------
Maximum number of running tasks user can have.

hosts_mask
----------
User can run only on renders which host name matches this mask.

hosts_mask_exclude
------------------
User can not run on renders which host name matches this mask.

errors_retries
--------------
Default Error Retries value for user jobs.

errors_avoid_host
-----------------
Default Errors Avoid Host value for user jobs.

errors_task_same_host
---------------------
Default Errors Task Same Host value for user jobs.

errors_forgive_time
-------------------
Default Errors Forgive Time value for user jobs.

jobs_life_time
--------------
Default Life Time value for user jobs.

annotation
----------
Annotate user GUI item.

