#include <rapid_log/rapid_log.h>
#include <Windows.h>

static UINT64 GetTimeMillisecond()
{
	LARGE_INTEGER liPerfFreq = {};
	QueryPerformanceFrequency(&liPerfFreq);

	LARGE_INTEGER currentTime = {};
	QueryPerformanceCounter(&currentTime);

	double timeValue = (double)currentTime.QuadPart / (double)liPerfFreq.QuadPart;
	timeValue *= 1000.f; // to ms
	return (UINT64)timeValue;
}

static std::wstring GetProcessPath()
{
	WCHAR szFilePath[MAX_PATH] = {};
	GetModuleFileNameW(NULL, szFilePath, MAX_PATH);

	int nLen = (int)wcslen(szFilePath);
	for (int i = nLen - 1; i >= 0; --i) {
		if (szFilePath[i] == '\\') {
			szFilePath[i + 1] = 0;
			break;
		}
	}

	return std::wstring(szFilePath);
}

int main()
{
	rapidlog::rapid_log_error_info error;
	bool res = rapidlog::init_rapid_log(GetProcessPath().c_str(), &error, nullptr);
	if (!res) {
		printf("failed to open file. code:%u desc:%s \n\n", error.error_code, error.error_desc.c_str());
		return 0;
	}

	UINT64 loopCount = 10000;
	UINT64 start = GetTimeMillisecond();
	for (UINT64 i = 0; i < loopCount; i++) {
		//---------------------------------- demo for normal usage ------------------------------
		// 会在函数开始和结束时 自动打印两条log，用来跟踪函数是否执行完成
		LOG_TRACE();
		// 使用C/C++风格的格式化参数 自己执行参数的类型
		LOG_INFO("This is for %s, num: %d", "wsh", 789);
		// 当使用C/C++风格的格式化参数时，即使实参传错了，也可以正常输出log
		LOG_UISTEP("User click button, hahahha wsh %s, num: %d", "demo", "test");
		// 使用%i% 格式化参数，注意索引i 是从1开始的 （推荐用这种方式进行参数的格式化）
		LOG_INFO("This is for %1%, num: %2%", "test", 123);

		//---------------------------------- demo for incorrect usage ------------------------------
		// 错误！ 参数多传了 会自动捕获到这个错误（不需要自己在调用的地方使用try-catch）
		LOG_INFO("This is for error %1%", "test", 1, 2, 3);
		// 错误！ 参数少传了
		LOG_INFO("This is for error %1%, num: %2%", "test");
		// 错误！ 参数格式化 要么都用%1% 或 %d, 不能两种方式混用
		LOG_INFO("This is for error %1%, num: %d", "test", 123);
	}

	UINT64 totalTime = GetTimeMillisecond() - start;
	UINT64 totalLogs = loopCount * 8;

	printf("take %llu ms for writing %llu logs， every log take %f ms \n\n", totalTime, totalLogs, float(totalTime) / float(totalLogs));
	printf("test end! \n\n");
	getchar();

	rapidlog::uninit_rapid_log();

	return 0;
}
