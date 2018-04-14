#include "stdafx.h"
#include "find_files.h"

using namespace std;

vector<wstring> find_files(const wchar_t * pattern)
{
	vector<wstring> ret;
	wchar_t path[MAX_PATH] = {};
	wcscpy_s(path, pattern);
	WIN32_FIND_DATA fd;
	HANDLE find = ::FindFirstFile(pattern, &fd);
	if (find != INVALID_HANDLE_VALUE) {
		do {
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				ret.emplace_back(fd.cFileName);
			}
		} while (::FindNextFile(find, &fd));
		::FindClose(find);
	}
	return ret;
}
