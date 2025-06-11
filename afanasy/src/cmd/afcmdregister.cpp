#include "afcmd.h"

#include "cmd_arguments.h"

#include "cmd_confirm.h"
#include "cmd_invalid.h"
#include "cmd_regexp.h"
#include "cmd_test.h"
#include "cmd_text.h"

#include "cmd_config.h"
#include "cmd_network.h"
#include "cmd_numeric.h"
#include "cmd_passwd.h"
#include "cmd_path.h"
#include "cmd_statistics.h"
#include "cmd_string.h"

#include "cmd_database.h"

#include "cmd_monitor.h"
#include "cmd_pool.h"
#include "cmd_render.h"
#include "cmd_user.h"

#include "cmd_job.h"
#include "cmd_task.h"

#include "cmd_json.h"

void AfCmd::RegisterCommands()
{
	addCmd(new CmdVerbose);
	addCmd(new CmdHelp);
#ifndef WINNT
	addCmd(new CmdFork);
#endif

	addCmd(new CmdText);
	addCmd(new CmdTextGenerate);
	addCmd(new CmdConfirm);
	addCmd(new CmdRegExp);
	addCmd(new CmdInvalid);

	addCmd(new CmdTestMsg);
	addCmd(new CmdTestThreads);

	addCmd(new CmdMonitorList);
	addCmd(new CmdMonitorLog);
	addCmd(new CmdMonitorMsg);

	addCmd(new CmdPoolList);
	addCmd(new CmdPoolLog);
	addCmd(new CmdPoolPriority);
	addCmd(new CmdPoolPause);
	addCmd(new CmdPoolUnpause);
	addCmd(new CmdPoolServiceAdd);
	addCmd(new CmdPoolServiceDel);

	addCmd(new CmdRenderList);
	addCmd(new CmdRenderPriority);
	addCmd(new CmdRenderNimby);
	addCmd(new CmdRenderNIMBY);
	addCmd(new CmdRenderUser);
	addCmd(new CmdRenderFree);
	addCmd(new CmdRenderPause);
	addCmd(new CmdRenderUnpause);
	addCmd(new CmdRenderEjectTasks);
	addCmd(new CmdRenderEjectNotMyTasks);
	addCmd(new CmdRenderExit);
	addCmd(new CmdRenderDelete);
	addCmd(new CmdRenderResoucesList);
	addCmd(new CmdRenderWOLSleep);
	addCmd(new CmdRenderWOLWake);
	addCmd(new CmdRenderServiceOn);
	addCmd(new CmdRenderServiceOff);

	addCmd(new CmdUserList);
	addCmd(new CmdUserJobsList);
	addCmd(new CmdUserAdd);
	addCmd(new CmdUserDelete);
	addCmd(new CmdUserPriority);
	addCmd(new CmdUserRunningTasksMaximum);
	addCmd(new CmdUserHostsMask);

	addCmd(new CmdJobsList);
	addCmd(new CmdJobsPause);
	addCmd(new CmdJobsStop);
	addCmd(new CmdJobsStart);
	addCmd(new CmdJobsRestart);
	addCmd(new CmdJobsDelete);
	addCmd(new CmdJobsWeight);
	addCmd(new CmdJobId);
	addCmd(new CmdJobLog);
	addCmd(new CmdJobProgress);
	addCmd(new CmdJobPriority);
	addCmd(new CmdJobRunningTasksMaximum);
	addCmd(new CmdJobHostsMask);
	addCmd(new CmdJobsSetUser);

	addCmd(new CmdTaskLog);
	addCmd(new CmdTaskOutput);
	addCmd(new CmdTaskRestart);

	addCmd(new CmdString);
	addCmd(new CmdPath);
	addCmd(new CmdNetwork);
	addCmd(new CmdNumeric);
	addCmd(new CmdNumericCmd);
	addCmd(new CmdPasswd);

	addCmd(new CmdDBCheck);
	addCmd(new CmdDBResetLogs);
	addCmd(new CmdDBResetJobs);
	addCmd(new CmdDBResetTasks);
	addCmd(new CmdDBResetAll);
	addCmd(new CmdDBUpdateTables);

	addCmd(new CmdConfigLoad);

	addCmd(new CmdStatistics);

	addCmd(new CmdJSON);
}
