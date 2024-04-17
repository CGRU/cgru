#include "../cinemotion/cnLoger.h"
#include "../cinemotion/cnStdLoger.h"

using namespace cn;

std::shared_ptr<Loger> Loger::out = StdLoger::get_instance();

void Loger::dive_in(int deep) {
	m_dive_cnt += deep;
	m_dive_str = std::string(m_dive_cnt, '.');
}

void Loger::dive_out(int raise) {
	m_dive_cnt -= raise;
	if (m_dive_cnt <= 0) {
		m_dive_cnt = 0;
		m_dive_str = "";
	}
	m_dive_str = std::string(m_dive_cnt, '.');
}

void Loger::rise() {
	m_dive_cnt = 0;
	m_dive_str = "";
}

void Loger::set_active_loger(std::shared_ptr<Loger> log)
{
	out = log;
	if (out == log) out = StdLoger::get_instance();
}

std::shared_ptr<Loger> Loger::get_active_loger()
{
	return out;
}

