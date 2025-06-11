/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	af::User class represents jobs users.
 */
#include "user.h"

#include <memory.h>
#include <stdio.h>

#include "address.h"
#include "environment.h"
#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

User::User(const std::string &username, const std::string &host) : m_host_name(host)
{
	initDefaultValues();
	m_name = username;
}

User::User(int i_id)
{
	initDefaultValues();
	m_id = i_id;
}

User::User(Msg *msg) { read(msg); }

void User::initDefaultValues()
{
	setSolveOrder();
	setSolveCapacity();

	m_errors_retries = af::Environment::getTaskErrorRetries();
	m_errors_avoid_host = af::Environment::getErrorsAvoidHost();
	m_errors_task_same_host = af::Environment::getTaskErrorsSameHost();
	m_errors_forgive_time = af::Environment::getErrorsForgiveTime();

	m_max_running_tasks_per_host = af::Environment::getMaxRunningTasksPerHost();

	m_jobs_life_time = af::Environment::getJobsLifeTime();

	m_jobs_num = 0;
	m_running_jobs_num = 0;

	m_time_register = 0;
	m_time_activity = 0;
}

User::~User() { AFINFO("User::~User:") }

void User::v_jsonWrite(std::ostringstream &o_str, int i_type) const
{
	o_str << "{";

	Node::v_jsonWrite(o_str, i_type);

	Work::jsonWrite(o_str, i_type);

	if (isPaused())
		o_str << ",\n\"paused\":true";

	o_str << ",\n\"time_register\":" << m_time_register;
	o_str << ",\n\"time_activity\":" << m_time_activity;
	o_str << ",\n\"errors_retries\":" << int(m_errors_retries);
	o_str << ",\n\"errors_avoid_host\":" << int(m_errors_avoid_host);
	o_str << ",\n\"errors_task_same_host\":" << int(m_errors_task_same_host);
	o_str << ",\n\"errors_forgive_time\":" << m_errors_forgive_time;

	if (m_jobs_life_time > 0)
		o_str << ",\n\"jobs_life_time\":" << m_jobs_life_time;

	if (m_host_name.size())
		o_str << ",\n\"host_name\":\"" << m_host_name << "\"";

	if (m_jobs_num > 0)
		o_str << ",\n\"jobs_num\":" << m_jobs_num;
	if (m_running_jobs_num > 0)
		o_str << ",\n\"running_jobs_num\":" << m_running_jobs_num;

	o_str << "\n}";
}

bool User::jsonRead(const JSON &i_object, std::string *io_changes)
{
	if (false == i_object.IsObject())
	{
		AFERROR("User::jsonRead: Not a JSON object.")
		return false;
	}

	bool _paused;
	if (jr_bool("paused", _paused, i_object, io_changes))
		setPaused(_paused);

	jr_uint8("errors_retries", m_errors_retries, i_object, io_changes);
	jr_uint8("errors_avoid_host", m_errors_avoid_host, i_object, io_changes);
	jr_uint8("errors_task_same_host", m_errors_task_same_host, i_object, io_changes);
	jr_int32("errors_forgive_time", m_errors_forgive_time, i_object, io_changes);
	jr_int32("jobs_life_time", m_jobs_life_time, i_object, io_changes);

	Work::jsonRead(i_object, io_changes);

	// Paramers below are not editable and read only on creation
	// When use edit parameters, log provided to store changes
	if (io_changes)
		return true;

	jr_int64("time_activity", m_time_activity, i_object);
	jr_int64("time_register", m_time_register, i_object);

	Node::jsonRead(i_object);

	return true;
}

void User::v_readwrite(Msg *msg)
{
	Node::v_readwrite(msg);
	Work::readwrite(msg);

	rw_int64_t(m_time_activity, msg);
	rw_String(m_host_name, msg);
	rw_uint8_t(m_errors_retries, msg);
	rw_uint8_t(m_errors_avoid_host, msg);
	rw_uint8_t(m_errors_task_same_host, msg);
	rw_int32_t(m_errors_forgive_time, msg);
	rw_int64_t(m_time_register, msg);
	rw_int32_t(m_jobs_life_time, msg);

	rw_int32_t(m_jobs_num, msg);
	rw_int32_t(m_running_jobs_num, msg);
}

int User::v_calcWeight() const
{
	int weight = Work::calcWeight();
	weight += sizeof(User) - sizeof(Work);
	weight += weigh(m_host_name);
	weight += weigh(m_annotation);
	return weight;
}

void User::v_generateInfoStream(std::ostringstream &stream, bool full) const
{
	if (full)
	{
		stream << "User name = \"" << m_name << "\" (id=" << m_id << "):";
		stream << "\n Priority = " << int(m_priority);
		stream << "\n    Each point gives 10% bonus";
		stream << "\n Jobs = " << m_jobs_num << " / active jobs = " << m_running_jobs_num;
		if (m_jobs_life_time > 0)
			stream << "\n Jobs life time = " << af::time2strHMS(m_jobs_life_time, true);
		stream << "\n Maximum Running Tasks = " << m_max_running_tasks;
		if (m_max_running_tasks < 1)
			stream << " (no limit)";
		stream << "\n Running Tasks Number = " << m_running_tasks_num;
		stream << "\n Running Tasks Capacity Total = " << m_running_capacity_total;

		Work::generateInfoStream(stream, full);

		stream << "\n Errors Solving: ";
		stream << "\n    Errors To Avoid Host = " << int(m_errors_avoid_host);
		stream << "\n    Maximum Errors Same Task = " << int(m_errors_task_same_host);
		stream << "\n    Task Errors To Retry = " << int(m_errors_retries);
		stream << "\n    Errors Forgive Time = " << af::time2strHMS(m_errors_forgive_time, true);
		if (m_errors_forgive_time == 0)
			stream << " (infinite, no forgiving)";

		if (m_host_name.size() != 0)
			stream << "\n Last host: \"" << m_host_name << "\"";
		stream << "\n Registration time: " << time2str(m_time_register);
		stream << "\n Last activity time: " << time2str(m_time_activity);
		if (m_annotation.size())
			stream << "\n" << m_annotation;
		if (m_custom_data.size())
			stream << "\nCustom Data:\n" << m_custom_data;
		// stream << "\n Memory = " << calcWeight() << " bytes.";
	}
	else
	{
		stream << "#" << m_id << ":" << int(m_priority) << " " << m_name << " j" << m_jobs_num << "/"
			   << m_running_jobs_num << " r" << m_running_tasks_num << "/" << m_max_running_tasks << " c"
			   << m_running_capacity_total << " " << m_host_name << " - " << v_calcWeight() << " bytes.";
	}
}
