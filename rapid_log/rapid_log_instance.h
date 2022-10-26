#pragma once
#include <iostream>
#include <Windows.h>
#include <Shlwapi.h>
#include <assert.h>
#include <string>
#include <mutex>
#include <map>
#include "memory_map_writer.h"
#include "rapid_log.h"

using namespace rapidlog;

class rapid_log_instance {
public:
	static rapid_log_instance *instance();

	virtual ~rapid_log_instance();

	bool init_log(const wchar_t *directory, std::weak_ptr<rapid_log_callback> callback);
	void uninit_log();

	void rapid_log(rapid_log_level level, const char *file, const char *func, int line, const char *msg);

protected:
	rapid_log_instance();

	void init_time_zone();

private:
	std::map<rapid_log_level, std::string> level_list_ = {
		{rapid_log_level::RAPID_LOG_DEBUG, "DEBUG"},   {rapid_log_level::RAPID_LOG_INFO, "INFO"},   {rapid_log_level::RAPID_LOG_WARN, "WARN"},
		{rapid_log_level::RAPID_LOG_UISTEP, "UISTEP"}, {rapid_log_level::RAPID_LOG_TRACE, "TRACE"},
	};

	std::recursive_mutex log_lock_;
	CMemoryMapWriter log_writer_;
	bool writer_inited_ = false;
	std::string time_zone_ = "";
	std::weak_ptr<rapid_log_callback> callback_;
};
