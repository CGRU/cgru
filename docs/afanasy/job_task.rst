========
Job Task
========

Tasks are exists only in non-numeric blocks, where each task can have its own name and command.
In numeric blocks tasks are generated on demand, as numeric block enough has information to generate any task.
Most blocks are numeric, as tasks are differ only by few numbers in a command.

There are some cases when tasks commands differ by some strings, and block can't be described by frame numbers.
For example *ffmpeg* converts various sequences and movies in a sigle job block (Rules constucts such jobs for previews).


.. code-block:: python

   import af

   job = af.Job(job_name)

   block = af.Block(name, service)

   job.blocks.append(block)

   task = af.Task(task_name)

   block.tasks.append(task)



Attributes
==========

If block is numeric all this attributes are generated on the fly by block.

name
----
``af.Task(str)``

Task name. Generated, if block is numeric.

command
-------
``af.Task.setCommand(str)``

Command to execute. Generated, if block is numeric.

files[]
-------
``af.Task.setFiles(str[])``

Files for preview. Generated, if block is numeric.

tst
---
Time when task was started (last start).

tdn
---
Time when task was done (last finish).

str
---
Number of times task has started (it can be manually or automatically restarted).

per
---
Running task progress percentage.

frm
---
Running frame for multiframe tasks which can be produced by numeric blocks when frames per render parameter > 1.

pfr
---
Running percentage of current running frame for multiframe tasks which can be produced by numeric blocks when frames per render parameter > 1.

err
---

Number of times the task produced an error.

hst
---
Host name where the task was started last time.

act
---
Last task activity.
This is a sting to informate user only, does not influence anything.
Activity can be parsed from task process output by Python parser class.

State
=====

==================== ======= ===
Read                 ``RDY`` Task can be executed. 
Running              ``RUN`` Task is running. 
Done                 ``DON`` Task is done. 
Error                ``ERR`` Task finished with error or failed to start. 
Skipped              ``SKP`` Task skipped. 
Warning              ``WRN`` Warning from parser. 
Parser Error         ``PER`` Error from parser. 
Parser Bad Result    ``PBR`` Bad result from parser. 
Restated Error Ready ``RER`` Automatically restarted ``ERR`` task.
==================== ======= ===

