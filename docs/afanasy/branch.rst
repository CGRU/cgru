.. _afanasy-branch:

======
Branch
======

A branch is like a folder in a file-system.
A branch can contain child branches (folders) and jobs (files),
so there is a hierarchy of branches (folders) and jobs (files).
Branches are designed to combine and manipulate a set of similar (department, project, scene, asset) jobs.


Creation
========

The first ROOT branch will be created by the system job.

Any job has a sting attribute branch.
When a job comes to the server, it looks whether the job branch exists.
If the branch exists, this branch becomes the new job parent.
If the branch does not exist, server tries to find the parent branch of the new job branch.
Then it tries to find the parent of the parent in a cycle (recursion).
When it find a matching parent branch (it must do it, as a root branch always exists),
it tries to create a child branch if the Auto Create Child ACC flag is set.
If the flag is not set, the job will be parented to the most base branch in the branches hierarchy.
And the job branch will be updated to the actual parent that the server could find/create.


Example
=======

For example, you have projects mounted in /prj folder.
So, you have such folders structure:

- /

   - prj/

      - bus/
      - car/
      - plane/
      - train/

Where *bus*, *car*, *plane* and *train* are project names.

Lets imagine that you have some scene file to render:

``/prj/train/shots/scene_a/work/scene.sc``

When you render that scene for the first time, the */prj* branch will be created.
As the root branch has Auto Create Child (ACC) flag set by default.
No more deeper branch(es) will created on this stage.
As the auto-created branch does not have ACC flag set.
At this stage job solving will be the same as there are no branches in Afanasy at all.
But if you set ACC flag on a new */prj* branch, each project will create it's own branch.
So you can manipulate jobs that belongs to some project.
For example give some project more priority.


Attributes
==========

name
----
Branch name, that represents branch full path.
Root branch name is always ``/``.

parent_path
-----------
Parent branch path (name).
It is an empty string for the root branch.

time_creation
-------------
Time when branch was created.

branches_num
------------
Number of child branches (direct childs, not childs of childs).

branches_total
--------------
Total number of child branches and all their sub-childs.

jobs_num
--------
Number of child jobs (direct childs, not childs of childs).

jobs_total
----------
Total number of child jobs and all sub-child branches jobs.

running_tasks_num
-----------------
Number of tasks that branch jobs running.

running_capacity_total
----------------------
Total capacity of all (total) running tasks.


Editable Parameters
===================

priority
--------
Branch solving priority.

max_tasks_per_second
--------------------
Maximum tasks limit that branch can produce per second.

max_running_tasks
-----------------
Maximum tasks limit that branch can run at the same time.

max_running_tasks_per_host
--------------------------
Maximum tasks limit that branch can run at the same time on the same host.

hosts_mask
----------
Branch can be solved only on machines that name matches this mask (regular expression).

hosts_mask_exclude
------------------
Branch can not be solved on machines that name matches this mask (regular expression).


Flags
=====

create_childs
-------------
Branch will create a child branch automatically, when a new job asks for it.

solve_jobs
----------
By default, branch solves its jobs users by priority.
But if this flag is set, branch will solve its jobs directly.

solve_method
------------
Solve child nodes by priority or order.

solve_need
----------
Solve child nodes by running capacity total or tasks number.

