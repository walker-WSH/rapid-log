#include "rapid_log_instance.h"

#pragma comment(lib, "Shlwapi.lib")

rapid_log_instance *rapid_log_instance::instance()
{
	static rapid_log_instance ins;
	return &ins;
}

rapid_log_instance::rapid_log_instance()
{
	init_time_zone();
}

rapid_log_instance::~rapid_log_instance()
{
	uninit_log();
}

void rapid_log_instance::init_time_zone()
{
	TIME_ZONE_INFORMATION tmp;
	::GetTimeZoneInformation(&tmp);

	time_zone_ = std::to_string(tmp.Bias / (-60));
	assert(!time_zone_.empty() && "failed to get time zone");
}

bool rapid_log_instance::init_log(const wchar_t *directory, std::weak_ptr<rapid_log_callback> callback)
{
	assert(directory);

	SYSTEMTIME st;
	GetLocalTime(&st);

	wchar_t file[MAX_PATH];
	_snwprintf_s(file, MAX_PATH, L"%04d%02d%02d-%02d%02d%02d.log", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	std::wstring dir = std::wstring(directory) + std::wstring(file);

	std::lock_guard<std::recursive_mutex> auto_lock(log_lock_);
	callback_ = callback;
	writer_inited_ = log_writer_.Open(dir.c_str(), true);
	return writer_inited_;
}

void rapid_log_instance::uninit_log()
{
	std::lock_guard<std::recursive_mutex> auto_lock(log_lock_);
	if (writer_inited_) {
		writer_inited_ = false;
		log_writer_.Close();
	}

	// Note: Here we should not change callback_
}

void rapid_log_instance::rapid_log(rapid_log_level level, const char *file, const char *func, int line, const char *msg)
{
#ifndef DEBUG
	if (level == rapid_log_level::RAPID_LOG_DEBUG)
		return;
#endif

	SYSTEMTIME st;
	GetLocalTime(&st);

	boost::format format_time("%1%/%2%/%3%-%4%:%5%:%6%+%7%(+%8%H)");
	format_time % st.wYear % st.wMonth % st.wDay;
	format_time % st.wHour % st.wMinute % st.wSecond;
	format_time % st.wMilliseconds % time_zone_;

	boost::format format_msg("%1% [%2%:%3%] %4%(%5%) %6% %7%\n");
	format_msg % format_time.str() % PathFindFileNameA(file) % line % func % GetCurrentThreadId() % level_list_[level] % msg;

	std::string temp(format_msg.str());
	OutputDebugStringA(temp.c_str());

	// callback_ won't be changed after init_log(), so here we do not use lock for it.
	auto cb = callback_.lock();
	if (cb) {
		rapid_log_info info;
		info.local_time = st;
		info.time_zone = time_zone_.c_str();
		info.level = level;
		info.file = file;
		info.func = func;
		info.line = line;
		info.msg = temp.c_str();

		cb->on_rapid_log(info);
	}

	if (level != rapid_log_level::RAPID_LOG_DEBUG) {
		std::lock_guard<std::recursive_mutex> auto_lock(log_lock_);
		if (writer_inited_)
			log_writer_.WriteData(temp.c_str(), (unsigned)temp.length());
	}
}
