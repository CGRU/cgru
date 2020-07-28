Installation
============

Linux Packages
--------------

Download_ the latest release and unpack.

- **Server**: You need to install **afanasy-server** package. Also you can to install **afanasy-render** package to monitor its resources, and should limit some heavy tasks not to run on server machine.

- **Workstation**: You need to install **cgru** package.

- **Render**: You need to install **afanasy-render** package.

CGRU will be installed in **/opt/cgru** folder.

+----------------------+-----------------------------+--------------------------+
|                      |                             |                          |
|  Strucure            |  Description                | Depends                  |
|                      |                             |                          |
+======================+=============================+==========================+
| * cgru               | **cgru**: Start menu item.  | PySide(1-2) or PyQt(4-5) |
|                      |                             |                          |
|   * afanasy-render   +-----------------------------+--------------------------+
|                      | **cgru-common**: All files, |                          |
|     * afanasy-common | except Afanasy binaries.    |                          |
|                      +-----------------------------+--------------------------+
|       * cgru-common  | **afanasy-common**: Afanasy | PostgreSQL libraries     |
|                      | binaries, except GUI.       | (libpq)                  |
|   * afanasy-qtgui    +-----------------------------+--------------------------+
|                      | **afanasy-render**: Afanasy |                          |
|     * cgru-common    | render startup scripts.     |                          |
|                      +-----------------------------+--------------------------+
| * afanasy-server     | **afanasy-server**: Afanasy | *PostgreSQL server,*     |
|                      | server startup scripts.     | *apache, php, php-pgsql* |
|   * afanasy-common   +-----------------------------+--------------------------+
|                      | **afanasy-qtgui**: Afanasy  | Qt libraries             |
|     * cgru-common    | Qt GUI binary.              |                          |
+----------------------+-----------------------------+--------------------------+

* *PostgreSQL server and Apache+PHP needed for afserver to store and view statistics only.*

**Installation Methods:**

 - Install and uninstall scripts provided with the packages.
 - Various GUI utilities native for each Linux distribution, can install it.
 - Commands like dpkg -i for .deb's and rpm -i for .rmp's.
 - The best way is to put this packages in you local company Linux repository. And to use native Linux ways to install and update software. In this way Linux system solves packages dependences itself.


MS Windows
----------

Download_ the latest release and unpack. Use Keeper to launch applications.

 - **Server**: Launch afserver.
 - **Render**: Launch afrender.
 - **Workstation**: Use Keeper to launch render client and other CGRU applications.


Mac OS X
--------

There is no release for this platform, yet. But you can build project yourself.

.. _Download: http://cgru.info/downloads

