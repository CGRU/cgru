#ifndef CN_LOGER_H
#define CN_LOGER_H

#include <sstream>
#include <memory>

#define CN_OK(...) \
	cn::Loger::out->ok(__VA_ARGS__);
	
#define CN_INFO(...) \
	cn::Loger::out->info(__VA_ARGS__);

#define CN_INFO_COL(col,...) \
	cn::Loger::out->info_col(col,__VA_ARGS__);

#define CN_WARN(...) \
	cn::Loger::out->warn(__VA_ARGS__);

#define CN_ERR(...) \
	cn::Loger::out->err(__VA_ARGS__," (",__FILE__,") line:(",__LINE__,")");

#define CN_DIVE() \
 	cn::Loger::out->dive_in();

#define CN_UP() \
	cn::Loger::out->dive_out();

#define CN_RISE() \
	cn::Loger::out->rise();

namespace cn
{
	class Loger
	{
	public:
		Loger() {}
		virtual ~Loger() {}

		template <typename... Args>
		void ok(Args... args)
		{
			std::stringstream l_ss;
			to_string(l_ss, args...);
			_ok(l_ss.str().c_str());
		}

		template <typename... Args>
		void warn(Args... args)
		{
			std::stringstream l_ss;
			to_string(l_ss, args...);
			_warn(l_ss.str().c_str());
		}

		template <typename... Args>
		void err(Args... args)
		{
			std::stringstream l_ss;
			to_string(l_ss, args...);
			_err(l_ss.str().c_str());
		}

		template <typename... Args>
		void info(Args... args)
		{
			std::stringstream l_ss;
			to_string(l_ss, args...);
			_info(l_ss.str().c_str());
		}

		template <typename... Args>
		void info_col(const char* color ,Args... args)
		{
			std::stringstream l_ss;
			to_string(l_ss, args...);
			_info_col(color, l_ss.str().c_str());
		}

	public:
		virtual void dive_in(int deep =4);
		virtual void dive_out(int raise =4);
		virtual void rise();
		virtual void new_line() const = 0;
		virtual void tag(const char*) = 0;
		virtual void flush() = 0;

	private:
		virtual void reset() = 0;
		virtual void _ok(	const char* msg) const = 0;
		virtual void _warn(	const char* msg) const = 0;
		virtual void _err(	const char* msg) const = 0;
		virtual void _info(	const char* msg) const = 0;
		virtual void _info_col(	const char* color, const char* msg) const = 0;
		virtual void _stream(const char* msg) const = 0;

	public:
		template<typename T>
		Loger& operator <<( const T& msg){
			std::stringstream l_ss;
			l_ss << msg;
			_stream(l_ss.str().c_str());
			return *this;
		}

		Loger& operator <<(const char* msg) {
			_stream(msg);
			return *this;
		}
		//
		template<typename T>
		static void to_string(std::stringstream& ss, const T& t){
			ss << t;
		}

		template<typename T, typename... Args>
		static void to_string(std::stringstream& ss, const T& t, Args... args)	{
			ss << t;
			to_string(ss, args...);
		}

	protected:
		int m_dive_cnt = 0;
		std::string m_dive_str;
		friend class ComposeLoger;

	public:
		static void set_active_loger(std::shared_ptr<Loger>);
		static std::shared_ptr<Loger> get_active_loger();

	public:
		static std::shared_ptr<Loger> out;

	};
}

#endif