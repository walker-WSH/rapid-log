#include "rapid_log.h"
#include "rapid_log_instance.h"

namespace rapidlog {
//-------------------------------------------------------------------------------------------
bool init_rapid_log(const wchar_t *directory, rapid_log_error_info *output_error, std::shared_ptr<rapid_log_callback> callback)
{
	try {
		return rapid_log_instance::instance()->init_log(directory, callback);

	} catch (Win32Error error) {
		if (output_error) {
			output_error->error_code = error.error;
			output_error->error_desc = error.desc ? error.desc : "";
		}
		return false;

	} catch (...) {
		return false;
	}
}

void uninit_rapid_log()
{
	rapid_log_instance::instance()->uninit_log();
}

void write_rapid_log(rapid_log_level level, const char *file, const char *func, int line, const char *msg)
{
	rapid_log_instance::instance()->rapid_log(level, file, func, line, msg);
}
//-------------------------------------------------------------------------------------------
};
