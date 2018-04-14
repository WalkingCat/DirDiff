#include "stdafx.h"
#include "find_files_wcs.h"
#include "find_files.h"

using namespace std;

map<wstring, vector<wstring>> find_files_wcs(const wstring & directory)
{
	map<wstring, vector<wstring>> ret;

	// ex. amd64_wvms_vsft.inf.resources_31bf3856ad364e35_10.0.14393.0_en-us_f32e284e2439eb0bs
	wregex re(L"(.*)_[0-9a-f]+_[0-9\\.]+_[0-9a-z-]+_[0-9a-f]+");
	wsmatch match;

	wchar_t path[MAX_PATH] = {};
	wcscpy_s(path, directory.c_str());
	PathAppend(path, L"*");
	WIN32_FIND_DATAW fd;
	HANDLE find = ::FindFirstFileW(path, &fd);
	if (find != INVALID_HANDLE_VALUE) {
		do {
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
				wstring file(fd.cFileName);
				if (regex_match(file, match, re)) {
					PathRemoveFileSpec(path);
					PathCombine(path, path, fd.cFileName);
					PathAppend(path, L"*");
					auto& files = ret[match[1].str()];
					const auto& found_files = find_files(path);
					files.insert(files.end(), found_files.begin(), found_files.end());
					PathRemoveFileSpec(path);
				}
			}
		} while (::FindNextFileW(find, &fd));
		::FindClose(find);
	}
	return ret;
}
