=====
afcmd
=====

``afcmd`` is a command line interface (CLI) to Afanasy server and statistics SQL database.

``afcmd [command]`` execute a command.

``afcmd`` without any arguments to see output of afanasy environment initialization.

``afcmd h`` display help - list all commands.

``afcmd h [command]`` display help for specified command.

``afcmd v [other arguments]`` put program in verbose mode.

afcmd cload
-----------
Ask Afanasy server to reread config files.
It can be reconfigured without restart.

afcmd db_check
--------------
Check database connection.

afcmd db_reset_all
------------------
Drop tables in Afanasy database if any, and create new.
This command should be executed before first Afanasy server start to create needed tables for statistics (if you need any).

