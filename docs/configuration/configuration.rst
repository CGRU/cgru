Configuration
=============

Create config.json file in a CGRU root folder:

.. code-block:: json

	{"cgru_config":{
		"af_servername":"afanasy",
		"":""
	}}


It should at least contain Afanasy server hostname or IP address.
Read cgru/config_default.json parameters comments for help.

Config files
------------

CGRU configuration files based JSON.
One config file can include other, where parameters can be overridden.
Config file can contain OS specific section,
which parameters will be read only if client platform parameters matches OS section name.

At first ``cgru/config_default.json`` is read, where global parameters are set.
It includes ``cgru/afanasy/config_default.json`` which configures Afanasy specific parameters.
At last it tries to load ``cgru/config.json``, where you should override some your company specific parameters.
At least you should specify Afanasy server location in it.

CGRU Keeper and AfWatch stores settings in ``$HOME/.cgru/config.json`` on UNIX
and ``%APPDATA%/cgru/config.json`` on MS Windows OS.
It is read with the same rules as the main config.
User can use it to set its own properties and override global config settings.

Each parameter should be preset only once in a config file.
If there will several parameters with the same name in a same file, which will be chosen is undefined.
Parameters order plays no role.
Included files are read after the end of file, no matter where include line is.
If in the next included file there will be a parameter with the same name, it value will be overridden.

.. seealso::

	JSON syntax reference: http://json.org

Better to check config with afcmd.
Just run this command with no arguments.
On error it will output error message, position and some text around it.
You can also view configuration in Keeper,
but it can not to start with bad config at all.
Keeper Configuration window will display result configuration and each config file contents in the order they where read (included).

.. note::

	There are no comments in JSON syntax.
	But some ways to comment JSON files exist.
	For example you can create an object with no name:
	``"":"Some comment text."``
	Or create an object with some unused name.
	To disable (comment) some parameter you can change it name to unused.
	For example you can just prefix name with "-".


Environment Variables
---------------------

Common user does not need to setup environment variables manually.
Use Keeper or start scrips they will setup environment.
To setup a console you should go to CGRU root folder and source setup script.

.. code-block:: bash

	cd cgru
	source ./setup.sh


.. _manual-environment-setup:

Manual Environment Setup
~~~~~~~~~~~~~~~~~~~~~~~~

Required Variables
..................

 - **CGRU_LOCATION** - CGRU root folder.
 - **AF_ROOT** - AFANASY root folder (``cgru/afanasy``).
 - **PYTHONPATH** - To import CGRU and AFANASY Python modules:

   - ``cgru/lib/python`` - CGRU Python general library path.
   - ``cgru/afanasy/python`` - Afanasy Python library path.

Optional Variables
..................

 - **CGRU_VERSION** - CGRU version string, will be shown in AfWatch and Browser. May be any. You can add some useful info to real version at work.
 - **AF_HOSTNAME** - Override hostname for Afanasy. Useful to run several clients on the same machine.
 - **AF_USERNAME** - Override user name for Afanasy.
 - **PATH** - Run commands.

   - ``cgru/bin`` - CGRU tools.
   - ``cgru/afanasy/bin`` - AFANASY applications.


.. _config-overrides:

Config Overrides
----------------

You can override any config parameter.

**By Command Argument:**

Use ``--[param_name] param_value`` arguments to an Afanasy executable. For example to make afwatch to connect to other afanasy server type:

.. code-block:: bash

	afwatch --af_servername otherserver

**By Environment Variable:**

Set ``CGRU_[PARAM_NAME]`` environment variable. For example to setup console to use other server port type:

.. code-block:: bash

	export CGRU_AF_SERVERPORT=51111

Python API will take environment overrides too.

