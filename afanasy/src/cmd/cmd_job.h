#pragma once

#include "cmd.h"

class CmdJobsList : public Cmd
{
  public:
	CmdJobsList();
	~CmdJobsList();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
	void v_msgOut(af::Msg &msg) final;
};
class CmdJobsWeight : public Cmd
{
  public:
	CmdJobsWeight();
	~CmdJobsWeight();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
	void v_msgOut(af::Msg &msg) final;
};
class CmdJobPriority : public Cmd
{
  public:
	CmdJobPriority();
	~CmdJobPriority();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobRunningTasksMaximum : public Cmd
{
  public:
	CmdJobRunningTasksMaximum();
	~CmdJobRunningTasksMaximum();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobHostsMask : public Cmd
{
  public:
	CmdJobHostsMask();
	~CmdJobHostsMask();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobsSetUser : public Cmd
{
  public:
	CmdJobsSetUser();
	~CmdJobsSetUser();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobId : public Cmd
{
  public:
	CmdJobId();
	~CmdJobId();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobLog : public Cmd
{
  public:
	CmdJobLog();
	~CmdJobLog();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobProgress : public Cmd
{
  public:
	CmdJobProgress();
	~CmdJobProgress();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
	void v_msgOut(af::Msg &msg) final;
};
class CmdJobsDelete : public Cmd
{
  public:
	CmdJobsDelete();
	~CmdJobsDelete();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobsPause : public Cmd
{
  public:
	CmdJobsPause();
	~CmdJobsPause();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobsStart : public Cmd
{
  public:
	CmdJobsStart();
	~CmdJobsStart();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobsStop : public Cmd
{
  public:
	CmdJobsStop();
	~CmdJobsStop();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
class CmdJobsRestart : public Cmd
{
  public:
	CmdJobsRestart();
	~CmdJobsRestart();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
