Appending new tasks/blocks to an existing job
=============================================

It is possible to add new tasks or blocks to a job that has already been sent to the farm. One can do it in several manners.

JSON API
--------

The JSON API for actions features the two following operations:

**append_tasks** which takes a tasks list following the very same spec as when submitting new jobs, e.g.::

	{
	 "action":
	 {
	  "user_name"  : "elie",
	  "host_name"  : "my_pc",
	  "type"       : "jobs",
	  "ids"        : [3],
	  "block_ids"  : [0],
	  "operation"  :
	  {
	   "type"      : "append_tasks",
	   "tasks"     :[
	    {
	        "name"    : "Extra Process A",
	        "command" : "python -c \"print('Process EXTRA task A')\""
	    },
	    {
	        "name"    : "Extra Process B",
	        "command" : "python -c \"print('Process EXTRA task B')\""
	    }
	    ]
	  }
	 }
	}

**append_blocks** which takes a blocks list also following jobs submission spec, e.g.::

	{
	 "action":
	 {
	  "user_name"  : "elie",
	  "host_name"  : "my_pc",
	  "type"       : "jobs",
	  "ids"        : [3],
	  "operation"  :
	  {
	   "type"      : "append_blocks",
	   "blocks"    :[
	    {
	      "name"              : "New numeric block",
	      "tasks_name"        : "frames @#@-@#@",
	      "service"           : "generic",
	      "parser"            : "generic",
	      "flags"             : 1,
	      "frame_first"       : 1,
	      "frame_last"        : 100,
	      "frames_per_task"   : 10,
	      "frames_inc"        : 2,
	      "command"           : "python -c \"print('Process frames @#@-@#@')\"",
	      "working_directory" : "E:\\tmp"
	    },
	    {
	      "name"              : "New non numeric block",
	      "tasks_name"        : "frames @#@-@#@",
	      "service"           : "generic",
	      "parser"            : "generic",
	      "working_directory" : "E:\\tmp",
	      "tasks"             : [
	      {
	        "command" : "python -c \"print('Process task A')\""
	      },
	      {
	        "command" : "python -c \"print('Process task B')\""
	      }
	      ]
	    }
	    ]
	  }
	 }
	}


Python af module
----------------

Two command methods are available in the ``Cmd`` object in ``af.py``.

**appendBlocks(jobId, blocks)**

Example::

	import af

	block = af.Block('generic', 'generic')
	block.setCommand("python -c \"print('Process frames @#@-@#@')\"")
	block.setNumeric(1, 100, 10)

	cmd = af.Cmd()
	print(cmd.appendBlocks(3, [block]))


**appendTasks(jobId, blockId, tasks)**

Example::

	import af

	task = af.Task('test')
	task.setCommand("python -c \"print('Process task A')\"")

	cmd = af.Cmd()
	print(cmd.appendTasks(3, 0, [task]))

Python afcmd module
-------------------

**Job.appendBlocks(blocks)**

Example::

	import afcmd

	block = af.Block('generic', 'generic')
	block.setCommand("python -c \"print('Process frames @#@-@#@')\"")
	block.setNumeric(1, 100, 10)

	job = afcmd.getJob(3)
	print(job.appendBlocks([block]))

	blockCopy = job.blocks[0]
	print(job.appendBlocks([blockCopy]))

**Block.appendTasks(tasks)**

Example::

	import afcmd

	task = af.Task('test')
	task.setCommand("python -c \"print('Process task A')\"")

	job = afcmd.getJob(3)
	block = job.blocks[0]
	print(block.appendTasks([tasks]))

Known limitations
-----------------

Numeric block
*************

It does not makes sense to append tasks to numeric blocks, only non-numeric block can have tasks appent.

afwatch
*******

When appending new blocks to a job that is opened in afwatch, one must reopen the job window to see the update.
