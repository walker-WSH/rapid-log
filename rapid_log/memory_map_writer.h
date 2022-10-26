#pragma once
#include <tchar.h>
#include <Windows.h>

struct Win32Error {
	const char *desc;
	DWORD error;

	inline Win32Error(const char *ds, DWORD err) : desc(ds), error(err) {}
};

//=======================================
class CMemoryMapWriter {
public:
	CMemoryMapWriter();
	~CMemoryMapWriter();

public:
	// Warning: This function may throw exception, please call it in try-catch.
	bool Open(const TCHAR *path, bool cover_write);

	void Close();

	unsigned int WriteData(const void *ptr, const unsigned int &len);

private:
	class impl;
	impl *self;
};
