===
API
===

Afanasy can communicate via JSON protocol.
And only JSON protocol.
Any GUI, CLI or Python script (API) constructs JSON objects to send to server.


Python API
==========

You can create jobs within Python, that exists in most common CG software.
Afanasy Python module helps you to construct a valid JSON job object for server.
Also it sends json data to server.

Example
-------

.. code-block:: bash

    # Import afanasy python module (must be in PYTHONPATH)
    import af

    # Create a job
    job = af.Job('somejob')

    # Set job depend mask
    job.setDependMask('another_job_name')

    # Set maximum tasks that can be executed simultaneously
    job.setMaxRunningTasks(15)

    # Set job hosts mask
    job.setHostsMask('render.*')

    # Start job paused
    job.offLine()

    # Create a block with provided name and service type
    block = af.Block('back', 'nuke')

    # Set block tasks command
    block.setCommand('nuke -i -X WriteBack -x scene.nk.tmp.nk @#@,@#@')

    # Set block tasks preview command arguments
    block.setFiles(['jpg/img.@####@.jpg'])

    # Set block to numeric type, providing first, last frame and frames per host
    block.setNumeric(1, 100, 10)

    # Add block to the job
    job.blocks.append(block)

    # Set command to execute by server after a job is deleted.
    job.setCmdPost('rm /projects/test/nuke/scene.nk.tmp.nk')

    # Send job to Afanasy server
    job.send()


Job Class
---------

 - Constructor:

    - ``job = af.Job(job_name = None)``

      Takes job name as a parameter (optional).

 - Variables:

    - ``job.blocks = []``

      Blocks list.

 - Some Functions:

    - ``job.offline()``

      Set job to Offline state.

    - ``job.output(output_blocks = True)``

      Print job information. If True print job blocks information too.

    - ``job.send()``

      Send job to Afanasy server.


Block Class
-----------

  - Constructor:

      - ``block = af.Block(block_name, service_name)``

        Construct a new block and return it.

  - Variables:

      - ``block.tasks = []``

        Tasks list. Used for not numeric blocks.


Task Object
-----------

  - Constructor:

      - ``task = af.Task(task_name)``

        Construct a new task and return it.


JSON Protocol
=============

You can use ``afcmd`` CLI to send JSON objects (files) to Afanasy server:


 - ``afcmd json [file|pipe]``: Test JSON syntax, output an error and position.
 - ``afcmd v json [file|pipe]``: Same as previous and output parsed JSON document structure.
 - ``afcmd json send [file]``: Send JSON data after successfully parsed.


Job
---

Here is an example of a minimum JSON object to send to server to construct a job:

.. code-block:: json

    {
        "job":
        {
            "name"                  : "job name",
            "user_name"             : "jimmy",
            "host_name"             : "host",
            "blocks":[
            {
                "name"              : "Nuke",
                "tasks_name"        : "frames @#@-@#@",
                "service"           : "nuke",
                "parser"            : "nuke",
                "frame_first"       : 1,
                "frame_last"        : 100,
                "frames_per_task"   : 10,
                "frames_inc"        : 2,
                "command"           : "nuke -F@#@,@#@ -x scene.nk -X Write1",
                "working_directory" : "/home/jimmy/work",
                "files"             : ["folder/img_L.@####@.jpg","folder/img_R.@####@.jpg"]
            }
            ]
        }
    }


Get
---

Get request are used to get information from server.

Here are some examples:

 - Get a list with all jobs:

   .. code-block:: json

    {
        "get":
        {
            "type" : "jobs"
        }
    }

 - Get jobs list from users with specified ids:

   .. code-block:: json

    {
        "get":
        {
            "type" : "jobs",
            "uids" : [1,2]
        }
    }

 - Get renders by host names pattern:

   .. code-block:: json

    {
        "get":
        {
            "type" : "renders",
            "mask" : "farmhost.*"
        }
    }

 - Get users list with special ids:

   .. code-block:: json

    {
        "get":
        {
            "type" : "users",
            "ids"  : [1,2]
        }
    }


Actions
-------

Actions are used to edit parameters and perform operations.

Any action should have ``host_name`` and ``user_name`` fields for logs.

Here are some examples:

 - Set render nimby

   .. code-block:: json

    {
        "action":
        {
            "user_name"  : "jimmy",
            "host_name"  : "pc01",
            "mask"       : "pc02",
            "type"       : "renders",
            "params"     :
            {
                "nimby"     : true
            }
        }
    }

 - Set user priority

   .. code-block:: json

    {
        "action":
        {
            "user_name"  : "jimmy",
            "host_name"  : "pc01",
            "mask"       : "bob",
            "type"       : "users",
            "params"     :
            {
                "priority"  : "50"
            }
        }
    }

 - Exit render

   .. code-block:: json

    {
        "action":
        {
            "user_name"  : "jimmy",
            "host_name"  : "pc01",
            "mask"       : "pc02",
            "type"       : "renders",
            "operation"  :
            {
                "type"      : "exit"
            }
        }
    }

 - Delete job

   .. code-block:: json

    {
        "action":
        {
            "user_name"  : "jimmy",
            "host_name"  : "pc01",
            "mask"       : "my3drender",
            "type"       : "jobs",
            "operation"  :
            {
                "type"      : "delete"
            }
        }
    }

