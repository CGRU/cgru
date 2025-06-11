#pragma once

#include "af.h"

namespace af
{
class HostResCustom : public Af
{
  public:
	HostResCustom();
	HostResCustom(Msg *msg);

	int32_t value;
	int32_t valuemax;
	uint8_t width;
	uint8_t height;
	uint8_t graphr;
	uint8_t graphg;
	uint8_t graphb;
	uint8_t labelsize;
	uint8_t labelr;
	uint8_t labelg;
	uint8_t labelb;
	uint8_t bgcolorr;
	uint8_t bgcolorg;
	uint8_t bgcolorb;
	std::string label;
	std::string tooltip;

	void v_generateInfoStream(std::ostringstream &stream, bool full = false) const; /// Generate information.

	void v_readwrite(Msg *msg); ///< Read or write Host Resources in message.

	void jsonWrite(std::ostringstream &o_str) const;
};

class HostRes : public Af
{
  public:
	HostRes();
	~HostRes();

	HostRes(Msg *msg);

  public:
	uint8_t cpu_loadavg[3];

	int32_t cpu_num;
	int32_t cpu_mhz;
	int32_t cpu_temp;

	uint8_t cpu_user;
	uint8_t cpu_nice;
	uint8_t cpu_system;
	uint8_t cpu_idle;
	uint8_t cpu_iowait;
	uint8_t cpu_irq;
	uint8_t cpu_softirq;

	int32_t mem_total_mb;
	int32_t mem_free_mb;
	int32_t mem_cached_mb;
	int32_t mem_buffers_mb;

	int32_t swap_total_mb;
	int32_t swap_used_mb;

	int32_t hdd_total_gb;
	int32_t hdd_free_gb;
	int32_t hdd_rd_kbsec;
	int32_t hdd_wr_kbsec;
	int8_t hdd_busy;

	int32_t net_recv_kbsec;
	int32_t net_send_kbsec;

	int8_t gpu_gpu_util;
	int8_t gpu_gpu_temp;
	int32_t gpu_mem_total_mb;
	int32_t gpu_mem_used_mb;
	std::string gpu_string;

	/// List of users currently logged in on a machine
	std::vector<std::string> logged_in_users;

	std::string hw_info;

	std::vector<HostResCustom *> custom;

  public:
	/// Generate information.
	void v_generateInfoStream(std::ostringstream &stream, bool full = false) const;

	inline bool isEmpty() const { return cpu_num == 0; }
	inline bool notEmpty() const { return cpu_num > 0; }

	void copy(const HostRes &other);

	void jsonWrite(std::ostringstream &o_str) const;

	void v_readwrite(Msg *msg); ///< Read or write Host Resources in message.
};
} // namespace af
