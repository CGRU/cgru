.. _software-clarisse:

============
Clarisse iFX
============

In-App Submission
=================

This a Python script.
It get some project attributes and creates a dialog.
In this dialog you can change parameters and send job to server.
By default script exports project to render archive with some temporary name.
And after render, when user deletes job, temporary archive will be deleted too.


General Tab
-----------

.. image:: images/clarisse_dialog_a.png

- Engine
    - clarisse_node
	    CGRU command used to launch Clarisse cnode. By default it will launch the latest (alphabetically) version.
    - clarisse_render
	    CGRU command used to launch Clarisse crender.
    - cnode
	    System command will be searched in PATH environment.
    - crender
	    System command will be searched in PATH environment.



Settings Tab
------------

.. image:: images/clarisse_dialog_b.png

Negative value means use defaults.

Conditions Tab
--------------

.. image:: images/clarisse_dialog_c.png

Empty field disables condition.


AfWatch
=======

.. image:: images/clarisse_afwatch_job.png

WebGUI
======

.. image:: images/clarisse_webgui_job.png


Setup
=====

Shelf Item
----------

.. image:: images/clarisse_add_shelf_item.png


AfStarter
=========

You can also send Clarisse render archive to Afanasy with a stand-alone dialog AfStarter_.
You do not need to open main Clarisse application (GUI) for it.

.. _AfStarter: ../afstarter/afstarter.html

Developers
==========

In-app submission dialog GUI is created with PyQt (PySide).
Qt binding in Python is represented by a Qt.
It chooses existing Qt binding automatically.

Submission script:

https://github.com/CGRU/cgru/blob/master/plugins/clarisse/afanasy_submit.py

