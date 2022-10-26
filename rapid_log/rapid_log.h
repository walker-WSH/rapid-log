#pragma once
#include <assert.h>
#include <iostream>
#include <memory>
#include <string>
#include <Windows.h>
#include <boost/format.hpp>

/*
* Two ways to format params.
* (1) Like C/C++, %s, %d, %f
* (2) %1%, %2%, %3%
* eg:
	LOG_INFO("This is for %s, num: %d", "rapid", 789);
	LOG_INFO("This is for %1%, num: %2%", "log", 123);
*/

#define RAPID_LOG_EXPORT __declspec(dllexport)

#define LOG_DEBUG(fmt, ...) rapidlog::format_log(rapidlog::rapid_log_level::RAPID_LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) rapidlog::format_log(rapidlog::rapid_log_level::RAPID_LOG_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) rapidlog::format_log(rapidlog::rapid_log_level::RAPID_LOG_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_UISTEP(fmt, ...) rapidlog::format_log(rapidlog::rapid_log_level::RAPID_LOG_UISTEP, __FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_TRACE() rapidlog::rapid_trace_log auto_track(__FILE__, __FUNCTION__, __LINE__)

namespace rapidlog {
//-------------------------------------------------------------------------------------------
enum class rapid_log_level {
	RAPID_LOG_DEBUG = 0,
	RAPID_LOG_INFO,
	RAPID_LOG_WARN,
	RAPID_LOG_UISTEP,
	RAPID_LOG_TRACE,
};

struct rapid_log_error_info {
	DWORD error_code = 0;
	std::string error_desc = "";
};

struct rapid_log_info {
	SYSTEMTIME local_time;
	const char *time_zone;

	rapid_log_level level;
	const char *file;
	const char *func;
	int line;
	const char *msg;
};

class rapid_log_callback {
public:
	virtual ~rapid_log_callback() = default;

	// Note: This callback may be called from any thread where you are calling write_rapid_log().
	virtual void on_rapid_log(const rapid_log_info &info) = 0;
};

// directory: end with '\'
// output_error: if function returns false, you can get error reason by it.
// callback: it will be saved as weak_ptr in rapid_log
RAPID_LOG_EXPORT bool init_rapid_log(const wchar_t *directory, rapid_log_error_info *output_error, std::shared_ptr<rapid_log_callback> callback);

// Write your log into file synchronously. We are using mapped memory to write logs, so the perfermence is good.
// Event your process crashes, the log won't be missing.
RAPID_LOG_EXPORT void write_rapid_log(rapid_log_level level, const char *file, const char *func, int line, const char *msg);

// You should not call write_rapid_log after this function.
// To make sure all logs in your code can be saved into file,
// you can also ignore this function since we will call it as late as possible.
RAPID_LOG_EXPORT void uninit_rapid_log();

//---------------------------------------------------------------------------------------------------------------------------------------------------------
template<typename... Arguments> void format_log(rapid_log_level level, const char *file, const char *func, int line, const char *fmt, Arguments &&...args)
{
	try {
		boost::format f(fmt);

		int unroll[]{0, (f % std::forward<Arguments>(args), 0)...};
		static_cast<void>(unroll);

		std::string msg = boost::str(f);
		write_rapid_log(level, file, func, line, msg.c_str());

	} catch (std::exception &e) {
		write_rapid_log(rapid_log_level::RAPID_LOG_WARN, file, func, line, e.what());
		assert(false && "log format error");
	} catch (...) {
		write_rapid_log(rapid_log_level::RAPID_LOG_WARN, file, func, line, "format error");
		assert(false && "log format error");
	}
}

class rapid_trace_log {
public:
	rapid_trace_log(const char *file, const char *func, int line) : file_(file), func_(func), line_(line)
	{
		char log[MAX_PATH];
		snprintf(log, MAX_PATH, "%s function enter", func_);
		write_rapid_log(rapid_log_level::RAPID_LOG_TRACE, file_, func_, line_, log);
	}

	virtual ~rapid_trace_log()
	{
		char log[MAX_PATH];
		snprintf(log, MAX_PATH, "%s function leave", func_);
		write_rapid_log(rapid_log_level::RAPID_LOG_TRACE, file_, func_, line_, log);
	}

private:
	const char *file_ = nullptr;
	const char *func_ = nullptr;
	int line_ = 0;
};

//-------------------------------------------------------------------------------------------
};
