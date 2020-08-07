======
Render
======


Render is a client application.
It runs on a remote host and communicates with server.
Server sends tasks to render to run. 


Launch Methods
==============

 -  MS Windows script

    ``start\AFANASY\99.render.cmd``

 -  UNIX script
    
    ``start/AFANASY/_afrender.sh``

 -  Linux daemon when linux packages are installed

    ``sudo systemctl start afrender``

 -  Setup CGRU environment and launch a command:

    .. code-block:: bash

    	cd cgru
    	source ./setup.sh
    	afrender


To register a render already in [nimby|NIMBY] state use command:

.. code-block:: bash
    
    afrender [nimby|NIMBY]


Attributes
==========

name
----
Each render has an unique name. If new render comes to server with the name which already exists, server will not register it. Users and jobs hosts masks are based on render names and Regular Expressions (they are Perl-like).
To launch another render on the same host use AF_HOSTNAME environment variable to override render name.

address.family
--------------
address.ip
----------
IP address with family (IPv4 or IPv6).

netifs[]
--------
Network interfaces information (name, mac and ips).

time_launch
-----------
Time the application was launched.

time_register
-------------
Time the render was registered on server.

time_update
-----------
Last time the render has update its resources usage.

wol_operation_time
------------------
Time the last any Wake-On-Lan operation was performed.

tasks[]
-------
Running tasks number and some its attributes (user, job, etc.).

capacity_used
-------------
Capacity used by running tasks.


Editable Parameters
===================

user_name
---------
Who launched a render. Can be changed only by administrators. Note that process do not change uid. This value for afanasy only.

priority
--------
Render with greater priority get task first.

capacity
--------
You can override host farm setup capacity.

max_tasks
---------
You can override host farm setup maximum running tasks.

services
--------
Services that render can run.
If empty it it configured by pool.

services_disabled
-----------------
You can disable some services.

annotation
----------
Annotate render GUI item.


State
=====

=========== ======= ===
Online      ``ONL`` Is online. 
Offine      ``OFF`` Is offset. 
nimby       ``Nby`` Is taken by his user. Only render user can render on it.
NIMBY       ``NBY`` Is taken by his user and he don't want to render on it.
Busy        ``RUN`` Executing one or more tasks.
Dirty       ``DRT`` Capacity changed or some service disabled.
WOLFalling  ``WFL`` Is falling a sleep. It was asked to sleep, but still online.
WOLSleeping ``WSL`` Is sleeping.
WOLWaking   ``WWK`` Is waking up. It was asked to wake up but still is not online.
Paused      ``PAU`` Is paused, like super-Nimby, never will be free automatically.
Sick        ``SIC`` Is seek, produced error only.
=========== ======= ===


Resources
=========

cpu_num
-------
CPUs x Cores number.

cpu_mhz
-------
Fist Processor frequency.

cpu_loadavg[3]
--------------
Load average.

cpu_user
--------
User usage percentage.

cpu_nice
--------
User 'nice' usage percentage (low priority processes).

cpu_system
----------
System usage percentage.

cpu_idle
--------
Idle percentage (CPU free).

cpu_iowait
----------
Waiting for I/O complete percentage.

cpu_irq
-------
Interrupts servicing percentage.

cpu_softirq
-----------
Soft interrupts servicing percentage.

mem_total_mb
------------
Total amount of memory in megabytes.

mem_free_mb
-----------
Free memory in megabytes.

mem_cached_mb
-------------
Cached memory in megabytes.

mem_buffers_mb
--------------
Buffered memory in megabytes.

swap_total_mb
-------------
Total swap space in megabytes.

swap_used_mb
------------
Used swap in megabytes.

hdd_total_gb
------------
Total disk space in gigabytes.

hdd_free_gb
-----------
Available free disk space in gigabytes (in root - '/').

hdd_rd_kbsec
------------
Disk reading in kilobytes per second.

hdd_wr_kbsec
------------
Disk writing in kilobytes per second.

hdd_busy
--------
Percentage of system ticks spend for the disk IO.

net_recv_kbsec
--------------
Network receiving traffic in kilobytes per second.

net_send_kbsec
--------------
Network sending traffic in kilobytes per second.


Paths Map
=========

CGRU has an ability to map paths.
Every client can have own paths map file to translate paths to server and from server.

Paths map is described in config files by pathsmap object.
It is an arrays of ``["CLIENT","SERVER"]`` paths pairs:

.. code-block:: json

    "pathsmap":[
        ["//server/projects/","/mnt/prj/"],
        ["//server/tools/","/mnt/tools/"]
    ]

When job constructs (on the client side) all commands and working directories are translated from client to server.
When task starts (on the client side) all commands and working directories are translated from server to client.
Server does know nothing about paths map.

MS Windows platform issues
--------------------------
 - You can write only ``/`` slashes in a config.
   It will try both slashes directions.
   Some applications allows client to use and ``\`` and ``/`` slashes, so pattern will be matched in any case.
 - When client searches a pattern it converts paths in lower case.
   So no matter how client wrote a path ``//server/projects/``, ``//SERVER/PROJECTS/``, ``//SERVER/projects/`` or ``//server/PROJECTS/``.
   It will work any way.
 - Module (Python Class) can works in *UnixSeparators* mode.
   During translation from server to client it uses ``/`` slashes for client paths.
   For example NUKE uses only ``/`` slashes on any platform.

A part of a real working ``config.json`` with map example:

.. code-block:: json

    "OS_windows":{
        "pathsmap":[
            ["P:/",             "/ps/prj/"],
            ["//box/project/",  "/ps/prj/"],
            ["Q:/",             "/ps/prj2/"],
            ["//box2/project/", "/ps/prj2/"],
            ["//sun/libs/",     "/ps/lib/"],
            ["//sun/vault/",    "/ps/vault/"],
            ["T:/",             "/ps/etc/"],
            ["c:/ps/",          "/ps/"],
            ["c:/temp/",        "/tmp/"]
        ]
    }


